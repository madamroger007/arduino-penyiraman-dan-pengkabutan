#include <DHT.h>

// --- 1. Definisi Pin ---
const int trigPin   = 26;       // HC-SR04 Trig → GPIO25
const int echoPin   = 25;       // HC-SR04 Echo → GPIO26 (via divider 5V→3.3V)
const int soilPin   = 33;       // Soil Moisture AO → GPIO33 (ADC1_CH5)
const int dhtPin    = 4;        // DHT11 Data → GPIO4

// --- 2. Konfigurasi Sensor ---
#define DHTTYPE DHT11
DHT dht(dhtPin, DHTTYPE);

// --- 3. Setup Awal ---
void setup() {
  Serial.begin(115200);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  // Soil sensor hanya perlu analogRead()
  dht.begin();                   // Mulai DHT11
  Serial.println("Memulai pembacaan sensor...");
}

// --- 4. Fungsi Mengukur Jarak HC-SR04 ---
long readUltrasonicCM() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH, 30000);  // timeout 30 ms (≈5 m)
  // Jarak = (durasi × kecepatan suara) / 2
  return (duration > 0) ? (duration * 0.0343 / 2) : -1;
}

// --- 5. Loop Utama ---
void loop() {
  // 5.1 Baca jarak ultrasonik
  long jarak = readUltrasonicCM();
  if (jarak >= 0) {
    Serial.print("Jarak: ");
    Serial.print(jarak);
    Serial.println(" cm");
  } else {
    Serial.println("Jarak: Timeout / Tidak terdeteksi");
  }

  // 5.2 Baca kelembapan tanah
  int soilVal = analogRead(soilPin);        // 0–4095 (0V–3.3V)
  float soilPct = map(soilVal, 4095, 0, 0, 100);
  Serial.print("Soil Moisture: ");
  Serial.print(soilVal);
  Serial.print(" (");
  Serial.print(soilPct);
  Serial.println("%)");

  // 5.3 Baca suhu & kelembapan DHT11
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (isnan(h) || isnan(t)) {
    Serial.println("Pembacaan DHT gagal!");
  } else {
    Serial.print("Suhu: ");
    Serial.print(t);
    Serial.print(" °C | Kelembapan: ");
    Serial.print(h);
    Serial.println(" %");
  }

  // 5.4 Jeda 3 detik sebelum siklus berikutnya
  Serial.println("-----------------------------");
  delay(3000);
}
