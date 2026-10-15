#include <SoftwareSerial.h>
#include <Wire.h>
#include <SPI.h>

// ==================== CONFIG ====================
#define SLAVE_ID       1        // Set to 1 or 2 depending on this board
#define I2C_ADDR_SLAVE1 0x08
#define I2C_ADDR_SLAVE2 0x09

// UART SoftwareSerial pins (set these to match wiring to master):
// Master used SoftwareSerial mySerial(2,3) where RX=2, TX=3.
// To pair: master TX(3) -> slave RX(3), master RX(2) <- slave TX(2)
const uint8_t UART_RX_PIN = 3;
const uint8_t UART_TX_PIN = 2;
SoftwareSerial mySerial(UART_RX_PIN, UART_TX_PIN);

// Outputs (LEDs) used by patterns. By default only use builtin LED for testing.
#define NUM_OUTPUTS 5
const int outputPins[NUM_OUTPUTS] = { 4,5,6,7,8 }; // change to {2,3,4,5,...} if you have multiple LEDs

// SPI buffer size
#define SPI_BUFFER_SIZE 64
// Generic buffers
#define CMD_BUFFER_SIZE 128

// ==================== STATE & BUFFERS ====================
volatile char spiBuf[SPI_BUFFER_SIZE];
volatile uint8_t spiIdx = 0;
volatile bool spiCmdReady = false;

char i2cBuf[CMD_BUFFER_SIZE];
volatile uint8_t i2cIdx = 0;
volatile bool i2cCmdReady = false;

char uartBuf[CMD_BUFFER_SIZE];
uint8_t uartIdx = 0;
bool uartCmdReady = false;

char lastCommand[CMD_BUFFER_SIZE] = {0};

// ==================== PROTOTYPES ====================
void processCommand(const char *cmd);
void handleUART();
void i2cReceive(int howMany);
void spiInitSlave();
ISR (SPI_STC_vect);

// Simple pattern functions
void pattern_running(int repeat=2);
void pattern_chase(int repeat=2);
void pattern_flash(int repeat=4);
void pattern_alternate(int repeat=4);
void pattern_running_lights(int repeat=3);
void pattern_wave(int repeat=3);
void pattern_blink_slow(int repeat=4);
void pattern_blink_fast(int repeat=8);

// ==================== SETUP ====================
void setup() {
  Serial.begin(115200); // debug to Serial Monitor
  while (!Serial) { ; }

  Serial.print("Slave starting. ID=");
  Serial.println(SLAVE_ID);

  // Setup outputs
  for (int i = 0; i < NUM_OUTPUTS; ++i) {
    pinMode(outputPins[i], OUTPUT);
    digitalWrite(outputPins[i], LOW);
  }

  // UART (SoftwareSerial)
  mySerial.begin(9600);
  Serial.println("SoftwareSerial started on RX=" + String(UART_RX_PIN) + " TX=" + String(UART_TX_PIN));

  // I2C (Wire) as slave
  uint8_t addr = (SLAVE_ID == 1) ? I2C_ADDR_SLAVE1 : I2C_ADDR_SLAVE2;
  Wire.begin(addr);
  Wire.onReceive(i2cReceive);
  Serial.print("I2C slave started at 0x");
  Serial.println(addr, HEX);

  // SPI as slave
  spiInitSlave();
  Serial.println("SPI slave initialized.");

  // Welcome blink
  digitalWrite(outputPins[0], HIGH);
  delay(80);
  digitalWrite(outputPins[0], LOW);
  delay(80);
  digitalWrite(outputPins[0], HIGH);
  delay(80);
  digitalWrite(outputPins[0], LOW);
}

// ==================== MAIN LOOP ====================
void loop() {
  // Handle SoftwareSerial input
  handleUART();

  // Check flags set by ISR / handlers
  if (spiCmdReady) {
    noInterrupts();
    // copy volatile buffer
    char tmp[SPI_BUFFER_SIZE];
    uint8_t n = (spiIdx < SPI_BUFFER_SIZE) ? spiIdx : SPI_BUFFER_SIZE-1;
    for (uint8_t i = 0; i < n; ++i) tmp[i] = spiBuf[i];
    tmp[n] = '\0';
    // reset
    spiIdx = 0;
    spiCmdReady = false;
    interrupts();

    Serial.print("SPI cmd: ");
    Serial.println(tmp);
    processCommand(tmp);
  }

  if (i2cCmdReady) {
    // i2cBuf is filled & null-terminated by handler
    i2cCmdReady = false;
    Serial.print("I2C cmd: ");
    Serial.println(i2cBuf);
    processCommand(i2cBuf);
    i2cIdx = 0; // reset buffer index
    i2cBuf[0] = '\0';
  }

  if (uartCmdReady) {
    uartCmdReady = false;
    Serial.print("UART cmd: ");
    Serial.println(uartBuf);
    // Master prints "SLV<id>:<cmd>" over UART; only respond if targeted to this slave
    // Example: "SLV1:running"
    if (strncmp(uartBuf, "SLV", 3) == 0 && isdigit(uartBuf[3]) ) {
      int target = uartBuf[3] - '0';
      if (target == SLAVE_ID && uartBuf[4] == ':') {
        processCommand(uartBuf + 5);
      } else {
        Serial.println("UART message not for this slave.");
      }
    } else {
      // also allow raw commands (in case master sends plain commands over UART)
      processCommand(uartBuf);
    }
    uartIdx = 0;
    uartBuf[0] = '\0';
  }

  // small idle delay (patterns use delays internally)
  delay(2);
}

