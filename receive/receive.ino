#include <SPI.h>
#include "RF24.h"
#include <LiquidCrystal_I2C.h>
#include "DHT.h"

// ==== Pin Definitions ====
const int enbA = 6; // Right motor PWM
const int enbB = 3; // Left motor PWM
const int IN1 = 4;  // Right Motor (-)
const int IN2 = 2;  // Right Motor (+)
const int IN3 = 7;  // Left Motor (+)
const int IN4 = 5;  // Left Motor (-)

#define DHTPIN 10      // DHT22 pin
#define DHTTYPE DHT22

// ==== LCD & DHT ====
LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(DHTPIN, DHTTYPE);

// ==== nRF24 ====
RF24 radio(8, 9);    // CE, CSN
const uint64_t pipe = 0xE8E8F0F0E1LL;
int data[2];

// ==== Other Vars ====
int RightSpd = 235;
int LeftSpd = 200;
String direction = "Stop";  // For displaying on LCD

// Timing for DHT/LCD update
unsigned long lastSensorUpdate = 0;
const unsigned long sensorInterval = 2000;  // 2 seconds

void setup() {
  // Motor pins
  pinMode(enbA, OUTPUT);
  pinMode(enbB, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Serial for debugging
  Serial.begin(115200);

  // nRF24L01 setup
  radio.begin();
  radio.openReadingPipe(1, pipe);
  radio.startListening();

  // DHT & LCD setup
  dht.begin();
  lcd.init();
  lcd.backlight();
}

void loop() {
  // ---- Handle RF24 car control ----
  if (radio.available()) {
    radio.read(data, sizeof(data));

    // Check for invalid data ranges (based on transmitter's mapping)
    if (data[0] < 300 || data[0] > 400 || data[1] < 100 || data[1] > 200) {
      stopCar();
      direction = "Stop";
      Serial.println("Invalid data");
    }
    else if (data[0] < 340) {
      moveForward();
      direction = "Forward";
      Serial.println(data[0]);
    }
    else if (data[0] > 360) {
      moveBackward();
      direction = "Backward";
      Serial.println(data[0]);
    }
    else if (data[1] < 140) {
      turnRight(); // Corrected to right
      direction = "Right";
      Serial.println(data[1]);

    }
    else if (data[1] > 160) {
      turnLeft(); // Corrected to left
      direction = "Left";
      Serial.println(data[1]);
    }
    else {
      stopCar();
      direction = "Stop";
      Serial.println(data[0]);
    }

    // Print for debug
    Serial.println(direction);
  }

  // ---- Handle DHT22 and LCD update every 2 seconds ----
  unsigned long now = millis();
  if (now - lastSensorUpdate > sensorInterval) {
    lastSensorUpdate = now;
    float humi = dht.readHumidity();
    float tempC = dht.readTemperature();

    lcd.clear();
    if (isnan(humi) || isnan(tempC)) {
      lcd.setCursor(0, 0);
      lcd.print("Sensor Fail");
      lcd.setCursor(0, 1);
      lcd.print("Dir: ");
      lcd.print(direction);
    } else {
      lcd.setCursor(0, 0);
      lcd.print("Temp: ");
      lcd.print(tempC, 1);
      lcd.print((char)223);
      lcd.print("C");

      lcd.setCursor(0, 1);
      lcd.print("Humi: ");
      lcd.print(humi, 0);
      lcd.print("% ");
      lcd.print("Dir: ");
      lcd.print(direction);
    }
  }
}

// ==== Motor Functions ====
void moveForward() {
  analogWrite(enbA, RightSpd);
  analogWrite(enbB, LeftSpd);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void moveBackward() {
  analogWrite(enbA, RightSpd);
  analogWrite(enbB, LeftSpd);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void turnLeft() {
  analogWrite(enbA, RightSpd);
  analogWrite(enbB, LeftSpd);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void turnRight() {
  analogWrite(enbA, RightSpd);
  analogWrite(enbB, LeftSpd);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void stopCar() {
  analogWrite(enbA, 0);
  analogWrite(enbB, 0);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}