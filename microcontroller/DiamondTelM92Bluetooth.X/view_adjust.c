/** 
 * @file
 * @author Jeff Lau
 *
 * UI module for adjusting the LCD viewing angle
 */
#include "view_adjust.h"
#include "storage.h"
#include "sound.h"

/**
 * Module state.
 */
static struct {
  /**
   * Return callback function pointer.
   */
  VIEW_ADJUST_ReturnCallback returnCallback;
  
  /**
   * Current LCD view angle.
   */
  uint8_t currentViewAngle;
} module;

/**
 * Get the view angle number to display to the suer for the current LCD view angle.
 * @return the view angle number to display to the suer for the current LCD view angle.
 */
static char getDisplayViewAngle(void) {
  return '0' + (8 - module.currentViewAngle);
}

/**
 * Display the current LCD view angle.
 */
void displayViewAngle(void) {
  HANDSET_DisableTextDisplay();
  HANDSET_ClearText();
  HANDSET_PrintString("VIEW   ANGLE ");
  HANDSET_PrintChar(getDisplayViewAngle());
  HANDSET_EnableTextDisplay();
}

/**
 * Update the display of the LCD view angle to match the current value.
 */
void updateDisplayedViewAngle(void) {
  HANDSET_SetLcdViewAngle(module.currentViewAngle);
  HANDSET_PrintCharAt(getDisplayViewAngle(), 0);
}

void VIEW_ADJUST_Start(VIEW_ADJUST_ReturnCallback returnCallback) {
  module.returnCallback = returnCallback;
  module.currentViewAngle = STORAGE_GetLcdViewAngle();
  displayViewAngle();
}

void VIEW_ADJUST_HANDSET_EventHandler(HANDSET_Event const* event) {
  if (event->type != HANDSET_EventType_BUTTON_DOWN) {
    return;
  }
  
  HANDSET_Button const button = event->button;
  
  if (button == HANDSET_Button_CLR) {
    SOUND_PlayButtonBeep(button, false);
    HANDSET_CancelCurrentButtonHoldEvents();
    STORAGE_SetLcdViewAngle(module.currentViewAngle);
    module.returnCallback();
  } else if (button == HANDSET_Button_UP) {
    // NOTE: The displayed view angle is in revers order from the raw view angle,
    //       so up is down and down is up.    
    if (module.currentViewAngle > 0) {
      SOUND_PlayButtonBeep(button, false);
      --module.currentViewAngle;
      updateDisplayedViewAngle();
    }
  } else if (button == HANDSET_Button_DOWN) {
    // NOTE: The displayed view angle is in revers order from the raw view angle,
    //       so up is down and down is up.    
    if (module.currentViewAngle < 7) {
      SOUND_PlayButtonBeep(button, false);
      ++module.currentViewAngle;
      updateDisplayedViewAngle();
    }
  }
}

