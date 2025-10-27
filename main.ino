#include "Global.h"

void setup() {
  initialSetup();
}

void loop() {
  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();

  // ----------- send data to MQTT ---------------
  long now = millis();
  if (now - previous_time > 1000) {
    previous_time = now;

    // Input Data MQTT
    int result = dht11.readTemperatureHumidity(temperature, humidity);
    String payload = "{";
    payload += "\"temperature\":";
    payload += String(temperature);
    payload += ",";
    payload += "\"humidity\":";
    payload += String(humidity);
    payload += "}";

    // Debug
    Serial.print("Mengirim payload: ");
    Serial.println(payload);

    if (result == 0) {
      if (humidity > 0 && humidity < 20) {

      } else if (humidity >= 20 && humidity < 60) {
     
      } else if (humidity >= 60) {
        
      }

    } else {
      Serial.println(DHT11::getErrorString(result));
    }

    mqttClient.publish(topic_data, payload.c_str());
  }
}