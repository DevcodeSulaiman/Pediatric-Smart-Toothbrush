# Hardware Notes

## Component Sourcing

All components are commercially available from standard electronics suppliers:

| Component | Source | Approx. Cost (USD) |
|-----------|--------|---------------------|
| ESP32 WROOM-32 Dev Board | AliExpress / Amazon / Mouser | $3-8 |
| MPU9255 GY-9255 Module | AliExpress / Amazon | $2-5 |
| Force-Sensitive Resistor | SparkFun / Adafruit / AliExpress | $3-7 |
| SSD1306 0.91" OLED | AliExpress / Amazon | $2-4 |
| DFPlayer Mini | AliExpress / Amazon | $1-3 |
| Active Buzzer | AliExpress / Amazon | $0.50-1 |
| 0.1uF ceramic cap | Electronics store | <$0.10 |
| Misc (LED, button, switch, wires) | Electronics store | $1-2 |

**Estimated total BOM cost:** ~$15-30 USD

## Design Notes

### Why MPU9255 and not MPU6050?
The MPU9255 includes a magnetometer (AK8963) which is unused in the current firmware (6-DOF Madgwick, no mag update). The MPU6050 would work equally well for this application. The MPU9255 was selected for future-proofing if compass-assisted localization is added.

### Why Active Buzzer and not Passive?
An active buzzer requires only a digital HIGH/LOW signal and contains its own oscillator — simpler firmware, no PWM tone generation needed. A passive buzzer would allow tone control but adds complexity.

### Prototype Construction
The current prototype uses foam + tape wrap construction (visible in photos). This is a functional proof-of-concept. A production version would use a custom 3D printed or injection molded handle.

## Calibration Data

The FSR calibration was performed empirically using known static loads applied with a precision scale. The voltage-force lookup table embedded in firmware is specific to this FSR + RC circuit configuration:

- Charge time: 2000 us (2 ms)
- Discharge time: 901 us (empirically determined)
- Reference voltage: 3.3V
- ADC resolution: 12-bit (0-4095)

If using a different FSR, the RC discharge time and lookup table voltages should be recalibrated.
