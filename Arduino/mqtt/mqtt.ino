#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <DHT.h>

// Konfigurasi Wi-Fi
const char* ssid = "adm";
const char* password = "adams161121..";

// Konfigurasi MQTT
const char* mqtt_server = "5ef609a0286140bb9cb1dd89dabdc139.s1.eu.hivemq.cloud";
const int mqtt_port = 8883; // Gunakan 1883 untuk koneksi tanpa SSL
const char* mqtt_user = "hivemq.webclient.1746975391787";
const char* mqtt_password = "8HZK!l@%q3?Ah7o0TrxU";

// Topik MQTT
const char* topic_siram_status = "otomatis/siram/status";
const char* topic_kabut_status = "otomatis/kabut/status";
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

const int DRY_VALUE = 4000;
const int WET_VALUE = 1200;

unsigned long lastPublish = 0;
const long interval = 1000; // 1 detik

WiFiClientSecure espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);


// === Callback MQTT ===
void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  if (String(topic) == topic_siram_status) {
    digitalWrite(RELAY1_PIN, message == "1" ? LOW : HIGH);
  } else if (String(topic) == topic_kabut_status) {
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
      client.subscribe(topic_siram_status);
      client.subscribe(topic_kabut_status);
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

  unsigned long now = millis();
  if (now - lastPublish >= interval) {
    lastPublish = now;

    // Baca sensor
    float suhu = dht.readTemperature();
    float kelembapan = dht.readHumidity();
    int nilaiSensorTanah = analogRead(SOIL_MOISTURE_PIN);

    // Konversi nilai kelembapan tanah ke persentase
    int kelembapanTanah = map(nilaiSensorTanah, DRY_VALUE, WET_VALUE, 0, 100);
    kelembapanTanah = constrain(kelembapanTanah, 0, 100);

    // Kirim data ke MQTT
    client.publish(topic_suhu_udara, String(suhu).c_str(), true);
    client.publish(topic_kelembapan_udara, String(kelembapan).c_str(), true);
    client.publish(topic_kelembapan_tanah, String(kelembapanTanah).c_str(), true);
  }
}
