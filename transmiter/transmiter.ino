#include <SPI.h>        //SPI library for communicate with the nRF24L01+
#include <RF24.h>       //The main library of the nRF24L01+
#include <Wire.h>       //For communicate
#include <MPU6050.h>    //The main library of the MPU6050
#include <I2Cdev.h>     //For communicate with MPU6050

// Define the object to access and control the Gyro and Accelerometer
MPU6050 mpu;
int16_t ax, ay, az;
int16_t gx, gy, gz;

// Variables for smoothed acceleration data
float smoothed_ax = 0, smoothed_ay = 0;
const float alpha = 0.2; // Smoothing factor (0 < alpha < 1, lower means more smoothing)

// Define packet for the direction (X axis and Y axis)
int data[2];

// Define object from RF24 library - 8 and 9 are digital pin numbers to which signals CE and CSN are connected
RF24 radio(8,9);

// Create a pipe address for the communication
const uint64_t pipe = 0xE8E8F0F0E1LL;

// To slow down the rate of serial print out
int delay_time = 100;

void setup(void) {
  Serial.begin(9600);
  Wire.begin();                  // Start I2C for MPU6050
  mpu.initialize();              // Initialize the MPU object
  radio.begin();                 // Start the nRF24 communication
  radio.openWritingPipe(pipe);   // Sets the address of the receiver to which the program will send data
}

void loop(void) {
  // Get acceleration and gyro values
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  // Apply exponential moving average to smooth acceleration data
  smoothed_ax = (alpha * ax) + ((1 - alpha) * smoothed_ax);
  smoothed_ay = (alpha * ay) + ((1 - alpha) * smoothed_ay);

  // Map smoothed data to usable ranges for the receiver
  data[0] = map(smoothed_ax, -17000, 17000, 300, 400); // Send X axis data (forward/backward)
  data[1] = map(smoothed_ay, -17000, 17000, 100, 200); // Send Y axis data (left/right)

  // Send the 2-element int array over the air
  radio.write(data, sizeof(data));

  // Serial output for debugging
  Serial.print("X axis data = ");
  Serial.println(data[0]);
  delay(delay_time);

  if (data[0] < 340) {
    Serial.println("forward");
    Serial.println();
    delay(delay_time);
  }
  if (data[0] > 360) {
    Serial.println("backward");
    Serial.println();
    delay(delay_time);
  }

  Serial.print("Y axis data = ");
  Serial.println(data[1]);
  delay(delay_time);

  if (data[1] > 160) {
    Serial.println("right");
    Serial.println();
    delay(delay_time);
  }
  if (data[1] < 140) {
    Serial.println("left");
    Serial.println();
    delay(delay_time);
  }

  if (data[0] > 340 && data[0] < 360 && data[1] > 140 && data[1] < 160) {
    Serial.println("stop");
    Serial.println();
    delay(delay_time);
  }
}