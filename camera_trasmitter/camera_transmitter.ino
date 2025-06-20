#include <Wire.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "MPU6050.h"

MPU6050 mpu;
RF24 radio(4, 5); // CE, CSN
const byte address[6] = "PANCH";

// Smoothing
#define SMOOTH_N 10
float bufferX[SMOOTH_N] = {0};
float bufferY[SMOOTH_N] = {0};
int idx = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  mpu.initialize();
  if (!mpu.testConnection()) {
    Serial.println("MPU6050 not found!");
    while (1);
  }
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_LOW);
  radio.stopListening();
}

void loop() {
  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);

  // Calculate X/Y tilt (degrees)
  float angle_x = atan2((float)ax, (float)az) * 180.0 / PI;
  float angle_y = atan2((float)ay, (float)az) * 180.0 / PI;

  // Moving averages
  bufferX[idx] = angle_x;
  bufferY[idx] = angle_y;
  idx = (idx + 1) % SMOOTH_N;
  float sumX = 0, sumY = 0;
  for (int i = 0; i < SMOOTH_N; i++) {
    sumX += bufferX[i];
    sumY += bufferY[i];
  }
  float smoothX = sumX / SMOOTH_N;
  float smoothY = sumY / SMOOTH_N;

  // Reverse mapping here!
  int tiltValue = constrain(map((int)smoothX, -30, 30, 180, 0), 0, 180); // X axis for tilt (reversed)
  int panValue  = constrain(map((int)smoothY, -30, 30, 180, 0), 0, 180); // Y axis for pan (reversed)
  
  if (abs(smoothX) < 3) tiltValue = 90;
  if (abs(smoothY) < 3) panValue = 90;

  int payload[2] = {panValue, tiltValue};
  radio.write(&payload, sizeof(payload));
  Serial.print("Pan: "); Serial.print(panValue);
  Serial.print("  Tilt: "); Serial.println(tiltValue);

  delay(10);
}
