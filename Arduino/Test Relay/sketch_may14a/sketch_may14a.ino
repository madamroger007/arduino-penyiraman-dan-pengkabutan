#define RELAY1_PIN 22  // Ganti dengan nomor pin digital yang sesuai
#define RELAY2_PIN 23  // Ganti dengan nomor pin digital yang sesuai

void setup() {
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);

  // Matikan kedua relay saat awal
  digitalWrite(RELAY1_PIN, HIGH);
  digitalWrite(RELAY2_PIN, HIGH);
}

void loop() {
  // Nyalakan relay 1
  digitalWrite(RELAY1_PIN, LOW);  // Aktifkan relay (aktif LOW)
  delay(1000);                    // Tunggu 1 detik

  // Matikan relay 1
  digitalWrite(RELAY1_PIN, HIGH); // Nonaktifkan relay
  delay(1000);                    // Tunggu 1 detik

  // Nyalakan relay 2
  digitalWrite(RELAY2_PIN, LOW);  // Aktifkan relay
  delay(1000);                    // Tunggu 1 detik

  // Matikan relay 2
  digitalWrite(RELAY2_PIN, HIGH); // Nonaktifkan relay
  delay(1000);                    // Tunggu 1 detik
}
