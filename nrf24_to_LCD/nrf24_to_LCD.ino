#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <LiquidCrystal_I2C.h>

//com5

// ——— PIN DEFINITIONS ———
#define CE_PIN   8
#define CSN_PIN  9

// I²C LCD at 0x27, size 16×2
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ——— RADIO SETUP ———
RF24 radio(CE_PIN, CSN_PIN);
const byte address[6] = "00001";

// ——— PAYLOAD STRUCT ———
struct SensorData {
  float temperature;
  float humidity;
};

void setup() {
  Serial.begin(9600);

  // —— LCD init —— 
  lcd.init();           // required for LiquidCrystal_I2C
  lcd.backlight();      // turn on backlight
  lcd.clear();
  lcd.print("Starting up");

  // —— Radio init + sanity check —— 
  if (!radio.begin()) {
    Serial.println(F("RF24 init failed"));
    lcd.clear();
    lcd.print("Radio init error");
    while (1);          // halt here
  }
  Serial.print(F("Chip OK? "));
  Serial.println(radio.isChipConnected() ? F("Yes") : F("No"));

  // —— Radio settings —— 
  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_1MBPS);
  radio.setChannel(76);
  radio.openReadingPipe(0, address);
  radio.startListening();    // put into RX mode

  lcd.clear();
  lcd.print("Waiting for data");
}

void loop() {
  if (radio.available()) {
    SensorData data;
    radio.read(&data, sizeof(data));

    // —— display on LCD —— 
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(data.temperature, 1);
    lcd.print(" C");

    lcd.setCursor(0, 1);
    lcd.print("Hum:  ");
    lcd.print(data.humidity, 1);
    lcd.print(" %");

    // —— serial log —— 
    Serial.print(F("Got -> "));
    Serial.print(data.temperature, 1);
    Serial.print(F("C, "));
    Serial.print(data.humidity, 1);
    Serial.println(F("%"));
  }
  delay(500);
}
