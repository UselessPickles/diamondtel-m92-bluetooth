/** 
 * @file
 * @author Jeff Lau
 *
 * UI module for ringtone selection.
 */

#include "ringtone_select.h"
#include "ringtone.h"
#include "volume_adjust.h"
#include "sound.h"
#include "storage.h"

/**
 * Module state.
 */
static struct {
  /**
   * Return callback function pointer.
   */
  RINGTONE_SELECT_ReturnCallback returnCallback;
  /**
   * True if the ringtone preview is currently "on".
   */
  bool isPreviewOn;
  /**
   * True if the suer is adjusting volume.
   */
  bool isAdjustingVolume;
  /**
   * The currently selected ringtone.
   */
  RINGTONE_Type currentRingtone;
} module;

/**
 * Display the name of the currently selected ringtone.
 */
static void displayRingtoneName(void) {
  // This function is also used as the return callback from volume adjustment,
  // so clear out this isAdjustingVolume flag.
  module.isAdjustingVolume = false;
  
  HANDSET_DisableTextDisplay();
  HANDSET_ClearText();
  HANDSET_PrintString("RINGER ");
  HANDSET_PrintString(RINGTONE_GetName(module.currentRingtone));
  HANDSET_EnableTextDisplay();
}

void RINGTONE_SELECT_Start(RINGTONE_SELECT_ReturnCallback returnCallback) {
  module.returnCallback = returnCallback;
  module.currentRingtone = STORAGE_GetRingtone();
  module.isAdjustingVolume = false;
  module.isPreviewOn = false;
  displayRingtoneName();
}

void RINGTONE_SELECT_Task(void) {
  if (module.isAdjustingVolume) {
    VOLUME_ADJUST_Task();
  }
}

void RINGTONE_SELECT_Timer10MS_event(void) {
  if (module.isAdjustingVolume) {
    VOLUME_ADJUST_Timer10MS_event();
    return;
  }
}

void RINGTONE_SELECT_HANDSET_EventHandler(HANDSET_Event const *event) {
  if (module.isAdjustingVolume) {
    VOLUME_ADJUST_HANDSET_EventHandler(event);
    return;
  }
  
  if (event->type != HANDSET_EventType_BUTTON_DOWN) {
    // handle button presses only
    return;
  }
  
  HANDSET_Button const button = event->button;
  
  if (button == HANDSET_Button_CLR) {
    RINGTONE_Stop();
    SOUND_PlayButtonBeep(button, false);
    HANDSET_CancelCurrentButtonHoldEvents();
    STORAGE_SetRingtone(module.currentRingtone);
    module.returnCallback();
  } else if (button == HANDSET_Button_RCL) {
    if (module.isPreviewOn) {
      module.isPreviewOn = false;
      RINGTONE_Stop();
    } else {
      module.isPreviewOn = true;
      RINGTONE_Start(module.currentRingtone);
    }
  } else if ((button == HANDSET_Button_UP) || (button == HANDSET_Button_DOWN)) {
    module.isAdjustingVolume = true;
    VOLUME_ADJUST_Start(VOLUME_Mode_ALERT, (button == HANDSET_Button_UP), displayRingtoneName);
  } else if ((button >= '1') && (button - '1' < RINGTONE_COUNT)) {
    module.currentRingtone = button - '1';
    
    if (module.isPreviewOn) {
      RINGTONE_Start(module.currentRingtone);
    } else {
      SOUND_PlayButtonBeep(button, false);
    }
    
    displayRingtoneName();
  }
}