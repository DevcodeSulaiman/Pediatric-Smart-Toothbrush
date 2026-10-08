#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>
#include <MadgwickAHRS.h>
#include "BluetoothSerial.h"
#include "DFRobotDFPlayerMini.h"

// ---------- OLED ----------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_ADDR 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ---------- Bluetooth & DFPlayer ----------
BluetoothSerial SerialBT;
HardwareSerial myDFSerial(2);
DFRobotDFPlayerMini dfPlayer;
int currentVolume = 25;
int currentBrushTrack = 1;

// ---------- Bitmap ----------
const unsigned char epd_bitmap_Screenshot_4_11_2025_14946_www [] PROGMEM = {
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x07, 0xf8, 0xc0, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x3f, 0xfd, 0xe0, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x20, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xea, 0xf5, 0xef, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xea, 0xf5, 0xef, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xea, 0xf5, 0xef, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xea, 0xf5, 0xef, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xea, 0xf5, 0xef, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xea, 0xf5, 0xef, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xea, 0xf5, 0xef, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xea, 0xd5, 0x6f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

// ---------- Pins & Constants ----------
const int CHARGE_PIN = 25;
const int ADC_PIN    = 35;
const int FSR_LED_PIN = 4;
#define BUTTON_PIN 14
#define BUZZER_PIN 27

#define MPU_ADDR 0x68
#define PWR_MGMT_1 0x6B
#define ACCEL_XOUT_H 0x3B

#define GYRO_SENS 131.0
#define ACCEL_SENS 16384.0

const float FORCE_THRESHOLD_N = 2.0f;
const float TARGET_ANGLE = 45.0;
const float ANGLE_TOLERANCE = 15.0;
const unsigned long ANGLE_HOLD_THRESHOLD = 500;
const unsigned long brushDuration = 120000;

// ---------- Timing & Globals ----------
Madgwick filter;
unsigned long lastIMUUpdate = 0;
unsigned long lastUIUpdate = 0;
const unsigned long IMU_INTERVAL_US = 10000; // 100 Hz
const unsigned long UI_INTERVAL_MS = 80;     // 12.5 Hz

float pitchOffset = 0.0;     
bool mpu_started = false;
bool brushing = false;
unsigned long brushStartTime = 0;
unsigned long angleOutStart = 0;
bool angleWarning = false;

// ---------- Utility Functions ----------
void showText(const char* l1, const char* l2 = "", const char* l3 = "") {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println(l1);
  if (strlen(l2)) display.println(l2);
  if (strlen(l3)) display.println(l3);
  display.display();
}

bool readRawMPU(float &ax, float &ay, float &az, float &gx, float &gy, float &gz) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(ACCEL_XOUT_H);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 14, true);
  if (Wire.available() < 14) return false;

  int16_t rawAx = Wire.read() << 8 | Wire.read();
  int16_t rawAy = Wire.read() << 8 | Wire.read();
  int16_t rawAz = Wire.read() << 8 | Wire.read();
  Wire.read(); Wire.read(); // temp
  int16_t rawGx = Wire.read() << 8 | Wire.read();
  int16_t rawGy = Wire.read() << 8 | Wire.read();
  int16_t rawGz = Wire.read() << 8 | Wire.read();

  ax = rawAx / ACCEL_SENS;
  ay = rawAy / ACCEL_SENS;
  az = rawAz / ACCEL_SENS;
  gx = rawGx / GYRO_SENS; // deg/s required by Madgwick
  gy = rawGy / GYRO_SENS;
  gz = rawGz / GYRO_SENS;
  return true;
}

void checkBrushingAngle(float currentAngle) {
  if (abs(currentAngle - TARGET_ANGLE) > ANGLE_TOLERANCE) {
    if (angleOutStart == 0) angleOutStart = millis();
    if ((millis() - angleOutStart > ANGLE_HOLD_THRESHOLD) && !angleWarning) {
      angleWarning = true;
      digitalWrite(BUZZER_PIN, HIGH);
      showText("! Incorrect", "Brushing Angle!");
    }
  } else {
    angleOutStart = 0;
    angleWarning = false;
    digitalWrite(BUZZER_PIN, LOW);
  }
}

// ---------- FSR RC Timing & Math ----------
float readFSRVoltage() {
  pinMode(CHARGE_PIN, OUTPUT);
  digitalWrite(CHARGE_PIN, HIGH);
  delayMicroseconds(2000); // 2ms is enough to saturate
  
  pinMode(CHARGE_PIN, INPUT); // High impedance mode
  delayMicroseconds(901);     // Exact calculated discharge time
  
  int adc = analogRead(ADC_PIN);
  return (adc / 4095.0f) * 3.3f;
}

