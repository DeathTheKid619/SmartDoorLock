/*
  Smart Door Lock System - ESP32 + MQTT

  This project implements a basic smart door locking system using:
  - ESP32 microcontroller
  - MQTT protocol (publish / subscribe)
  - JSON-formatted messages (ArduinoJson)

  Core functionality:
  - Connect to WiFi
  - Connect to an MQTT broker
  - Receive lock / unlock commands
  - Control a locking actuator (relay / solenoid / servo)
  - Monitor door status using a reed switch
  - Publish status updates and system events

  NOTE:
  - This version does NOT include RFID, keypad, or user authentication
  - It represents the BASE implementation, designed to be easily extensible
*/

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// =====================================================
// WIFI CONFIGURATION
// =====================================================
// TODO: Replace these values with your WiFi credentials
const char* WIFI_SSID     = "YOUR_WIFI_SSID";        // <-- your WiFi name
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";   // <-- your WiFi password

// =====================================================
// MQTT CONFIGURATION
// =====================================================
// Public test broker (can be replaced with local or cloud broker)
const char* MQTT_BROKER    = "test.mosquitto.org";
const int   MQTT_PORT      = 1883;

// Unique MQTT client ID
const char* MQTT_CLIENT_ID = "door1-esp32";

// MQTT topics used by the system
const char* TOPIC_CMD    = "home/door1/cmd";     // receives lock/unlock commands
const char* TOPIC_STATUS = "home/door1/status";  // publishes door status
const char* TOPIC_EVENT  = "home/door1/event";   // publishes system events

// =====================================================
// HARDWARE PIN CONFIGURATION
// =====================================================
const int PIN_LOCK = 23;   // Lock actuator (relay / solenoid)
                            // LOW  = locked
                            // HIGH = unlocked

const int PIN_REED = 22;   // Reed switch (door position sensor)
                            // HIGH = door closed
                            // LOW  = door open

// =====================================================
// GLOBAL OBJECTS
// =====================================================
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// =====================================================
// SYSTEM STATE VARIABLES
// =====================================================
bool lastDoorClosed = false;                 // previous door state
unsigned long lastStatusPublish = 0;         // timestamp of last status publish
const unsigned long STATUS_INTERVAL = 5000;  // publish status every 5 seconds

// =====================================================
// WIFI CONNECTION FUNCTION
// =====================================================
void connectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // Wait until the ESP32 is connected to WiFi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

// =====================================================
// GENERIC JSON PUBLISH FUNCTION
// =====================================================
// Publishes a JsonDocument to a given MQTT topic
void publishJson(const char* topic, JsonDocument& doc, bool retained = false) {
  char buffer[128];                     // JSON output buffer
  size_t length = serializeJson(doc, buffer);
  mqttClient.publish(topic, (uint8_t*)buffer, length, retained);
}

// =====================================================
// PUBLISH DOOR STATUS (JSON)
// =====================================================
void publishStatus(bool closed) {
  JsonDocument doc;
  doc["device"] = "door1";
  doc["door"] = closed ? "closed" : "open";
  doc["timestamp"] = millis();

  publishJson(TOPIC_STATUS, doc, true); // retained message
}

// =====================================================
// PUBLISH SYSTEM EVENT (JSON)
// =====================================================
void publishEvent(const char* eventType) {
  JsonDocument doc;
  doc["device"] = "door1";
  doc["event"] = eventType;
  doc["timestamp"] = millis();

  publishJson(TOPIC_EVENT, doc);
}

// =====================================================
// MQTT MESSAGE CALLBACK
// =====================================================
// This function is called whenever a message is received
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message;

  // Convert payload bytes to String
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  message.trim();

  // Process commands
  if (message.equalsIgnoreCase("lock")) {
    digitalWrite(PIN_LOCK, LOW);     // lock the door
    publishEvent("lock_command_received");
  }
  else if (message.equalsIgnoreCase("unlock")) {
    digitalWrite(PIN_LOCK, HIGH);    // unlock the door
    publishEvent("unlock_command_received");
  }
}

// =====================================================
// MQTT CONNECTION FUNCTION
// =====================================================
void connectMQTT() {
  while (!mqttClient.connected()) {
    if (mqttClient.connect(MQTT_CLIENT_ID)) {
      mqttClient.subscribe(TOPIC_CMD);   // subscribe to command topic
      publishEvent("mqtt_connected");
    } else {
      delay(2000); // retry after delay
    }
  }
}

// =====================================================
// SETUP FUNCTION (runs once)
// =====================================================
void setup() {
  // Configure hardware pins
  pinMode(PIN_LOCK, OUTPUT);
  pinMode(PIN_REED, INPUT);

  // Default state: door locked
  digitalWrite(PIN_LOCK, LOW);

  // Connect to WiFi
  connectWiFi();

  // Configure MQTT client
  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);
}

// =====================================================
// MAIN LOOP (runs continuously)
// =====================================================
void loop() {
  // Ensure MQTT connection
  if (!mqttClient.connected()) {
    connectMQTT();
  }
  mqttClient.loop();

  // Read door sensor
  bool doorClosed = (digitalRead(PIN_REED) == HIGH);

  // Publish status if door state has changed
  if (doorClosed != lastDoorClosed) {
    publishStatus(doorClosed);
    lastDoorClosed = doorClosed;
  }

  // Publish status periodically (heartbeat)
  if (millis() - lastStatusPublish > STATUS_INTERVAL) {
    publishStatus(doorClosed);
    lastStatusPublish = millis();
  }
}