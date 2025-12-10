/**
 * @file main.cpp
 * @brief RFID-based vinyl player jukebox for Raspberry Pi Pico 2
 * 
 * This project creates a vinyl-style music player where RFID cards act as "records".
 * Place a card on the reader to play its associated track, which loops continuously
 * until the card is removed. Volume is controlled via a potentiometer.
 * 
 * Hardware:
 * - Raspberry Pi Pico 2
 * - RC522 RFID reader (SPI)
 * - DFPlayer PRO audio module (UART)
 * - Potentiometer for volume control
 * 
 * @author Jérémy Martin, generated with GitHub Copilot and ChatGPT
 * @date December 2025
 */

#include <Arduino.h>
#include "RfidReader.h"
#include "AudioPlayer.h"
#include "CardRouter.h"

// ========== PIN CONFIGURATION ==========

// RC522 RFID Reader pins (SPI0)
// Default SPI pins: SCK=GP18, MOSI=GP19, MISO=GP16
const uint8_t RFID_SS_PIN  = 17;  // RC522 SDA/SS pin
const uint8_t RFID_RST_PIN = 20;  // RC522 RST pin

// DFPlayer PRO Audio Module pins (UART0/Serial1)
const uint8_t DF_TX_PIN = 12;  // Pico GP12 (TX) -> DFPlayer RX
const uint8_t DF_RX_PIN = 13;  // Pico GP13 (RX) <- DFPlayer TX

// Volume control potentiometer
const uint8_t POT_PIN = 26;    // GP26 (ADC0) - analog input
const uint8_t MIN_VOLUME = 1;  // Minimum volume level
const uint8_t MAX_VOLUME = 25; // Maximum volume level

// ========== GLOBAL OBJECTS ==========

RfidReader  rfid(RFID_SS_PIN, RFID_RST_PIN);  // RFID reader instance
AudioPlayer audio(DF_TX_PIN, DF_RX_PIN);      // Audio player instance

// ========== STATE VARIABLES ==========

String currentUID = "";                    // UID of currently playing card
int lastVolume = -1;                      // Last volume setting (for change detection)
bool isPlaying = false;                   // Tracks if music is currently playing
int missedReads = 0;                      // Counter for consecutive failed card reads
const int REMOVAL_THRESHOLD = 5;          // Failed reads needed before card is considered removed

/**
 * @brief Initialize hardware and modules
 * 
 * Sets up serial communication, RFID reader, audio player, and potentiometer.
 * LED blinks during initialization to indicate system is alive.
 */
void setup() {
  // LED indicates system is initializing
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  
  // Initialize serial communication for debugging
  Serial.begin(115200);
  
  // Wait for Serial Monitor (max 3 seconds) but don't block forever
  unsigned long start = millis();
  while (!Serial && (millis() - start) < 3000) {
    delay(100);
  }
  
  Serial.println();
  Serial.println("Pico 2 RFID Jukebox starting...");
  
  digitalWrite(LED_BUILTIN, LOW);  // LED off after initialization
  
  // Initialize potentiometer pin
  pinMode(POT_PIN, INPUT);
  
  // Initialize RFID reader
  rfid.begin();
  
  // Initialize audio player
  audio.begin();
  if (!audio.isReady()) {
    Serial.println("Warning: DFPlayer not ready. RFID will still work.");
  }
}

/**
 * @brief Main loop - handles volume control and card detection
 * 
 * Continuously:
 * 1. Reads potentiometer and updates volume if changed
 * 2. Checks for RFID card presence
 * 3. Plays associated track when new card is detected
 * 4. Pauses music when card is removed
 * 5. Handles card read failures with debouncing
 */
void loop() {
  // ========== VOLUME CONTROL ==========
  // Read potentiometer and update volume if it has changed
  int potValue = analogRead(POT_PIN);
  int currentVolume = map(potValue, 0, 1023, MIN_VOLUME, MAX_VOLUME);
  
  if (currentVolume != lastVolume) {
    lastVolume = currentVolume;
    audio.setVolume(currentVolume);
    Serial.print("Volume: ");
    Serial.println(currentVolume);
  }
  
  // ========== CARD DETECTION ==========
  String uid;
  bool cardDetected = rfid.readCard(uid);
  
  if (cardDetected) {
    // Card successfully read
    missedReads = 0;  // Reset missed read counter
    
    if (uid != currentUID) {
      // New or different card detected
      currentUID = uid;
      Serial.print("Card detected. UID = ");
      Serial.println(uid);
      
      // Look up track number for this card
      uint16_t track = trackForUID(uid);
      
      if (track == 0) {
        // Unknown card - no track mapped
        Serial.println("No track mapped for this card.");
        if (isPlaying) {
          audio.pause();  // Pause only if music is playing
        }
        isPlaying = false;
      } else {
        // Valid card - play associated track
        Serial.print("Playing track ");
        Serial.println(track);
        audio.playTrack(track);
        isPlaying = true;
      }
    }
    // Same card still present - continue playing
  } else {
    // ========== CARD REMOVAL DETECTION ==========
    // No card detected in this iteration
    if (currentUID != "") {
      // We had a card previously
      missedReads++;
      
      // Only consider card removed after multiple consecutive misses
      // This provides debouncing for unreliable RFID reads
      if (missedReads >= REMOVAL_THRESHOLD) {
        Serial.println("Card removed - pausing music.");
        if (isPlaying) {
          audio.pause();  // Pause only if music is playing
        }
        isPlaying = false;
        currentUID = "";
        missedReads = 0;
      }
    }
  }
  
  // Poll every 100ms - balance between responsiveness and CPU usage
  delay(100);
}