float getForceN() {
    float Vt = readFSRVoltage(); 
    
    float V_0N   = 3.27f; // 0g
    float V_0_5N = 2.44f; // 50g
    float V_1N   = 1.99f; // 100g
    float V_2N   = 1.65f; // 200g (Threshold)
    float V_4N   = 1.14f; // 400g
    float V_8N   = 0.54f; // 800g

    float F = 0.0f;
    if (Vt >= V_0N) return 0.0f;
    if (Vt <= V_8N) return 8.0f;

    if (Vt < V_0N && Vt >= V_0_5N) {
        F = 0.0f + (0.5f - 0.0f) * ((V_0N - Vt) / (V_0N - V_0_5N));
    } 
    else if (Vt < V_0_5N && Vt >= V_1N) {
        F = 0.5f + (1.0f - 0.5f) * ((V_0_5N - Vt) / (V_0_5N - V_1N));
    } 
    else if (Vt < V_1N && Vt >= V_2N) {
        F = 1.0f + (2.0f - 1.0f) * ((V_1N - Vt) / (V_1N - V_2N));
    }
    else if (Vt < V_2N && Vt >= V_4N) {
        F = 2.0f + (4.0f - 2.0f) * ((V_2N - Vt) / (V_2N - V_4N));
    }
    else if (Vt < V_4N && Vt > V_8N) {
        F = 4.0f + (8.0f - 4.0f) * ((V_4N - Vt) / (V_4N - V_8N));
    }
    return F;
}

void initMPU() {
  Wire.begin(21, 22);
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(PWR_MGMT_1);
  Wire.write(0x00); // Wake
  Wire.endTransmission();
  delay(50);
}

// ---------- Bluetooth Handler ----------
void handleBTCommands() {
  if (SerialBT.available()) {
    char c = SerialBT.read();
    if (c == '+') {
      currentVolume++;
      if (currentVolume > 30) currentVolume = 30;
      dfPlayer.volume(currentVolume);
      delay(30);
    } else if (c == '-') {
      currentVolume--;
      if (currentVolume < 0) currentVolume = 0;
      dfPlayer.volume(currentVolume);
      delay(30);
    } else if (c >= '1' && c <= '9') {
      currentBrushTrack = c - '0';
      dfPlayer.play(currentBrushTrack);
      delay(30);
    }
  }
}

// ---------- Setup ----------
void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(FSR_LED_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(FSR_LED_PIN, LOW);

  analogReadResolution(12);
  analogSetPinAttenuation(ADC_PIN, ADC_11db);

  Serial.begin(115200);
  SerialBT.begin("SmartBrush");
  myDFSerial.begin(9600, SERIAL_8N1, 16, 17); 
  
  filter.begin(100); // Initialize Madgwick for 100Hz

  if (!dfPlayer.begin(myDFSerial, false, true)) {
    Serial.println("⚠ DFPlayer not found! Check wiring.");
  } else {
    delay(50);
    dfPlayer.volume(currentVolume);
    delay(50);
    Serial.println("🎵 DFPlayer ready");
  }

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED init failed!");
    for (;;) ;
  }
  display.clearDisplay();
  display.drawBitmap(0, 0, epd_bitmap_Screenshot_4_11_2025_14946_www, 128, 32, SSD1306_WHITE);
  display.display();
  delay(1500);
  
  showText("Hold normal to teeth", "then press button", "");
}

// ---------- Main Loop ----------
void loop() {
  unsigned long currentMicros = micros();
  unsigned long currentMillis = millis();

  // 1. High-Frequency IMU & AHRS Update (Strictly ~100Hz)
  if (mpu_started && (currentMicros - lastIMUUpdate >= IMU_INTERVAL_US)) {
    lastIMUUpdate = currentMicros;
    float ax, ay, az, gx, gy, gz;
    if (readRawMPU(ax, ay, az, gx, gy, gz)) {
      // 6-DOF update (ignores magnetometer)
      filter.updateIMU(gx, gy, gz, ax, ay, az); 
    }
  }

  // 2. Low-Frequency UI & Sensor Update (12.5Hz)
  if (currentMillis - lastUIUpdate >= UI_INTERVAL_MS) {
    lastUIUpdate = currentMillis;

    handleBTCommands();

    float F = getForceN();
    digitalWrite(FSR_LED_PIN, (F >= FORCE_THRESHOLD_N) ? HIGH : LOW);

    if (!mpu_started) {
      if (digitalRead(BUTTON_PIN) == LOW) {
        delay(20); 
        if (digitalRead(BUTTON_PIN) == LOW) {
          showText("Initializing MPU...", "Keep steady", "");
          initMPU();

          // Pump the Madgwick filter to converge (1 second)
          unsigned long calibStart = millis();
          while (millis() - calibStart < 1000) {
            float ax, ay, az, gx, gy, gz;
            if (readRawMPU(ax, ay, az, gx, gy, gz)) {
              filter.updateIMU(gx, gy, gz, ax, ay, az);
            }
            delay(5); // Roughly 200 iterations
          }
          
          pitchOffset = filter.getPitch(); // Extract converged pitch as offset
          
          mpu_started = true;
          brushing = true;
          brushStartTime = millis();
          
          dfPlayer.play(currentBrushTrack);
          delay(50); 
          
          showText("Brushing", "Started!", "");
          delay(200); 
        }
      }
    } else {
      // Brushing active: Extract Euler Pitch from quaternion filter
      float currentPitch = filter.getPitch();
      float adjPitch = currentPitch - pitchOffset; 
      
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0,0);
      display.print("Pitch: "); display.println(adjPitch, 2);
      display.print("Force: "); display.print(F, 2); display.println(" N");
      display.display();

      if (brushing && millis() - brushStartTime >= brushDuration) {
        brushing = false;
        dfPlayer.stop();
        delay(50);
        showText("Brushing", "Complete!", "");
      }
      
      if (brushing) {
          checkBrushingAngle(fabs(adjPitch));
      }
    }
  }
}