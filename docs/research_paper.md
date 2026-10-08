# A Smart Pediatric Toothbrush with Embedded Force and Orientation Sensing for Real-Time Brushing Feedback

**Authors:** Sabhyata Soni, Mohammed Talha, Md Sulaiman Qamar, Ishi Soni  
**Affiliation:** Department of EEE, UIET, Panjab University, Chandigarh

---

## Abstract

This paper details the design and testing of a pediatric smart toothbrush featuring embedded sensing aimed at providing live feedback about brushing forces and angles. Orientation is calculated using an MPU9255 IMU in conjunction with Madgwick's AHRS applied to quaternion representation of rotational motion, constraining brush position to a +/-15 degree window about the baseline 45 degree orientation to teeth. Indirect force feedback is integrated using an FSR positioned at the index-finger rest zone as a proxy for axial bristle pressure, with a calibrated threshold of 150 g (1.5 N) to 300 g (3.0 N). All signal conditioning, logic evaluation, and actuation execute on an ESP32 microcontroller, complemented by an Android app developed in MIT App Inventor for sound selection, volume control via DFPlayer module, and 2-minute session enforcement.

**Index Terms:** pediatric oral health, smart toothbrush, force sensing, inertial measurement unit, IoT, real-time feedback, embedded systems

---

## I. Introduction

Dental caries is the most common chronic disease in children worldwide, with nearly 500 million children suffering from untreated tooth decay in their primary teeth. Poor brushing technique - including incorrect brush angle, excessive or insufficient pressure, and insufficient duration - is a primary driver.

Children should ideally brush twice daily at 45 degrees to the gum line for 2 minutes. In reality, children brush with uneven pressure, skip lingual and posterior surfaces, and fail to maintain correct orientation. This gap leads to plaque buildup, enamel abrasion, gingivitis, and significant quality-of-life impacts.

Commercial smart brushes are expensive and adult-oriented. Research prototypes use IMU systems, computer vision, and gamified apps but none deliver an integrated, low-cost, child-appropriate solution measuring both pressure and orientation in a single self-contained device.

---

## II. Background and Related Work

### A. IMU-Based Orientation Tracking
- Lee et al. (2007): ~100% gesture recognition with 3-axis accelerometer + magnetometer
- Hussain et al. (2021): 97.15% accuracy classifying 16 mouth routine sections
- Wang et al. (2025): 96.13% accuracy for 18 brushing regions with dual IMU + ML

### B. Force and Pressure Sensing
- Akhtaruzzaman (2021): FSR in handle with LED indicators at 1.0-2.5 N thresholds
- Yang et al. (2025) - OpenBrush: 500g load cell in neck, 0.43g mean absolute error
- Li and Jing (2023): Strain gauges + IMU + Unity, 97% offline but 74% real-time accuracy

> Novel aspect: This work is the first to use fingertip torque on the brush handle as a proxy for axial bristle force, avoiding bristle-level instrumentation entirely.

### C. Child-Centric Interfaces
- Kim et al. (2009): 3D visualization coaching via IMU
- Marcon et al. (2016): Computer vision + fiducial markers for tracking

### D. Identified Gap
No prior system delivers a fully integrated, low-cost toothbrush estimating both orientation and force and translating both into child-appropriate real-time feedback in a single embedded device.

---

## III. System Design and Implementation

### A. Design Objectives
1. Real-time detection of excessively strong brushing
2. Monitoring of brush orientation relative to 45-degree baseline
3. Multimodal audio and visual feedback recognizable by children
4. Low cost and complexity for accessible deployment

### B. Sensing Architecture

**Orientation:** MPU9255 IMU on the handle. Raw data fused via Madgwick's AHRS in quaternion form (avoids gimbal lock). System enforces +/-15 degree angular tolerance around 45 degree gingival angle (30 degree total deadband). Angle violation triggers buzzer.

**Force:** FSR beneath index-finger rest zone. RC timing method: 0.1 uF capacitor charged through FSR for 2 ms, then discharged for 901 us. ADC reads residual voltage converted via piecewise lookup table to force in Newtons. Threshold: 2.0 N.

### C. Microcontroller and Signal Processing

ESP32 WROOM-32 manages all processing:
- ADC sampling of FSR (RC timing method)
- I2C communication with MPU9255 and SSD1306 OLED
- Madgwick filter computation (beta = 0.1)
- LED and buzzer control
- Threshold logic
- Bluetooth communication

Dual-frequency non-blocking loop:
- 100 Hz IMU update (micros-based, 10 ms)
- 12.5 Hz UI/feedback update (millis-based, 80 ms)

### D. Feedback Actuators

| Actuator | Trigger | Response |
|----------|---------|----------|
| Red LED | Force >= 2.0 N | LED ON |
| Active Buzzer | Angle > +/-15 deg for > 500 ms | Buzzer ON |
| 0.91" OLED SSD1306 | Continuous | Live pitch + force telemetry |
| MIT App Inventor App | Caregiver control | Audio tracks + volume + 2-min timer |

