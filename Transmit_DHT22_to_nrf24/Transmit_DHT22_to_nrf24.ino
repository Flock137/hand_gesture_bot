#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "DHT.h"

//com13

// ——— PIN DEFINITIONS ———
#define DHTPIN      10          // DHT22 data pin
#define DHTTYPE     DHT22

#define CE_PIN      8          // nRF24 CE
#define CSN_PIN     9         // nRF24 CSN

// ——— LIBRARY INIT ———
DHT dht(DHTPIN, DHTTYPE);
RF24 radio(CE_PIN, CSN_PIN);

// ——— RADIO ADDRESS ———
const byte address[6] = "00001";

// ——— SHARED PAYLOAD STRUCT ———
struct SensorData {
  float temperature;
  float humidity;
};

void setup() {
  Serial.begin(9600);
  dht.begin();

  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.openWritingPipe(address);
  radio.stopListening();           // transmitter mode
}

void loop() {
  // read sensor
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (isnan(h) || isnan(t)) {
    Serial.println("DHT read failed!");
  } else {
    // package and send
    SensorData data = { t, h };
    bool ok = radio.write(&data, sizeof(data));
    //Serial.print(ok);
    if (ok) {
      Serial.print("Sent: ");
      Serial.print(t); Serial.print("°C, ");
      Serial.print(h); Serial.println("%");
    } else {
      Serial.println("Send failed");
    }
  }
  delay(2000);
}
