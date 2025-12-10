/**
 * @file CardRouter.cpp
 * @brief Implementation of RFID UID to track number mapping
 * @author Jérémy Martin, generated with GitHub Copilot and ChatGPT
 * @date 2025
 * 
 * This file contains the actual UID-to-track mappings for the vinyl player.
 * To add new cards:
 * 1. Place card on reader and note the UID printed to Serial
 * 2. Add a new if statement comparing the UID
 * 3. Return the desired track number (1-9999)
 * 
 * The track numbers correspond to files on the SD card:
 * - Track 1  → /0001.mp3
 * - Track 10 → /0010.mp3
 * - Track 99 → /0099.mp3
 */

#include "CardRouter.h"

/**
 * Map a card UID to its corresponding track number
 * 
 * This function implements a simple lookup table using if statements.
 * For large numbers of cards, consider using a HashMap or array-based
 * lookup for better performance.
 * 
 * Current mappings (update these with your actual card UIDs):
 * - C1:9E:CC:E4 → Track 1
 * - B1:A0:CC:E4 → Track 2
 * - E1:96:CC:E4 → Track 3
 * - 91:A2:CC:E4 → Track 4
 * - F1:94:CC:E4 → Track 5
 * - C1:98:CC:E4 → Track 6
 * 
 * @param uid The card UID as a formatted string (e.g., "C1:98:CC:E4")
 * @return Track number (1-9999) for known cards, 0 for unknown cards
 */
uint16_t trackForUID(const String &uid) {
  // Known card mappings - replace these with your actual UIDs
  if (uid == "C1:9E:CC:E4") return 1;  // Card 1 plays track 1
  if (uid == "B1:A0:CC:E4") return 2;  // Card 2 plays track 2
  if (uid == "E1:96:CC:E4") return 3;  // Card 3 plays track 3
  if (uid == "91:A2:CC:E4") return 4;  // Card 4 plays track 4
  if (uid == "F1:94:CC:E4") return 5;  // Card 5 plays track 5
  if (uid == "C1:98:CC:E4") return 6;  // Card 6 plays track 6

  // Unknown card - return 0 (no playback)
  // This prevents unexpected behavior when unregistered cards are scanned
  return 0;
}
