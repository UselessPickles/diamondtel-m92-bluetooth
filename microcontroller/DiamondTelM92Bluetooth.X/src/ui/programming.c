/** 
 * @file
 * @author Jeff Lau
 */

#include "programming.h"
#include "indicator.h"
#include "../constants.h"
#include "../storage/storage.h"
#include "../sound/sound.h"
#include "../util/timeout.h"
#include "../../mcc_generated_files/pin_manager.h"
#include <string.h>
#include <xc.h>
#include <stdio.h>

typedef enum State {
  State_ENABLE_DUAL_NUMBER,
  State_SET_TEL_NUMBER1,
  State_SET_TEL_NUMBER2,
  State_SET_SECURITY_CODE,
  State_DISABLE_CUMULATIVE_RESET,
  State_DISABLE_OWN_TEL,
  State_ENABLE_CALLER_ID,
  State_INPUT
} State;

/**
 * Module state.
 */
static struct {
  PROGRAMMING_ReturnCallback returnCallback;
  State state;
  State inputReason;
  char input[HANDSET_TEXT_DISPLAY_LENGTH + 1];
  uint8_t inputLength;
} module;

static void initState(State newState) {
  char buffer[STANDARD_PHONE_NUMBER_LENGTH + 1];
  size_t len;
  
  HANDSET_DisableTextDisplay();
  HANDSET_ClearText();

  if (newState == State_INPUT) {
    newState = 0;
  } else if ((newState == State_SET_TEL_NUMBER2) && !STORAGE_GetDualNumberEnabled()) {
    // Skip over SET_TEL_NUMBER2 if dual numbers is not enabled
    ++newState;
  }
  
  switch (newState) {
    case State_ENABLE_DUAL_NUMBER: 
      HANDSET_PrintString("DUAL NO      ");
      HANDSET_PrintChar('0' + STORAGE_GetDualNumberEnabled());
      break;
      
    case State_SET_TEL_NUMBER1: 
    case State_SET_TEL_NUMBER2: {
      uint8_t index = (newState - State_SET_TEL_NUMBER1);
      STORAGE_GetOwnNumber(index, buffer);
      len = strlen(buffer);
      HANDSET_PrintString("NO");
      HANDSET_PrintChar('1' + index);
      HANDSET_PrintCharN(' ', STANDARD_PHONE_NUMBER_LENGTH - len + 1);
      HANDSET_PrintString(buffer);
    }
    break;
      
    case State_SET_SECURITY_CODE:
      HANDSET_PrintString("SEC       ");
      HANDSET_PrintString(STORAGE_GetSecurityCode(buffer));
      break;
      
    case State_DISABLE_CUMULATIVE_RESET:  
      HANDSET_PrintString("DIS CU RESET ");
      HANDSET_PrintChar('0' + !STORAGE_GetCumulativeTimerResetEnabled());
      break;

    case State_DISABLE_OWN_TEL:  
      HANDSET_PrintString("DIS OWNTEL   ");
      HANDSET_PrintChar('0' + !STORAGE_GetShowOwnNumberEnabled());
      break;
      
    case State_ENABLE_CALLER_ID:  
      HANDSET_PrintString("CALL ID      ");
      HANDSET_PrintChar('0' + STORAGE_GetCallerIdEnabled());
      break;
  }
  
  HANDSET_SetTextBlink(true);
  HANDSET_EnableTextDisplay();
  
  module.state = newState;
}

static void startInput(char firstDigit) {
  module.inputReason = module.state;
  
  module.input[0] = firstDigit;
  module.input[1] = 0;
  module.inputLength = 1;
  
  HANDSET_ClearText();
  HANDSET_SetTextBlink(false);
  HANDSET_PrintChar(firstDigit);
  
  module.state = State_INPUT;
}

