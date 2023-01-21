/**
 * @file
 * @author Jeff Lau
 *
 * UI module for alphanumeric string input.
 */

#include "string_input.h"
#include "char_input.h"
#include "indicator.h"
#include "../sound/sound.h"
#include <string.h>

/**
 * Module state.
 */
static struct {
  /**
   * The string buffer where the user input is maintained.
   */
  char* buffer;
  /**
   * Max allowed length for the user input.
   */
  size_t maxLength;
  /**
   * The prompt text to display when the string input is empty.
   */
  char const* prompt;
  /**
   * True if lowercase characters are allowed.
   */
  bool allowLowercase;
  /**
   * True if the first character of the string input is allowed to be numeric.
   */
  bool allowNumericStart;
  /**
   * Return callback function pointer.
   */
  STRING_INPUT_ReturnCallback returnCallback;
  /**
   * True if the prompt text is currently displayed.
   */
  bool isPromptDisplayed;
  /**
   * Current length of the user input.
   */
  size_t length;
  /**
   * True if the input mode is currently for alphabetic characters.
   * False if the input mode is currently for numeric characters.
   */
  bool isAlphaInput;
} module;

/**
 * Handle a single character input.
 * @param c - The character to be appended to the input string.
 */
static void handleCharInput(char c) {
  // If the input is currently empty, then we first clear the screen and 
  // print the character using "standard" printing, so that the subsequent
  // character printing will properly "push" this character up through the 
  // display positions.
  if (module.length == 0) {
    HANDSET_ClearText();
    HANDSET_PrintChar(c);
  }
  
  // Add the character to the input string buffer
  module.buffer[module.length] = c;
  module.buffer[++module.length] = 0;
  
  if (module.length < module.maxLength) {
    // Print a placeholder `_` character where subsequent character input will 
    // occur. The CHAR_INPUT module will overwrite this character as the user
    // presses buttons.
    HANDSET_PrintChar('_');
  }
}

/**
 * CHAR_INPUT_PromptClearedCallback implementation used to keep 
 * `isPromptDisplayed` state synchronized.
 */
static void handlePromptCleared(void) {
  module.isPromptDisplayed = false;
}

/**
 * Start/reinitialize CHAR_INPUT based on current module state.
 */
static void startCharInput(void) {
  CHAR_INPUT_Start(module.isPromptDisplayed, module.allowLowercase, handleCharInput, handlePromptCleared);
}

/**
 * Display the prompt text.
 */
static void displayPrompt(void) {
  HANDSET_DisableTextDisplay();
  HANDSET_ClearText();
  HANDSET_PrintString(module.prompt);
  HANDSET_EnableTextDisplay();
  
  module.isPromptDisplayed = true;
}

/**
 * Display the current content of the string input buffer.
 */
static void displayBufferContent(void) {
  HANDSET_DisableTextDisplay();
  HANDSET_ClearText();
  HANDSET_PrintString(module.buffer);
  
  if (module.length < module.maxLength) {
    // Print a placeholder `_` character where subsequent character input will 
    // occur. The CHAR_INPUT module will overwrite this character as the user
    // presses buttons.
    HANDSET_PrintChar('_');
  }
  
  HANDSET_EnableTextDisplay();  

  module.isPromptDisplayed = false;
}

void STRING_INPUT_Start(
    char* buffer, 
    size_t maxLength, 
    char const* prompt, 
    bool allowLowercase, 
    bool allowNumericStart, 
    STRING_INPUT_ReturnCallback returnCallback
) {
  module.buffer = buffer;
  module.maxLength = maxLength;
  module.prompt = prompt;
  module.allowLowercase = allowLowercase;
  module.allowNumericStart = allowNumericStart;
  module.returnCallback = returnCallback;
  
  module.length = strlen(buffer);

  // Always start in alphabetic input mode
  module.isAlphaInput = true;
  INDICATOR_StartFlashing(HANDSET_Indicator_FCN);

  if (module.length == 0) {
    displayPrompt();
  } else {
    displayBufferContent();
  }
  
  startCharInput();
}

