/** 
 * @file
 * @author Jeff Lau
 * 
 * UI module for adjusting volume level.
 */

#include "volume_adjust.h"
#include "../sound/sound.h"
#include "../util/timeout.h"

typedef enum State {
  State_IDLE,
  State_ADJUSTING_DOWN,
  State_ADJUSTING_UP,
} State;

/**
 * Module state.
 */
static struct {
  /**
   * Return callback function pointer.
   */
  VOLUME_ADJUST_ReturnCallback returnCallback;
  /**
   * Current state.
   */
  State state;
  /**
   * Context sensitive timeout used for state transitions.
   */
  timeout_t stateTimeout;
  /**
   * The volume mode that is being adjusted.
   */
  VOLUME_Mode volumeAdjustMode;
} module;

/**
 * Amount of time (in hundredths of a second) to wait after the last user input
 * before automatically exiting volume adjust.
 */
#define VOLUME_ADJUST_IDLE_TIMEOUT (75)

/**
 * Amount of time (in hundredths of a second) to wait while the up/down button
 * is pressed before repeating the same volume adjustment.
 */
#define VOLUME_ADJUST_REPEAT_DELAY (50)

/**
 * Display labels for each volume mode, ordered such that the VOLUME_Mode enum 
 * value is used as an index into this array.
 */
char const* const volumeModeLabels[] = {
  "ALERT",
  "SP   ",
  "MUSIC",
  "HS   ",
  "HF   ",
  "TONE "
};

/**
 * Play the volume adjust sound effect for current volume mode.
 */
static void playVolumeAdjustSound(void) {
  if (SOUND_GetVolumeLevel(module.volumeAdjustMode) == VOLUME_Level_OFF) {
    SOUND_Stop(SOUND_Channel_FOREGROUND);
    return;
  }
  
  switch (module.volumeAdjustMode) {
    case VOLUME_Mode_ALERT:
      SOUND_PlayEffect(SOUND_Channel_FOREGROUND, SOUND_Target_SPEAKER, module.volumeAdjustMode, SOUND_Effect_ALERT, true);
      break;
    case VOLUME_Mode_SPEAKER:
    case VOLUME_Mode_GAME_MUSIC:
      SOUND_PlaySingleTone(SOUND_Channel_FOREGROUND, SOUND_Target_SPEAKER, module.volumeAdjustMode, TONE_LOW, 0);
      break;
    case VOLUME_Mode_TONE:
      SOUND_PlayDualTone(SOUND_Channel_FOREGROUND, SOUND_Target_SPEAKER, module.volumeAdjustMode, TONE_LOW, TONE_HIGH, 300);
      break;
    case VOLUME_Mode_HANDSET:
    case VOLUME_Mode_HANDS_FREE:
      SOUND_PlaySingleTone(SOUND_Channel_FOREGROUND, SOUND_Target_EAR, module.volumeAdjustMode, TONE_LOW, 0);
      break;
  }
}

/**
 * Adjust the volume up or down.
 * 
 * This function:
 * - Updates the volume level of the current volume mode.
 * - Plays the appropriate sound for volume adjustment of the current volume mode.
 * 
 * @param up - True to adjust volume up. False to adjust volume down.
 */
static void adjustVolumeLevel(bool up) {
  VOLUME_Level volumeLevel = SOUND_GetVolumeLevel(module.volumeAdjustMode);
  
  if (up) {
    if (volumeLevel < VOLUME_Level_MAX) {
      ++volumeLevel;
    }
  } else {
    uint8_t minLevel = (module.volumeAdjustMode > VOLUME_Mode_GAME_MUSIC);
    
    if (volumeLevel > minLevel) {
      --volumeLevel;
    }
  }
  
  SOUND_SetVolumeLevel(module.volumeAdjustMode, volumeLevel);
  playVolumeAdjustSound();
}

/**
 * Fully updates the display to show the current volume mode level.
 */
