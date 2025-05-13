#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <DHT.h>

// Konfigurasi Wi-Fi
const char* ssid = "Konsol";
const char* password = "20011116..";

// Konfigurasi MQTT
const char* mqtt_server = "5ef609a0286140bb9cb1dd89dabdc139.s1.eu.hivemq.cloud";
const int mqtt_port = 8883; // Gunakan 1883 untuk koneksi tanpa SSL
const char* mqtt_user = "hivemq.webclient.1746975391787";
const char* mqtt_password = "8HZK!l@%q3?Ah7o0TrxU";

// Topik MQTT
const char* topic_siram_status = "otomatis/siram/status";
const char* topic_kabut_status = "otomatis/kabut/status";
const char* topic_siram_perintah = "otomatis/siram/perintah";
const char* topic_kabut_perintah = "otomatis/kabut/perintah";
const char* topic_kelembapan_tanah = "sensor/kelembapan_tanah";
const char* topic_suhu_udara = "sensor/suhu_udara";
const char* topic_kelembapan_udara = "sensor/kelembapan_udara";
const char* topic_ketinggian_air = "sensor/ketinggian_air";

// Konfigurasi pin
#define RELAY1_PIN 22
#define RELAY2_PIN 23
#define DHTPIN 4
#define DHTTYPE DHT11
#define SOIL_MOISTURE_PIN 33 // A0
#define TRIG_PIN 26
#define ECHO_PIN 25
const int DRY_VALUE = 4000;
const int WET_VALUE = 1200;

WiFiClientSecure espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);

// === Fungsi untuk Mengukur Jarak dari HC-SR04 ===
float readDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // Timeout 30ms
  float distance = duration * 0.034 / 2;
  return distance;
}

// === Callback MQTT ===
void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  if (String(topic) == "otomatis/siram/status") {
    digitalWrite(RELAY1_PIN, message == "1" ? LOW : HIGH);
  } else if (String(topic) == "otomatis/kabut/status") {
    digitalWrite(RELAY2_PIN, message == "1" ? LOW : HIGH);
  }
}

// === Koneksi ke WiFi ===
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Menghubungkan ke ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi terhubung");
  Serial.println("Alamat IP: ");
  Serial.println(WiFi.localIP());
}

// === Setup Awal ===
void setup() {
  Serial.begin(115200);
  dht.begin();

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);

  digitalWrite(RELAY1_PIN, HIGH); // Relay non-aktif (aktif LOW)
  digitalWrite(RELAY2_PIN, HIGH);

  setup_wifi();

  espClient.setInsecure(); // Nonaktifkan verifikasi sertifikat SSL (hanya untuk pengujian)
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

// === Reconnect MQTT ===
void reconnect() {
  while (!client.connected()) {
    Serial.print("Menghubungkan ke MQTT...");
    if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {
      Serial.println("terhubung");
      client.subscribe("otomatis/siram/status");
      client.subscribe("otomatis/kabut/status");
    } else {
      Serial.print("gagal, rc=");
      Serial.print(client.state());
      Serial.println(" coba lagi dalam 5 detik");
      delay(5000);
    }
  }
}

// === Loop Utama ===
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Baca sensor
  const float tinggiTandon = 150.0; // Tinggi tandon dalam cm
  float suhu = dht.readTemperature();
  float kelembapan = dht.readHumidity();
  int nilaiSensorTanah = analogRead(SOIL_MOISTURE_PIN);
  float jarakAir = readDistanceCM();

  // Konversi nilai kelembapan tanah ke persentase
  int kelembapanTanah = map(nilaiSensorTanah, DRY_VALUE, WET_VALUE, 0, 100);
  kelembapanTanah = constrain(kelembapanTanah, 0, 100);

// Pastikan jarakAir tidak melebihi tinggiTandon
jarakAir = constrain(jarakAir, 0, tinggiTandon);

// Hitung ketinggian air
float ketinggianAir = tinggiTandon - jarakAir;

// Hitung persentase ketinggian air
int persentaseAir = (ketinggianAir / tinggiTandon) * 100;
persentaseAir = constrain(persentaseAir, 0, 100);

  // Kirim data ke MQTT
  client.publish("sensor/suhu_udara", String(suhu).c_str(), true);
  client.publish("sensor/kelembapan_udara", String(kelembapan).c_str(), true);
  client.publish("sensor/kelembapan_tanah", String(kelembapanTanah).c_str(), true);
  client.publish("sensor/ketinggian_air", String(persentaseAir).c_str(), true);

  delay(5000); // Kirim data setiap 5 detik
}
