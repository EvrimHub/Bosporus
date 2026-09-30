# Glossary – Bosphorus project

Abbreviations and terms used while working through the embedded electronics and
embedded Linux parts of this project. Grouped by topic, alphabetical within each group.

## A note on "ASIC"

Worth clarifying up front, since it came up: **ASIC = Application-Specific Integrated
Circuit** — a chip custom-designed for one narrow purpose (e.g. a Bitcoin-mining chip).
The ESP32-S3 and the NORA-W106 module are **not** ASICs — the ESP32-S3 is a **SoC**
(general-purpose, programmable), and NORA-W106 is a **module** (a SoC + supporting
components, pre-packaged). Common mix-up, worth keeping straight for interviews.

## Chip / hardware building blocks

| Abbreviation | Long name |
|---|---|
| ASIC | Application-Specific Integrated Circuit |
| IC | Integrated Circuit |
| LDO | Low-Dropout (voltage regulator) |
| PCB | Printed Circuit Board |
| PMIC | Power Management Integrated Circuit |
| SoC | System on Chip |

## Communication interfaces / buses

| Abbreviation | Long name |
|---|---|
| CAN | Controller Area Network |
| CDC | Communications Device Class (USB virtual serial port) |
| GPIO | General Purpose Input/Output |
| HID | Human Interface Device |
| I2C | Inter-Integrated Circuit |
| I2S | Inter-IC Sound |
| JTAG | Joint Test Action Group (hardware debug interface) |
| SDIO | Secure Digital Input Output |
| SPI | Serial Peripheral Interface |
| TWAI | Two-Wire Automotive Interface (Espressif's CAN implementation) |
| UART | Universal Asynchronous Receiver/Transmitter |
| USB | Universal Serial Bus |

## Board pins / signals

| Abbreviation | Long name |
|---|---|
| CS | Chip Select |
| GND | Ground |
| RST | Reset |
| RX / TX | Receive / Transmit |
| SCL / SDA | Serial Clock / Serial Data (I2C signal lines) |
| VBUS | Voltage Bus (USB 5V power line) |
| VCC | Voltage Common Collector (general supply voltage pin) |
| VIN | Voltage In |

## Signal processing

| Abbreviation | Long name |
|---|---|
| ADC | Analog-to-Digital Converter |
| EIRP | Equivalent Isotropically Radiated Power |
| PWM | Pulse Width Modulation |
| RTC | Real-Time Clock |
| ULP | Ultra Low Power (coprocessor) |

## Memory

| Abbreviation | Long name |
|---|---|
| PSRAM | Pseudo-Static Random Access Memory |
| RAM | Random Access Memory |
| ROM | Read-Only Memory |
| SRAM | Static Random Access Memory |

## Wireless / networking

| Abbreviation | Long name | Explanation |
|---|---|
| BLE | Bluetooth Low Energy |
| LAN | Local Area Network |
| MQTT | Message Queuing Telemetry Transport |
| SSH | Secure Shell | a protocol for a command-line session on a remote machine |
| SSH server & client| daemon | a background program running on the Pi, listening for incoming connection. Client ist the ssh command on Mac/PC initiating the conntection|
| SSID | Service Set Identifier (WiFi network name) |
| WLAN | Wireless Local Area Network |

## Storage / connectors

| Abbreviation | Long name |
|---|---|
| HDMI | High-Definition Multimedia Interface |
| NVMe | Non-Volatile Memory Express |
| SD / microSD | Secure Digital / micro Secure Digital (memory card) |
| USB-C | USB Type-C (connector standard) |

## Software / operating systems

| Abbreviation | Long name |
|---|---|
| BSP | Board Support Package |
| CLI | Command Line Interface |
| DT / DTB | Device Tree / Device Tree Blob |
| IDE | Integrated Development Environment |
| OS | Operating System |
| PID 1 | Process ID 1 (the first process started by the kernel, i.e. init) |
| RTOS | Real-Time Operating System |
| rootfs | Root Filesystem |
| Git / GitHub / GitLab | Git is the free, open-source version-control tool itself; GitHub (owned by Microsoft) and GitLab (an independent public company) are two competing commercial platforms that host Git repositories and add collaboration features on top. |
| Flask | Flask is a backend (server-side) framework, query the database, apply logic and return HTML or JSON |

## Certifications (seen printed on the board)

| Abbreviation | Long name |
|---|---|
| CE | Conformité Européenne |
| FCC | Federal Communications Commission |
| IC (cert.) | Industry Canada — **note**: same letters as I2C, but unrelated meaning in this context |
| RoHS | Restriction of Hazardous Substances |
| UKCA | UK Conformity Assessed |

## Project management terms used

| Abbreviation | Long name |
|---|---|
| DoD | Definition of Done |
| PM | Project Manager |
