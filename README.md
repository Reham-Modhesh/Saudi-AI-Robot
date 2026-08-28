This project provides a robust implementation of Voice Activity Detection (VAD) and Speech-to-Text using an **ESP32** and the **Gemini 2.5 Flash API**. 

Unlike many simple examples, this code handles I2S microphone sampling, local audio playback for verification, and optimized JSON transmission to the cloud to avoid RAM overflows on the ESP32.

## 🚀 Features
* **Voice Activity Detection (VAD):** Automatically starts recording when you speak and stops after a period of silence.
* **High-Fidelity Sampling:** Uses 16kHz sample rate, ideal for Gemini's audio processing.
* **PSRAM Optimized:** Uses external RAM to store up to 5 seconds of audio.
* **Local Feedback:** Plays your audio back through a speaker before sending it to the cloud (optional debugging step).

## 🛠️ Hardware Requirements
* **Microcontroller:** ESP32-S3 (or any ESP32 with PSRAM).
* **Microphone:** INMP441 (I2S Digital Mic).
* **Speaker:** MAX98357A I2S Amplifier + 4/8 Ohm Speaker.



## 📋 Pin Mapping
<img width="2316" height="1255" alt="image" src="https://github.com/user-attachments/assets/d9228a7f-67a5-48dd-974b-cce985b8fd6f" />


| Component | Pin (ESP32) |
| :--- | :--- |
| **Mic SCK** | GPIO 12 |
| **Mic WS**  | GPIO 13 |
| **Mic SD**  | GPIO 11 |
| **Mic L/R** | GND     |
| **Mic GND** | GND     |
| **Mic VDD** | 3.3V    |
| **Speaker BCLK** | GPIO 6 |
| **Speaker LRC**  | GPIO 5 |
| **Speaker DIN**  | GPIO 7 |
| **Speaker GAIN** | 3.3V   |
| **Speaker SD**   | ---    |
| **Speaker GND**  | GND    |
| **Speaker VIN**  | 3.3V   |


## ⚙️ Setup Instructions

1.  **API Key:** Get your Google AI Studio API Key from [aistudio.google.com](https://aistudio.google.com/).
2.  **Libraries:** Install the following in your Arduino IDE:
    * `ArduinoJson`
    * `WiFi` & `HTTPClient`
3.  **Configuration:** Update the `ssid`, `password`, and `apiKey` variables in the code.
4.  **Partition Scheme:** Ensure you select a "Huge APP" or "Large SPIFFS" partition scheme with **PSRAM Enabled** in the Tools menu.

## 🧠 How it Works
1.  **Listening:** The ESP32 constantly monitors the I2S microphone.
2.  **Triggering:** When the volume exceeds `VOICE_THRESHOLD`, it begins filling the `audioBuffer`.
3.  **Packaging:** Once silence is detected, it wraps the raw PCM data into a **WAV header** and encodes it to **Base64**.
4.  **Inference:** The data is sent to Gemini with a system prompt to "Transcribe exactly what is said."
5.  <img width="658" height="275" alt="image" src="https://github.com/user-attachments/assets/10bd4ba8-c133-46ed-8baa-d4198a9dd2eb" />


##💰 Cost Note: 
This project uses the Gemini 2.5 Flash Free Tier, which allows for up to 250 requests per day at no cost. It's perfect for hobbyists and students building their first AI companion

## 🤝 Contributing
This is a module from my upcoming **AI Companion Robot** project. If you find bugs or ways to optimize the I2S buffer, feel free to open a Pull Request!

## 📄 License
This code is released under the MIT License.
