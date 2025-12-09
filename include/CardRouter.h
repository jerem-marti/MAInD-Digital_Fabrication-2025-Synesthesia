#pragma once

#include <Arduino.h>

// Map a card UID string to a track number
// Returns 0 if card is unknown
uint16_t trackForUID(const String &uid);
