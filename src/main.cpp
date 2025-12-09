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

RfidReader  rfid(RFID_SS_PIN, RFID_RST_PIN);
AudioPlayer audio(DF_TX_PIN, DF_RX_PIN);

String lastUID = "";

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

  rfid.begin();
  audio.begin();  // if this fails, audio.isReady() will be false

  if (!audio.isReady()) {
    Serial.println("Warning: DFPlayer not ready. RFID will still work.");
  }
}

void loop() {
  String uid;
  if (!rfid.readCard(uid)) {
    // No card present
    return;
  }

  Serial.print("Card detected. UID = ");
  Serial.println(uid);

  // Avoid retriggering same card if it stays on the reader
  if (uid == lastUID) {
    Serial.println("Same card as before, ignoring.");
    return;
  }

  lastUID = uid;

  // Map UID to track
  uint16_t track = trackForUID(uid);

  if (track == 0) {
    Serial.println("No track mapped for this card. Pausing audio.");
    audio.pause();
  } else {
    Serial.print("Mapped to track ");
    Serial.println(track);
    audio.playTrack(track);
  }
}
