
#include <SPI.h>        //SPI library for communicate with the nRF24L01+
#include <RF24.h>       //The main library of the nRF24L01+
#include <Wire.h>       //For communicate
#include <MPU6050.h>    //The main library of the MPU6050
#include <I2Cdev.h>     //For communicate with MPU6050

// Personal port: COM5
// Please adjust it according to what your Device Manager said

//Define the object to access and cotrol the Gyro and Accelerometer (We don't use the Gyro data)
MPU6050 mpu;
int16_t ax, ay, az;
int16_t gx, gy, gz;

//Define packet for the direction (X axis and Y axis)
int data[2];

//Define object from RF24 library - 8 and 9 are a digital pin numbers to which signals CE and CSN are connected.
RF24 radio(8,9);

//Create a pipe addresses for the communicate                                    
const uint64_t pipe = 0xE8E8F0F0E1LL;

//To slow down the rate of serial print out 
int delay_time = 100; 


void setup(void){
  Serial.begin(9600);
  Wire.begin();                  //Start I2C for MPU6050 
  mpu.initialize();              //Initialize the MPU object
  radio.begin();                 //Start the nRF24 communicate     
  radio.openWritingPipe(pipe);   //Sets the address of the receiver to which the program will send data.
}

void loop(void){
  
  //With this function, the acceleration and gyro values of the axes are taken. 
  //If you want to control the car axis differently, you can change the axis name in the map command.
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  //In two-way control, the X axis (data [0]) of the MPU6050 allows the robot to move forward and backward. 
  //Y axis (data [1]) allows the robot to right and left turn.
  //Converts raw ±16 g readings into two integers in ranges usable by the receiver
  data[0] = map(ax, -17000, 17000, 300, 400 ); //Send X axis data 
  data[1] = map(ay, -17000, 17000, 100, 200);  //Send Y axis data
  //Sends the 2-element int array over the air every loop, and prints it over Serial.
  radio.write(data, sizeof(data));

  Serial.print("X axis data = ");
  Serial.println(data[0]);
  delay(delay_time);

  if(data[0] < 340){
    Serial.println("forward");
    Serial.println();
    delay(delay_time);
  }
  if(data[0] > 360){
    Serial.println("backward");
    Serial.println();
    delay(delay_time);
  }


  Serial.print("Y axis data = ");
  Serial.println(data[1]);
  delay(delay_time);

  if(data[1] > 160){
    Serial.println("right");
    Serial.println();
    delay(delay_time);
  }
  if(data[1] < 140){
    Serial.println("left");
    Serial.println();
    delay(delay_time);

  }

  if(data[0] > 340 && data[0] < 360 && data[1] > 140 && data[1] < 160){
    Serial.println("stop");
    Serial.println();
    delay(delay_time);

  }

}