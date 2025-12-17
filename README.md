# RFID Vinyl Player with DFPlayer PRO

A Raspberry Pi Pico 2 jukebox that plays different music tracks based on RFID cards. Each card acts as a "vinyl record" that triggers a specific song.

## Project Overview

### Goal
Create an interactive music player where:
- **RC522 RFID reader** detects when cards are placed on the reader
- **DFPlayer PRO (DF1201S)** plays the corresponding track from is internal memory
- **Potentiometer** controls volume
- **Pico 2** orchestrates the entire system using Arduino framework

### Architecture

The code is organized into modular components:

| Module | Purpose |
|--------|---------|
| **RfidReader** | Handles RC522 communication and returns card UID as a formatted string |
| **AudioPlayer** | Wraps DFPlayer PRO and manages track playback via AT commands |
| **CardRouter** | Maps RFID card UIDs to track numbers |
| **main.cpp** | Application logic and state machine |

## Hardware Setup

### RC522 RFID Reader → Pico 2

Uses **SPI0** with default pins for simplicity:

| RC522 Pin | Function | Pico 2 Pin | Notes |
|-----------|----------|-----------|-------|
| SDA / SS | Chip Select | **GP17** | RFID_SS_PIN |
| SCK | Clock | **GP18** | SPI0 default |
| MOSI | Data In | **GP19** | SPI0 default |
| MISO | Data Out | **GP16** | SPI0 default |
| RST | Reset | **GP20** | RFID_RST_PIN |
| IRQ | Interrupt | **GP21** | (optional, not used) |
| 3.3V | Power | **3V3** | - |
| GND | Ground | **GND** | - |

**All RFID signals are clustered in GP16–21 for easy wiring.**

### DFPlayer PRO → Pico 2

Uses **Serial1 (UART0)** for AT command communication:

| Connection | Pico 2 Pin | Notes |
|-----------|-----------|-------|
| DFPlayer RX | **GP13** | Pico TX |
| DFPlayer TX | **GP12** | Pico RX |
| VCC / VIN | **5V or 3.3V** | Check your power design |
| GND | **GND** | Common ground |

**Baud Rate:** 115200 (fixed for DFPlayer PRO AT commands)

### Potentiometer (Volume Control)

| Connection | Pico 2 Pin | Notes |
|-----------|-----------|-------|
| Potentiometer (middle) | **GP26** | ADC0, analog input |
| GND | **GND** | - |
| 3.3V | **3V3** | - |

Maps ADC reading (0–1023) to volume (1–25).

## Software Setup

### Prerequisites

- **PlatformIO** (VS Code extension)
- **Python 3.8+** (for PlatformIO CLI)
- **Git** (for version control)

### Installation Steps

1. **Clone the repository:**
   ```bash
   git clone <repo-url>
   cd <project-folder>
   ```

2. **Open in VS Code with PlatformIO:**
   - Install the PlatformIO extension in VS Code
   - Open the project folder
   - PlatformIO will auto-detect `platformio.ini`

3. **Install dependencies:**
   ```bash
   pio pkg install
   ```

4. **Apply DFRobot library patch (CRITICAL):**
   
   The DFRobot_DF1201S library requires a one-line fix for Arduino-Pico compatibility.
   
   **File:** `.pio/libdeps/pico2/DFRobot_DF1201S/src/DFRobot_DF1201S.cpp`
   
   **Find line in `getPlayMode()` function:**
   ```cpp
   playMode = str[10];  // ❌ Ambiguous assignment
   ```
   
   **Change to:**
   ```cpp
   playMode += str[10];  // ✅ Correct for Arduino-Pico
   ```
   
   **Why:** Arduino-Pico's String class doesn't support direct assignment from char in this context. The `+=` operator works around this.

5. **Build the project:**
   ```bash
   pio run -e pico2
   ```

### Uploading Firmware

Since automatic upload via picotool can be unreliable on Windows, use the manual method:

1. **Put Pico 2 into BOOTSEL mode:**
   - Hold the BOOTSEL button
   - Plug in USB cable (or press RESET while holding BOOTSEL)
   - Release BOOTSEL
   - Pico 2 appears as a USB drive

