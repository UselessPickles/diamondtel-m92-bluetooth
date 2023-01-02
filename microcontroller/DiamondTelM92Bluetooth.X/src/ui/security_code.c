#include "security_code.h"
#include "../storage/storage.h"
#include "../sound/sound.h"

static struct {
  SECURITY_CODE_Callback successCallback;
  SECURITY_CODE_Callback failureCallback;
  char const* securityCode;
  bool isPrompt;
  uint8_t nextDigitIndex;
  uint8_t totalDigitCount;
} module;

static void init(bool isPrompt, SECURITY_CODE_Callback successCallback, SECURITY_CODE_Callback failureCallback) {
  module.isPrompt = isPrompt;
  module.successCallback = successCallback;
  module.failureCallback = failureCallback;
  module.nextDigitIndex = 0;
  module.totalDigitCount = 0;
  module.securityCode = STORAGE_GetSecurityCode();

  if (isPrompt) {
    HANDSET_DisableTextDisplay();
    HANDSET_ClearText();
    HANDSET_PrintString("INPUT     CODE");
    HANDSET_EnableTextDisplay();
  }  
}

void SECURITY_CODE_Prompt(SECURITY_CODE_Callback successCallback, SECURITY_CODE_Callback failureCallback) {
  init(true, successCallback, failureCallback);
}

void SECURITY_CODE_Watch(SECURITY_CODE_Callback successCallback) {
  init(false, successCallback, NULL);
}

bool SECURITY_CODE_HANDSET_EventHandler(HANDSET_Event const* event) {
  HANDSET_Button const button = event->button;
  
  if (event->type != HANDSET_EventType_BUTTON_DOWN) {
    return false;
  }
  
  if (button == HANDSET_Button_CLR) {
    if (module.isPrompt) {
      SOUND_PlayButtonBeep(button, false);
      HANDSET_CancelCurrentButtonHoldEvents();
      module.failureCallback();
      return true;
    } else {
      return false;
    }
  }
  
  if (HANDSET_IsButtonNumeric(button)) {
    SOUND_PlayButtonBeep(button, false);
    ++module.totalDigitCount;

    if (button == module.securityCode[module.nextDigitIndex]) {
      ++module.nextDigitIndex;
    } else {
      module.nextDigitIndex = 0;
    }

    if (module.nextDigitIndex == SECURITY_CODE_LENGTH) {
      HANDSET_CancelCurrentButtonHoldEvents();
      module.successCallback();
    } else if (module.isPrompt && (module.totalDigitCount == SECURITY_CODE_LENGTH)) {
      HANDSET_CancelCurrentButtonHoldEvents();
      module.failureCallback();
    }  
  }
  
  return true;
}

