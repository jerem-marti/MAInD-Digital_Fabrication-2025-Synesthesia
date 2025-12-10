#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>

class RfidReader {
public:
  RfidReader(uint8_t ssPin, uint8_t rstPin);

  // Call in setup
  void begin();

  // Try to read a card
  // Returns true if a card was read and uidOut contains its UID
  // UID is formatted as "AA:BB:CC:DD"
  bool readCard(String &uidOut);
  
  // Check if any card is present in the field
  // Returns true if a card is detected (doesn't read UID)
  bool isCardPresent();

private:
  uint8_t _ssPin;
  uint8_t _rstPin;
  MFRC522 _mfrc522;
};
