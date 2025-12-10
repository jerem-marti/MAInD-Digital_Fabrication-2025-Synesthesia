/**
 * @file RFIDReader.h
 * @brief RC522 RFID reader driver with continuous card detection support
 * @author Jérémy Martin, generated with GitHub Copilot and ChatGPT
 * @date 2025
 * 
 * This class wraps the MFRC522 library to provide simplified RFID card
 * reading with support for continuous detection (vinyl player behavior).
 * 
 * Hardware Setup:
 * - SPI Interface: SPI0 on Raspberry Pi Pico 2
 *   - SCK:  GP18 (default)
 *   - MOSI: GP19 (default)
 *   - MISO: GP16 (default)
 *   - SS:   Configurable (typically GP17)
 *   - RST:  Configurable (typically GP20)
 * - Voltage: 3.3V (compatible with Pico 2)
 * 
 * Key Features:
 * - Non-halting card detection: Cards remain readable after first read
 * - Presence detection: Check if card is in field without reading UID
 * - UID formatting: Returns formatted strings like "AA:BB:CC:DD"
 * 
 * Important Implementation Detail:
 * This implementation does NOT call PICC_HaltA() after reading, which
 * allows continuous detection of the same card. This is crucial for
 * the vinyl player use case where we need to know when a card is removed.
 */

#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>

/**
 * @class RfidReader
 * @brief Wrapper for RC522 RFID reader with continuous detection
 * 
 * Provides simplified methods to read RFID card UIDs and detect card
 * presence without halting the card communication (allows re-reading).
 */
class RfidReader {
public:
  /**
   * @brief Constructor
   * @param ssPin GPIO pin for SPI Slave Select (chip select)
   * @param rstPin GPIO pin for RFID module reset
   */
  RfidReader(uint8_t ssPin, uint8_t rstPin);

  /**
   * @brief Initialize the RC522 module and SPI interface
   * 
   * Must be called in setup() before using other methods.
   * Initializes SPI bus and performs RC522 self-test.
   */
  void begin();

  /**
   * @brief Attempt to read a card's UID
   * 
   * This method checks for a new card, reads its UID, and formats it
   * as a colon-separated hex string (e.g., "C1:98:CC:E4").
   * 
   * Important: Does NOT halt the card after reading, allowing the same
   * card to be read again on the next call. This enables continuous
   * detection for the vinyl player use case.
   * 
   * @param uidOut Output string that receives the formatted UID
   * @return true if a card was successfully read, false otherwise
   */
  bool readCard(String &uidOut);
  
  /**
   * @brief Check if any card is present in the RFID field
   * 
   * Lightweight method to detect card presence without reading the full UID.
   * Useful for quick presence checks.
   * 
   * @return true if a card is detected, false otherwise
   */
  bool isCardPresent();

private:
  uint8_t _ssPin;      ///< SPI Slave Select pin
  uint8_t _rstPin;     ///< Reset pin
  MFRC522 _mfrc522;    ///< MFRC522 library instance
};
