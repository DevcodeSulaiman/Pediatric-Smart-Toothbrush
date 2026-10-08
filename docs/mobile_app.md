# MIT App Inventor Companion App Guide

## SmartBrush — Bluetooth Companion App

The companion mobile application for the Pediatric Smart Toothbrush is built with MIT App Inventor and communicates with the ESP32 via Classic Bluetooth (SPP — Serial Port Profile).

---

## Functionality Overview

| Feature | Description |
|---------|-------------|
| Bluetooth connection | Pairs with ESP32 named "SmartBrush" |
| Audio track selection | Buttons to select tracks 1-9 (sends single digit character) |
| Volume control | Plus/minus buttons to adjust DFPlayer volume |
| 2-minute timer | Countdown timer encouraging full brushing session |
| Status display | Connection status and active session indicator |

---

## Bluetooth Communication Protocol

The app communicates with the ESP32 via a single-character command protocol over Bluetooth SPP:

| Character Sent | Effect on Toothbrush |
|----------------|---------------------|
| `+` | Volume up (increments by 1, max 30) |
| `-` | Volume down (decrements by 1, min 0) |
| `1` | Play audio track 1 on DFPlayer Mini |
| `2` | Play audio track 2 on DFPlayer Mini |
| `3` | Play audio track 3 on DFPlayer Mini |
| `4`-`9` | Play audio tracks 4-9 respectively |

The ESP32 listens for these characters in the `handleBTCommands()` function running at 12.5 Hz.

---

## Pairing Instructions

1. Power on the toothbrush (SPST switch ON)
2. The ESP32 broadcasts as Bluetooth device named: **`SmartBrush`**
3. On your Android device, go to **Settings > Bluetooth**
4. Scan for devices and select **SmartBrush**
5. Pair (no PIN required)
6. Open the SmartBrush app
7. Tap the "Connect" button and select SmartBrush from the device list
8. Connection status should show "Connected"

---

## MIT App Inventor — Building from Source

### App Setup

1. Go to [MIT App Inventor](https://ai2.appinventor.mit.edu/)
2. Create a new project or import the `.aia` file if provided
3. The app uses the following components:
   - `BluetoothClient1` — for Bluetooth SPP connection
   - `ListPicker1` — to select paired device
   - `Clock1` — for 2-minute countdown timer
   - `Label` components — status display
   - `Button` components — volume and track controls

### Key App Blocks Logic

**Connection:**
```
when ListPicker1.AfterPicking
    if BluetoothClient1.Connect(ListPicker1.Selection)
        set StatusLabel.Text = "Connected"
    else
        set StatusLabel.Text = "Failed to connect"
```

**Volume Control:**
```
when PlusButton.Click
    BluetoothClient1.SendText("+")

when MinusButton.Click
    BluetoothClient1.SendText("-")
```

**Track Selection:**
```
when Track1Button.Click
    BluetoothClient1.SendText("1")

when Track2Button.Click
    BluetoothClient1.SendText("2")
[... repeat for tracks 3-9]
```

**2-Minute Timer:**
```
Initialize: timerSeconds = 120

when StartButton.Click
    Clock1.TimerEnabled = true
    timerSeconds = 120

when Clock1.Timer  (fires every 1000ms)
    timerSeconds = timerSeconds - 1
    set TimerLabel.Text = timerSeconds + "s"
    if timerSeconds <= 0
        Clock1.TimerEnabled = false
        set StatusLabel.Text = "Brushing Complete!"
```

---

## Audio Content

The DFPlayer Mini plays audio tracks from a FAT32 microSD card. The tracks are selected by sending digit characters `1`-`9` from the app.

**Suggested audio content for children:**
- Upbeat music or nursery rhymes (2-minute duration recommended)
- Educational audio ("Keep brushing the top teeth!")
- Countdown songs

**Audio file preparation:**
1. Format microSD as FAT32
2. Name files: `0001.mp3`, `0002.mp3`, etc. in root directory
3. Any standard MP3 file works (44.1 kHz recommended)

---

## Compatibility

| Platform | Status |
|----------|--------|
| Android | Supported (MIT App Inventor target) |
| iOS | Not supported (MIT App Inventor BT Classic not available on iOS) |

The companion app is designed primarily for Android devices. iOS support would require a different BLE-based communication approach (BLE is available on the ESP32 but requires firmware changes).

---

## Troubleshooting

| Issue | Solution |
|-------|----------|
| Cannot find SmartBrush device | Ensure SPST power switch is ON; try refreshing Bluetooth scan |
| Connected but commands not working | Ensure app has Bluetooth permissions on Android 12+ |
| Volume commands not responding | DFPlayer may not be initialized; check wiring and DFPlayer LED |
| Audio not playing | Verify microSD is FAT32 formatted with correct file naming |
| App disconnects during session | Keep phone within ~10m of toothbrush; avoid interference |
