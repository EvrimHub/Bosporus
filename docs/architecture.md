# Systemarchitektur – Bosporus

## Übersicht

```
┌────────────────┐        MQTT        ┌──────────────────────┐      HTTP/Query      ┌──────────────┐
│   Sensor-Node   │ ─────────────────▶ │       Gateway         │ ────────────────────▶ │  Dashboard    │
│  ESP32,         │                    │  Raspberry Pi,        │                        │  Grafana /    │
│  FreeRTOS       │                    │  Embedded Linux       │                        │  Web-UI       │
└────────────────┘                    └──────────────────────┘                        └──────────────┘
```

## Komponente 1: Sensor-Node

- **Hardware**: ESP32 DevKit + DHT22 (Temperatur/Luftfeuchtigkeit)
- **Software**: FreeRTOS-Task liest Sensor periodisch aus (z. B. alle 30s), zweiter Task
  verwaltet die WLAN-/MQTT-Verbindung
- **Aufgabe**: Rohdaten erfassen, in ein einfaches JSON-Format verpacken, per MQTT publizieren
- **Lernthemen**: Task-Scheduling, Interrupts vs. Polling, Power-Management, WLAN-Stack

## Komponente 2: Gateway

- **Hardware**: Raspberry Pi 4 / Zero 2 W
- **Software**: Selbst gebautes Linux-Image (Buildroot), MQTT-Broker (Mosquitto),
  kleines Verarbeitungsskript (Python/Node), Datenhaltung in SQLite oder InfluxDB
- **Aufgabe**: Daten empfangen, validieren, persistieren, für das Dashboard bereitstellen
- **Lernthemen**: Bootloader, Kernel-Konfiguration, Root-Filesystem, Cross-Compiling,
  Systemd/Init-Prozesse, Paketverwaltung in einem minimalen Linux-Image

## Komponente 3: Dashboard

- **Software**: Grafana (oder alternativ ein einfaches selbstgebautes Web-Frontend)
- **Aufgabe**: Zeitreihen visualisieren, Schwellenwerte/Alarme konfigurieren
- **Lernthemen**: Datenquellen-Anbindung, Zeitreihenvisualisierung

## Datenfluss im Detail

1. Sensor-Node misst Wert → verpackt als JSON → publiziert auf MQTT-Topic `sensor/raum1/klima`
2. Gateway abonniert Topic → Verarbeitungsskript validiert und schreibt in Datenbank
3. Dashboard fragt Datenbank ab bzw. bindet sie als Datenquelle ein → stellt Verlauf dar

## Erweiterungsmöglichkeiten (optional, für spätere Ausbaustufen)

- OTA-Firmware-Updates für den Sensor-Node
- TLS-Verschlüsselung der MQTT-Verbindung
- Mehrere Sensor-Nodes gleichzeitig (Skalierungsfrage)
- Yocto-Variante des Gateway-Images zum Vergleich mit Buildroot
