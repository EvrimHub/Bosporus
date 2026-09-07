# Project plan – Bosporus

## Phase overview

| Phase | Title                          | Content                                                                | Milestone                                     | Status | Start Date | Finish Date |
|-------|----------------------------------|--------------------------------------------------------------------------|-------------------------------------------------|--------|------|-------------|
| 0     | Preparation                     | Order hardware, set up development environment                          | PlatformIO + toolchain working                 | ✅ Done | 07.07.2026 | 13.07.2026 |
| 1     | Sensor node                     | Program the ESP32, read the sensor, set up MQTT connection              | Sensor sends readings                          | ✅ Done (Serial; MQTT publish still open) | 10.07.2026| 13.07.2026
| 2     | Embedded Linux gateway          | Configure and build the Buildroot image, flash it onto the Raspberry Pi | Custom Linux image boots on the Pi             | ✅ Done | 16.07.2026 | 11.08.2026
| 3     | Integration                     | 1. Start Mosquitto on the Pi 2. Test it from mac 3. Update ESP32 firmware(connect Wifi, publish real sensor reading via MQTT, write Python script(subscribes/writes into SQLite))       | Readings are stored persistently                | ✅ Done | 11.08.2026 | 18.08.2026 |
| 4     | Visualization                   | Connect Grafana/dashboard                                                | Live history of readings visible                | started | 26.08.2026 | 30.08.2026 |
| 5     | Documentation & portfolio       | Finalize docs, clean up GitHub repo, optionally write a short write-up   | Presentable project for job applications         | 🔄 Ongoing | — |

*(Replace the "add date" placeholders with the actual dates you did the work — that's
useful context for anyone reading the repo, and honestly satisfying to fill in.)*

## Timeframe

- **Week 28**: Phase 0 + 1 (sensor node) + doc
- **Week 29**: Phase 2 (embedded Linux – typically the most time-consuming part) + doc
- **Week 33**: Phase 3 (integration) + doc
- **Week 33**: Phase 4 (dashboard) + doc


## Definition of done per phase

A phase counts as complete when:
1. the functionality demonstrably works (a short demo/screenshot/log),
2. the related configuration/code is version-controlled in the repo,
3. the documentation (README or the relevant docs/ file) has been updated.