void STRING_INPUT_HANDSET_EventHandler(HANDSET_Event const* event) {
  bool const isButtonDown = (event->type == HANDSET_EventType_BUTTON_DOWN);
  HANDSET_Button const button = event->button;
  
  if (
      (button == HANDSET_Button_CLR) && 
      (event->type == HANDSET_EventType_BUTTON_HOLD) && 
      (event->holdDuration == HANDSET_HoldDuration_SHORT)
      ) {
    // Short hold of the CLR button clears out the string input and resets
    // to alphabetic input mode.
    module.length = 0;
    module.buffer[0] = 0;
    module.isAlphaInput = true;
    INDICATOR_StartFlashing(HANDSET_Indicator_FCN);
    displayPrompt();
    startCharInput();
    return;
  } else if (isButtonDown) {
    switch (button) {
      case HANDSET_Button_CLR:
        if (module.length == 0) {
          SOUND_PlayButtonBeep(button, false);
          HANDSET_CancelCurrentButtonHoldEvents();

          if (module.isPromptDisplayed) {
            // If we're already displaying the prompt, the the CLR button 
            // exits/cancels string input.
            INDICATOR_StopFlashing(HANDSET_Indicator_FCN, false);
            module.returnCallback(STRING_INPUT_Result_CANCEL, module.buffer);
          } else {
            displayPrompt();
            startCharInput();
          }
        } else {
          SOUND_PlayButtonBeep(button, false);
          
          // Delete a character from the end of the string buffer.
          module.buffer[--module.length] = 0;
          
          if (module.length == 0) {
            // Revert to alphabetic input mode if the input is not allowed
            // to start with a numeric character.
            if ((!module.allowNumericStart) && (!module.isAlphaInput)) {
              module.isAlphaInput = true;
              INDICATOR_StartFlashing(HANDSET_Indicator_FCN);
            }
            
            // Display the prompt, because the user just cleared out the input.
            displayPrompt();
            // Reinitialized CHAR_INPUT to account for the prompt being 
            // displayed now.
            startCharInput();
          } else {
            displayBufferContent();
          }
        }
        return;
        
      case HANDSET_Button_FCN:
        // FCN switches between alphabetic and numeric input modes, but
        // only if allowed...
        if ((module.length > 0) || (module.allowNumericStart)) {
          SOUND_PlayButtonBeep(button, false);
          
          if (module.isAlphaInput) {
            module.isAlphaInput = false;
            INDICATOR_StopFlashing(HANDSET_Indicator_FCN, false);
          } else {
            module.isAlphaInput = true;
            INDICATOR_StartFlashing(HANDSET_Indicator_FCN);
          }
        }
        return;
        
      case HANDSET_Button_STO:
        // STO applies the user input, but only if the input is not empty...
        if (module.length > 0) {
          SOUND_PlayButtonBeep(button, false);
          HANDSET_CancelCurrentButtonHoldEvents();
          INDICATOR_StopFlashing(HANDSET_Indicator_FCN, false);
          module.returnCallback(STRING_INPUT_Result_APPLY, module.buffer);
        }
        return;
    }
  }
  
  // This block of code handles character input button presses, but only
  // if the input string has not reached the max allowed length...
  if (module.length < module.maxLength) {
    if (module.isAlphaInput) {
      // Alphabetic input mode is handled entirely by CHAR_INPUT.
      CHAR_INPUT_HANDSET_EventHandler(event);
    } else if (HANDSET_IsButtonPrintable(button)) {
      // Numeric input mode handling of all "printable" buttons.
      // (`*` and `#` are included in "numeric" mode)...
      if (isButtonDown) {
        // Similar to how CHAR_INPUT works, pressing the button does not
        // immediately "apply" the character, but instead only "previews" it.
        // The release of the button is what triggers the character to be
        // appended to the input.
        SOUND_PlayDTMFButtonBeep(button, false);
        
        if (module.isPromptDisplayed) {
          // If the prompt is currently displayed, then we need to clear it 
          // before printing the character.
          HANDSET_ClearText();
          HANDSET_PrintChar(button);
          module.isPromptDisplayed = false;
          startCharInput();
        } else {
          // The prompt is not displayed, so we overwrite the '_' placeholder
          // at position 0
          HANDSET_PrintCharAt(button, 0);
        }
      } else if (event->type == HANDSET_EventType_BUTTON_UP) {
        handleCharInput(button);
      }
    }
  }
}
