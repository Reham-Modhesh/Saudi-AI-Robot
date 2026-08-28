# 🇸🇦 Saudi AI Robot

An interactive AI-powered robot designed for Saudi National Day celebrations.

## 🤖 About the Project

Saudi AI Robot is an interactive mobile robot that combines robotics,
voice interaction, and artificial intelligence to create an engaging
experience for visitors during Saudi National Day celebrations.

The robot is designed to detect people in its surroundings. When a
person approaches, the robot stops and initiates a voice interaction,
allowing the visitor to communicate with the robot naturally.

The robot uses an ESP32-S3 for voice processing and AI interaction,
while an Arduino-based controller manages the robot's movement and
sensors.

## ✨ Features

- 🚗 Mobile robotic platform
- 👤 Person detection
- 🎙️ Voice interaction
- 🧠 Gemini AI integration
- 🔊 Voice responses
- 🖥️ Interactive display
- 🇸🇦 Saudi National Day themed interactions
- 🤖 Autonomous robot movement

## 🏗️ System Architecture

The system consists of two main controllers:

### Arduino UNO R3

Responsible for:

- Motor control
- Robot movement
- Ultrasonic sensing
- Basic navigation and robotic control

### ESP32-S3

Responsible for:

- Microphone input
- Voice Activity Detection (VAD)
- Speech-to-Text (STT)
- Gemini AI communication
- Audio response

The two controllers communicate with each other to coordinate
robotic movement and AI interaction.

## 🛠️ Hardware

- Arduino UNO R3
- ESP32-S3
- Robot car chassis
- DC motors
- Motor driver
- Ultrasonic sensor
- INMP441 I2S microphone
- MAX98357A amplifier
- Speaker
- Display
- Servo motor
- Battery / power supply

## 🧠 AI Interaction

The robot uses Google's Gemini API to enable voice-based interaction.

The interaction process is:

1. The robot detects a nearby person.
2. The robot stops and initiates an interaction.
3. The microphone captures the person's voice.
4. Speech is processed and converted into text.
5. The text is sent to Gemini.
6. Gemini generates a response.
7. The response is played through the robot's speaker.
8. The robot continues the interaction.

## 🎯 Project Goal

The goal of this project is to demonstrate how artificial intelligence
and robotics can be combined to create an interactive and engaging
experience for Saudi National Day celebrations.

The project also aims to showcase practical applications of voice AI,
embedded systems, sensors, and mobile robotics.

## 📚 Acknowledgment

This project builds upon the open-source
ESP32 Gemini STT implementation by Reaganhoo,
licensed under the MIT License.

The original implementation was modified and integrated into the
Saudi AI Robot project.

## 📄 License

This project preserves the original MIT License from the underlying
open-source implementation.

See the `LICENSE` file for the complete license text.