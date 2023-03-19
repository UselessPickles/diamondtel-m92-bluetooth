#include "sound_test.h"
#include "../util/timeout.h"
#include "../sound/sound.h"
#include "indicator.h"
#include "volume_adjust.h"
#include "../../mcc_generated_files/pin_manager.h"
#include <string.h>

#define MAX_FREQ_DIGITS (4)

typedef enum {
  State_FREQ_TEST,
  State_VOLUME_ADJUST
} State;

static struct {
  SOUND_TEST_ReturnCallback returnCallback;
  State state;
  char freqDigits[MAX_FREQ_DIGITS + 1];
  uint8_t freqDigitsLength;
} module;

static void playFreq(void) {
  if (module.freqDigitsLength == 0) {
    SOUND_Stop(SOUND_Channel_BACKGROUND);
  } else {
    uint16_t freq = 0;
    
    for (uint8_t i = 0; i < module.freqDigitsLength; ++i) {
      freq = freq * 10 + (module.freqDigits[i] - '0');
    }
    
    tone_t const tone = TONE_CalculateToneFromFrequency(freq);
    
    SOUND_PlayDualTone(
      SOUND_Channel_BACKGROUND,
      SOUND_Target_SPEAKER,
      VOLUME_Mode_TONE,
      tone,
      tone,
      0
    );
  }
}

static void printFreqDigits(void) {
  HANDSET_PrintCharNAt(' ', MAX_FREQ_DIGITS - module.freqDigitsLength, MAX_FREQ_DIGITS - 1);
  HANDSET_PrintStringNAt(module.freqDigits, module.freqDigitsLength, module.freqDigitsLength - 1);
  
  if (!module.freqDigitsLength) {
    HANDSET_ShowFlashingCursorAt(0);
  }
}

static void displayToneTest(void) {
  HANDSET_ClearText();
  HANDSET_DisableTextDisplay();
  HANDSET_PrintString("Freq?         ");
  HANDSET_EnableTextDisplay();
  printFreqDigits();
  module.state = State_FREQ_TEST;
}

void SOUND_TEST_Start(SOUND_TEST_ReturnCallback returnCallback) {
  IO_BT_RESET_SetLow();

  SOUND_SetDefaultAudioSource(SOUND_AudioSource_MCU);
  HANDSET_SetAllIndicators(false);
  INDICATOR_StopFlashing(HANDSET_Indicator_PWR, true);
  INDICATOR_StopFlashing(HANDSET_Indicator_NO_SVC, true);

  module.freqDigitsLength = 0;
  module.returnCallback = returnCallback;
  displayToneTest();
  playFreq();
}

void SOUND_TEST_Task(void) {
  if (module.state == State_VOLUME_ADJUST) {
    VOLUME_ADJUST_Task();
    return;
  }
}

void SOUND_TEST_HANDSET_EventHandler(HANDSET_Event const* event) {
  if (module.state == State_VOLUME_ADJUST) {
    VOLUME_ADJUST_HANDSET_EventHandler(event);
    return;
  }
  
  HANDSET_Button const button = event->button;
  bool const isButtonDown = event->type == HANDSET_EventType_BUTTON_DOWN;
  
  
  if (isButtonDown && (button == HANDSET_Button_CLR) && module.freqDigitsLength) {
    SOUND_PlayButtonBeep(button, true);
    module.freqDigitsLength = 0;
    playFreq();
    printFreqDigits();
  } else if (isButtonDown && HANDSET_IsButtonNumeric(button) && module.freqDigitsLength < MAX_FREQ_DIGITS) {
    SOUND_PlayButtonBeep(button, true);
    module.freqDigits[module.freqDigitsLength++] = button;
    playFreq();
    printFreqDigits();
  } else if (isButtonDown && ((button == HANDSET_Button_UP) || (button == HANDSET_Button_DOWN))) {
    VOLUME_ADJUST_Start(
      VOLUME_Mode_TONE,
      true,
      (button == HANDSET_Button_UP),
      displayToneTest  
    );
    
    module.state = State_VOLUME_ADJUST;
  } else if (isButtonDown && (button == HANDSET_Button_END)) {
    SOUND_PlayButtonBeep(button, true);
    module.returnCallback();
  }
}

void SOUND_TEST_Timer10MS_Interrupt(void) {
  if (module.state == State_VOLUME_ADJUST) {
    VOLUME_ADJUST_Timer10MS_Interrupt();
    return;
  }
}