static void displayVolumeLevel(void) {
  VOLUME_Level volumeLevel = SOUND_GetVolumeLevel(module.volumeAdjustMode);
  
  HANDSET_DisableTextDisplay();
  HANDSET_ClearText();
  HANDSET_PrintString(volumeModeLabels[module.volumeAdjustMode]);
  HANDSET_PrintCharN(' ', 2);
  
  if (!volumeLevel) {
    HANDSET_PrintString("OFF    ");
  } else {
    HANDSET_PrintCharN('-', volumeLevel);
    HANDSET_PrintCharN(' ', VOLUME_Level_MAX - volumeLevel);
  }
  
  HANDSET_EnableTextDisplay();
}

/**
 * Minimally updates the display of the volume level to handle a change in 
 * volume level. Assumptions:
 * - The current volume mode level was already displayed.
 * - The volume level has not changed by more than 1 in either direction since
 *   the display was last updated.
 */
static void displayVolumeLevelChange(void) {
  VOLUME_Level volumeLevel = SOUND_GetVolumeLevel(module.volumeAdjustMode);
  
  if (!volumeLevel) {
    HANDSET_PrintStringAt("OFF", 6);
  } else if (volumeLevel == 1) {
    HANDSET_PrintStringAt("-  ", 6);
  } else {
    HANDSET_PrintStringAt("-- ", 8 - volumeLevel);
  }
}

/**
 * Continues volume adjustment in the specified direction (up/down button
 * press handler after we're already in the volume adjust UI).
 * 
 * @param up - True to adjust volume up. False to adjust volume down.
 */
static void continueVolumeAdjust(bool up) {
  adjustVolumeLevel(up);
  displayVolumeLevelChange();
  TIMEOUT_Start(&module.stateTimeout, VOLUME_ADJUST_REPEAT_DELAY);
  module.state = up ? State_ADJUSTING_UP : State_ADJUSTING_DOWN;
}


void VOLUME_ADJUST_Start(VOLUME_Mode volumeMode, bool isInitialAdjustUp, VOLUME_ADJUST_ReturnCallback returnCallback) {
  module.volumeAdjustMode = volumeMode;
  module.returnCallback = returnCallback;
  adjustVolumeLevel(isInitialAdjustUp);
  displayVolumeLevel();
  TIMEOUT_Start(&module.stateTimeout, VOLUME_ADJUST_REPEAT_DELAY);
  module.state = isInitialAdjustUp ? State_ADJUSTING_UP : State_ADJUSTING_DOWN;
}

void VOLUME_ADJUST_Task(void) {
  TIMEOUT_Task(&module.stateTimeout);
  
  switch (module.state) {
    case State_IDLE:
      if (!TIMEOUT_IsPending(&module.stateTimeout)) {
        module.returnCallback();
      }
      break;
      
    case State_ADJUSTING_DOWN:
    case State_ADJUSTING_UP:
      if (!TIMEOUT_IsPending(&module.stateTimeout)) {
        continueVolumeAdjust(module.state - State_ADJUSTING_DOWN);
      }
      break;
  }
}

void VOLUME_ADJUST_Timer10MS_Interrupt(void) {
  TIMEOUT_Timer_Interrupt(&module.stateTimeout);
}

void VOLUME_ADJUST_HANDSET_EventHandler(HANDSET_Event const* event) {
  bool const isButtonDown = (event->type == HANDSET_EventType_BUTTON_DOWN);
  
  if (!isButtonDown && (event->type != HANDSET_EventType_BUTTON_UP)) {
    // Only handle button press and release events.
    return;
  }
  
  uint8_t const button = event->button;
  
  switch (button) {
    case HANDSET_Button_CLR:
      if (isButtonDown) {
        SOUND_PlayButtonBeep(button, false);
        HANDSET_CancelCurrentButtonHoldEvents();
        module.returnCallback();
      }
      break;

    case HANDSET_Button_UP:  
    case HANDSET_Button_DOWN:
      if (isButtonDown) {
        continueVolumeAdjust(button == HANDSET_Button_UP);
      } else {
        SOUND_Stop(SOUND_Channel_FOREGROUND);
        TIMEOUT_Start(&module.stateTimeout, VOLUME_ADJUST_IDLE_TIMEOUT);
        module.state = State_IDLE;
      }
  }
}
