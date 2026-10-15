// temp_arduino_handshake.ino
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ---------------- CONFIG ----------------
#define DHTPIN    13
#define DHTTYPE   DHT11
#define RELAY_PIN 5
#define JOY_X     A0
#define JOY_Y     A1
#define JOY_SW    3   // joystick push button (use INPUT_PULLUP)

const bool RELAY_ACTIVE_LOW = true;

// OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// ---------------- Objects & state ----------------
DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

float temperature = NAN;
float humidity = NAN;
bool fanState = false;
bool autoMode = false;
float maxThreshold = 30.0;
float minThreshold = 25.0;

unsigned long lastDhtMillis = 0;
const unsigned long readInterval = 2000UL;

// handshake
bool pythonConnected = false;
unsigned long lastStatusSent = 0;
const unsigned long STATUS_INTERVAL = 2000UL; // send status periodically only after handshake

// ---------------- Helpers ----------------
void physicalWriteRelay(bool on) {
  if (RELAY_ACTIVE_LOW) {
    digitalWrite(RELAY_PIN, on ? LOW : HIGH);
  } else {
    digitalWrite(RELAY_PIN, on ? HIGH : LOW);
  }
}

void setFan(bool on) {
  fanState = on;
  physicalWriteRelay(on);
  Serial.print("FAN:"); Serial.println(on ? "ON" : "OFF"); // immediate status feedback
}

void updateDisplay() {
  display.clearDisplay();

  display.drawLine(64, 0, 64, 64, SSD1306_WHITE);
  display.drawLine(0, 32, 128, 32, SSD1306_WHITE);

  display.setCursor(0, 5);
  display.print("Temp:");
  if (isnan(temperature)) display.print("--");
  else { display.print(temperature, 1); display.print("C"); }

  display.setCursor(0, 20);
  display.print("Hum: ");
  if (isnan(humidity)) display.print("--");
  else { display.print(humidity, 1); display.print("%"); }

  display.setCursor(70, 5);
  display.print("Fan: ");
  display.println(fanState ? "ON" : "OFF");

  display.setCursor(70, 16);
  display.print("Auto: ");
  display.println(autoMode ? "ON" : "OFF");

  display.setCursor(70, 36);
  display.print("Max:");
  display.print(maxThreshold,1);
  display.setCursor(64, 46);
  display.print(" Min:");
  display.print(minThreshold,1);

  display.setCursor(0, 46);
  display.print(pythonConnected ? "PC: READY" : "PC: WAIT");

  display.display();
}

// --- Joystick control for PC only ---
const int JOY_LEFT_THRESHOLD  = 300; 
const int JOY_RIGHT_THRESHOLD = 700; 
const int JOY_UP_THRESHOLD    = 300; 
const int JOY_DOWN_THRESHOLD  = 700; 

unsigned long lastJoyMillis = 0;
const unsigned long joyInterval = 150UL;
String lastDirection = "CENTER";

String getDirection(int xValue, int yValue, bool buttonPressed) {
  if (buttonPressed) return "ENTER";

  if (xValue < JOY_LEFT_THRESHOLD)  return "LEFT";
  if (xValue > JOY_RIGHT_THRESHOLD) return "RIGHT";
  if (yValue < JOY_UP_THRESHOLD)    return "UP";
  if (yValue > JOY_DOWN_THRESHOLD)  return "DOWN";

  return "CENTER";
}

void handleJoystick() {
  unsigned long now = millis();
  if (now - lastJoyMillis < joyInterval) return;
  lastJoyMillis = now;

  int x = analogRead(JOY_X);
  int y = analogRead(JOY_Y);
  bool buttonPressed = (digitalRead(JOY_SW) == LOW);

  String dir = getDirection(x, y, buttonPressed);

  if (dir != lastDirection) {
    // Always just send joystick direction to PC
    Serial.println(dir);   // e.g. "UP", "DOWN", "LEFT", "RIGHT", "ENTER", "CENTER"
    lastDirection = dir;
  }
}


void readDHTIfNeeded() {
  unsigned long now = millis();
  if (now - lastDhtMillis < readInterval) return;
  lastDhtMillis = now;

  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (!isnan(t)) temperature = t;
  if (!isnan(h)) humidity = h;

  if (autoMode && !isnan(temperature)) {
    if (temperature >= maxThreshold) setFan(true);
    else if (temperature <= minThreshold) setFan(false);
  }

  updateDisplay();
}

void sendStatus() {
  // send key:value lines
  if (!isnan(temperature)) {
    Serial.print("TEMP:"); Serial.println(temperature,2);
  } else {
    Serial.println("TEMP:NAN");
  }
  if (!isnan(humidity)) {
    Serial.print("HUM:"); Serial.println(humidity,1);
  } else {
    Serial.println("HUM:NAN");
  }
  Serial.print("FAN:"); Serial.println(fanState ? "ON" : "OFF");
  Serial.print("AUTO:"); Serial.println(autoMode ? "1" : "0");
  Serial.print("MAX:"); Serial.println(maxThreshold,1);
  Serial.print("MIN:"); Serial.println(minThreshold,1);
}

String rxbuf = "";
void handleLine(String s) {
  s.trim();
  s.toUpperCase();

  // handshake
  if (s == "HELLO") {
    pythonConnected = true;
    Serial.println("READY");
    // send immediate status after handshake
    sendStatus();
    updateDisplay();
    return;
  } else if (s == "PING") {
    // lightweight keepalive
    Serial.println(pythonConnected ? "READY" : "WAIT");
    return;
  }

  // ignore other commands until handshake done
  if (!pythonConnected) return;

  // handle real commands only when pythonConnected == true
  if (s == "GET_STATUS") {
    sendStatus();
  } else if (s == "FAN_ON") {
    setFan(true);
    sendStatus();
  } else if (s == "FAN_OFF") {
    setFan(false);
    sendStatus();
  } else if (s == "AUTO_ON") {
    autoMode = true;
    sendStatus();
  } else if (s == "AUTO_OFF") {
    autoMode = false;
    sendStatus();
  } else if (s.startsWith("SET_MAX:")) {
    String v = s.substring(8);
    float fv = v.toFloat();
    if (fv > -100 && fv < 200) {
      maxThreshold = fv;
      sendStatus();
    }
  } else if (s.startsWith("SET_MIN:")) {
    String v = s.substring(8);
    float fv = v.toFloat();
    if (fv > -100 && fv < 200) {
      minThreshold = fv;
      sendStatus();
    }
  } else {
    // unknown — ignore
  }
}

void processSerial() {
  while (Serial.available()) {
    char c = (char)Serial.read();
    if (c == '\n' || c == '\r') {
      if (rxbuf.length() > 0) {
        handleLine(rxbuf);
        rxbuf = "";
      }
    } else {
      rxbuf += c;
      // guard buffer
      if (rxbuf.length() > 200) rxbuf = "";
    }
  }
}

// ---------------- Setup ----------------
void setup() {
  Serial.begin(9600);
  dht.begin();
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(JOY_SW, INPUT_PULLUP);

  setFan(false);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED failed");
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Waiting for PC...");
    display.display();
  }
}

// ---------------- Loop ----------------
void loop() {
  processSerial();
  handleJoystick();
  readDHTIfNeeded();

  // Only send periodic status if pythonConnected (avoids spamming serial before handshake)
  unsigned long now = millis();
  if (pythonConnected && (now - lastStatusSent >= STATUS_INTERVAL)) {
    lastStatusSent = now;
    sendStatus();
  }

  delay(1000);
}
