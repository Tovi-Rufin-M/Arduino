#include <SPI.h>

const int led = 7;

volatile bool blinkFlag = false;

void setup() {
  pinMode(led, OUTPUT);
  SPI.begin();
  SPCR |= _BV(SPE);  // Enable SPI in slave mode
  SPI.attachInterrupt(); // Enable interrupt
}

ISR(SPI_STC_vect) {
  char c = SPDR; // Read received data
  if (c == 'B') blinkFlag = true;
}

void loop() {
  if (blinkFlag) {
    digitalWrite(led, HIGH);
    delay(500);
    digitalWrite(led, LOW);
    delay(500);
    blinkFlag = false;
  }
}
