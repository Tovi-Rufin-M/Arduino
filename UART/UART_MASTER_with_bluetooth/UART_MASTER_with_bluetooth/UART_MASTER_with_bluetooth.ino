#include <SoftwareSerial.h>
#include <ArduinoBLE.h>

// RX, TX for Slave UART
SoftwareSerial mySerial(2, 3); // (RX=2, TX=3)

// Define button pins
const int button1 = 4;
const int button2 = 5;
const int button3 = 6;
const int button4 = 7;
const int button5 = 8;
const int button6 = 9;

// BLE Service & Characteristic
BLEService controlService("180C"); // custom service UUID
BLEStringCharacteristic commandChar("2A56", BLEWrite, 20); // writable characteristic

// Mode: 0 = button control, 1 = Bluetooth control
int controlMode = 0;

void setup() {
  // UART with Slave
  mySerial.begin(9600);

  // Configure buttons as inputs with internal pull-up resistors
  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);
  pinMode(button3, INPUT_PULLUP);
  pinMode(button4, INPUT_PULLUP);
  pinMode(button5, INPUT_PULLUP);
  pinMode(button6, INPUT_PULLUP);

  // Start BLE
  if (!BLE.begin()) {
    // If BLE fails, stay in button mode
    while (1);
  }

  BLE.setLocalName("ArduinoR4_Master");
  BLE.setAdvertisedService(controlService);

  // Add characteristic to service
  controlService.addCharacteristic(commandChar);
  BLE.addService(controlService);

  // Start advertising
  BLE.advertise();
}

void loop() {
  // Listen for BLE connections
  BLEDevice central = BLE.central();

  if (central) {
    controlMode = 1; // Bluetooth mode active when phone connects

    // While central device is connected
    while (central.connected()) {
      BLE.poll();

      if (commandChar.written()) {
        String command = commandChar.value();
        command.trim();
        mySerial.println(command); // forward to Slave
        delay(200);
      }
    }

    controlMode = 0; // back to button mode after disconnect
  }

  // If no Bluetooth, use buttons
  if (controlMode == 0) {
    if (digitalRead(button1) == LOW) {
      mySerial.println("BTN1");
      delay(200);
    }
    else if (digitalRead(button2) == LOW) {
      mySerial.println("BTN2");
      delay(200);
    }
    else if (digitalRead(button3) == LOW) {
      mySerial.println("BTN3");
      delay(200);
    }
    else if (digitalRead(button4) == LOW) {
      mySerial.println("BTN4");
      delay(200);
    }
    else if (digitalRead(button5) == LOW) {
      mySerial.println("BTN5");
      delay(200);
    }
    else if (digitalRead(button6) == LOW) {
      mySerial.println("BTN6");
      delay(200);
    }
  }
}
