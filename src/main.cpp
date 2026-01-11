#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// =======================
// CONFIGURARE REȚEA
// =======================
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// =======================
// CONFIGURARE MQTT
// =======================
const char* MQTT_BROKER = "test.mosquitto.org";
const int   MQTT_PORT   = 1883;

const char* MQTT_CLIENT_ID = "door1-esp32";

// Topic-uri
const char* TOPIC_CMD    = "home/door1/cmd";
const char* TOPIC_STATUS = "home/door1/status";
const char* TOPIC_EVENT  = "home/door1/event";

// =======================
// CONFIGURARE PINI
// =======================
const int PIN_LOCK = 23;   // actuator (relay / solenoid)
const int PIN_REED = 22;   // senzor stare ușă

// =======================
// OBIECTE GLOBALE
// =======================
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// =======================
// STARE INTERNĂ
// =======================
bool doorClosed = false;
bool lastDoorClosed = false;

// =======================
// FUNCȚII MQTT
// =======================
void publishStatus(bool closed) {
  const char* payload = closed ? "closed" : "open";
  mqttClient.publish(TOPIC_STATUS, payload, true);
}

void publishEvent(const char* event) {
  mqttClient.publish(TOPIC_EVENT, event);
}

// =======================
// CALLBACK MQTT
// =======================
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  message.trim();

  if (String(topic) == TOPIC_CMD) {
    if (message == "lock") {
      digitalWrite(PIN_LOCK, LOW);
      publishEvent("locked");
    }
    else if (message == "unlock") {
      digitalWrite(PIN_LOCK, HIGH);
      delay(700);                  // timp activare
      digitalWrite(PIN_LOCK, LOW);
      publishEvent("unlocked");
    }
  }
}

// =======================
// RECONNECT MQTT
// =======================
void reconnectMQTT() {
  while (!mqttClient.connected()) {
    if (mqttClient.connect(MQTT_CLIENT_ID)) {
      mqttClient.subscribe(TOPIC_CMD);
      publishEvent("device_online");
    } else {
      delay(3000);
    }
  }
}

// =======================
// SETUP
// =======================
void setup() {
  pinMode(PIN_LOCK, OUTPUT);
  pinMode(PIN_REED, INPUT_PULLUP);

  digitalWrite(PIN_LOCK, LOW);

  Serial.begin(115200);

  // WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  // MQTT
  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);
}

// =======================
// LOOP
// =======================
void loop() {
  if (!mqttClient.connected()) {
    reconnectMQTT();
  }

  mqttClient.loop();

  doorClosed = digitalRead(PIN_REED) == HIGH;

  if (doorClosed != lastDoorClosed) {
    publishStatus(doorClosed);
    lastDoorClosed = doorClosed;
  }

  delay(50);
}