#include "AudioPlayer.h"

AudioPlayer::AudioPlayer(uint8_t txPin, uint8_t rxPin)
  : _txPin(txPin),
    _rxPin(rxPin),
    _ready(false) {}

void AudioPlayer::sendATCommand(const String& cmd) {
  Serial1.println(cmd);  // AT commands need \r\n (println adds them)
  delay(50);  // Wait for command to process
}

bool AudioPlayer::begin() {
  Serial.println("AudioPlayer: initializing DFPlayer PRO...");

  // Configure Serial1 pins
  Serial1.setTX(_txPin);
  Serial1.setRX(_rxPin);
  Serial1.begin(115200);  // DFPlayer PRO uses 115200 baud
  
  delay(1000);  // Wait for DFPlayer to boot
  
  // Switch to MUSIC function (will say "music")
  Serial.println("AudioPlayer: switching to MUSIC mode...");
  sendATCommand("AT+FUNCTION=MUSIC");
  delay(500);  // Wait for mode switch to complete
  
  // Set play mode to repeat one song infinitely
  sendATCommand("AT+PLAYMODE=1");
  
  // Set initial volume
  setVolume(15);
  
  _ready = true;
  Serial.println("AudioPlayer: DFPlayer PRO ready.");
  return true;
}

void AudioPlayer::setVolume(uint8_t vol) {
  if (vol > 30) vol = 30;
  sendATCommand("AT+VOL=" + String(vol));
}

void AudioPlayer::playTrack(uint16_t track) {
  if (track == 0) return;
  Serial.print("AudioPlayer: play track ");
  Serial.println(track);
  
  // Play file by number (files must be named 0001.mp3, 0002.mp3, etc.)
  String filename = "/";
  if (track < 10) filename += "000";
  else if (track < 100) filename += "00";
  else if (track < 1000) filename += "0";
  filename += String(track) + ".mp3";
  
  playFile(filename);
  
  // Ensure repeat mode is set for this track
  delay(100);
  sendATCommand("AT+PLAYMODE=1");  // Repeat single track
}

void AudioPlayer::playFile(const String& path) {
  sendATCommand("AT+PLAYFILE=" + path);
}

void AudioPlayer::pause() {
  Serial.println("AudioPlayer: pause");
  sendATCommand("AT+PLAY=PP");  // Toggle play/pause
}

void AudioPlayer::stop() {
  pause();
}
