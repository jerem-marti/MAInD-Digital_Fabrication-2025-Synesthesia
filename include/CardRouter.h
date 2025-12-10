/**
 * @file CardRouter.h
 * @brief Maps RFID card UIDs to music track numbers
 * @author Jérémy Martin, generated with GitHub Copilot and ChatGPT
 * @date 2025
 * 
 * This module provides the mapping between RFID card UIDs and their
 * corresponding music track numbers. Each RFID card acts as a "vinyl record"
 * that triggers a specific song when placed on the reader.
 * 
 * Usage:
 * 1. Read the UID from your RFID cards using Serial.println()
 * 2. Add each UID to the trackForUID() function in CardRouter.cpp
 * 3. Assign a track number (1-9999) to each UID
 * 4. Return 0 for unknown cards (no playback)
 * 
 * Track Numbering:
 * - Track 1 corresponds to file /0001.mp3 on the SD card
 * - Track 2 corresponds to file /0002.mp3
 * - Track 0 is reserved for "unknown card" (no playback)
 * 
 * Example Mapping:
 * - UID "C1:98:CC:E4" → Track 6 → Plays /0006.mp3
 * - UID "B1:A0:CC:E4" → Track 2 → Plays /0002.mp3
 * - Unknown UID       → Track 0 → No playback
 */

#pragma once

#include <Arduino.h>

/**
 * @brief Map a card UID string to a track number
 * 
 * Looks up the provided UID in the internal mapping table and returns
 * the corresponding track number. Unknown UIDs return 0.
 * 
 * @param uid Card UID in format "AA:BB:CC:DD" (uppercase hex with colons)
 * @return Track number (1-9999) for known cards, 0 for unknown cards
 */
uint16_t trackForUID(const String &uid);
