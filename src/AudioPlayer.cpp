/**
 * @file AudioPlayer.cpp
 * @brief Implementation of DFPlayer PRO control via AT commands
 * @author Jérémy Martin, generated with GitHub Copilot and ChatGPT
 * @date 2025
 */

#include "AudioPlayer.h"

// Constructor: Store pin configuration
AudioPlayer::AudioPlayer(uint8_t txPin, uint8_t rxPin)
  : _txPin(txPin),
    _rxPin(rxPin),
    _ready(false) {}

/**
 * Send an AT command to the DFPlayer PRO module
 * All commands require \r\n termination (provided by println)
 * Small delay ensures command is processed before next operation
 */
void AudioPlayer::sendATCommand(const String& cmd) {
  Serial1.println(cmd);  // AT commands need \r\n (println adds them)
  delay(50);  // Wait for command to process
}

/**
 * Initialize the DFPlayer PRO in music playback mode
 * 
 * Initialization sequence:
 * 1. Configure Serial1 pins for UART communication
 * 2. Start UART at 115200 baud (DFPlayer PRO fixed rate)
 * 3. Wait 1 second for DFPlayer boot sequence
 * 4. Switch to MUSIC mode (from other modes like BT/AUX)
 * 5. Configure single track looping (PLAYMODE=1)
 * 6. Set default volume
 * 
 * @return Always returns true after initialization
 */
bool AudioPlayer::begin() {
  Serial.println("AudioPlayer: initializing DFPlayer PRO...");

  // Configure Serial1 pins (Pico 2 supports flexible UART pin assignment)
  Serial1.setTX(_txPin);
  Serial1.setRX(_rxPin);
  Serial1.begin(115200);  // DFPlayer PRO uses 115200 baud
  
  delay(1000);  // Wait for DFPlayer to boot (critical for reliable operation)
  
  // Switch to MUSIC function (DFPlayer will announce "music" if speaker connected)
  Serial.println("AudioPlayer: switching to MUSIC mode...");
  sendATCommand("AT+FUNCTION=MUSIC");
  delay(500);  // Wait for mode switch to complete
  
  // Set play mode to repeat one song infinitely
  // Mode 1 = Loop single track (vs mode 0 = play once)
  sendATCommand("AT+PLAYMODE=1");
  
  // Set initial volume (middle range)
  setVolume(15);
  
  _ready = true;
  Serial.println("AudioPlayer: DFPlayer PRO ready.");
  return true;
}

/**
 * Set the output volume level
 * DFPlayer PRO volume range: 0 (mute) to 30 (maximum)
 * Values above 30 are clamped to prevent distortion
 */
void AudioPlayer::setVolume(uint8_t vol) {
  if (vol > 30) vol = 30;  // Clamp to maximum
  sendATCommand("AT+VOL=" + String(vol));
}

/**
 * Play a specific track by number
 * 
 * Converts track number to filename format /000X.mp3
 * For example:
 *   Track 1   → /0001.mp3
 *   Track 42  → /0042.mp3
 *   Track 999 → /0999.mp3
 * 
 * After starting playback, re-enforces PLAYMODE=1 to ensure
 * the track loops infinitely (workaround for DFPlayer reset behavior)
 * 
 * @param track Track number (1-9999). Track 0 is ignored (used for "unknown card")
 */
void AudioPlayer::playTrack(uint16_t track) {
  if (track == 0) return;  // Skip invalid/unknown tracks
  
  Serial.print("AudioPlayer: play track ");
  Serial.println(track);
  
  // Build filename with leading zeros: /0001.mp3, /0002.mp3, etc.
  String filename = "/";
  if (track < 10) filename += "000";
  else if (track < 100) filename += "00";
  else if (track < 1000) filename += "0";
  filename += String(track) + ".mp3";
  
  // Play the constructed filename
  playFile(filename);
  
  // Re-enforce loop mode after starting playback
  // This ensures the track continues looping even after DFPlayer state changes
  delay(100);
  sendATCommand("AT+PLAYMODE=1");  // Repeat single track
}

/**
 * Play a file by its full path on the SD card
 * Path format: "/0001.mp3" or "/folder/song.mp3"
 */
void AudioPlayer::playFile(const String& path) {
  sendATCommand("AT+PLAYFILE=" + path);
}

/**
 * Toggle play/pause state using PP command
 * 
 * Note: In main application, we set volume to 0 instead of using this
 * to avoid toggle state confusion when cards are removed/reinserted
 */
void AudioPlayer::pause() {
  Serial.println("AudioPlayer: pause");
  sendATCommand("AT+PLAY=PP");  // Toggle play/pause
}

/**
 * Stop playback (alias for pause, as DFPlayer doesn't distinguish)
 */
void AudioPlayer::stop() {
  pause();
}
