/* 
 * File:   volume.h
 * Author: Jeff
 *
 * API for controlling the volume level of sound to the phone handset.
 * 
 * Separate volume levels are maintained for each of the volume "modes".
 * 
 * Volume can be enabled/disabled independently of current volume mode and 
 * volume level.
 */

#include "volume.h"
#include "storage.h"
#include "timeout.h"
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
  
  /**
   * A timeout used to defer the storing of a volume level change.
   * 
   * This helps avoid excessive EEPROM updates as the user is changing the 
   * volume level.
   */
  timeout_t deferredStoreTimeout;
  
  /**
   * The volume mode of the volume level change whose storage has been deferred.
   */
  VOLUME_Mode deferredStoreMode; 
  
  /**
   * The volume level change whose storage has been deferred.
   */
  VOLUME_Level deferredStoreLevel;
} module;

/**
 * The delay (in hundredths of a second) between a volume level change and 
 * when it is stored.
 */
#define DEFERRED_STORE_TIMEOUT (500)

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
  // 0, 8, 20, 48, 72, 96, 160
  0, 12, 28, 64, 104, 160, 256
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
  TIMEOUT_Cancel(&module.deferredStoreTimeout);
}

void VOLUME_Task(void) {
  if (TIMEOUT_Task(&module.deferredStoreTimeout)) {
    STORAGE_SetVolumeLevel(module.deferredStoreMode, module.deferredStoreLevel);
  }
}

void VOLUME_Timer10MS_event(void) {
  TIMEOUT_Timer_event(&module.deferredStoreTimeout);
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
  // If a deferred storage is pending, and the requested mode matches the mode
  // of the deferred storage, then return the level value that has not yet been
  // stored.
  if (TIMEOUT_IsPending(&module.deferredStoreTimeout) && (mode == module.deferredStoreMode)) {
    return module.deferredStoreLevel;
  } else {
    return STORAGE_GetVolumeLevel(mode);
  }
}

void VOLUME_SetLevel(VOLUME_Mode mode, VOLUME_Level level) {
  if (module.isEnabled && (mode == module.currentMode)) {
    setPotentiometerLevel(level);
  }
  
  // We can only keep track of one deferred storage of volume level change,
  // so if there's a pending deferred storage for a different volume mode, then
  // we must store it now before we setup the deferred storage of this new
  // change
  if (TIMEOUT_IsPending(&module.deferredStoreTimeout) && (mode != module.deferredStoreMode)) {
    STORAGE_SetVolumeLevel(module.deferredStoreMode, module.deferredStoreLevel);
  }

  // Defer the storage of this change
  module.deferredStoreMode = mode;
  module.deferredStoreLevel = level;
  TIMEOUT_Start(&module.deferredStoreTimeout, DEFERRED_STORE_TIMEOUT);
}

