/**
 * @file RFIDReader.cpp
 * @brief Implementation of RC522 RFID reader control
 * @author Jérémy Martin, generated with GitHub Copilot and ChatGPT
 * @date 2025
 */

#include "RfidReader.h"

// Constructor: Initialize with pin configuration
RfidReader::RfidReader(uint8_t ssPin, uint8_t rstPin)
  : _ssPin(ssPin),
    _rstPin(rstPin),
    _mfrc522(ssPin, rstPin) {}

/**
 * Initialize the SPI interface and RC522 module
 * 
 * Uses default Pico 2 SPI0 pins:
 * - SCK:  GP18 (clock)
 * - MOSI: GP19 (master out, slave in)
 * - MISO: GP16 (master in, slave out)
 * Plus the custom SS and RST pins specified in constructor
 */
void RfidReader::begin() {
  // SPI0 default pins on Pico 2:
  // SCK  = GP18
  // MOSI = GP19
  // MISO = GP16
  SPI.begin();
  _mfrc522.PCD_Init();  // Initialize the RC522 chip
  delay(50);  // Allow RC522 to stabilize

  Serial.println("RfidReader: RC522 initialized");
}

/**
 * Read a card's UID and format it as a string
 * 
 * Process:
 * 1. Check if a new card is present in the RF field
 * 2. Attempt to read the card's serial number (UID)
 * 3. Format UID bytes as colon-separated hex (e.g., "C1:98:CC:E4")
 * 4. Stop crypto communication but DON'T halt the card
 * 
 * The key difference from typical RFID usage: We do NOT call PICC_HaltA()
 * after reading. This allows the same card to be detected again on the
 * next loop iteration, which is essential for knowing when the card is
 * removed from the reader (vinyl player behavior).
 * 
 * @param uidOut Reference to string that will receive the formatted UID
 * @return true if card was successfully read, false if no card or read failed
 */
bool RfidReader::readCard(String &uidOut) {
  uidOut = "";  // Clear output string

  // Check if a new card is present in the RF field
  if (!_mfrc522.PICC_IsNewCardPresent()) {
    return false;
  }

  // Attempt to read the card's serial number (UID)
  if (!_mfrc522.PICC_ReadCardSerial()) {
    return false;
  }

  // Build UID string in format "AA:BB:CC:DD"
  // Most RFID cards have 4-byte UIDs, but some use 7 or 10 bytes
  for (byte i = 0; i < _mfrc522.uid.size; i++) {
    // Add leading zero for single-digit hex values
    if (_mfrc522.uid.uidByte[i] < 0x10) {
      uidOut += "0";
    }
    // Convert byte to hex string
    uidOut += String(_mfrc522.uid.uidByte[i], HEX);
    
    // Add colon separator between bytes (except after last byte)
    if (i < _mfrc522.uid.size - 1) {
      uidOut += ":";
    }
  }

  // Convert to uppercase for consistency (some cards may have lowercase hex)
  uidOut.toUpperCase();

  // Stop the crypto1 cipher but DON'T halt the card
  // This is the key modification that enables continuous detection:
  // - PCD_StopCrypto1() ends the encrypted session
  // - NOT calling PICC_HaltA() allows the card to be read again
  _mfrc522.PCD_StopCrypto1();

  return true;
}

/**
 * Lightweight check for card presence without reading UID
 * 
 * Useful for quick detection without the overhead of UID reading.
 * If a card is detected, we also select it to keep the reader ready.
 * 
 * @return true if a card is present in the RF field
 */
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
