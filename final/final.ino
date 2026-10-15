#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

//
// -------------------- Configuration --------------------
//
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define DHT_PIN 2
#define DHT_TYPE DHT11
DHT dht(DHT_PIN, DHT_TYPE);

// Joystick pins (analogs) and button
const int JOY_X_PIN = A0;    // vertical axis
const int JOY_Y_PIN = A1;    // horizontal axis
const int JOY_BUTTON_PIN = 3;

// Fan output pin (use transistor or relay driver)
const int FAN_PIN = 8;

// Joystick thresholds
const int LOW_THRESHOLD = 400;
const int HIGH_THRESHOLD = 600;

// Timing
const unsigned long SEND_INTERVAL_MS = 800;  // how often to send status when enabled
const unsigned long LOOP_DELAY_MS = 150;     // main loop delay for stability

//
// -------------------- State --------------------
bool fanState = false;     // true == ON
bool autoMode = false;     // automatic mode enabled
float tempMax = 30.0;      // default thresholds
float tempMin = 24.0;

bool enabledByPC = false;  // set true when PC sends START
unsigned long lastSendMs = 0;

//
// -------------------- Setup --------------------
void setup() {
  Serial.begin(9600);
  dht.begin();

  // OLED init
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true) yield(); // stop here
  }

  pinMode(JOY_BUTTON_PIN, INPUT_PULLUP);
  pinMode(FAN_PIN, OUTPUT);
  digitalWrite(FAN_PIN, LOW);

  // Initial OLED message
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Waiting for PC...");
  display.display();

  // Announce readiness to host
  Serial.println("READY");
  // Do NOT enable auto behavior here. Wait for PC to send START.
}

//
// -------------------- Loop --------------------
void loop() {
  // Read inputs
  int xVal = analogRead(JOY_X_PIN);
  int yVal = analogRead(JOY_Y_PIN);
  bool buttonPressed = (digitalRead(JOY_BUTTON_PIN) == LOW);

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Determine joystick direction label
  String direction = computeJoystickDirection(xVal, yVal, buttonPressed);

  // Always check serial commands (non-blocking)
  processSerialCommands();

  // Automatic fan control (Arduino-side) when autoMode is true
  if (!isnan(temperature) && autoMode) {
    if (temperature >= tempMax && !fanState) {
      setFan(true);
    } else if (temperature <= tempMin && fanState) {
      setFan(false);
    }
  }

  // Periodic status output when enabled by PC
  unsigned long now = millis();
  if (enabledByPC && (now - lastSendMs >= SEND_INTERVAL_MS)) {
    lastSendMs = now;
    sendStatus(direction, temperature, humidity);
  }

  // Always update OLED with current readings (local display)
  updateDisplay(direction, temperature, humidity);

  delay(LOOP_DELAY_MS);
}

//
// -------------------- Helpers --------------------
String computeJoystickDirection(int xVal, int yVal, bool buttonPressed) {
  String dir = "CENTER";
  if (buttonPressed) {
    dir = "ENTER";
    return dir;
  }
  if (xVal < LOW_THRESHOLD) dir = "UP";
  else if (xVal > HIGH_THRESHOLD) dir = "DOWN";

  if (yVal < LOW_THRESHOLD) {
    if (dir == "CENTER") dir = "RIGHT";
    else dir += "+RIGHT";
  } else if (yVal > HIGH_THRESHOLD) {
    if (dir == "CENTER") dir = "LEFT";
    else dir += "+LEFT";
  }
  return dir;
}

void setFan(bool on) {
  fanState = on;
  digitalWrite(FAN_PIN, on ? HIGH : LOW);
  Serial.print("FAN:"); Serial.println(fanState ? "ON" : "OFF");
}

void sendStatus(const String &direction, float temperature, float humidity) {
  Serial.print("DIR:"); Serial.println(direction);
  if (!isnan(temperature)) {
    Serial.print("TEMP:"); Serial.println(temperature);
  } else {
    Serial.println("TEMP:NAN");
  }

  if (!isnan(humidity)) {
    Serial.print("HUM:"); Serial.println(humidity);
  } else {
    Serial.println("HUM:NAN");
  }

  Serial.print("FAN:"); Serial.println(fanState ? "ON" : "OFF");
  Serial.print("AUTO:"); Serial.println(autoMode ? "1" : "0");
  Serial.print("MAX:"); Serial.println(tempMax);
  Serial.print("MIN:"); Serial.println(tempMin);
}

