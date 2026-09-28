# Bosporus – Embedded Systems Portfolio Project

An end-to-end project to refresh and demonstrate skills in embedded electronics,
embedded Linux, and system integration – set up and documented like a real product
development project.

> **About the name**: The Bosporus is the strait that connects two continents – just
> like this gateway connects the sensor-hardware world with the software/visualization world.

## Motivation

As a technical project lead in the embedded space – including hands-on development
experience of my own, such as a C++ application on an embedded Linux board with an
AVR32 processor – it matters to me not just to coordinate technical work, but to
understand it from practical experience: cross-compiling, bootloader/kernel, toolchains,
component supply chains, testing strategies.

This project brings that understanding up to the current state of the art – from modern
build systems to today's development workflows – and serves as practical evidence of
this competence for a technical project leadership role in the embedded space.

## Project goal

This project ties together three core embedded disciplines in one system:

- **Embedded electronics / microcontroller**: A sensor node reads environmental data
  (temperature, humidity) and transmits it wirelessly.
- **Embedded Linux**: A gateway based on a self-built Linux image processes the data.
- **System integration & visualization**: A dashboard makes the data visible.

Alongside the technical build, the focus is deliberately also on **project management
artifacts** (requirements, architecture, milestone plan, risk register) – as evidence
that technical understanding and structured project control go hand in hand.

## Architecture overview

```
Sensor node  --MQTT-->  Gateway  --Data-->  Dashboard
(ESP32,                 (Raspberry Pi,       (Grafana /
 FreeRTOS)               Embedded Linux)       Web UI)
```

Details in [docs/architecture.md](docs/architecture.md).

## Technologies used

| Layer            | Technology                                |
|-------------------|-------------------------------------------|
| Sensor node       | ESP32, FreeRTOS, PlatformIO, DHT22 sensor |
| Gateway OS        | Buildroot (optionally Yocto later)        |
| Communication     | MQTT (Mosquitto broker)                   |
| Data storage      | SQLite / InfluxDB                         |
| Visualization     | Grafana                                   |

## Project structure

```
bosporus/
├── README.md
├── docs/
│   ├── architecture.md      # Detailed system architecture
│   ├── requirements.md      # Functional & non-functional requirements
│   ├── project-plan.md      # Phases, milestones, timeline
│   ├── progress-log.md      # Dated build log with photos/screenshots
│   ├── risk-register.md     # Risks and mitigations
│   ├── glossary.md          # Abbreviations and terms used
│   └── images/              # Photos and screenshots from the build
├── sensor-node/              # Firmware for the ESP32 (to follow)
├── gateway/                   # Buildroot configuration, scripts (to follow)
└── dashboard/                 # Grafana configuration / web UI (to follow)
```

## How To

### Putting Bosporus into operation

1. **Start the gateway.** Connect the Raspberry Pi to the router with an
   Ethernet cable, then power it via its USB-C port. Wait about a minute until
   it has booted (red LED solid, green LED blinking slowly).
2. **Start the sensor node.** Connect the Arduino Nano ESP32 to the Mac via
   USB. (The ESP32-S3 chip sits inside the board's u-blox NORA-W106 module.)
   Place the board close to the router, ideally with a line of sight (LOS)
   to it, because its small antenna has a limited range.
3. **Check the sensor readings.** In VS Code, open the project
   `bosporus-sensor-node` and click the plug symbol ("PlatformIO: Serial
   Monitor") in the status bar at the bottom left. The serial output appears
   in the terminal panel. It should show a successful WiFi and MQTT
   connection, followed by the published temperature and humidity readings.
4. **Log into the Pi** from a Mac terminal:
```bash
   ssh root@192.168.1.169
```
   - If the image has been reflashed since your last login, SSH shows the
     warning "REMOTE HOST IDENTIFICATION HAS CHANGED". This is expected,
     because a fresh image generates new SSH host keys. Remove the old entry
     and log in again:
```bash
     ssh-keygen -R 192.168.1.169
     ssh root@192.168.1.169
```
   - If the Pi's IP address has changed, find the new one in the router's
     list of connected devices and use it instead. The ESP32 firmware must
     then be updated too, because the broker address is hard-coded in
     `src/main.cpp` (`MQTT_BROKER`).
5. **Start the dashboard:**
```bash
   python3 /opt/bosporus/dashboard.py
```
   Keep this terminal open: closing the SSH session also stops the dashboard.
6. **Open the dashboard** in a browser using the address shown in the
   output, e.g. `Running on http://192.168.1.169:5000`. The temperature and
   humidity are plotted there. Use Safari: Firefox's HTTPS-Only Mode
   redirects to `https://`, which the Flask development server does not
   support.
7. **Shut down.** Before unplugging the Pi, shut it down cleanly to protect
   the SD card and the database:
```bash
   poweroff
```
