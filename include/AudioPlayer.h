/**
 * @file AudioPlayer.h
 * @brief DFPlayer PRO (DF1201S) driver using AT command protocol
 * @author Jérémy Martin, generated with GitHub Copilot and ChatGPT
 * @date 2025
 * 
 * This class provides a simplified interface to control the DFPlayer PRO
 * audio module using AT commands over UART. The DFPlayer PRO communicates
 * at 115200 baud and requires specific AT commands to switch modes, control
 * playback, and set volume.
 * 
 * Hardware Setup:
 * - TX Pin: Connect to DFPlayer PRO RX (typically GP12)
 * - RX Pin: Connect to DFPlayer PRO TX (typically GP13)
 * - Baud Rate: 115200 (fixed for DFPlayer PRO)
 * 
 * SD Card File Naming:
 * - Files must be named: /0001.mp3, /0002.mp3, /0003.mp3, etc.
 * - Place files in the root directory of the SD card
 * 
 * Important AT Commands Used:
 * - AT+FUNCTION=MUSIC : Switch to music playback mode
 * - AT+PLAYMODE=1     : Set to loop single track infinitely
 * - AT+VOL=n          : Set volume (0-30)
 * - AT+PLAYFILE=path  : Play specific file by path
 * - AT+PLAY=PP        : Toggle play/pause
 */

#pragma once

#include <Arduino.h>

/**
 * @class AudioPlayer
 * @brief Wrapper for DFPlayer PRO (DF1201S) using AT commands
 * 
 * Provides high-level methods to control audio playback on the DFPlayer PRO
 * module. Handles UART communication, command formatting, and timing delays
 * required for reliable operation.
 */
class AudioPlayer {
public:
  /**
   * @brief Constructor
   * @param txPin GPIO pin for UART TX (Pico → DFPlayer RX)
   * @param rxPin GPIO pin for UART RX (Pico ← DFPlayer TX)
   */
  AudioPlayer(uint8_t txPin, uint8_t rxPin);

  /**
   * @brief Initialize Serial1 and configure the DFPlayer PRO
   * 
   * This method:
   * 1. Configures Serial1 with the specified TX/RX pins at 115200 baud
   * 2. Waits for DFPlayer to boot (1 second)
   * 3. Switches to MUSIC mode using AT+FUNCTION=MUSIC
   * 4. Sets playback mode to loop single track (AT+PLAYMODE=1)
   * 5. Sets initial volume to 15
   * 
   * @return true if initialization succeeded, false otherwise
   */
  bool begin();

  /**
   * @brief Set the output volume
   * @param vol Volume level (0-30, where 0 is mute and 30 is maximum)
   */
  void setVolume(uint8_t vol);

  /**
   * @brief Play a specific track by number
   * 
   * Plays the file /000X.mp3 where X is the track number.
   * Also re-enforces AT+PLAYMODE=1 to ensure infinite looping.
   * 
   * @param track Track number (1-9999). 0 is ignored.
   */
  void playTrack(uint16_t track);

  /**
   * @brief Pause/unpause playback
   * 
   * Sends AT+PLAY=PP command to toggle play/pause state.
   * Note: Used carefully in main loop with state tracking to avoid
   * unintended toggle behavior.
   */
  void pause();

  /**
   * @brief Stop playback (alias for pause)
   */
  void stop();

  /**
   * @brief Play a file by its full path
   * @param path Full file path on SD card (e.g., "/0001.mp3")
   */
  void playFile(const String& path);

  /**
   * @brief Check if the audio player is ready
   * @return true if initialization completed successfully
   */
  bool isReady() const { return _ready; }

private:
  uint8_t _txPin;  ///< UART TX pin
  uint8_t _rxPin;  ///< UART RX pin
  bool _ready;     ///< Initialization status flag
  
  /**
   * @brief Send an AT command to the DFPlayer PRO
   * 
   * Appends \r\n (via println) and waits 50ms for command processing.
   * 
   * @param cmd AT command string (without \r\n)
   */
  void sendATCommand(const String& cmd);
};
