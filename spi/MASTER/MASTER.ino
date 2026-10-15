#include <SPI.h>

const int buttonPin = A0;   
const int slaveSelect = 10;

void setup() {
  pinMode(buttonPin, INPUT_PULLUP); 
  pinMode(slaveSelect, OUTPUT);
  digitalWrite(slaveSelect, HIGH);

  SPI.begin();  // Initialize SPI as master
}

void loop() {
  int buttonState = digitalRead(buttonPin);

  digitalWrite(slaveSelect, LOW);  

  if (buttonState == LOW) { 
    SPI.transfer(1); 
  } else {
    SPI.transfer(0); 
  }

  digitalWrite(slaveSelect, HIGH);
  delay(100);
}
