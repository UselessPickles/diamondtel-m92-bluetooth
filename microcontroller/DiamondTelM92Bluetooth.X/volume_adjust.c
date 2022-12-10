#include "volume_adjust.h"
#include "sound.h"
#include "timeout.h"

typedef enum State {
  State_IDLE,
  State_ADJUSTING_DOWN,
  State_ADJUSTING_UP,
} State;

/**
 * Module state.
 */
static struct {
  VOLUME_ADJUST_ReturnCallback returnCallback;
  State state;
  timeout_t stateTimeout;
  VOLUME_Mode volumeAdjustMode;
} module;

#define VOLUME_ADJUST_IDLE_TIMEOUT (75)
#define VOLUME_ADJUST_REPEAT_DELAY (50)

char const* const volumeModeLabels[] = {
  "ALERT",
  "SP   ",
  "MUSIC",
  "HS   ",
  "HF   ",
  "TONE "
};

static void playVolumeAdjustSound(VOLUME_Mode volumeMode) {
  if (VOLUME_GetLevel(volumeMode) == VOLUME_Level_OFF) {
    SOUND_Stop(SOUND_Channel_FOREGROUND);
    return;
  }
  
  switch (volumeMode) {
    case VOLUME_Mode_ALERT:
      SOUND_PlayEffect(SOUND_Channel_FOREGROUND, SOUND_Target_SPEAKER, volumeMode, SOUND_Effect_ALERT, true);
      break;
    case VOLUME_Mode_SPEAKER:
    case VOLUME_Mode_GAME_MUSIC:
      SOUND_PlaySingleTone(SOUND_Channel_FOREGROUND, SOUND_Target_SPEAKER, volumeMode, TONE_LOW, 0);
      break;
    case VOLUME_Mode_TONE:
      SOUND_PlayDualTone(SOUND_Channel_FOREGROUND, SOUND_Target_SPEAKER, volumeMode, TONE_LOW, TONE_HIGH, 300);
      break;
    case VOLUME_Mode_HANDSET:
    case VOLUME_Mode_HANDS_FREE:
      SOUND_PlaySingleTone(SOUND_Channel_FOREGROUND, SOUND_Target_EAR, volumeMode, TONE_LOW, 0);
      break;
  }
}

static void adjustVolumeLevel(VOLUME_Mode volumeMode, bool up) {
  VOLUME_Level volumeLevel = VOLUME_GetLevel(volumeMode);
  
  if (up) {
    if (volumeLevel < VOLUME_Level_MAX) {
      ++volumeLevel;
    }
  } else {
    uint8_t minLevel = (volumeMode > VOLUME_Mode_GAME_MUSIC);
    
    if (volumeLevel > minLevel) {
      --volumeLevel;
    }
  }
  
  VOLUME_SetLevel(volumeMode, volumeLevel);
  playVolumeAdjustSound(volumeMode);
}

static void displayVolumeLevel(VOLUME_Mode volumeMode) {
  VOLUME_Level volumeLevel = VOLUME_GetLevel(volumeMode);
  
  HANDSET_DisableTextDisplay();
  HANDSET_ClearText();
  HANDSET_PrintString(volumeModeLabels[volumeMode]);
  HANDSET_PrintCharN(' ', 2);
  
  if (!volumeLevel) {
    HANDSET_PrintString("OFF    ");
  } else {
    HANDSET_PrintCharN('-', volumeLevel);
    HANDSET_PrintCharN(' ', VOLUME_Level_MAX - volumeLevel);
  }
  
  HANDSET_EnableTextDisplay();
}

static void displayVolumeLevelChange(VOLUME_Mode volumeMode) {
  VOLUME_Level volumeLevel = VOLUME_GetLevel(volumeMode);
  
  if (!volumeLevel) {
    HANDSET_PrintStringAt("OFF", 6);
  } else if (volumeLevel == 1) {
    HANDSET_PrintStringAt("-  ", 6);
  } else {
    HANDSET_PrintStringAt("-- ", 8 - volumeLevel);
  }
}

static void continueVolumeAdjust(bool up) {
  adjustVolumeLevel(module.volumeAdjustMode, up);
  displayVolumeLevelChange(module.volumeAdjustMode);
  TIMEOUT_Start(&module.stateTimeout, VOLUME_ADJUST_REPEAT_DELAY);
  module.state = up ? State_ADJUSTING_UP : State_ADJUSTING_DOWN;
}


void VOLUME_ADJUST_Start(VOLUME_Mode volumeMode, bool isInitialAdjustUp, VOLUME_ADJUST_ReturnCallback returnCallback) {
  module.volumeAdjustMode = volumeMode;
  module.returnCallback = returnCallback;
  adjustVolumeLevel(volumeMode, isInitialAdjustUp);
  displayVolumeLevel(volumeMode);
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

void VOLUME_ADJUST_Timer10MS_event(void) {
  TIMEOUT_Timer_event(&module.stateTimeout);
}

void VOLUME_ADJUST_HANDSET_EventHandler(HANDSET_Event const* event) {
  bool const isButtonDown = (event->type == HANDSET_EventType_BUTTON_DOWN);
  
  if (!isButtonDown && (event->type != HANDSET_EventType_BUTTON_UP)) {
    return;
  }
  
  uint8_t const button = event->button;
  
  switch (button) {
    case HANDSET_Button_CLR:
      if (isButtonDown) {
        SOUND_PlayButtonBeep(button, true);
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
