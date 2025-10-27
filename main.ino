#include "Global.h"

void setup() {
  initialSetup();
}

void loop() {
  if (!mqttClient.connected()) {
    reconnect();  // PERHATIAN: Ini memblokir jika MQTT terputus
  }
  mqttClient.loop();

  handleButtons();  // Cek tombol di setiap loop, ini harus sering dipanggil agar responsif

  long now = millis();
  if (now - previous_time > 1000) {
    previous_time = now;

    int result = dht11.readTemperatureHumidity(temperature, humidity);
    String payload = "{";
    payload += "\"temperature\":";
    payload += String(temperature);
    payload += ",";
    payload += "\"humidity\":";
    payload += String(humidity);
    payload += "}";

    Serial.print("Mengirim payload: ");
    Serial.println(payload);

    if (result == 0) {
      updateDisplay();  // Update OLED dengan data terbaru
      updateLED(humidity);
    } else {
      Serial.println(DHT11::getErrorString(result));
    }

    mqttClient.publish(topic_data, payload.c_str());
  }
}