#pragma once

#include <Arduino.h>

// Wrapper for DFPlayer PRO (DF1201S) using AT commands
class AudioPlayer {
public:
  AudioPlayer(uint8_t txPin, uint8_t rxPin);

  // Initialize Serial1 and the DFPlayer
  // Returns true if init succeeded
  bool begin();

  void setVolume(uint8_t vol);      // 0..30
  void playTrack(uint16_t track);   // track number starting at 1
  void pause();
  void stop();                      // just an alias for pause()
  void playFile(const String& path); // play by file path
  bool isReady() const { return _ready; }

private:
  uint8_t _txPin;
  uint8_t _rxPin;
  bool _ready;
  
  void sendATCommand(const String& cmd);
};
