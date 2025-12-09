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

private:
  uint8_t _ssPin;
  uint8_t _rstPin;
  MFRC522 _mfrc522;
};