2. **Copy firmware to Pico:**
   ```bash
   copy .pio/build/pico2/firmware.uf2 X:
   ```
   (Replace `X:` with your Pico drive letter)

3. **Reboot normally:**
   - Unplug USB
   - Plug back in (without BOOTSEL)
   - Firmware loads automatically

### Serial Monitor Setup

For debugging and UID discovery:

1. **Check COM port:**
   - Windows: Device Manager → Ports (COM & LPT)
   - Look for "USB Serial Device"

2. **Update `platformio.ini` (optional):**
   ```ini
   monitor_speed = 115200
   monitor_port = COM6  ; Change to your port
   ```

3. **View output:**
   - PlatformIO: Click "Serial Monitor" in bottom taskbar
   - Baud: 115200
   - You'll see UID output when cards are scanned

## Code Structure

### `include/RFIDReader.h` & `src/RFIDReader.cpp`

**Purpose:** Wrapper around MFRC522 library for RC522 communication

**Key methods:**
- `begin()` - Initialize SPI and RC522
- `readCard(String &uidOut)` - Read card UID, returns true if successful
- `isCardPresent()` - Check card presence (lightweight check)

**Important:** Does NOT halt the card after reading. This allows continuous detection on the same card (vinyl player behavior).

**Usage:**
```cpp
RfidReader rfid(17, 20);  // SS=GP17, RST=GP20
rfid.begin();
String uid;
if (rfid.readCard(uid)) {
  Serial.println("Card: " + uid);  // e.g., "C1:98:CC:E4"
}
```

### `include/AudioPlayer.h` & `src/AudioPlayer.cpp`

**Purpose:** Wrapper around DFPlayer PRO using AT commands

**Key methods:**
- `begin()` - Initialize Serial1 and DFPlayer PRO at 115200 baud
- `playTrack(uint16_t track)` - Play track number (1–9999)
- `setVolume(uint8_t vol)` - Set volume (0–30)
- `pause()` - Toggle play/pause (via AT+PLAY=PP)
- `isReady()` - Check initialization status

**Important AT commands used:**
```
AT+FUNCTION=MUSIC     - Switch to music mode
AT+PLAYMODE=1         - Loop single track infinitely
AT+PROMPT=OFF         - Disable voice announcements
AT+VOL=n              - Set volume (0–30)
AT+PLAYFILE=/path.mp3 - Play specific file
AT+PLAY=PP            - Toggle play/pause
```

**Usage:**
```cpp
AudioPlayer audio(13, 12);  // TX=GP13, RX=GP12
audio.begin();
audio.playTrack(1);         // Plays /0001.mp3
audio.setVolume(15);        // Volume 15/30
```

### `include/CardRouter.h` & `src/CardRouter.cpp`

**Purpose:** Map RFID card UIDs to track numbers

**Function:**
```cpp
uint16_t trackForUID(const String &uid);
```

**Returns:**
- Track number (1–9999) if card is known
- 0 if card is unknown (no playback)

**To add cards:**
1. Scan a card and note its UID from Serial Monitor
2. Add to `CardRouter.cpp`:
   ```cpp
   if (uid == "C1:98:CC:E4") return 6;  // Card plays track 6
   ```

### `src/main.cpp`

**Purpose:** Application logic and state machine

**Setup:**
- Initialize Serial (115200 baud)
- Wait briefly for Serial Monitor connection
- Initialize RFID reader
- Initialize audio player

**Loop:**
- Poll RFID reader for new cards
- Look up track number via CardRouter
- Play track if known, pause if unknown
- Debounce by tracking the last UID (prevent rapid re-triggers)

**State variables:**
```cpp
String currentUID;        // Last scanned card UID
bool isPlaying;          // Current playback state
uint8_t missedReads;     // Counter for card removal detection
uint8_t lastVolume;      // Last set volume level
```

## DFPlayer File Setup

### File naming convention

Files must be named with leading zeros:
```
/0001.mp3
/0002.mp3
/0003.mp3
...
/0999.mp3
```

### Directory structure on DFPlayer memory

```
DFPlayer Root
├── 0001.mp3
├── 0002.mp3
├── 0003.mp3
└── 0006.mp3
```

Place all files in the **root directory** (not in subfolders).

