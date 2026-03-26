#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#define SDA_PIN 21
#define SCL_PIN 22

MAX30105 particleSensor;

const char* ssid = "UA-Alumnos";
const char* password = "41umn05WLC";
const char* mqtt_server = "34.225.208.14";
const byte RATE_SIZE = 15;
byte rates[RATE_SIZE];
byte rateSpot = 0;
long lastBeat = 0;
float beatsPerMinute;
int beatAvg;
unsigned long lastMsgTime = 0;

WiFiClient espClient;
PubSubClient client(espClient);

void onBeatDetected() {
  Serial.println("¡Latido detectado!");
}

void setup() {
    
  Serial.begin(115200);
  delay(3000);
  Wire.begin(SDA_PIN, SCL_PIN);
  Serial.println("Escaneando I2C...");
  
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("Sensor no encontrado");
    while (1);
  }
  
  Serial.println("Conectadose al wifi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi conectado");

  client.setServer(mqtt_server, 1883);
  if (client.connect("ESP32")) {
    Serial.println("Conectado al MQTT");
  } else {
    Serial.println("MQTT fallo, codigo: " + String(client.state()));
  }

  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x0A);
  particleSensor.setPulseAmplitudeGreen(0);
  Serial.println("Sensor listo! Apoya el dedo...");
}

void loop() {

 long irValue = particleSensor.getIR();

  if (checkForBeat(irValue) == true) {
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20) {
      rates[rateSpot++] = (byte)beatsPerMinute;
      rateSpot %= RATE_SIZE;

      beatAvg = 0;
      for (byte x = 0; x < RATE_SIZE; x++) {
        beatAvg += rates[x];
      }
      beatAvg /= RATE_SIZE;
    }
  }

  if (millis() - lastMsgTime > 500) {
    lastMsgTime = millis();
     Serial.print("IR=");
     Serial.print("IR=");
    Serial.print(irValue);
    Serial.print(", BPM=");
    Serial.print(beatsPerMinute);
    Serial.print(", Avg BPM=");
    Serial.print(beatAvg);

    if (irValue < 50000) {
      Serial.print(" -- No hay dedo!");
    }
    Serial.println();
}
}