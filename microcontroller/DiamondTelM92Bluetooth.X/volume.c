
#include "volume.h"
#include "storage.h"
#include "mcc_generated_files/spi1.h"
#include "mcc_generated_files/pin_manager.h"

/**
 * Module state.
 */
static struct {
  /**
   * True if audio output is enabled.
   */
  bool isEnabled;
  /**
   * Current actual output volume level set on the digital potentiometer.
   */
  VOLUME_Level currentLevel;
  /**
   * Current volume mode.
   */
  VOLUME_Mode currentMode; 
} module;

/**
 * Digital potentiometer wiper position values, indexed by VOLUME_Level.
 * 
 * NOTE: The entry for index 0 (VOLUME_Level_OFF) is never used because audio 
 *       input is completely disconnect for that volume level. 
 * 
 *        
 */
static const uint16_t wiperPositionLookup[] = { 
  // The entry for index 0 (VOLUME_Level_OFF) is never used because audio 
  // input is completely disconnected for that volume level. 
  0,
  0, 8, 20, 48, 72, 96, 160
};


/**
 * Sets the MCP4151 digital potentiometer to produce the desired sound level
 * via SPI commands.
 * 
 * Does nothing if the desired sound level already matches the current sound 
 * level.
 * 
 * @param level - The desired sound level.
 */
static void setPotentiometerLevel(VOLUME_Level level) {
  if (level > VOLUME_Level_MAX) {
    level = VOLUME_Level_MAX;
  }

  if (level == module.currentLevel) {
    return;
  }
  
  uint16_t cmdSetWiperPosition = wiperPositionLookup[level];
 
  // Begin SPI communication to the potentiometer
  SPI1_CS_DPOT_SetLow();

  if (level == VOLUME_Level_OFF) {
    // Write to TCON register
    SPI1_ExchangeByte(0x40);
    // Hardware Shutdown (disconnect terminal A and set wiper == terminal B)
    SPI1_ExchangeByte(0b00000111);
  } else {
    // Write wiper position
    SPI1_ExchangeByte(cmdSetWiperPosition >> 8);
    SPI1_ExchangeByte(cmdSetWiperPosition & 0xFF);
    
    if (module.currentLevel == VOLUME_Level_OFF) {
      // Write to TCON register
      SPI1_ExchangeByte(0x40);
      // Disable Hardware Shutdown
      SPI1_ExchangeByte(0b00001111);
    }
  }
  
  // End SPI communication
  SPI1_CS_DPOT_SetHigh();
  module.currentLevel = level;
}

void VOLUME_Initialize(void) {
  SPI1_Open(SPI1_DEFAULT);
  module.currentLevel = 0xFF;
  module.currentMode = 0xFF;
  VOLUME_Disable();
  VOLUME_SetMode(VOLUME_Mode_SPEAKER);
}

void VOLUME_Enable(void) {
  module.isEnabled = true;
  setPotentiometerLevel(VOLUME_GetLevel(module.currentMode));
}

void VOLUME_Disable(void) {
  module.isEnabled = false;
  setPotentiometerLevel(VOLUME_Level_OFF);
}

void VOLUME_SetMode(VOLUME_Mode mode) {
  if (mode == module.currentMode) {
    return;
  }
  
  module.currentMode = mode;
  
  if (module.isEnabled) {
    setPotentiometerLevel(VOLUME_GetLevel(mode));
  }
}

VOLUME_Level VOLUME_GetLevel(VOLUME_Mode mode) {
  return STORAGE_GetVolumeLevel(mode);
}

void VOLUME_SetLevel(VOLUME_Mode mode, VOLUME_Level level) {
  if (module.isEnabled && (mode == module.currentMode)) {
    setPotentiometerLevel(level);
  }
  
  STORAGE_SetVolumeLevel(mode, level);
}

