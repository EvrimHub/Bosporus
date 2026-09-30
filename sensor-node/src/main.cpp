#include <Arduino.h>
#include <DHT.h>
#include <WiFi.h>
#include <secrets.h>
#include <PubSubClient.h>
#define DHTPIN 2
#define DHTTYPE DHT22

const char *MQTT_BROKER = "192.168.1.169"; // your Pi's IP
const int MQTT_PORT = 1883;
const char *MQTT_TOPIC = "sensor/room1/climate"; // matches architecture.md
const char *MQTT_CLIENT_ID = "esp32-sensor-node";

DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;
PubSubClient mqttClient(espClient);

void connectWiFi()
{
  Serial.print("Connecting to WiFi");
  Serial.println(WIFI_SSID);

  Serial.print("SSID length: ");
  Serial.println(strlen(WIFI_SSID));
  Serial.print("Password length: ");
  Serial.println(strlen(WIFI_PASSWORD));

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20)
  {
    delay(500);
    Serial.print("status=");
    Serial.println(WiFi.status());
    attempts++;
  }
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.print("WiFi connected, IP address: ");
    Serial.println(WiFi.localIP());
  }
  else
  {
    Serial.println("WiFi connection FAILED after 20 attempts.");
  }
}

void connectMQTT()
{
  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  while (!mqttClient.connected())
  {
    Serial.print("Connecting to MQTT broker...");
    if (mqttClient.connect(MQTT_CLIENT_ID))
    {
      Serial.println("Connected to MQTT broker");
    }
    else
    {
      Serial.print("Failed to connect to MQTT broker, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

void scanNetworks()
{
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(500);

  Serial.println("Scanning for WiFi networks...");
  int n = WiFi.scanNetworks();
  Serial.print("Scan returned: ");
  Serial.println(n);

  if (n <= 0)
  {
    Serial.println("No networks found at all.");
  }
  else
  {
    for (int i = 0; i < n; i++)
    {
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (RSSI: ");
      Serial.print(WiFi.RSSI(i));
      Serial.println(")");
    }
  }
}

void setup()
{
  Serial.begin(115200);
  dht.begin();
  scanNetworks();
  connectWiFi();
  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
}

void loop()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    connectWiFi();
  }
  if (!mqttClient.connected())
  {
    connectMQTT();
  }
  mqttClient.loop();

  delay(2000);
  float humidity = dht.readHumidity();
  float tempC = dht.readTemperature();

  if (isnan(humidity) || isnan(tempC))
  {
    Serial.println("Failed to read from DHT sensor");
    return;
  }

  char payload[64];
  snprintf(payload, sizeof(payload), "{\"temperature\": %.1f, \"humidity\": %.1f}", tempC, humidity);

  Serial.print("Publishing: ");
  Serial.println(payload);
  mqttClient.publish(MQTT_TOPIC, payload);
  // Serial.print("Humidity: ");
  // Serial.print(humidity);
  // Serial.print(" % Temperature: ");
  // Serial.print(tempC);
  // Serial.println(" °C");

  // put function definitions here:
  // int myFunction(int x, int y) {
  //  return x + y;
}