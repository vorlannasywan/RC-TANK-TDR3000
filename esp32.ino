#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// Konfigurasi WiFi
const char* ssid = "vorlan";         // Ganti dengan SSID WiFi Anda
const char* password = "0nyadelapan";  // Ganti dengan password WiFi Anda

// Konfigurasi MQTT
const char* mqtt_server = "192.168.13.248";  // IP broker Mosquitto
const int mqtt_port = 1884;                  // Port broker
const char* mqtt_topic = "kontroller/topic";

//Timer DHT
bool vibrationDetected = false; 
unsigned long previousMillis = 0;  // Menyimpan waktu sebelumnya
const long interval = 1000;       // Interval waktu dalam milidetik (1 detik)

// Deklarasi pin motor
#define IN1 19
#define IN2 18
#define IN3 4
#define IN4 2
#define ENA 5
#define ENB 15

// Deklarasi pin relay untuk mode tembak
#define RELAY_PIN 25

// Deklarasi sensor DHT dan piezo
#define DHTPIN 22
#define DHTTYPE DHT11
#define PIEZOPIN 34
DHT dht(DHTPIN, DHTTYPE);

int motorSpeed = 255;

// Objek WiFi dan MQTT
WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMessageTime = 0;
const unsigned long timeout = 200;  // Timeout 250 ms
bool motorActive = false;

// Fungsi untuk menghubungkan ke WiFi
void setup_wifi() {
  delay(10);
  Serial.println("Menghubungkan ke WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi Terhubung!");
}

// Fungsi callback untuk menangani pesan MQTT
void callback(char* topic, byte* message, unsigned int length) {
  String msg;
  for (int i = 0; i < length; i++) {
    msg += (char)message[i];
  }
  Serial.print("Pesan diterima di topik: ");
  Serial.println(topic);
  Serial.print("Isi pesan: ");
  Serial.println(msg);

  if (String(topic) == "controller/move/forward") {
    gerakMaju();
  } else if (String(topic) == "controller/move/backward") {
    gerakMundur();
  } else if (String(topic) == "controller/move/turn_right") {
    belokKanan();
  } else if (String(topic) == "controller/move/turn_left") {
    belokKiri();
  } else if (String(topic) == "controller/fire") {
    modeTembak();
  }

  lastMessageTime = millis();
  motorActive = true;
}

// Fungsi untuk mematikan motor
void matikanMotor() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
  digitalWrite(RELAY_PIN, HIGH);
  motorActive = false;
}

// Fungsi untuk menghubungkan ke broker MQTT
void reconnect() {
  while (!client.connected()) {
    Serial.println("Menghubungkan ke MQTT...");
    if (client.connect("ESP32Client")) {
      Serial.println("Terhubung ke MQTT!");
      client.subscribe("controller/move/forward");
      client.subscribe("controller/move/backward");
      client.subscribe("controller/move/turn_right");
      client.subscribe("controller/move/turn_left");
      client.subscribe("controller/fire");
    } else {
      Serial.print("Gagal terhubung. Status: ");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

// Fungsi untuk mode tembak
void modeTembak() {
  Serial.println("Mode tembak diaktifkan!");
  digitalWrite(RELAY_PIN, LOW);
  delay(250);
  digitalWrite(RELAY_PIN, HIGH);
  Serial.println("Mode tembak selesai!");
}

// Fungsi gerakan motor
void gerakMaju() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, motorSpeed);
  analogWrite(ENB, motorSpeed);
}

void gerakMundur() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENA, motorSpeed);
  analogWrite(ENB, motorSpeed);
}

void belokKanan() {
    digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, motorSpeed);
  analogWrite(ENB, motorSpeed);
 
}

void belokKiri() {
 digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENA, motorSpeed);
  analogWrite(ENB, motorSpeed);
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  dht.begin();
  pinMode(PIEZOPIN, INPUT);
  digitalWrite(RELAY_PIN, HIGH);

  Serial.println("Sistem siap!");
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();  // Membaca kelembapan
    int piezoValue = analogRead(PIEZOPIN);
    unsigned long currentMillis = millis();  // Waktu sekarang

    // Mengirim data suhu dan kelembapan setiap interval tertentu
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;  // Perbarui waktu sebelumnya
        if (!isnan(temperature)) {
            Serial.print("Suhu: ");
            Serial.print(temperature);
            Serial.println(" °C");

            // Konversi nilai suhu ke string
            char tempString[8];  // Buffer untuk menyimpan string
            dtostrf(temperature, 6, 2, tempString);  // Konversi float ke string
            client.publish("controller/temp", tempString);
        } else {
            Serial.println("Gagal membaca data dari sensor DHT!");
        }

        if (!isnan(humidity)) {
            Serial.print("Kelembapan: ");
            Serial.print(humidity);
            Serial.println(" %");

            // Konversi nilai kelembapan ke string
            char humString[8];  // Buffer untuk menyimpan string
            dtostrf(humidity, 6, 2, humString);  // Konversi float ke string
            client.publish("controller/humidity", humString);
        } else {
            Serial.println("Gagal membaca data kelembapan dari sensor DHT!");
        }

        Serial.println("Timer triggered!");  // Aksi dilakukan
    }

    // Logika untuk pembacaan nilai piezo
    if (piezoValue > 1500) {
        // Pastikan hanya mengirim sekali untuk satu deteksi
        if (!vibrationDetected) {
            vibrationDetected = true;  // Tandai getaran terdeteksi
            Serial.print("Getaran terdeteksi! Nilai Piezo: ");
            Serial.println(piezoValue);
            client.publish("controller/hited", "Hited");
        }
    } else {
        vibrationDetected = false;  // Reset jika tidak ada getaran
        Serial.println("Tidak ada getaran yang terdeteksi.");
    }

    // Logika timeout untuk motor
    if (motorActive && currentMillis - lastMessageTime > timeout) {
        Serial.println("Timeout tercapai, mematikan motor...");
        matikanMotor();
    }
}
