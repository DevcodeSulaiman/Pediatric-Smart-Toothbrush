/*
  Smart Brush – Final Integrated Code (Manual Madgwick AHRS integrated, ±180° pitch)
  - MPU9255 orientation + FSR pressure detection (2.0N LUT)
  - SSD1306 OLED display
  - DFPlayer Mini music playback (UART2 patched)
  - BluetoothSerial control (music select + volume + stop)
  - Automatic 2-minute brushing timer
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>
#include "BluetoothSerial.h"
#include "DFRobotDFPlayerMini.h"

// ---------- OLED ----------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_ADDR 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ---------- Bluetooth + DFPlayer ----------
BluetoothSerial SerialBT;
HardwareSerial myDFSerial(2); // UART2 for DFPlayer
DFRobotDFPlayerMini dfPlayer;
int currentVolume = 20;
const int MAX_VOLUME = 30;
const int MIN_VOLUME = 0;

// ---------- FSR ----------
const int CHARGE_PIN = 25;
const int ADC_PIN = 35;
const int FSR_LED_PIN = 4;
const float FORCE_THRESHOLD_N = 2.0f;
float pitchOffset = 0.0f;

// ---------- MPU9255 ----------
#define MPU_ADDR 0x68
#define PWR_MGMT_1 0x6B
#define ACCEL_XOUT_H 0x3B

// ---------- Pins ----------
#define BUTTON_PIN 14
#define BUZZER_PIN 27

// ---------- Orientation / Madgwick params ----------
#define GYRO_SENS 131.0f    // raw -> deg/s
#define ACCEL_SENS 16384.0f // raw -> g
float beta = 0.06f; // Madgwick gain (tune: 0.02..0.12). Lower -> smoother but slower to correct drift
const float gyroBiasFilterAlpha = 0.02f; // smoothing when estimating gyro bias
const float angleLPFalpha = 0.18f; // low-pass for displayed pitch (0..1)

const float TARGET_ANGLE = 45.0f;
const float ANGLE_TOLERANCE = 15.0f;
const unsigned long ANGLE_HOLD_THRESHOLD = 500;

// Madgwick quaternion state
volatile float q0 = 1.0f, q1 = 0.0f, q2 = 0.0f, q3 = 0.0f;

// gyro bias (deg/s)
float gyroBiasX = 0, gyroBiasY = 0, gyroBiasZ = 0;

// timing
unsigned long lastTime = 0;

// LPF filtered pitch for display
float pitchFiltered = 0.0f;

// brushing state
bool mpu_started = false;
bool brushing = false;
unsigned long brushStartTime = 0;
const unsigned long brushDuration = 120000; // 2 minutes
unsigned long angleOutStart = 0;
bool angleWarning = false;
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

// ---------- Utility ----------
void showText(const char* l1, const char* l2 = "", const char* l3 = "") {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(l1);
  if (strlen(l2)) display.println(l2);
  if (strlen(l3)) display.println(l3);
  display.display();
}

// ---------- FSR RC Timing & Math (LUT) ----------
float readFSRVoltage() {
  pinMode(CHARGE_PIN, OUTPUT);
  digitalWrite(CHARGE_PIN, HIGH);
  delay(2); // Fully saturate capacitor to 3.3V
  
  pinMode(CHARGE_PIN, INPUT); // High impedance mode
  delayMicroseconds(901);     // Exact calculated discharge time
  
  int adc = analogRead(ADC_PIN);
  return (adc / 4095.0f) * 3.3f;
}

float getForceN() {
    float Vt = readFSRVoltage(); 
    
    // LUT for 2.0N threshold at 901µs
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

// ---------- Raw MPU Read ----------
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

  // convert to physical units
  ax = rawAx / ACCEL_SENS; // g
  ay = rawAy / ACCEL_SENS;
  az = rawAz / ACCEL_SENS;
  gx = rawGx / GYRO_SENS; // deg/s
  gy = rawGy / GYRO_SENS;
  gz = rawGz / GYRO_SENS;
  return true;
}

// ---------- Madgwick AHRS (IMU-only) ----------
void MadgwickAHRSupdateIMU(float gx, float gy, float gz, float ax, float ay, float az, float dt) {
  // gx,gy,gz in rad/s ; ax,ay,az normalized (g)
  float q0l = q0, q1l = q1, q2l = q2, q3l = q3;
  float recipNorm;
  float s0, s1, s2, s3;
  float _2q0 = 2.0f * q0l;
  float _2q1 = 2.0f * q1l;
  float _2q2 = 2.0f * q2l;
  float _2q3 = 2.0f * q3l;
  float _4q0 = 4.0f * q0l;
  float _4q1 = 4.0f * q1l;
  float _4q2 = 4.0f * q2l;
  float _8q1 = 8.0f * q1l;
  float _8q2 = 8.0f * q2l;
  float q0q0 = q0l * q0l;
  float q1q1 = q1l * q1l;
  float q2q2 = q2l * q2l;
  float q3q3 = q3l * q3l;

  // Normalise accelerometer measurement
  recipNorm = sqrtf(ax * ax + ay * ay + az * az);
  if (recipNorm == 0.0f) return;
  recipNorm = 1.0f / recipNorm;
  ax *= recipNorm; ay *= recipNorm; az *= recipNorm;

  // Gradient descent algorithm corrective step
  s0 = _4q0 * q2q2 + _2q2 * ax + _4q0 * q1q1 - _2q1 * ay;
  s1 = _4q1 * q3q3 - _2q3 * ax + 4.0f * q0q0 * q1l - _2q0 * ay - _4q1 + _8q1 * q1q1 + _8q1 * q2q2 + _4q1 * az;
  s2 = 4.0f * q0q0 * q2l + _2q0 * ax + _4q2 * q3q3 - _2q3 * ay - _4q2 + _8q2 * q1q1 + _8q2 * q2q2 + _4q2 * az;
  s3 = 4.0f * q1q1 * q3l - _2q1 * ax + 4.0f * q2q2 * q3l - _2q2 * ay;
  // normalize step magnitude
  recipNorm = sqrtf(s0 * s0 + s1 * s1 + s2 * s2 + s3 * s3);
  if (recipNorm == 0.0f) return;
  recipNorm = 1.0f / recipNorm;
  s0 *= recipNorm; s1 *= recipNorm; s2 *= recipNorm; s3 *= recipNorm;

  // Rate of change of quaternion
  float qDot0 = 0.5f * (-q1l * gx - q2l * gy - q3l * gz) - beta * s0;
  float qDot1 = 0.5f * ( q0l * gx + q2l * gz - q3l * gy) - beta * s1;
  float qDot2 = 0.5f * ( q0l * gy - q1l * gz + q3l * gx) - beta * s2;
  float qDot3 = 0.5f * ( q0l * gz + q1l * gy - q2l * gx) - beta * s3;

  // Integrate to yield quaternion
  q0 += qDot0 * dt;
  q1 += qDot1 * dt;
  q2 += qDot2 * dt;
  q3 += qDot3 * dt;

  // Normalise quaternion
  recipNorm = sqrtf(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
  recipNorm = 1.0f / recipNorm;
  q0 *= recipNorm; q1 *= recipNorm; q2 *= recipNorm; q3 *= recipNorm;
}

// ---------- Quaternion -> gravity/angles ----------
void quaternionToGravity(float &gx, float &gy, float &gz) {
  // gravity vector (body frame) for vector (0,0,1) transformed by quaternion
  gx = 2.0f * (q1 * q3 - q0 * q2);
  gy = 2.0f * (q0 * q1 + q2 * q3);
  gz = q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3;
}

// ---------- Angle Check ----------
void checkBrushingAngle(float currentAngle) {
  if (abs(currentAngle - TARGET_ANGLE) > ANGLE_TOLERANCE) {
    if (angleOutStart == 0) angleOutStart = millis();
    if ((millis() - angleOutStart > ANGLE_HOLD_THRESHOLD) && !angleWarning) {
      angleWarning = true;
      digitalWrite(BUZZER_PIN, HIGH);
    }
  } else {
    angleOutStart = 0;
    angleWarning = false;
    digitalWrite(BUZZER_PIN, LOW);
  }
}

// ---------- MPU Init ----------
void initMPU() {
  Wire.begin(21, 22);
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(PWR_MGMT_1);
  Wire.write(0x00); // wake
  Wire.endTransmission();
  delay(50);
}

// ---------- Bluetooth Commands ----------
void handleBTCommands() {
  if (SerialBT.available()) {
    char c = SerialBT.read();
    if (c == '+') {
      currentVolume++;
      if (currentVolume > MAX_VOLUME) currentVolume = MAX_VOLUME;
      dfPlayer.volume(currentVolume);
      delay(30);
      Serial.println("Volume +");
    } else if (c == '-') {
      currentVolume--;
      if (currentVolume < MIN_VOLUME) currentVolume = MIN_VOLUME;
      dfPlayer.volume(currentVolume);
      delay(30);
      Serial.println("Volume -");
    } else if (c >= '1' && c <= '9') {
      int track = c - '0';
      currentBrushTrack = track;
      dfPlayer.play(currentBrushTrack);
      delay(30);
      Serial.printf("BT Track Selected: %d\n", currentBrushTrack);
    } else if (c == 's') {
      dfPlayer.stop();
      delay(30);
      Serial.println("Stopped by BT");
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

  // Critical ADC configuration for 3.3V range
  analogReadResolution(12);
  analogSetPinAttenuation(ADC_PIN, ADC_11db);

  Serial.begin(115200);
  SerialBT.begin("SmartBrush");
  myDFSerial.begin(9600, SERIAL_8N1, 16, 17); // RX=16, TX=17
  
  delay(50);

  // Initialize DFPlayer (ACK disabled for clone chips, reset enabled)
  if (!dfPlayer.begin(myDFSerial, false, true)) {
    Serial.println("DFPlayer not found!");
  } else {
    delay(50);
    dfPlayer.volume(currentVolume);
    delay(50);
    Serial.println("DFPlayer ready");
  }

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED init failed!");
    for (;;);
  }

  display.clearDisplay();
  display.drawBitmap(0, 0, epd_bitmap_Screenshot_4_11_2025_14946_www, 128, 32, SSD1306_WHITE);
  display.display();
  delay(1000);
  showText("Hold normal to teeth", "then press button");
  Serial.println("Waiting for button...");
  
  dfPlayer.play(4);
  delay(50);

  lastTime = millis();
}

// ---------- Loop ----------
void loop() {
  handleBTCommands();

  float F = getForceN();
  digitalWrite(FSR_LED_PIN, (F >= FORCE_THRESHOLD_N) ? HIGH : LOW);

  // ----- WAIT FOR CALIBRATION -----
  if (!mpu_started) {
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 500) {
      Serial.printf("Force ≈ %.2f N\n", F);
      lastPrint = millis();
    }

    if (digitalRead(BUTTON_PIN) == LOW) {
      delay(30);
      if (digitalRead(BUTTON_PIN) == LOW) {
        showText("Calibrating...", "Please hold steady");
        Serial.println("Starting calibration...");
        
        dfPlayer.play(5);
        delay(50); // Prevent UART collision
        
        initMPU();

        const int samples = 100;
        float sumPitch = 0;
        int validSamples = 0;
        float sumGx = 0, sumGy = 0, sumGz = 0;

        for (int i = 0; i < samples; i++) {
          float ax, ay, az, gx, gy, gz;
          if (readRawMPU(ax, ay, az, gx, gy, gz)) {
            // accel-based pitch estimate (limited ±90) to set offset
            float accelPitch = atan2f(ax, az) * 180.0f / PI; 
            sumPitch += accelPitch;
            validSamples++;

            // accumulate gyro to estimate bias (deg/s)
            sumGx += gx;
            sumGy += gy;
            sumGz += gz;
          }

          if (i % 10 == 0) {
            display.clearDisplay();
            display.setCursor(10, 10);
            display.print("Calibrating");
            for (int j = 0; j < (i / 10) % 4; j++) display.print(".");
            display.display();
          }
          delay(25);
        }

        if (validSamples > 0) {
          pitchOffset = sumPitch / validSamples;
          gyroBiasX = sumGx / validSamples;
          gyroBiasY = sumGy / validSamples;
          gyroBiasZ = sumGz / validSamples;
        } else {
          pitchOffset = 0.0f;
          gyroBiasX = gyroBiasY = gyroBiasZ = 0.0f;
        }

        // reset quaternion to identity
        q0 = 1.0f; q1 = q2 = q3 = 0.0f;
        lastTime = millis();

        showText("Calibration", "Complete!");
        Serial.printf("Pitch offset: %.2f  gyroBiasX: %.2f gy: %.2f gz: %.2f\n", pitchOffset, gyroBiasX, gyroBiasY, gyroBiasZ);
        
        dfPlayer.play(6);
        delay(1200);

        // start brushing
        mpu_started = true;
        brushing = true;
        brushStartTime = millis();
        showText("Brushing Started!");
        
        dfPlayer.play(currentBrushTrack);
        delay(50); // Prevent UART collision
      }
    }
    return; // wait until calibrated
  }

  // ----- NORMAL OPERATION -----
  unsigned long now = millis();
  float dt = (now - lastTime) / 1000.0f;
  if (dt <= 0.0f) dt = 0.001f;
  lastTime = now;

  float ax, ay, az, gx_deg, gy_deg, gz_deg;
  if (readRawMPU(ax, ay, az, gx_deg, gy_deg, gz_deg)) {

    // subtract gyro bias and convert to rad/s
    float gx = (gx_deg - gyroBiasX) * (PI / 180.0f);
    float gy = (gy_deg - gyroBiasY) * (PI / 180.0f);
    float gz = (gz_deg - gyroBiasZ) * (PI / 180.0f);

    // Run Madgwick IMU update
    MadgwickAHRSupdateIMU(gx, gy, gz, ax, ay, az, dt);

    // Get gravity vector from quaternion
    float g_x, g_y, g_z;
    quaternionToGravity(g_x, g_y, g_z);

    // Continuous ±180° pitch using atan2(gx, gz)
    float pitch360 = atan2f(g_x, g_z) * 180.0f / PI;

    // apply user pitch offset (from accel-based calibration)
    float adjPitch = pitch360 - pitchOffset;
    // normalize to -180..180
    if (adjPitch > 180.0f) adjPitch -= 360.0f;
    if (adjPitch < -180.0f) adjPitch += 360.0f;

    // Apply a simple low-pass filter to reduce jitter on display and checks
    pitchFiltered = (angleLPFalpha * adjPitch) + ((1.0f - angleLPFalpha) * pitchFiltered);

    // Display
    display.clearDisplay();
    display.setCursor(0, 0);
    display.printf("Angle: %.2f\n", pitchFiltered);
    display.printf("Force: %.2f N\n", F);
    display.display();

    // Timer
    if (brushing && (millis() - brushStartTime >= brushDuration)) {
      brushing = false;
      dfPlayer.stop();
      delay(50); // Prevent UART collision
      
      showText("Brushing", "Complete!");
      Serial.println("2 min done. Music stopped.");
    }

    if (brushing) {
      checkBrushingAngle(fabs(pitchFiltered - 2.0f)); 
    }
  } 

  delay(20); 
}