static bool storeInput(void) {
  switch (module.inputReason) {
    case State_ENABLE_DUAL_NUMBER:
    case State_DISABLE_CUMULATIVE_RESET:
    case State_DISABLE_OWN_TEL:
    case State_ENABLE_CALLER_ID: {
      char firstDigit = module.input[0];
      bool isOnOffInput = (module.inputLength == 1) && ((firstDigit == '0') || (firstDigit == '1'));
      
      if (!isOnOffInput) {
        return false;
      }
      
      bool isOn = (firstDigit == '1');
      
      switch (module.inputReason) {
        case State_ENABLE_DUAL_NUMBER:
          printf("[PROG] Store DUAL NO: %d\r\n", isOn);
          STORAGE_SetDualNumberEnabled(isOn);
          break;
          
        case State_DISABLE_CUMULATIVE_RESET:
          printf("[PROG] Store DIS CU RESET: %d\r\n", isOn);
          STORAGE_SetCumulativeTimerResetEnabled(!isOn);
          break;
          
        case State_DISABLE_OWN_TEL:
          printf("[PROG] Store DIS OWN TEL: %d\r\n", isOn);
          STORAGE_SetShowOwnNumberEnabled(!isOn);
          break;
          
        case State_ENABLE_CALLER_ID: 
          printf("[PROG] Store CALL ID: %d\r\n", isOn);
          STORAGE_SetCallerIdEnabled(isOn);
          break;
      }
      
      break;
    }
      
    case State_SET_TEL_NUMBER1:
    case State_SET_TEL_NUMBER2: {
      if (
          (module.inputLength != STANDARD_PHONE_NUMBER_LENGTH) &&
          (module.inputLength != SHORT_PHONE_NUMBER_LENGTH)
          ) {
        return false;
      }
      
      uint8_t index = (module.inputReason - State_SET_TEL_NUMBER1);
      
      printf("[PROG] Store TEL%d: %s\r\n", index + 1, module.input);
      STORAGE_SetOwnNumber(index, module.input);
    }
    break;

    case State_SET_SECURITY_CODE:
      if (module.inputLength != 4) {
        return false;
      }

      printf("[PROG] Store SEC: %s\r\n", module.input);
      STORAGE_SetSecurityCode(module.input);
      break;
  }
  
  return true;
}

void PROGRAMMING_Start(PROGRAMMING_ReturnCallback returnCallback) {
  module.returnCallback = returnCallback;
  
  IO_BT_RESET_SetLow();
  
  SOUND_SetDefaultAudioSource(SOUND_AudioSource_MCU);
  HANDSET_SetAllIndicators(false);
  INDICATOR_StopFlashing(HANDSET_Indicator_PWR, true);
  INDICATOR_StopFlashing(HANDSET_Indicator_NO_SVC, true);
  
  initState(0);
}

void PROGRAMMING_HANDSET_EventHandler(HANDSET_Event const* event) {
  if (event->type != HANDSET_EventType_BUTTON_DOWN) {
    return;
  }

  uint8_t button = event->button;

  // The limited duration of the button beep sound is consistent with the 
  // original programming functionality on the phone, which is strangely
  // inconsistent with all primary functionality of the original phone where
  // button beeps almost always last as long as the button is held.
  SOUND_PlayButtonBeep(button, true);
  
  if (module.state == State_INPUT) {
    if (button == HANDSET_Button_CLR) {
      initState(module.inputReason);
    } else if (HANDSET_IsButtonNumeric(button)) {
      HANDSET_PrintChar(button);
      
      if (module.inputLength < HANDSET_TEXT_DISPLAY_LENGTH) {
        module.input[module.inputLength++] = button;
        module.input[module.inputLength] = 0;
      } else {
        memmove(module.input, module.input + 1, HANDSET_TEXT_DISPLAY_LENGTH);
        module.input[HANDSET_TEXT_DISPLAY_LENGTH - 1] = button;
      }
    } else if (button == HANDSET_Button_SEND) {
      if (storeInput()) {
        initState(module.inputReason + 1);
      }
    }
  } else {
    if (HANDSET_IsButtonNumeric(button)) {
      startInput(button);
    } else if (button == HANDSET_Button_SEND) {
      initState(module.state + 1);
    } else if (button == HANDSET_Button_END) {
      module.returnCallback();
    }
  }
}