// ==================== UART handling ====================
void handleUART() {
  while (mySerial.available()) {
    char c = mySerial.read();
    if (c == '\r') continue;
    if (c == '\n' || uartIdx >= CMD_BUFFER_SIZE-1) {
      uartBuf[uartIdx] = '\0';
      uartCmdReady = true;
      return;
    } else {
      uartBuf[uartIdx++] = c;
      uartBuf[uartIdx] = '\0';
    }
  }
}

// ==================== I2C receive handler ====================
void i2cReceive(int howMany) {
  // Called in ISR context: keep brief
  while (Wire.available() && i2cIdx < CMD_BUFFER_SIZE - 1) {
    char c = (char)Wire.read();
    if (c == '\0' || c == '\n') {
      i2cBuf[i2cIdx] = '\0';
      i2cCmdReady = true;
      i2cIdx = 0;
      return;
    } else {
      i2cBuf[i2cIdx++] = c;
    }
  }
  // Terminate if buffer full
  if (i2cIdx >= CMD_BUFFER_SIZE - 1) {
    i2cBuf[i2cIdx] = '\0';
    i2cCmdReady = true;
    i2cIdx = 0;
  }
}

// ==================== SPI slave init & ISR ====================
void spiInitSlave() {
  // Set MISO as output
  pinMode(MISO, OUTPUT);
  // Enable SPI in slave mode
  SPCR |= _BV(SPE);
  // Enable SPI interrupt
  SPCR |= _BV(SPIE);
  // Make sure index/reset
  spiIdx = 0;
  spiCmdReady = false;
  // attachInterrupt not used; ISR defined below
}

// SPI Transfer Complete ISR -> used to collect bytes coming from master
ISR (SPI_STC_vect) {
  char c = SPDR; // received byte
  // store until NUL terminator or buffer full
  if (spiIdx < SPI_BUFFER_SIZE - 1) {
    spiBuf[spiIdx++] = c;
    if (c == '\0' || c == '\n') {
      spiBuf[spiIdx - 1] = '\0'; // ensure null-terminated
      spiCmdReady = true;
      // don't reset spiIdx here because main copies and resets
    }
  } else {
    // buffer full - null terminate and flag ready
    spiBuf[SPI_BUFFER_SIZE - 1] = '\0';
    spiCmdReady = true;
  }
}

// ==================== COMMAND PROCESSING ====================
void processCommand(const char *cmd) {
  if (cmd == NULL || strlen(cmd) == 0) return;
  // copy to lastCommand
  strncpy(lastCommand, cmd, CMD_BUFFER_SIZE-1);
  lastCommand[CMD_BUFFER_SIZE-1] = '\0';

  // normalize to lowercase for comparisons
  String s = String(cmd);
  s.trim();
  s.toLowerCase();

  Serial.print("Processing command: ");
  Serial.println(s);

  if (s.indexOf("running") >= 0 && s.indexOf("lights") >= 0) {
    pattern_running_lights();
  } else if (s.indexOf("running") >= 0) {
    pattern_running();
  } else if (s.indexOf("chase") >= 0) {
    pattern_chase();
  } else if (s.indexOf("flash") >= 0) {
    pattern_flash();
  } else if (s.indexOf("alternate") >= 0) {
    pattern_alternate();
  } else if (s.indexOf("wave") >= 0) {
    pattern_wave();
  } else if (s.indexOf("blink slow") >= 0) {
    pattern_blink_slow();
  } else if (s.indexOf("blink fast") >= 0) {
    pattern_blink_fast();
  } else {
    // unknown command: blink BUILTIN quickly 3x
    for (int i = 0; i < 3; ++i) {
      digitalWrite(outputPins[0], HIGH);
      delay(80);
      digitalWrite(outputPins[0], LOW);
      delay(80);
    }
    Serial.println("Unknown command: " + s);
  }
}

