#include "CardRouter.h"

// Replace the examples below with your real UIDs
// You can print UIDs with Serial and then paste them here.

uint16_t trackForUID(const String &uid) {

  if (uid == "C1:9E:CC:E4") return 1;  // play track 1
  if (uid == "B1:A0:CC:E4") return 2;  // play track 1
  if (uid == "E1:96:CC:E4") return 3;  // play track 1
  if (uid == "91:A2:CC:E4") return 4;  // play track 1
  if (uid == "F1:94:CC:E4") return 5;  // play track 1
  if (uid == "C1:98:CC:E4") return 6;  // play track 1

  // Unknown card
  return 0;
}
