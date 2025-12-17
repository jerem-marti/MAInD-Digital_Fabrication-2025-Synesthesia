# Setup Guide: From Clone to Working System

Follow this guide to get the vinyl player working from scratch after cloning the repository.

## Prerequisites

- **Hardware:**
  - Raspberry Pi Pico 2
  - RC522 RFID reader module
  - DFPlayer PRO (DF1201S) audio module
  - SD card (with MP3 files in format /0001.mp3, /0002.mp3, etc.)
  - Jumper wires and breadboard
  - USB cable for Pico 2
  - (Optional) Potentiometer for volume control

- **Software:**
  - VS Code with PlatformIO extension
  - Python 3.8+
  - Git

## Step 1: Clone the Repository

```bash
git clone <your-repo-url>
cd vinyl-player
```

## Step 2: Install PlatformIO

1. Open VS Code
2. Go to Extensions (Ctrl+Shift+X)
3. Search for "PlatformIO IDE"
4. Click Install
5. Wait for installation to complete (may take several minutes)
6. Reload VS Code

## Step 3: Open Project in PlatformIO

1. In VS Code, go to **File → Open Folder**
2. Select the `vinyl-player` directory
3. PlatformIO will auto-detect the project
4. Wait for indexing to complete (bottom status bar will show "✓")

## Step 4: Install Libraries

Open a terminal in VS Code (Ctrl+`) and run:

```bash
pio pkg install
```

This downloads:
- MFRC522 library (for RC522 RFID reader)
- DFRobot_DF1201S library (for DFPlayer PRO)

**Estimated time:** 2–3 minutes

## Step 5: Apply Critical Library Patch ⚠️

**IMPORTANT:** This step is necessary for the code to compile.

The DFRobot_DF1201S library has a compatibility issue with Arduino-Pico. We need to fix one line:

1. Navigate to: `.pio/libdeps/pico2/DFRobot_DF1201S/src/DFRobot_DF1201S.cpp`
2. Find the function `getPlayMode()` (around line 200–250)
3. Locate this line:
   ```cpp
   playMode = str[10];  // ❌ This causes ambiguous assignment error
   ```
4. Change it to:
   ```cpp
   playMode += str[10];  // ✅ Fixed for Arduino-Pico compatibility
   ```
5. Save the file

**Why?** Arduino-Pico's String class doesn't support direct char-to-String assignment in this context. The `+=` operator works around this limitation.

## Step 6: Wire Hardware

### RC522 RFID Reader Wiring

Connect to **SPI0 pins** on Pico 2:

| RC522 Pin | Pico 2 Pin | Color (typical) |
|-----------|-----------|-----------------|
| VCC       | 3V3       | Red             |
| GND       | GND       | Black           |
| SCK       | GP18      | Orange          |
| MOSI      | GP19      | Yellow          |
| MISO      | GP16      | Green           |
| SDA       | GP17      | Blue            |
| RST       | GP20      | Purple          |

**Diagram:**
```
Pico 2                    RC522
3V3 ────────────────→ VCC
GND ────────────────→ GND
GP18 (SCK) ─────────→ SCK
GP19 (MOSI) ────────→ MOSI
GP16 (MISO) ────────→ MISO
GP17 (CS) ──────────→ SDA
GP20 (RST) ─────────→ RST
```

### DFPlayer PRO Wiring

Connect to **Serial1 (UART0)** pins:

| DFPlayer Pin | Pico 2 Pin | Notes |
|-------------|-----------|-------|
| VCC / VIN   | 5V        | Or 3.3V (check module specs) |
| GND         | GND       | Common ground |
| RX          | GP13      | Pico TX |
| TX          | GP12      | Pico RX |

**Diagram:**
```
Pico 2                    DFPlayer PRO
GP13 (TX) ──────────→ RX
GP12 (RX) ──────────→ TX
5V ─────────────────→ VCC/VIN
GND ────────────────→ GND
```

### Optional: Potentiometer (Volume Control)

| Potentiometer | Pico 2 Pin |
|---------------|-----------|
| Left pin      | GND       |
| Middle pin    | GP26      |
| Right pin     | 3V3       |

## Step 7: Prepare SD Card

1. Format SD card as **FAT32**
2. Create MP3 files with naming convention:
   ```
   /0001.mp3
   /0002.mp3
   /0003.mp3
   ...
   /0999.mp3
   ```
3. Place files in **root directory** (not in subfolders)
4. Insert SD card into DFPlayer PRO

## Step 8: Update Card Mappings

1. Open `src/CardRouter.cpp`
2. This file contains the mapping from RFID card UIDs to track numbers
3. You'll populate this with your actual card UIDs (see Step 10)

## Step 9: Build the Firmware

In VS Code terminal (Ctrl+`):

```bash
pio run -e pico2
```

