#include <Arduino.h>
#include "RfidReader.h"
#include "AudioPlayer.h"
#include "CardRouter.h"

// RC522 pins on Pico 2
// SPI0 default pins are used for SCK, MOSI, MISO:
//   SCK  = GP18
//   MOSI = GP19
//   MISO = GP16
// We only need to define SS and RST.
const uint8_t RFID_SS_PIN  = 17;  // RC522 SDA / SS
const uint8_t RFID_RST_PIN = 20;  // RC522 RST

// DFPlayer pins on Pico 2 - UART0 (Serial1)
// GP12 = UART0 TX, GP13 = UART0 RX
const uint8_t DF_TX_PIN = 12;  // Pico GP12 (TX) -> DFPlayer RX
const uint8_t DF_RX_PIN = 13;  // Pico GP13 (RX) <- DFPlayer TX

// Potentiometer for volume control
const uint8_t POT_PIN = 26;    // GP26 (ADC0) - analog input
const uint8_t MIN_VOLUME = 1;  // Minimum volume
const uint8_t MAX_VOLUME = 25; // Maximum volume

RfidReader  rfid(RFID_SS_PIN, RFID_RST_PIN);
AudioPlayer audio(DF_TX_PIN, DF_RX_PIN);

String currentUID = "";       // Currently playing track's UID
int lastVolume = -1;
bool isPlaying = false;       // Track if music is currently playing
int missedReads = 0;          // Count consecutive failed card reads
const int REMOVAL_THRESHOLD = 5;  // Number of failed reads before considering card removed

void setup() {
  // Built-in LED for debugging (blinks to show life)
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  
  Serial.begin(115200);
  
  // Wait up to 3 seconds for Serial, but don't block forever
  unsigned long start = millis();
  while (!Serial && (millis() - start) < 3000) {
    delay(100);
  }
  
  Serial.println();
  Serial.println("Pico 2 RFID Jukebox starting...");
  
  digitalWrite(LED_BUILTIN, LOW);  // LED off after Serial ready

  // Initialize potentiometer pin
  pinMode(POT_PIN, INPUT);
  
  rfid.begin();
  audio.begin();  // if this fails, audio.isReady() will be false

  if (!audio.isReady()) {
    Serial.println("Warning: DFPlayer not ready. RFID will still work.");
  }
}

void loop() {
  // Read potentiometer and update volume if changed
  int potValue = analogRead(POT_PIN);
  int currentVolume = map(potValue, 0, 1023, MIN_VOLUME, MAX_VOLUME);
  
  if (currentVolume != lastVolume) {
    lastVolume = currentVolume;
    audio.setVolume(currentVolume);
    Serial.print("Volume: ");
    Serial.println(currentVolume);
  }
  
  // Try to read card
  String uid;
  bool cardDetected = rfid.readCard(uid);
  
  if (cardDetected) {
    // Card detected this loop iteration
    missedReads = 0;  // Reset missed read counter
    
    if (uid != currentUID) {
      // New/different card
      currentUID = uid;
      Serial.print("Card detected. UID = ");
      Serial.println(uid);
      
      uint16_t track = trackForUID(uid);
      
      if (track == 0) {
        Serial.println("No track mapped for this card.");
        if (isPlaying) {
          audio.pause();  // Only pause if something is actually playing
        }
        isPlaying = false;
      } else {
        Serial.print("Playing track ");
        Serial.println(track);
        audio.playTrack(track);
        isPlaying = true;
      }
    }
    // Same card - still present, keep playing
  } else {
    // No card detected this loop
    if (currentUID != "") {
      // We had a card before
      missedReads++;
      
      // Only consider card removed after multiple consecutive misses
      if (missedReads >= REMOVAL_THRESHOLD) {
        Serial.println("Card removed - pausing music.");
        if (isPlaying) {
          audio.pause();  // Only pause if music is actually playing
        }
        isPlaying = false;
        currentUID = "";
        missedReads = 0;
      }
    }
  }
  
  delay(100);  // Poll every 100ms
}
