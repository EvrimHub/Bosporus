# Anforderungen – Bosporus

## Funktionale Anforderungen

| ID     | Anforderung                                                                 | Priorität |
|--------|-------------------------------------------------------------------------------|-----------|
| F-01   | Der Sensor-Node erfasst Temperatur und Luftfeuchtigkeit in festen Intervallen | Muss      |
| F-02   | Der Sensor-Node überträgt Messwerte drahtlos an das Gateway                   | Muss      |
| F-03   | Das Gateway empfängt, validiert und speichert die Messwerte dauerhaft         | Muss      |
| F-04   | Die Messwerte werden zeitlich als Verlauf visualisiert                        | Muss      |
| F-05   | Bei Über-/Unterschreiten eines Schwellenwerts wird ein Hinweis angezeigt      | Soll      |
| F-06   | Mehrere Sensor-Nodes können parallel angebunden werden                       | Kann      |

## Nicht-funktionale Anforderungen

| ID     | Anforderung                                                                  |
|--------|----------------------------------------------------------------------------|
| N-01   | Das Gateway-Image ist ein selbst konfiguriertes, minimales Linux (kein Standard-OS-Image) |
| N-02   | Die Kommunikation zwischen Sensor-Node und Gateway ist nachvollziehbar dokumentiert |
| N-03   | Das System läuft stabil über mindestens 48 Stunden ohne manuellen Eingriff  |
| N-04   | Der Aufbau ist reproduzierbar (Konfigurationsdateien/Skripte sind versioniert) |
| N-05   | Das Projekt ist so dokumentiert, dass eine dritte Person den Aufbau nachvollziehen kann |

## Abgrenzung (bewusst nicht Teil dieser ersten Ausbaustufe)

- Keine Serienfertigung / kein PCB-Design in Version 1 (Breadboard-Aufbau genügt)
- Keine produktionsreife Security-Härtung (Fokus liegt auf Lernen der Grundprinzipien)
- Keine Cloud-Anbindung in Version 1 (rein lokales Netzwerk)
