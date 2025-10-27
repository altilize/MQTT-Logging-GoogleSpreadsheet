#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <DHT11.h>

// ------ OLED ---------- //
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


// --------------- SETUP WIFI & MQTT ---------- //
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
const int LED_KUNING_1_PIN = 26;
const int LED_KUNING_2_PIN = 27;
const int LED_MERAH_1_PIN = 12;
const int LED_MERAH_2_PIN = 14;
const int MIN_BRIGHTNESS = 40;   // Kecerahan minimum (agar tidak mati total)
const int MAX_BRIGHTNESS = 255;  // Kecerahan maksimum (default ESP32)

// ------- Define Button -------- //
const int BUTTON_LEFT = 19;
const int BUTTON_SELECT = 18;
const int BUTTON_RIGHT = 17;

#define BUZZER 5

// --- Variabel State untuk UI dan Tombol ---
int currentSlide = 1;  // 1 untuk Humidity, 2 untuk Temperature
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;
int lastButtonLeftState = HIGH;
int lastButtonRightState = HIGH;

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
      delay(5000);  // PERHATIAN: Delay ini memblokir, membuat tombol tidak responsif saat reconnect
    }
  }
}

// -------- Update Tampilan OLED -----------
void updateDisplay() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  // Buffer untuk menampung string yang akan ditampilkan
  char lineBuffer[20];

  display.setTextSize(2);
  display.setCursor(0, 0);  // Reset cursor

  if (currentSlide == 1) {
    snprintf(lineBuffer, sizeof(lineBuffer), "Humidity");
  } else {  // currentSlide == 2
    snprintf(lineBuffer, sizeof(lineBuffer), "Temp.");
  }
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(lineBuffer, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w) / 2, 0);
  display.println(lineBuffer);

  display.setTextSize(3);
  display.setCursor(0, 25);

  if (currentSlide == 1) {
    snprintf(lineBuffer, sizeof(lineBuffer), "%d %%", humidity);
  } else {  // currentSlide == 2
    snprintf(lineBuffer, sizeof(lineBuffer), "%d C", temperature);
  }
  display.getTextBounds(lineBuffer, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w) / 2, 25);  // Tengah di Y 25
  display.println(lineBuffer);

  display.setTextSize(1);
  display.setCursor(0, SCREEN_HEIGHT - 8);  // Paling bawah layar

  if (currentSlide == 2) {
    display.print("< ");
  } else {
    display.print("  ");
  }

  display.setCursor((SCREEN_WIDTH - (8 * 5)) / 2, SCREEN_HEIGHT - 8);
  display.print("MENU");
  // Panah Kanan
  display.setCursor(SCREEN_WIDTH - (8 * 2), SCREEN_HEIGHT - 8);  // Pindah ke kanan (8*2 karena panah + spasi)
  if (currentSlide == 1) {                                       // Jika sedang di slide 1 (humidity), bisa lanjut ke kanan (temp)
    display.print(" >");
  } else {  // Jika di slide 2 (temp), kosongkan ruang
    display.print("  ");
  }

  display.display();  // Tampilkan semua ke layar
}

// ----------- Handle Tombol & Debounce --------
void handleButtons() {
  if ((millis() - lastDebounceTime) > debounceDelay) {
    int readingLeft = digitalRead(BUTTON_LEFT);
    int readingRight = digitalRead(BUTTON_RIGHT);

    // Kiri: Kembali ke Humidity (Slide 1)
    if (readingLeft == LOW && lastButtonLeftState == HIGH) {
      if (currentSlide == 2) {  // Hanya pindah jika sedang di slide Temp
        currentSlide = 1;
        updateDisplay();
        lastDebounceTime = millis();
      }
    }

    // Kanan: Pindah ke Temperature (Slide 2)
    if (readingRight == LOW && lastButtonRightState == HIGH) {
      if (currentSlide == 1) {  // Hanya pindah jika sedang di slide Humidity
        currentSlide = 2;
        updateDisplay();
        lastDebounceTime = millis();
      }
    }

    // Simpan state tombol saat ini
    lastButtonLeftState = readingLeft;
    lastButtonRightState = readingRight;
  }
}


// ----------------- Initial Setup ---------------- //
void initialSetup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);  // PERHATIAN: Delay ini memblokir, tombol tidak akan responsif saat konek WiFi
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Connected to Wi-Fi");
  wifiClient.setInsecure();

  setupMQTT();

  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);

  pinMode(BUTTON_LEFT, INPUT_PULLUP);
  pinMode(BUTTON_RIGHT, INPUT_PULLUP);
  pinMode(BUTTON_SELECT, INPUT_PULLUP);

  pinMode(LED_HIJAU_1_PIN, OUTPUT);
  pinMode(LED_HIJAU_2_PIN, OUTPUT);
  pinMode(LED_KUNING_1_PIN, OUTPUT);
  pinMode(LED_KUNING_2_PIN, OUTPUT);
  pinMode(LED_MERAH_1_PIN, OUTPUT);
  pinMode(LED_MERAH_2_PIN, OUTPUT);
  digitalWrite(LED_HIJAU_1_PIN, LOW);
  digitalWrite(LED_HIJAU_2_PIN, LOW);
  digitalWrite(LED_KUNING_1_PIN, LOW);
  digitalWrite(LED_KUNING_2_PIN, LOW);
  digitalWrite(LED_MERAH_1_PIN, LOW);
  digitalWrite(LED_MERAH_2_PIN, LOW);
  Wire.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Gagal inisialisasi SSD1306"));
    for (;;)
      ;  
  }

  // Tampilkan layar awal saat booting
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("Connecting to WiFi..."));
  display.display();
  delay(2000);  // Beri sedikit waktu untuk melihat pesan

  // Setelah koneksi WiFi, tampilkan UI pertama kali
  updateDisplay();
}

