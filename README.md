# Smart Door Lock System (ESP32 + MQTT)

## Description
This project implements a smart door lock system, based on an ESP32 microcontroller and the MQTT protocol. The system allows remote control of a locking mechanism, monitoring the door status and publishing events in an IoT architecture.

---

## System architecture
The system is organized in a publish/subscribe architecture, using an MQTT broker:

- **Embedded MQTT Client (ESP32)**
- controls the locking mechanism
- reads the door status
- publishes status and events
- receives control commands

- **MQTT Broker**
- mediates communication between clients

- **Receiving clients (application, dashboard, PC)**
- send commands
- receive notifications

---

## MQTT Topics used

| Topic | Direction | Description |
|----|--------|-----|
| `home/door1/cmd` | Subscribe | Lock/unlock commands |
| `home/door1/status` | Publish | Door status (open / closed) |
| `home/door1/event` | Publish | System events |

---

## Implemented functionalities
- WiFi network connection
- MQTT broker connection
- Locking mechanism control (lock / unlock)
- Door status monitoring via sensor
- MQTT event publishing
- Extensible architecture for future functionalities

---

## Hardware used
- ESP32 Dev Module
- Actuator (relay / electromagnet / servo)
- Door position sensor (reed switch)

---

## Project structure

SmartDoorLock/
├── src/main.cpp # source code
├── platformio.ini # PlatformIO configuraion
├── README.md
├── include/
├── lib/
└── test/

---

## Future Extensions
- User Authentication (RFID / Keypad)
- JSON Messages for Events
- MQTT Authentication and Encryption
- Database Integration
- Mobile Application

---

## Documentation
The full project documentation (report and diagrams) is available in the `docs/` folder.

---

## Author
**Bănățean Alexandru-Ioan**

Politehnica University of Timișoara