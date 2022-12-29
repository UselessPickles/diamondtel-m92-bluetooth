#include "clr_codes.h"
#include <stdbool.h>
#include <string.h>
#include "timeout.h"

#define MAX_INPUT_LENGTH (7)

static struct {
  CLR_CODES_EventHandler eventHandler;
  bool isActive;
  timeout_t activeTimeout;
  char input[MAX_INPUT_LENGTH + 1];
  uint8_t inputLength;
} module;

static char const PROGRAM_CODE[] = {
  HANDSET_Button_CLR_1,
  HANDSET_Button_CLR_5,
  HANDSET_Button_CLR_9,
  HANDSET_Button_CLR_1,
  HANDSET_Button_CLR_4,
  HANDSET_Button_CLR_2,
  HANDSET_Button_CLR_6,
  0
};

static char const PROGRAM_RESET_CODE[] = {
  HANDSET_Button_CLR_8,
  HANDSET_Button_CLR_2,
  HANDSET_Button_CLR_9,
  HANDSET_Button_CLR_1,
  HANDSET_Button_CLR_1,
  HANDSET_Button_CLR_1,
  HANDSET_Button_CLR_2,
  0
};

static char const SELF_DIAGNOSTICS_CODE[] = {
  HANDSET_Button_CLR_5,
  HANDSET_Button_CLR_4,
  HANDSET_Button_CLR_6,
  HANDSET_Button_CLR_5,
  HANDSET_Button_CLR_6,
  HANDSET_Button_CLR_7,
  HANDSET_Button_CLR_2,
  0
};

static char const FACTORY_RESET_CODE[] = {
  HANDSET_Button_CLR_3,
  HANDSET_Button_CLR_3,
  HANDSET_Button_CLR_3,
  HANDSET_Button_CLR_2,
  HANDSET_Button_CLR_8,
  HANDSET_Button_CLR_5,
  HANDSET_Button_CLR_8,
  0
};

static void resetInput(void) {
  module.inputLength = 0;
  module.input[0] = 0;
}

void CLR_CODES_Initialize(void) {
  module.isActive = false;
  TIMEOUT_Cancel(&module.activeTimeout);
}

void CLR_CODES_Start(CLR_CODES_EventHandler eventHandler) {
  module.eventHandler = eventHandler;
  resetInput();
  module.isActive = true;
  TIMEOUT_Start(&module.activeTimeout, 1000);
}

void CLR_CODES_Task(void) {
  if (TIMEOUT_Task(&module.activeTimeout)) {
    module.isActive = false;
  }
}

void CLR_CODES_Timer10MS_Tick(void) {
  if (!module.isActive) {
    return;
  }
  
  TIMEOUT_Timer_Tick(&module.activeTimeout);
}

void CLR_CODES_HANDSET_EventHandler(HANDSET_Event const* event) {
  if (!module.isActive || !(event->type == HANDSET_EventType_BUTTON_DOWN)) {
    return;
  }
  
  HANDSET_Button button = event->button;
  
  if (HANDSET_IsButtonClrCode(button)) {
    if (module.inputLength != MAX_INPUT_LENGTH) {
      module.input[module.inputLength++] = button;
      module.input[module.inputLength] = 0;

      if (strcmp(module.input, PROGRAM_CODE) == 0) {
        module.eventHandler(CLR_CODES_EventType_PROGRAM);
      } else if (strcmp(module.input, PROGRAM_RESET_CODE) == 0) {
        module.eventHandler(CLR_CODES_EventType_PROGRAM_RESET);
      } else if (strcmp(module.input, SELF_DIAGNOSTICS_CODE) == 0) {
        module.eventHandler(CLR_CODES_EventType_SELF_DIAGNOSTICS);
      } else if (strcmp(module.input, FACTORY_RESET_CODE) == 0) {
        module.eventHandler(CLR_CODES_EventType_FACTORY_RESET);
      }
    }
  } else {
    resetInput();
  }
}

