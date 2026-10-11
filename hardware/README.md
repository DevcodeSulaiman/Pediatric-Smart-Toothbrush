# Hardware Notes

## Component Sourcing

All components are commercially available from standard electronics suppliers:

| Component | Source | Approx. Cost (USD) |
|-----------|--------|---------------------|
| ESP32 WROOM-32 Dev Board | locally available | Rs. 350 |
| MPU9255 (GY-9255) Module | locally available | Rs. 230 |
| Robodo SEN36 Force Sensing Resistor | Amazon | Rs. 449 |
| SSD1306 0.91" OLED | locally available | Rs. 150 |
| DFPlayer Mini | locally available | Rs. 120 |
| 0.5W 8 ohm miniature speaker | locally available | Rs. 15 |
| Active Buzzer | locally available | Rs. 10 |
| 0.1uF ceramic cap | locally available | <$0.10 |
| Misc (LED, button, switch, wires) | locally available | Rs. 20 |

**Estimated total BOM cost:** ~$15-30 USD

## Design Notes

### Why MPU9255 and not MPU6050?
The MPU9255 is much more readily and locally available than the MPU6050 which is why MPU9255 costs lesser than the MPU6050. The technology was aimed to be readily available with no artificial obsolescence such that anyone would be able to replace any broken parts easily without restrictions.

### Prototype Construction
The current prototype uses foam + tape wrap construction (visible in photos). This is a functional proof-of-concept. A production version would use a custom 3D printed or injection molded handle.

## Calibration Data

The FSR calibration was performed empirically using known static loads applied with a precision scale. The voltage-force lookup table embedded in firmware is specific to this FSR + RC circuit configuration:

- Charge time: 2000 us (2 ms)
- Discharge time: 901 us (empirically determined)
- Reference voltage: 3.3V
- ADC resolution: 12-bit (0-4095)

If using a different FSR, the RC discharge time and lookup table voltages should be recalibrated.