// --------- LED ------------/
// -------- Update Tampilan LED (Versi analogWrite) --------
// -------- Update Tampilan LED (Versi "Bar Level" - Hanya Humidity) --------
void updateLED(int hum) {
  // ----- RESET SEMUA LED KE 0 (MATI) DULU -----
  analogWrite(LED_HIJAU_1_PIN, 0);
  analogWrite(LED_HIJAU_2_PIN, 0);
  analogWrite(LED_KUNING_1_PIN, 0);
  analogWrite(LED_KUNING_2_PIN, 0);
  analogWrite(LED_MERAH_1_PIN, 0);
  analogWrite(LED_MERAH_2_PIN, 0);

  long brightness;
  int deviation = abs(hum - 50); 


  // Zona 6: PARAH BANGET (Deviasi > 25 / Hum < 25 atau > 75)
  // 2 Hijau + 2 Kuning + 1 Merah (Full) + Merah ke-2 (Fade-in)
  if (deviation > 25) {
    analogWrite(LED_HIJAU_1_PIN, MAX_BRIGHTNESS);
    analogWrite(LED_HIJAU_2_PIN, MAX_BRIGHTNESS);
    analogWrite(LED_KUNING_1_PIN, MAX_BRIGHTNESS);
    analogWrite(LED_KUNING_2_PIN, MAX_BRIGHTNESS);
    analogWrite(LED_MERAH_1_PIN, MAX_BRIGHTNESS);
    // LED Merah ke-2 fade-in, lalu mentok di MAX
    brightness = map(deviation, 25, 30, MIN_BRIGHTNESS, MAX_BRIGHTNESS);
    if (brightness > MAX_BRIGHTNESS) brightness = MAX_BRIGHTNESS; // Batasi
    analogWrite(LED_MERAH_2_PIN, brightness);
    digitalWrite(BUZZER, HIGH);
  }
  // Zona 5: PARAH (Deviasi 20-25 / Hum 25-30 atau 70-75)
  // 2 Hijau + 2 Kuning (Full) + Merah 1 (Fade-in)
  else if (deviation > 20) { 
    analogWrite(LED_HIJAU_1_PIN, MAX_BRIGHTNESS);
    analogWrite(LED_HIJAU_2_PIN, MAX_BRIGHTNESS);
    analogWrite(LED_KUNING_1_PIN, MAX_BRIGHTNESS);
    analogWrite(LED_KUNING_2_PIN, MAX_BRIGHTNESS);
    brightness = map(deviation, 20, 25, MIN_BRIGHTNESS, MAX_BRIGHTNESS);
    analogWrite(LED_MERAH_1_PIN, brightness);
    digitalWrite(BUZZER, HIGH);
  }
  // Zona 4: TIDAK IDEAL BANGET (Deviasi 15-20 / Hum 30-35 atau 65-70)
  // 2 Hijau + 1 Kuning (Full) + Kuning ke-2 (Fade-in)
  else if (deviation > 15) { 
    analogWrite(LED_HIJAU_1_PIN, MAX_BRIGHTNESS);
    analogWrite(LED_HIJAU_2_PIN, MAX_BRIGHTNESS);
    analogWrite(LED_KUNING_1_PIN, MAX_BRIGHTNESS);
    brightness = map(deviation, 15, 20, MIN_BRIGHTNESS, MAX_BRIGHTNESS);
    analogWrite(LED_KUNING_2_PIN, brightness);
    digitalWrite(BUZZER, LOW);
  }
  // Zona 3: TIDAK IDEAL (Deviasi 10-15 / Hum 35-40 atau 60-65)
  // 2 Hijau (Full) + 1 Kuning (Fade-in)
  else if (deviation > 10) { 
    analogWrite(LED_HIJAU_1_PIN, MAX_BRIGHTNESS);
    analogWrite(LED_HIJAU_2_PIN, MAX_BRIGHTNESS);
    brightness = map(deviation, 10, 15, MIN_BRIGHTNESS, MAX_BRIGHTNESS);
    analogWrite(LED_KUNING_1_PIN, brightness);
    digitalWrite(BUZZER, LOW);
  }
  // Zona 2: IDEAL MENUJU TIDAK IDEAL (Deviasi 5-10 / Hum 40-45 atau 55-60)
  // 1 Hijau (Full) + Hijau ke-2 (Fade-in)
  else if (deviation > 5) { 
    analogWrite(LED_HIJAU_1_PIN, MAX_BRIGHTNESS);
    brightness = map(deviation, 5, 10, MIN_BRIGHTNESS, MAX_BRIGHTNESS);
    analogWrite(LED_HIJAU_2_PIN, brightness);
    digitalWrite(BUZZER, LOW);
  }
  // Zona 1: IDEAL (Deviasi 0-5 / Hum 45-55)
  // 1 Hijau (Fade-out saat menjauh dari 50)
  else if (deviation <= 5) { 
    brightness = map(deviation, 0, 5, MAX_BRIGHTNESS, MIN_BRIGHTNESS);
    analogWrite(LED_HIJAU_1_PIN, brightness);
    digitalWrite(BUZZER, LOW);
  }
  
  if (hum <= 0) {
      analogWrite(LED_HIJAU_1_PIN, 0);
      analogWrite(LED_HIJAU_2_PIN, 0);
      analogWrite(LED_KUNING_1_PIN, 0);
      analogWrite(LED_KUNING_2_PIN, 0);
      analogWrite(LED_MERAH_1_PIN, MAX_BRIGHTNESS);
      analogWrite(LED_MERAH_2_PIN, MAX_BRIGHTNESS);
      digitalWrite(BUZZER, HIGH);
  }
}