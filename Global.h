#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <DHT11.h>
#include "Oled.h"
#include "Setup.h"

// ================================================
// |          SETUP WIFI dan MQTT Details         |
// ================================================

// Wi-Fi credentials
const char* ssid = "hii";
const char* password = "mosesjaguar";

// MQTT broker details private
const char* mqtt_broker = "080522bed59b4533b611733a5b9823a2.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char* mqtt_username = "mosesjaguar";
const char* mqtt_password = "Fmosesn552288";

const char* topic_data = "esp32/data";

// ---------------- SETUP DHT --------------
#define DHT11PIN 32
DHT11 dht11(32);
int temperature = 0;
int humidity = 0;

WiFiClientSecure wifiClient;
PubSubClient mqttClient(wifiClient);
long previous_time = 0;

// --------- Define LED ------------- //
const int LED_HIJAU_1_PIN = 33;
const int LED_HIJAU_2_PIN = 25;
const int LED_KUNING_1_PIN = 27;
const int LED_KUNING_2_PIN = 26;
const int LED_MERAH_1_PIN = 12;
const int LED_MERAH_2_PIN = 14;

// ------- Define Button -------- //
const int BUTTON_LEFT = 19; 
const int BUTTON_SELECT = 18;
const int BUTTON_RIGHT = 17;

// ----------------- Function MQTT ----------------- //
void setupMQTT() {
  mqttClient.setServer(mqtt_broker, mqtt_port);
}

void reconnect() {
  Serial.println("Connecting to MQTT Broker...");
  while (!mqttClient.connected()) {
    Serial.println("Reconnecting to MQTT Broker...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);

    if (mqttClient.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Connected to MQTT Broker.");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}
