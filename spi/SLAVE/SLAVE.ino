#include <SPI.h>

volatile byte receivedData = 0;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(MISO, OUTPUT);   // Required for SPI slave
  SPCR |= _BV(SPE);        // Enable SPI in Slave mode
  SPI.attachInterrupt();   // Enable SPI interrupt
}

ISR(SPI_STC_vect) {
  receivedData = SPDR; // Get data from master
}

void loop() {
  if (receivedData == 1) {
    digitalWrite(LED_BUILTIN, HIGH); // Turn LED on
  } else {
    digitalWrite(LED_BUILTIN, LOW);  // Turn LED off
  }
}