void updateDisplay(const String &direction, float temperature, float humidity) {
  display.clearDisplay();
  display.drawLine(64, 0, 64, 64, SSD1306_WHITE);
  display.drawLine(0, 32, 128, 32, SSD1306_WHITE);

  display.setCursor(0, 0); display.print("Dir:"); display.setCursor(0, 10); display.print(direction);
  display.setCursor(70, 0); display.print("Temp:"); display.setCursor(70, 10);
  if (!isnan(temperature)) { display.print(temperature); display.print("C"); } else display.print("--C");
  display.setCursor(0, 40); display.print("Hum:"); display.setCursor(0, 50);
  if (!isnan(humidity)) { display.print(humidity); display.print("%"); } else display.print("--%");
  display.setCursor(70, 40); display.print("FAN: "); display.setCursor(70, 50); display.print(fanState ? "ON" : "OFF");

  display.display();
}

//
// -------------------- Serial command processing --------------------
void processSerialCommands() {
  while (Serial.available()) {
    String line = Serial.readStringUntil('\n');
    line.trim();
    if (line.length() == 0) continue;

    // We'll operate on an uppercase copy for command matching
    String up = line;
    up.toUpperCase();

    if (up.indexOf(':') != -1) {
      String key = up.substring(0, up.indexOf(':'));
      String val = line.substring(line.indexOf(':') + 1); // keep original case for floats
      key.trim(); val.trim();
      handleKeyValue(key, val);
    } else {
      handleCommand(up);
    }
  }
}

void handleKeyValue(const String &key, const String &val) {
  if (key == "SET_MAX") {
    float v = val.toFloat();
    if (!isnan(v)) {
      tempMax = v;
      Serial.print("MAX:"); Serial.println(tempMax);
    }
  } else if (key == "SET_MIN") {
    float v = val.toFloat();
    if (!isnan(v)) {
      tempMin = v;
      Serial.print("MIN:"); Serial.println(tempMin);
    }
  } else if (key == "DIR") {
    // optional: PC sent DIR:..., echo back
    String vv = val;
    vv.toUpperCase();
    if (vv == "UP" || vv == "DOWN" || vv == "LEFT" || vv == "RIGHT" || vv == "ENTER") {
      Serial.print("DIR_ECHO:"); Serial.println(vv);
    }
  } else {
    // Unknown key:value -> ignore or log
    Serial.print("UNKNOWN_KV:"); Serial.println(key);
  }
}

void handleCommand(const String &cmd) {
  if (cmd == "START") {
    enabledByPC = true;
    lastSendMs = millis();
    Serial.println("ENABLED");
    display.clearDisplay();
    display.setCursor(0, 0); display.println("Enabled by PC"); display.display();
  } else if (cmd == "STOP") {
    enabledByPC = false;
    Serial.println("DISABLED");
    display.clearDisplay();
    display.setCursor(0, 0); display.println("Stopped by PC"); display.display();
    // Optionally, do not change fan state here so manual override persists
  } else if (cmd == "FAN_ON") {
    setFan(true);
  } else if (cmd == "FAN_OFF") {
    setFan(false);
  } else if (cmd == "AUTO_ON") {
    autoMode = true;
    Serial.println("AUTO:1");
  } else if (cmd == "AUTO_OFF") {
    autoMode = false;
    Serial.println("AUTO:0");
  } else if (cmd == "GET_STATUS") {
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if (!isnan(t)) { Serial.print("TEMP:"); Serial.println(t); } else { Serial.println("TEMP:NAN"); }
    if (!isnan(h)) { Serial.print("HUM:"); Serial.println(h); } else { Serial.println("HUM:NAN"); }
    Serial.print("FAN:"); Serial.println(fanState ? "ON" : "OFF");
    Serial.print("AUTO:"); Serial.println(autoMode ? "1" : "0");
    Serial.print("MAX:"); Serial.println(tempMax);
    Serial.print("MIN:"); Serial.println(tempMin);
  } else {
    Serial.print("UNKNOWN_CMD:"); Serial.println(cmd);
  }
}