---

## IV. Experimental Validation

### A. Force Sensor Calibration

| Load | Force (N) | Voltage |
|------|-----------|---------|
| 0 g | 0.0 N | ~3.27 V |
| 50 g | 0.5 N | ~2.44 V |
| 100 g | 1.0 N | ~1.99 V |
| 200 g | 2.0 N | ~1.65 V (Threshold) |
| 400 g | 4.0 N | ~1.14 V |
| 800 g | 8.0 N | ~0.54 V |

Consistent threshold detection at 2.0 N setpoint with <8% repeatability error.

### B. Orientation Tracking Accuracy

Fixed at known angles using precision goniometer jig:
- Mean Absolute Angular Error: **2.3 degrees +/- 0.8 degrees**
- Dynamic tracking lag (~2 Hz brushing cadence): < 50 ms
- Filter convergence time: ~2 seconds

### C. End-to-End System Latency

- Average sensor-event to LED actuation latency: **68 ms +/- 12 ms**
- Within perceptually acceptable range (~100 ms) for behavioral guidance

---

## V. Discussion

### A. Performance

FSR-fused force proxy reliably classifies safe vs. excessive force. IMU pipeline achieves 2.3 degree mean error within +/-15 degree tolerance. The indirect FSR approach is a deliberate tradeoff: maintains hygiene and standard brush compatibility at the cost of absolute force measurement accuracy.

Combined audio-visual feedback suits children's attention spans. 2-minute timer enforces adequate duration. DFPlayer Mini operates offline for low-resource deployment.

### B. Limitations
1. Lab validation only - no child user participants
2. No oral quadrant localization
3. Grip-style sensitivity affects FSR readings
4. Bluetooth audio latency not characterized

### C. Future Directions
1. Pediatric user study (ages 5-12)
2. Magnetometer-assisted spatial localization
3. Miniaturized electronics with wireless charging
4. Long-term field deployment study

---

## VI. Conclusion

A smart pediatric toothbrush has been designed, implemented, and technically validated using FSR force proxy and MPU9255 IMU with Madgwick AHRS. The ESP32-based system with MIT App Inventor Bluetooth app corrects children's brushing behavior via lightweight, low-cost embedded architecture.

Key results:
- Mean angular error: 2.3 degrees (within +/-15 degree tolerance)
- Force detection repeatability: <8% error
- End-to-end feedback latency: 68 ms

This work provides an open-architecture, affordable platform filling the specific gap of simultaneous pediatric force and orientation feedback without proprietary hardware ecosystems.

---

## References

[1] Q. Yang et al., "OpenBrush: An open-source manual toothbrush for force and motion sensing," Proc. UbiComp Companion, 2025.
[2] D. Jacobson et al., "Evaluating child toothbrushing behavior changes associated with a mobile game app," Pediatr. Dent., vol. 41, no. 4, pp. 299-303, 2019.
[3] N. Fraihat et al., "Clinical effectiveness and cost-effectiveness of oral-health promotion in dental caries prevention among children," Int. J. Environ. Res. Public Health, vol. 16, no. 15, 2019.
[4] British Dental Association, "Cost of living crisis leaves children's oral health on the line," BDA News, 2023.
[5] V. Maini et al., "Artificial-intelligence-based smart toothbrushes for oral health and patient education: A review," Hygiene, vol. 5, no. 1, 2025.
[6] K. France et al., "Caregivers' experiences with smart toothbrush use by children with autism spectrum disorder," Children, vol. 11, no. 4, 2024.
[7] M. Marcon, A. Sarti, and S. Tubaro, "Toothbrush motion analysis to help children learn proper tooth brushing," Comput. Vis. Image Underst., vol. 148, pp. 34-45, 2016.
[8] K.-S. Kim et al., "Interactive toothbrushing education by a smart toothbrush system via 3D visualization," Comput. Methods Programs Biomed., vol. 96, no. 2, pp. 125-132, 2009.
[9] K.-H. Lee et al., "Tooth brushing pattern classification using three-axis accelerometer and magnetic sensor," Proc. 29th IEEE EMBS, 2007.
[10] Z. Hussain et al., "Do you brush your teeth properly? An off-body sensor-based approach," Proc. IEEE ICDH, 2021.
[11] M. Akhtaruzzaman, "Force-sensitive classic toothbrush: System analysis, design, and simulation," Electrica, vol. 21, no. 2, 2021.
[12] H. C. Wang et al., "Machine learning-based toothbrushing region recognition," Biosensors, vol. 15, no. 12, 2025.
[13] H. Li and L. Jing, "3D Monitoring of Toothbrushing Regions and Force Using Multimodal Sensors and Unity," IEEE Access, vol. 11, 2023.
[14] S. O. H. Madgwick, A. J. L. Harrison, and R. Vaidyanathan, "Estimation of IMU and MARG orientation using a gradient descent algorithm," Proc. IEEE ICORR, 2011.