// ==================== PATTERN IMPLEMENTATIONS ====================
// These patterns are intentionally simple and safe for 1..N outputs.
// If NUM_OUTPUTS==1 they collapse to simple blink variants.

void clearAllOutputs() {
  for (int i = 0; i < NUM_OUTPUTS; ++i) digitalWrite(outputPins[i], LOW);
}

void pattern_running(int repeat) {
  if (NUM_OUTPUTS == 1) {
    while (repeat--) {
      digitalWrite(outputPins[0], HIGH);
      delay(120);
      digitalWrite(outputPins[0], LOW);
      delay(60);
    }
  } else {
    for (int r = 0; r < repeat; ++r) {
      for (int i = 0; i < NUM_OUTPUTS; ++i) {
        clearAllOutputs();
        digitalWrite(outputPins[i], HIGH);
        delay(100);
      }
    }
    clearAllOutputs();
  }
}

void pattern_chase(int repeat) {
  for (int r = 0; r < repeat; ++r) {
    for (int i = 0; i < NUM_OUTPUTS; ++i) {
      digitalWrite(outputPins[i], HIGH);
      delay(70);
      digitalWrite(outputPins[i], LOW);
    }
  }
}

void pattern_flash(int repeat) {
  for (int r = 0; r < repeat; ++r) {
    for (int i = 0; i < NUM_OUTPUTS; ++i) {
      digitalWrite(outputPins[i], HIGH);
      delay(200);
      digitalWrite(outputPins[i], LOW);
      delay(200);
    }
  }
}

void pattern_alternate(int repeat) {
  for (int r = 0; r < repeat; ++r) {
    for (int i = 0; i < NUM_OUTPUTS; ++i) {
      digitalWrite(outputPins[i], (i % 2 == 0) ? HIGH : LOW);
    }
    delay(200);
    for (int i = 0; i < NUM_OUTPUTS; ++i) {
      digitalWrite(outputPins[i], (i % 2 == 0) ? LOW : HIGH);
    }
    delay(200);
  }
  clearAllOutputs();
}

void pattern_running_lights(int repeat) {
  for (int r = 0; r < repeat; ++r) {
    for (int i = 0; i < NUM_OUTPUTS; ++i) {
      digitalWrite(outputPins[i], HIGH);
      delay(80);
      digitalWrite(outputPins[i], LOW);
    }
  }
  clearAllOutputs();
}

void pattern_wave(int repeat) {
  for (int r = 0; r < repeat; ++r) {
    for (int offset = 0; offset < NUM_OUTPUTS; ++offset) {
      for (int i = 0; i < NUM_OUTPUTS; ++i) {
        int pos = (i + offset) % NUM_OUTPUTS;
        digitalWrite(outputPins[pos], HIGH);
        delay(200);
        digitalWrite(outputPins[pos], LOW);
      }
      // small pause between wave steps
      delay(70);
    }
  }
  // ensure all outputs are off at the end
  clearAllOutputs();
}


void pattern_blink_slow(int repeat) {
  for (int r = 0; r < repeat; ++r) {
    for (int offset = 0; offset < NUM_OUTPUTS; ++offset) {
      digitalWrite(outputPins[offset], HIGH);
      delay(400);
      digitalWrite(outputPins[offset], LOW);
      delay(400);
    }
  }
}

void pattern_blink_fast(int repeat) {
  for (int r = 0; r < repeat; ++r) {
    for (int offset = 0; offset < NUM_OUTPUTS; ++offset) {
      digitalWrite(outputPins[offset], HIGH);
      delay(200);
      digitalWrite(outputPins[offset], LOW);
      delay(200);
    }
  }
}
void seq_random() {
  int pick = random(0, NUM_OUTPUTS);
  digitalWrite(outputPins[pick], HIGH);
  delay(100);
  digitalWrite(outputPins[pick], LOW);
  delay(100);
}
void seq_pingpong() {
  for (int i = 0; i < NUM_OUTPUTS; i++) {
    digitalWrite(outputPins[i], HIGH);
    delay(150);
    digitalWrite(outputPins[i], LOW);
    delay(150);
  }
  for (int i = NUM_OUTPUTS - 2; i > 0; i--) {  // bounce back
    digitalWrite(outputPins[i], HIGH);
    delay(150);
    digitalWrite(outputPins[i], LOW);
    delay(150);
  }
}
void seq_glow() {
  for (int b = 0; b < 255; b += 15) {   // gradually brighten
    for (int i = 0; i < NUM_OUTPUTS; i++) {
      analogWrite(outputPins[i], b);
    }
    delay(50);
  }
  for (int b = 255; b >= 0; b -= 15) {  // gradually dim
    for (int i = 0; i < NUM_OUTPUTS; i++) {
      analogWrite(outputPins[i], b);
    }
    delay(50);
  }
  delay(50);
}