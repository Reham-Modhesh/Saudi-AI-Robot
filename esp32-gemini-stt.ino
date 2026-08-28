#include <driver/i2s.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <base64.h>
#include <ArduinoJson.h>

// --- CONFIGURATION ---
const char* ssid = "WIFINAME";
const char* password = "WIFIPASSWORD";
const char* apiKey = "YOURAPI";

// --- PIN MAPPING ---
#define MIC_WS 13
#define MIC_SD 11
#define MIC_SCK 12

#define SPK_DOUT 7
#define SPK_LRC  5
#define SPK_BCLK 6

#define I2S_PORT I2S_NUM_0 

// --- AUDIO SETTINGS ---
const int32_t VOICE_THRESHOLD = 900; 
const int SILENCE_TIMEOUT_MS = 1500;
const int SAMPLE_RATE = 16000;
const int RECORD_TIME_SECONDS = 5;
const int BUFFER_SIZE = SAMPLE_RATE * RECORD_TIME_SECONDS;

int16_t *audioBuffer; 
int bufferIndex = 0;
bool isRecording = false;
unsigned long lastVoiceTime = 0;
unsigned long bootTime = 0;
static int triggerCount = 0;

// --- WAV HEADER HELPER ---
void writeWavHeader(int16_t* samples, int sampleCount, uint8_t* header) {
    int32_t fileSize = 36 + (sampleCount * 2);
    int32_t dataSize = sampleCount * 2;
    memcpy(&header[0], "RIFF", 4);
    memcpy(&header[4], &fileSize, 4);
    memcpy(&header[8], "WAVEfmt ", 8);
    int32_t fmtSize = 16;
    memcpy(&header[16], &fmtSize, 4);
    int16_t format = 1; 
    memcpy(&header[20], &format, 2);
    int16_t channels = 1; 
    memcpy(&header[22], &channels, 2);
    int32_t rate = 16000;
    memcpy(&header[24], &rate, 4);
    int32_t byteRate = rate * 2;
    memcpy(&header[28], &byteRate, 4);
    int16_t blockAlign = 2;
    memcpy(&header[32], &blockAlign, 2);
    int16_t bits = 16;
    memcpy(&header[34], &bits, 2);
    memcpy(&header[36], "data", 4);
    memcpy(&header[40], &dataSize, 4);
}

void setupMic() {
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 64
    };
    i2s_pin_config_t pin_config = {.bck_io_num = MIC_SCK, .ws_io_num = MIC_WS, .data_out_num = -1, .data_in_num = MIC_SD};
    i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_PORT, &pin_config);
    i2s_start(I2S_PORT);
}

// 1. First, we play the audio back to you locally
void playBackAudio() {
    Serial.println("\n[Speaker] Playing back...");
    i2s_driver_uninstall(I2S_PORT);
    i2s_config_t i2s_speaker_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
        .dma_buf_count = 8,
        .dma_buf_len = 64
    };
    i2s_pin_config_t speaker_pin_config = {.bck_io_num = SPK_BCLK, .ws_io_num = SPK_LRC, .data_out_num = SPK_DOUT, .data_in_num = -1};
    i2s_driver_install(I2S_PORT, &i2s_speaker_config, 0, NULL);
    i2s_set_pin(I2S_PORT, &speaker_pin_config);
    size_t bytes_written;
    i2s_write(I2S_PORT, audioBuffer, bufferIndex * 2, &bytes_written, portMAX_DELAY);
    delay(200);
    i2s_driver_uninstall(I2S_PORT);
}

