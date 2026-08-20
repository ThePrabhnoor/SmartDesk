# SmartDesk Monorepo 🖥️🪴💡

Welcome to the **SmartDesk** project! This repository has evolved from a single smart plant monitor (PlantSense-AI) into a comprehensive, modular monorepo for automating and monitoring your entire desk setup. 

## Project Architecture

The repository is structured to easily support new smart devices (nodes) while maintaining a clean, centralized firmware hub that runs on a single ESP32. 

```
SmartDesk/
├── ESP32_Main/        # Unified firmware flashed to the ESP32 (combines all modules)
├── PlantSense/        # Subproject for the Plant Monitor (Sensors, AI analysis)
│   ├── ESP/           # Isolated ESP code strictly for PlantSense (legacy/standalone)
│   ├── MacWidget/     # Mac desktop widget for monitoring plant health
│   └── Google Home/   # Integrations for Google Assistant
├── SmartLamp/         # Subproject for the AC Smart Lamp (Relay control)
│   ├── ESP/           # Isolated ESP code strictly for SmartLamp
│   ├── MacWidget/     # Mac desktop widget for controlling the lamp
│   └── Google Home/   # Integrations for Google Assistant via Sinric Pro
├── Mac App/           # Unified macOS Application (formerly PlantSenseApp)
├── Firebase/          # Realtime Database security rules and configurations
├── Docs/              # Schematics, circuit diagrams, and project documentation
└── Dumps/             # Deprecated or unused components (e.g., legacy React dashboards)
```

## Hardware Setup

The primary controller is an **ESP32** which manages multiple connected peripherals simultaneously:

1. **PlantSense Module:**
   - **DHT11 Sensor:** Measures ambient temperature and humidity around the plant.
   - **Analog Soil Moisture Sensor:** Actively power-cycled to prevent corrosion, measuring the moisture level of the soil.
2. **SmartLamp Module:**
   - **5V Relay (Single Channel):** Switches a 230V AC mains line (COM to NO) to toggle the conventional desk lamp. 

## Software Integration

- **Firebase Realtime Database:** Logs all sensor data and syncs real-time state for the Mac widgets/apps. The database is secured so only authenticated clients (like the ESP32) can write to it.
- **Sinric Pro:** Bridges the ESP32 to Google Home, allowing the desk lamp to act as a seamless "Smart Switch" controllable via voice.
- **Gemini API:** Analyzes environmental data to provide plain-text botanical advice (e.g., "Water the plant, soil is dry") stored in Firebase.

## Getting Started

1. **Firmware Configuration:**
   - Navigate to `ESP32_Main/Config.h`.
   - Update your Wi-Fi, Firebase, Gemini API, and Sinric Pro credentials.
2. **Flashing the ESP32:**
   - Open `ESP32_Main/SmartDesk.ino` in the Arduino IDE.
   - Ensure the required libraries (Firebase ESP32, Sinric Pro, etc.) are installed.
   - Compile and flash to your board.
3. **Firebase Setup:**
   - Go to your Firebase project and enable Realtime Database.
   - Apply the security rules found in `Firebase/database.rules.json`.
   - Keep your generated `serviceAccountKey.json` inside the `Firebase/` folder (it is Git-ignored for security).

## Future Expansion

SmartDesk is designed for scalability. To add a new feature (e.g., a Motion Sensor or LED strip):
1. Create a new subfolder in the root directory (e.g., `SmartLEDs/`).
2. Write modular `.cpp` and `.h` logic.
3. Include and initialize it inside the `ESP32_Main/SmartDesk.ino` loop.

---
*Built to automate your workspace, one module at a time.*
