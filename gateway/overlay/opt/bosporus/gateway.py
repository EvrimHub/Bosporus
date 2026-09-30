import json
import sqlite3
import time
import paho.mqtt.client as mqtt

MQTT_BROKER = "localhost"  # script runs ON the Pi, same machine as the broker
MQTT_PORT = 1883
MQTT_TOPIC = "sensor/room1/climate"

DB_PATH = "/opt/bosporus/bosporus.db"

def init_db():
    conn = sqlite3.connect(DB_PATH)
    conn.execute("""
        CREATE TABLE IF NOT EXISTS readings (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            timestamp INTEGER NOT NULL,
            temperature REAL,
            humidity REAL
        )
    """)
    conn.commit()
    conn.close()

def on_connect(client, userdata, flags, rc):
    print(f"Connected to broker, rc={rc}")
    client.subscribe(MQTT_TOPIC)

def on_message(client, userdata, msg):
    try:
        payload = json.loads(msg.payload.decode())
        temperature = payload.get("temperature")
        humidity = payload.get("humidity")

        conn = sqlite3.connect(DB_PATH)
        conn.execute(
            "INSERT INTO readings (timestamp, temperature, humidity) VALUES (?, ?, ?)",
            (int(time.time()), temperature, humidity)
        )
        conn.commit()
        conn.close()

        print(f"Stored: temp={temperature}, humidity={humidity}")
    except Exception as e:
        print(f"Failed to process message: {e}")

def main():
    init_db()
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message
    client.connect(MQTT_BROKER, MQTT_PORT)
    client.loop_forever()

if __name__ == "__main__":
    main()