**Expected output:**
```
....
Compiling .pio/build/pico2/src/main.cpp.o
Linking .pio/build/pico2/firmware.elf
Building .pio/build/pico2/firmware.uf2
...
==== [SUCCESS] Took X.XX seconds ====
```

If you get compilation errors, ensure the DFRobot patch (Step 5) was applied correctly.

## Step 10: Upload Firmware

### Manual Method (Recommended for Windows)

1. **Put Pico 2 into BOOTSEL mode:**
   - Hold BOOTSEL button
   - Plug in USB cable (or press RESET while holding BOOTSEL)
   - Release BOOTSEL
   - Pico appears as a USB mass storage device (check File Explorer)

2. **Copy firmware file:**
   ```bash
   copy .pio/build/pico2/firmware.uf2 X:
   ```
   (Replace `X:` with your Pico drive letter)

3. **Reboot normally:**
   - Unplug USB cable
   - Plug back in (without BOOTSEL held)
   - Firmware loads automatically

## Step 11: Discover Card UIDs

1. **Open Serial Monitor:**
   - In VS Code, click the **Serial Monitor** button in the PlatformIO toolbar (bottom bar)
   - Set baud rate to **115200**

2. **Scan your RFID cards:**
   - Present each card to the RC522 reader
   - Serial Monitor will print the UID (format: `AA:BB:CC:DD`)
   - Example output:
     ```
     Card detected: C1:98:CC:E4
     ```

3. **Note down each UID** and decide which track it should play

## Step 12: Update CardRouter

Now that you know your card UIDs, add them to `src/CardRouter.cpp`:

```cpp
uint16_t trackForUID(const String &uid) {
  // Your actual cards:
  if (uid == "C1:98:CC:E4") return 1;   // This card plays track 1
  if (uid == "B1:A0:CC:E4") return 2;   // This card plays track 2
  if (uid == "E1:96:CC:E4") return 3;   // This card plays track 3
  
  // Add more cards here...
  
  // Unknown card
  return 0;  // No playback for unknown cards
}
```

**Important:** 
- Track 1 = plays `/0001.mp3`
- Track 2 = plays `/0002.mp3`
- Return 0 for unknown cards (prevents accidental playback)

## Step 13: Rebuild and Test

1. **Rebuild firmware with new card mappings:**
   ```bash
   pio run -e pico2
   ```

2. **Upload to Pico (repeat Step 10)**

3. **Test the system:**
   - Open Serial Monitor (115200 baud)
   - Present a known card to the reader
   - Music should play from the DFPlayer
   - Remove the card → music pauses
   - Present the same card again → music resumes

4. **Troubleshoot if needed** (see README.md troubleshooting section)

## Verification Checklist

- [ ] PlatformIO installed and project opens
- [ ] Libraries installed (`pio pkg install` completed)
- [ ] DFRobot patch applied (line 200+ in DF1201S.cpp)
- [ ] Hardware wired correctly (check each connection twice)
- [ ] Firmware builds without errors (`pio run -e pico2`)
- [ ] Firmware uploaded to Pico 2 (via BOOTSEL method)
- [ ] Serial Monitor shows RFID reader initialized
- [ ] Card UIDs discovered and noted
- [ ] CardRouter.cpp updated with your card UIDs
- [ ] Music plays when card is presented
- [ ] Music pauses when card is removed

## Common Issues During Setup

### "Cannot find serial device"
- Ensure Pico 2 is plugged in
- Try a different USB cable
- Check Device Manager for "USB Serial Device"

### "Build fails with 'playMode' error"
- DFRobot patch not applied (Step 5)
- Check file path: `.pio/libdeps/pico2/DFRobot_DF1201S/src/DFRobot_DF1201S.cpp`
- Ensure you changed `=` to `+=`

### "RFID reader not detected"
- Verify SPI wiring (pins 16, 18, 19)
- Check 3.3V power to RC522
- Try swapping SS (GP17) and RST (GP20) connections

### "DFPlayer not responding"
- Verify UART wiring (GP12, GP13)
- Check DFPlayer power (5V, not 3.3V)
- Ensure SD card is inserted and formatted
- Try pressing the play button on DFPlayer PCB

## Next Steps

Once the system is working:

1. **Experiment with volume control:**
   - Uncomment potentiometer code in `main.cpp` (if GPIO26 available)
   - Volume maps to range 1–25

2. **Add more cards:**
   - Scan new cards, get UID
   - Add to `CardRouter.cpp`
   - Rebuild and upload

3. **Optimize:**
   - Adjust debounce threshold in `main.cpp` if needed
   - Fine-tune volume mapping
   - Consider adding LED feedback

## Support

For detailed information about:
- **Code architecture:** See `README.md`
- **Hardware details:** See `README.md` "Hardware Setup"
- **Individual modules:** See code documentation in `include/` and `src/`
- **Troubleshooting:** See `README.md` "Troubleshooting"

---

**Setup Time Estimate:** 30–45 minutes (including hardware wiring)  
**Status:** This guide is verified and tested
