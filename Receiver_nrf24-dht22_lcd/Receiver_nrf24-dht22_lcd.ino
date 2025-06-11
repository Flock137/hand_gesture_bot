#include <SPI.h>
#include "RF24.h"
#include <LiquidCrystal_I2C.h>
#include "DHT.h"

// ==== Pin Definitions ====
const int enbA = 3;
const int enbB = 6;
const int IN1 = 2;    //Right Motor (-)
const int IN2 = 4;    //Right Motor (+)
const int IN3 = 5;    //Left Motor (+)
const int IN4 = 7;    //Right Motor (-)

#define DHTPIN 10      // Moved DHT22 to pin 10 to avoid conflicts
#define DHTTYPE DHT22

// ==== LCD & DHT ====
LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(DHTPIN, DHTTYPE);

// ==== nRF24 ====
RF24 radio(8, 9);    // CE, CSN
const uint64_t pipe = 0xE8E8F0F0E1LL;
int data[2];

// ==== Other Vars ====
int RightSpd = 250;
int LeftSpd = 250;
String direction = "stop";  // For displaying on LCD

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
  Serial.begin(9600);

  // nRF24L01 setup
  radio.begin();
  radio.openReadingPipe(1, pipe);
  radio.startListening();

  // DHT & LCD setup
  dht.begin();
  lcd.init();
  lcd.backlight();
}
// Next time, limit the data array from 0 -> 20000
// if sastisfied, continue, else -> stop

void loop() {
  // ---- Handle RF24 car control ----
  if (radio.available()) {
    radio.read(data, sizeof(data));

    if (data[0] < 340) {
      // forward
      moveForward();
      direction = "Forward";
      Serial.println(data[0]);
    }
    

    else if (data[0] > 360) {
      // backward
      moveBackward();
      direction = "Backward";
      Serial.println(data[0]);
    }

    else if (data[1] < 140) {
      turnRight(); //Left
      direction = "Left";
      Serial.println(data[1]);

    }

    else if (data[1] > 160) {
      turnLeft(); //Right
      direction = "Right";
      Serial.println(data[1]);

    }

    else if (data[0] > 340 && data[0] < 360 && data[1] > 140 && data[1] < 160 || data[0] == 0 || data[1] == 0 ) {
      stopCar();
      direction = "Stop";
      Serial.println(data[0]);
    }

    else if (data[0] < 0 || data[0] > 20000 || data[1] < 0 || data[1] > 20000){
      stopCar();
      direction = "Stop";
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
      lcd.print("C ");

      lcd.setCursor(0, 1);
      lcd.print("Humi: ");
      lcd.print(humi, 0);
      lcd.print("%");

      //lcd.setCursor(0, 1);
      //lcd.print("Dir: ");
      //lcd.print(direction);
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
void turnRight() {
  analogWrite(enbA, RightSpd);
  analogWrite(enbB, LeftSpd);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}
void turnLeft() {
  analogWrite(enbA, RightSpd);
  analogWrite(enbB, LeftSpd);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}
void stopCar() {
  analogWrite(enbA, 0);
  analogWrite(enbB, 0);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}
