#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <ESP32Servo.h>

RF24 radio(4, 5); // CE, CSN
const byte address[6] = "PANCH";
Servo panServo, tiltServo;

void setup() {
  Serial.begin(115200);
  panServo.attach(17);   // Pan servo to GPIO 17
  tiltServo.attach(15);  // Tilt servo to GPIO 15
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_LOW);
  radio.startListening();
}

void loop() {
  if (radio.available()) {
    int payload[2] = {90, 90};
    radio.read(&payload, sizeof(payload));
    panServo.write(payload[0]*0.3);
    tiltServo.write(payload[1]*0.3);
    Serial.print("Pan: "); Serial.print(payload[0]);
    Serial.print("  Tilt: "); Serial.println(payload[1]);
  }
}
