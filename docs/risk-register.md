# Risikoregister – Bosporus

| ID | Risiko                                                        | Wahrscheinlichkeit | Auswirkung | Gegenmaßnahme                                                              |
|----|----------------------------------------------------------------|---------------------|------------|------------------------------------------------------------------------------|
| R1 | Lieferverzögerung bei Hardware (ESP32, Sensor, Raspberry Pi)     | Mittel              | Mittel     | Frühzeitig bestellen, Alternativboards/-sensoren als Ersatz einplanen        |
| R2 | Buildroot-Image bootet nicht (Kernel-Panic, fehlende Treiber)    | Hoch                | Hoch       | Zeitpuffer einplanen, offizielle Board-Defconfig als Startpunkt nutzen       |
| R3 | Instabile WLAN-/MQTT-Verbindung des Sensor-Nodes                 | Mittel              | Niedrig    | Reconnect-Logik im Code, Log-Ausgabe zur Fehlersuche                        |
| R4 | Zeitmangel neben Bewerbungsprozess/Beruf                         | Hoch                | Mittel     | Projekt bewusst in kleine, in sich abgeschlossene Phasen unterteilt (siehe Projektplan) |
| R5 | Zu großer Funktionsumfang, Projekt wird nie "fertig"             | Mittel              | Hoch       | Klare Abgrenzung in requirements.md, MVP zuerst, Erweiterungen optional      |
| R6 | Wissenslücken bei Embedded Linux verzögern Phase 2 deutlich       | Hoch                | Hoch       | Buildroot vor Yocto (geringere Einstiegshürde), Community-Foren/Doku aktiv nutzen |

## Hinweis zur Nutzung

Dieses Register kann direkt in Bewerbungsgesprächen als Gesprächsanker dienen: Es zeigt,
dass Risiken in technischen Projekten nicht nur benannt, sondern aktiv mit Gegenmaßnahmen
gesteuert wurden – eine Kernkompetenz für die Rolle der technischen Projektleitung.