### Maximum files

DFPlayer PRO can handle up to 9999 files (0001–9999).

## Configuration

### Adjustable Parameters in `main.cpp`

```cpp
// RFID settings
const uint8_t RFID_SS_PIN = 17;      // Chip Select
const uint8_t RFID_RST_PIN = 20;     // Reset

// DFPlayer settings
const uint8_t DF_TX_PIN = 13;        // Pico TX
const uint8_t DF_RX_PIN = 12;        // Pico RX

// Volume control
const uint8_t POT_PIN = 26;           // Potentiometer ADC
const uint8_t MIN_VOLUME = 1;         // Minimum (0–30)
const uint8_t MAX_VOLUME = 25;        // Maximum (0–30)

// Debouncing
const uint8_t REMOVAL_THRESHOLD = 5;  // Consecutive missed reads before "removed"
const uint16_t POLL_INTERVAL = 100;   // Milliseconds between reads
```

## Troubleshooting

### "No RFID chip detected"

**Possible causes:**
1. RC522 wiring incorrect (check SPI pins: 16, 18, 19)
2. SS or RST pin wrong (check GP17, GP20)
3. SPI bus not initialized
4. Card not supported (try another card)

**Debug steps:**
- Check Serial Monitor for RC522 init messages
- Verify RC522 power (3.3V, not 5V)
- Confirm GND is connected

### "DFPlayer not responding"

**Possible causes:**
1. Serial1 pins wrong (should be GP12/13)
2. Baud rate not 115200
3. DFPlayer power insufficient
4. DFRobot library patch not applied

**Debug steps:**
- Verify UART pins in code (GP12 RX, GP13 TX)
- Check DFPlayer boot messages in Serial Monitor
- Ensure DFRobot patch is applied (see Software Setup)

### "Cards readable only first time"

**This is expected behavior** if using standard MFRC522 code that calls `PICC_HaltA()`. Our code does NOT call this, allowing re-reading of the same card.

If you want the opposite behavior, uncomment the `PICC_HaltA()` call in `RFIDReader.cpp`.

### "Compilation errors about String ambiguity"

**Make sure DFRobot patch is applied:**
```cpp
// ❌ Wrong (ambiguous):
playMode = str[10];

// ✅ Correct:
playMode += str[10];
```

See Software Setup section for file location.

## Reusing This Project

### When cloning:

1. **Install PlatformIO** in VS Code
2. **Install libraries:**
   ```bash
   pio pkg install
   ```
3. **Apply DFRobot patch** (see Software Setup section)
4. **Update card mappings** in `CardRouter.cpp` with your card UIDs
5. **Adjust pins** in `main.cpp` if your hardware differs
6. **Build and upload** firmware
7. **Test** by scanning cards and monitoring Serial output

### Common modifications:

- **Change volume range:** Adjust `MIN_VOLUME` and `MAX_VOLUME` in `main.cpp`
- **Add potentiometer:** Uncomment volume code in `main.cpp`
- **Different pin layout:** Update `RFID_SS_PIN`, `RFID_RST_PIN`, `DF_TX_PIN`, `DF_RX_PIN`
- **Disable voice announcements:** Already disabled via `AT+PROMPT=OFF` in `AudioPlayer.cpp`

## Dependencies

| Library | Version | Purpose |
|---------|---------|---------|
| MFRC522 | ^1.4.11 | RC522 RFID reader |
| DFRobot_DF1201S | Latest | DFPlayer PRO control |

Both are specified in `platformio.ini` and auto-installed by PlatformIO.

## References

- **Raspberry Pi Pico 2:** https://www.raspberrypi.com/products/raspberry-pi-pico-2/
- **MFRC522 Library:** https://github.com/miguelbalboa/rfid
- **DFRobot_DF1201S:** https://github.com/DFRobot/DFRobot_DF1201S
- **Max Gerhardt's Platform:** https://github.com/maxgerhardt/platform-raspberrypi
- **Arduino-Pico Core:** https://github.com/earlephilhower/arduino-pico

## Author

SUPSI MAIND Project – Digitally Designed Objects & Fast Prototyping

---

**Last Updated:** December 2025  
**Status:** Fully functional – all features working
