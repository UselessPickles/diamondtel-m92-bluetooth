/**
 * @file
 * @author Jeff Lau
 *
 * UI module for single alphabetic character input.
 */

#include "char_input.h"
#include "../sound/sound.h"

/**
 * Module state.
 */
static struct {
  /**
   * Return callback function pointer.
   */
  CHAR_INPUT_ReturnCallback returnCallback;
  /**
   * "Prompt cleared" notification callback function pointer.
   * 
   * May be NULL if the parent component does not need this notification.
   */
  CHAR_INPUT_PromptClearedCallback promptClearedCallback;
  /**
   * True if lowercase characters are allowed.
   */
  bool allowLowercase;
  /**
   * True if prompt text is currently displayed (needs to be cleared out
   * upon the first user interaction).
   */
  bool isPromptDisplayed;
  /**
   * The current button that the user has been pressing to cycle through its
   * characters.
   */
  uint8_t currentButton;
  /**
   * The current index into the character lookup array for `currentButton`.
   */
  uint8_t currentButtonCharIndex;
} module;

/**
 * Lookup table of all associated alpha characters for buttons 1-9.
 * 
 * For each button, uppercase characters are listed first, followed by 
 * the lowercase equivalents. This allows access to lowercase characters to be
 * controlled by simply limiting the max index when cycling through the 
 * alpha characters.
 */
static char const ALPHA_LOOKUP[9][6] = {
  {'Q', 'Z', ' ', 'q', 'z', ' '},
  {'A', 'B', 'C', 'a', 'b', 'c'},
  {'D', 'E', 'F', 'd', 'e', 'f'},
  {'G', 'H', 'I', 'g', 'h', 'i'},
  {'J', 'K', 'L', 'j', 'k', 'l'},
  {'M', 'N', 'O', 'm', 'n', 'o'},
  {'P', 'R', 'S', 'p', 'r', 's'},
  {'T', 'U', 'V', 't', 'u', 'v'},
  {'W', 'X', 'Y', 'w', 'x', 'y'},
};


void CHAR_INPUT_Start(
    bool isPromptDisplayed, 
    bool allowLowercase, 
    CHAR_INPUT_ReturnCallback returnCallback,
    CHAR_INPUT_PromptClearedCallback promptClearedCallback
) {
  module.isPromptDisplayed = isPromptDisplayed;
  module.allowLowercase = allowLowercase;
  module.returnCallback = returnCallback;
  module.promptClearedCallback = promptClearedCallback;
  
  module.currentButton = 0;
  
  // Prevent a previously pressed/held button from being interpreted
  // as a char input.
  HANDSET_CancelCurrentButtonHoldEvents();
}

void CHAR_INPUT_HANDSET_EventHandler(HANDSET_Event const* event) {
  HANDSET_Button const button = event->button;
  
  if (button < '1' || button > '9') {
    // We only care about handling buttons 1-9
    return;
  }
  
  if (event->type == HANDSET_EventType_BUTTON_DOWN) {
    SOUND_PlayDTMFButtonBeep(button, false);

    if (button != module.currentButton) {
      module.currentButton = button;
      module.currentButtonCharIndex = 0;
    }

    uint8_t const c = ALPHA_LOOKUP[button - '1'][module.currentButtonCharIndex];

    if (module.isPromptDisplayed) {
      HANDSET_ClearText();
      HANDSET_PrintChar(c);
      module.isPromptDisplayed = false;

      if (module.promptClearedCallback) {
        module.promptClearedCallback();
      }
    } else {
      HANDSET_PrintCharAt(c, 0);
    }
  } else if (event->type == HANDSET_EventType_BUTTON_UP) {
    // Only handle the button release if we are currently keeping track of a 
    // button press/hold.
    if (module.currentButton != 0) {
      HANDSET_PrintCharAt('_', 0);

      // Cycle through to the next alpha character for the next press of this same button
      if (++module.currentButtonCharIndex == (module.allowLowercase ? 6 : 3)) {
        module.currentButtonCharIndex = 0;
      }
    }
  } else if (
      (event->type == HANDSET_EventType_BUTTON_HOLD) &&
      (event->holdDuration == HANDSET_HoldDuration_SHORT)
    ) {
    SOUND_StopButtonBeep();
    SOUND_PlayEffect(SOUND_Channel_FOREGROUND, SOUND_Target_SPEAKER, VOLUME_Mode_SPEAKER, SOUND_Effect_REORDER_TONE, false);
    HANDSET_CancelCurrentButtonHoldEvents();

    // Reset current button state so that a subsequent press of the same button 
    // will restart at the first alpha character for the button.
    module.currentButton = 0;

    module.returnCallback(ALPHA_LOOKUP[button - '1'][module.currentButtonCharIndex]);    
  }
}

