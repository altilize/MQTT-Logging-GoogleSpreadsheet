# ESP32 Humidity Monitor with MQTT Logging

ESP32 firmware that reads a DHT11 sensor, shows the values on an OLED with a button-driven UI, drives an LED bar and buzzer alarm, and publishes JSON telemetry to HiveMQ Cloud over TLS.

<!-- TODO: add demo GIF/photo -->

## Overview

Every second the ESP32 reads temperature and humidity from a DHT11 and publishes a JSON payload to an MQTT topic on a HiveMQ Cloud cluster (port 8883, TLS). The data is intended to be logged to a Google Spreadsheet by a subscriber on the broker side; that bridge is not included in this repository.

Locally, a six-LED bar shows how far humidity is from the 50% target, and a buzzer sounds when it leaves the 30 to 70% range or the sensor fails.

## Features

- MQTT over TLS (`WiFiClientSecure` + `PubSubClient`) with username/password auth
- JSON payload: `{"temperature": <int>, "humidity": <int>}` on topic `esp32/data`, 1 Hz
- 128x64 SSD1306 OLED with two screens (Humidity, Temperature), switched with left/right buttons and debounced in software
- Six-zone LED bar graph (2 green, 2 yellow, 2 red) with PWM brightness interpolation inside each zone
- Non-blocking buzzer latch with a 3 s minimum on-time
- Sensor-error indication (both red LEDs on plus alarm)

## Humidity zones

| Deviation from 50% RH | Humidity range | LEDs | Buzzer |
| --- | --- | --- | --- |
| 0 to 5 | 45 to 55% | Green 1 (dimming) | Off |
| 5 to 10 | 40 to 45 / 55 to 60% | + Green 2 | Off |
| 10 to 15 | 35 to 40 / 60 to 65% | + Yellow 1 | Off |
| 15 to 20 | 30 to 35 / 65 to 70% | + Yellow 2 | Off |
| 20 to 25 | 25 to 30 / 70 to 75% | + Red 1 | On |
| > 25 | < 25 / > 75% | + Red 2 | On |

## Hardware

| Part | ESP32 pin |
| --- | --- |
| DHT11 data | GPIO 32 |
| SSD1306 OLED (I2C, 0x3C) | SDA 21, SCL 22 (default) |
| Green LEDs | GPIO 33, 25 |
| Yellow LEDs | GPIO 26, 27 |
| Red LEDs | GPIO 12, 14 |
| Buttons left / select / right (to GND) | GPIO 19, 18, 17 |
| Buzzer (active low) | GPIO 16 |

## Build and run

1. Install the ESP32 board package and these libraries in the Arduino IDE: `PubSubClient`, `DHT11` (Dhruba Saha), `Adafruit GFX`, `Adafruit SSD1306`.
2. In `Global.h`, set your Wi-Fi credentials, HiveMQ Cloud host, username and password.
3. Open `main.ino` and upload.
4. Subscribe to `esp32/data` on the broker (for example with the HiveMQ web client) to see the telemetry.

The sketch uses `setInsecure()` and does not verify the broker certificate. Load the broker CA with `setCACert()` for production use.