// 2. Then, we send the SAME data to Gemini
void sendToGemini() {
    if (WiFi.status() != WL_CONNECTED) { Serial.println("WiFi Down!"); return; }

    Serial.println("[Cloud] Uploading to Gemini...");
    
    // 1. Prepare the WAV data
    size_t wavSize = 44 + (bufferIndex * 2);
    uint8_t* fullWav = (uint8_t*)ps_malloc(wavSize);
    if (!fullWav) { Serial.println("PSRAM Error!"); return; }
    
    uint8_t header[44];
    writeWavHeader(audioBuffer, bufferIndex, header);
    memcpy(fullWav, header, 44);
    memcpy(fullWav + 44, audioBuffer, bufferIndex * 2);

    // 2. Encode to Base64 (This string is already large)
    String base64Audio = base64::encode(fullWav, wavSize);
    free(fullWav); 

    // 3. Construct the JSON in pieces to save RAM
    //String head = "{\"contents\":[{\"parts\":[{\"text\":\"You are a multilingual transcription assistant. The audio may be in English, Mandarin, or Malay. Please transcribe exactly what is said in its original script (e.g., use Simplified Chinese characters for Mandarin)..\"},{\"inline_data\":{\"mime_type\":\"audio/wav\",\"data\":\"";
    String head = "{\"contents\":[{\"parts\":[{\"text\":\"Transcribe this audio exactly.\"},{\"inline_data\":{\"mime_type\":\"audio/wav\",\"data\":\"";
    String tail = "\"}}]}]}";

    HTTPClient http;
    http.setTimeout(25000); 
    
    // Using v1 (stable)
    String url = "https://generativelanguage.googleapis.com/v1/models/gemini-2.5-flash:generateContent?key=" + String(apiKey);
    http.begin(url);
    http.addHeader("Content-Type", "application/json");

    // Send the data by combining the parts directly in the POST call
    // This avoids creating a 3rd giant string
    int httpResponseCode = http.POST(head + base64Audio + tail);
    
    if (httpResponseCode > 0) {
        String response = http.getString();
        if (httpResponseCode != 200) {
            Serial.printf("Server Error %d\n", httpResponseCode);
            Serial.println(response);
        } else {
            DynamicJsonDocument doc(8192);
            DeserializationError error = deserializeJson(doc, response);
            if (!error) {
                const char* transcription = doc["candidates"][0]["content"]["parts"][0]["text"];
                Serial.println("\n---------------------------");
                Serial.print("SUCCESS! GEMINI HEARD: ");
                Serial.println(transcription ? transcription : "[Silence]");
                Serial.println("---------------------------\n");
            }
        }
    } else {
        Serial.printf("Connection Failed: %s\n", http.errorToString(httpResponseCode).c_str());
    }
    http.end();
}


void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
    Serial.println("\nWiFi Connected!");

    setupMic();
    bootTime = millis();
    if (psramInit()) {
        audioBuffer = (int16_t *)ps_malloc(BUFFER_SIZE * sizeof(int16_t));
        Serial.println("System Ready.");
    }
}

void loop() {
    int32_t raw_sample = 0;
    size_t bytes_read;
    i2s_read(I2S_PORT, &raw_sample, sizeof(raw_sample), &bytes_read, portMAX_DELAY);

    if (millis() - bootTime < 3000) return;

    if (bytes_read > 0) {
        int32_t clean_sample = raw_sample >> 14; 
        int32_t volume = abs(clean_sample);

        if (volume > VOICE_THRESHOLD && !isRecording) {
            
            if (++triggerCount > 10) { 
                Serial.println("[Ear] Recording...");
                isRecording = true;
                bufferIndex = 0;
                triggerCount = 0;
            }
        }

        if (isRecording) {
            // Apply 8x gain for better STT and louder playback
            int32_t val = clean_sample * 8;
            if (val > 32767) val = 32767;
            if (val < -32768) val = -32768;

            audioBuffer[bufferIndex++] = (int16_t)val;
            if (volume > VOICE_THRESHOLD) lastVoiceTime = millis();

            if (bufferIndex >= BUFFER_SIZE || (millis() - lastVoiceTime > SILENCE_TIMEOUT_MS)) {
                isRecording = false;
                if (bufferIndex > 5000) { 
                    playBackAudio(); // 1. Local Playback
                    sendToGemini();  // 2. Cloud Transcription
                    setupMic();      // 3. Back to Listening
                    triggerCount = 0;           // Reset the "noise" counter
                    bootTime = millis() - 1500; // Reset bootTime but only wait 1.5 seconds
                }
                bufferIndex = 0;
            }
        }
    }
}
