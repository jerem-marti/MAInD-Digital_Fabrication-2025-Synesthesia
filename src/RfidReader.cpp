#include "RfidReader.h"

RfidReader::RfidReader(uint8_t ssPin, uint8_t rstPin)
  : _ssPin(ssPin),
    _rstPin(rstPin),
    _mfrc522(ssPin, rstPin) {}

void RfidReader::begin() {
  // SPI0 default pins on Pico 2:
  // SCK  = GP18
  // MOSI = GP19
  // MISO = GP16
  SPI.begin();
  _mfrc522.PCD_Init();
  delay(50);

  Serial.println("RfidReader: RC522 initialized");
}

bool RfidReader::readCard(String &uidOut) {
  uidOut = "";

  // No new card
  if (!_mfrc522.PICC_IsNewCardPresent()) {
    return false;
  }

  // Cannot read card
  if (!_mfrc522.PICC_ReadCardSerial()) {
    return false;
  }

  // Build UID string "AA:BB:CC:DD"
  for (byte i = 0; i < _mfrc522.uid.size; i++) {
    if (_mfrc522.uid.uidByte[i] < 0x10) {
      uidOut += "0";
    }
    uidOut += String(_mfrc522.uid.uidByte[i], HEX);
    if (i < _mfrc522.uid.size - 1) {
      uidOut += ":";
    }
  }

  uidOut.toUpperCase();

  // Don't halt the card - just stop crypto
  // This allows the card to be read again in the next loop
  _mfrc522.PCD_StopCrypto1();

  return true;
}

bool RfidReader::isCardPresent() {
  // Simple approach: try to detect a new card presence
  // This will detect when card first appears
  bool present = _mfrc522.PICC_IsNewCardPresent();
  
  // If detected, we need to select it to keep the reader ready
  if (present) {
    _mfrc522.PICC_ReadCardSerial();
  }
  
  return present;
}
