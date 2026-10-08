# Hardware Setup Guide

## Pediatric Smart Toothbrush — Wiring and Assembly

This document provides detailed wiring instructions and assembly guidance for building the smart toothbrush prototype.

---

## Components Required

| Item | Quantity | Notes |
|------|----------|-------|
| ESP32 WROOM-32 Dev Board | 1 | Any ESP32 DevKit with USB programming |
| MPU9255 IMU Module (GY-9255) | 1 | 6-DOF accel + gyro + magnetometer (magnetometer unused) |
| Force-Sensitive Resistor (FSR) | 1 | Circular disc type, 0.5" diameter recommended |
| 0.1 uF ceramic capacitor | 1 | For FSR RC timing circuit |
| SSD1306 OLED Display | 1 | 0.91-inch, 128x32 pixels, I2C interface |
| DFPlayer Mini | 1 | With FAT32-formatted microSD card with MP3 files |
| Small speaker (8 ohm) | 1 | 0.5-2W |
| Active Buzzer (5V) | 1 | Pre-driven, not passive |
| Red LED | 1 | Standard 5mm |
| 220 ohm resistor | 1 | For LED current limiting |
| Tactile push button | 1 | Momentary normally-open |
| SPST toggle switch | 1 | For main power |
| LiPo battery or USB power bank | 1 | 3.7V-5V supply |
| Jumper wires | Many | For prototype connections |
| Toothbrush handle enclosure | 1 | See prototype photos for form factor |

---

## Pin Wiring Table

| Component | Component Pin | ESP32 Pin | Notes |
|-----------|--------------|-----------|-------|
| **MPU9255** | VCC | 3.3V | NOT 5V — use 3.3V rail |
| **MPU9255** | GND | GND | |
| **MPU9255** | SDA | GPIO 21 | I2C data |
| **MPU9255** | SCL | GPIO 22 | I2C clock |
| **MPU9255** | AD0 | GND | Sets I2C address to 0x68 |
| **SSD1306 OLED** | VCC | 3.3V | |
| **SSD1306 OLED** | GND | GND | |
| **SSD1306 OLED** | SDA | GPIO 21 | Shared I2C bus with MPU |
| **SSD1306 OLED** | SCL | GPIO 22 | Shared I2C bus with MPU |
| **FSR** | Pin 1 | GPIO 25 | Charge pin |
| **FSR** | Pin 2 | GPIO 35 (via cap) | ADC read pin |
| **0.1uF Cap** | Pin 1 | GPIO 35 | One side to ADC |
| **0.1uF Cap** | Pin 2 | GND | Other side to ground |
| **DFPlayer Mini** | VCC | 5V (VIN) | Needs 5V |
| **DFPlayer Mini** | GND | GND | |
| **DFPlayer Mini** | RX | GPIO 17 | UART TX from ESP32 |
| **DFPlayer Mini** | TX | GPIO 16 | UART RX to ESP32 |
| **DFPlayer Mini** | SPK1 | Speaker + | |
| **DFPlayer Mini** | SPK2 | Speaker - | |
| **Active Buzzer** | + | GPIO 27 | Via 100 ohm resistor optional |
| **Active Buzzer** | - | GND | |
| **Red LED** | Anode (+) | GPIO 4 | Via 220 ohm resistor |
| **Red LED** | Cathode (-) | GND | |
| **Push Button** | Pin 1 | GPIO 14 | With INPUT_PULLUP in firmware |
| **Push Button** | Pin 2 | GND | |
| **SPST Switch** | Pin 1 | Battery + | Main power switch |
| **SPST Switch** | Pin 2 | ESP32 VIN | |

---

## FSR RC Circuit Detail

The FSR does not use a traditional voltage divider. Instead, it uses RC timing:

```
     GPIO 25 (CHARGE_PIN)
          |
         [FSR]
          |
     +----+---- GPIO 35 (ADC_PIN)
     |
   [0.1uF]
     |
    GND
```

**Charge sequence in firmware:**
1. `CHARGE_PIN` set as OUTPUT, driven HIGH for 2000 us
2. `CHARGE_PIN` set as INPUT (high impedance — stops charging)
3. Wait exactly 901 us (discharge through FSR into ADC input impedance)
4. Read ADC value on `ADC_PIN`

The 901 us discharge time was empirically determined to give good dynamic range across the force range of interest (0–8 N).

---

## I2C Bus

Both the MPU9255 and SSD1306 OLED share the I2C bus on GPIO 21 (SDA) and GPIO 22 (SCL):

- MPU9255 address: `0x68` (AD0 tied to GND)
- SSD1306 address: `0x3C`

No I2C pull-up resistors needed if using module breakout boards (most include onboard pull-ups).

---

## DFPlayer Mini Audio Setup

1. Format a microSD card as FAT32
2. Create a folder called `01` on the root
3. Place MP3 files named `001.mp3`, `002.mp3`, etc. in the `01` folder
4. Alternatively, place MP3 files directly at root: `0001.mp3`, `0002.mp3`, etc.
5. Insert card into DFPlayer
6. The app sends track numbers `1`-`9` via Bluetooth to select tracks

**Serial communication:** 9600 baud on HardwareSerial 2 (GPIO 16/17)

---

## Assembly Notes

1. **IMU placement:** Mount MPU9255 rigidly against the toothbrush handle. Any mechanical flex between IMU and handle introduces orientation error. Use adhesive mount or rigid enclosure.

2. **FSR placement:** The FSR should be placed at the natural index-finger rest position — where the user's index finger contacts the handle during brushing. This maximizes correlation between fingertip contact force and axial brushing pressure.

3. **OLED orientation:** Position the OLED so it is visible when holding the brush. The 128x32 display shows 3 lines of text at size 1.

4. **Buzzer placement:** The active buzzer should be audible but not startling. Consider mounting inside the handle cavity for a more natural tone.

5. **Button accessibility:** The push button initiates calibration at session start. It should be easily accessible when the brush is in the pre-brushing grip position.

6. **Cable management:** The prototype uses taped-wrap construction (visible in photos). For a cleaner build, 3D print a handle enclosure with component pockets.

---

## Power

The prototype can be powered via:
- USB from ESP32 programming port (easiest for development)
- LiPo battery + LiPo charging module connected to ESP32 VIN (5V) or 3.3V
- USB power bank

The SPST switch interrupts battery positive to ESP32 VIN for a clean power-off.

---

## Testing Checklist

- [ ] I2C scan confirms 0x68 (MPU) and 0x3C (OLED) visible
- [ ] Serial output shows FSR voltage readings changing with finger pressure
- [ ] OLED displays startup splash, then "Hold normal to teeth / then press button"
- [ ] Button press triggers "Initializing MPU..." and convergence
- [ ] Pitch value on OLED changes with brush angle
- [ ] Buzzer activates when brush deviates beyond +/-15 degrees for 500ms
- [ ] LED activates when FSR force exceeds 2.0 N
- [ ] DFPlayer plays audio track on session start
- [ ] Bluetooth "SmartBrush" device visible and pairable
- [ ] Volume +/- commands work from companion app
- [ ] 2-minute timer ends session and stops audio
