/** 
 * @file
 * @author Jeff Lau
 */

#include "call_timer.h"
#include "../telephone/handset.h"
#include "../storage/storage.h"
#include "../sound/sound.h"
#include "../../mcc_generated_files/tmr0.h"
#include <stdint.h>
#include <xc.h>

static struct {
  bool isRunning;
  bool isDisplayUpdateEnabled;
  bool isCallWaiting;
  volatile uint8_t timerInterruptCount;
  volatile bool secondTimerExpired;
  struct {
    uint8_t minuteHundreds;
    uint8_t minuteTens;
    uint8_t minuteOnes;
    uint8_t secondTens;
    uint8_t secondOnes;
  } callTime;
} module;

static void displayCallTime(void) {
  HANDSET_DisableTextDisplay();
  HANDSET_ClearText();
  
  HANDSET_PrintChar(module.isCallWaiting ? 'W' : ' ');

  HANDSET_PrintChar(
      module.callTime.minuteHundreds
      ? (module.callTime.minuteHundreds % 10) + '0'
      : ' '
  );
  
  HANDSET_PrintChar(module.callTime.minuteTens + '0');
  HANDSET_PrintChar(module.callTime.minuteOnes + '0');
  HANDSET_PrintChar(' ');
  HANDSET_PrintChar(module.callTime.secondTens + '0');
  HANDSET_PrintChar(module.callTime.secondOnes + '0');
  HANDSET_EnableTextDisplay();
}

static void timer100MS_Interrupt(void) {
  if (++module.timerInterruptCount == 10) {
    module.secondTimerExpired = true;
    module.timerInterruptCount = 0;
  }
}

void CALL_TIMER_Initialize(void) {
  PMD1bits.TMR0MD = 0;
  TMR0_Initialize();
  
  module.timerInterruptCount = 0;
  module.secondTimerExpired = false;
  module.isRunning = false;
  module.isDisplayUpdateEnabled = false;
  
  TMR0_SetInterruptHandler(&timer100MS_Interrupt);
}

void CALL_TIMER_Task(void) {
  if (!module.isRunning) {
    return;
  }
  
  bool playWarningBeep = false;
  
  if (module.secondTimerExpired) {
    module.secondTimerExpired = false;
    
    if (++module.callTime.secondOnes == 10) {
      module.callTime.secondOnes = 0;

      if (++module.callTime.secondTens == 6) {
        module.callTime.secondTens = 0;

        if (++module.callTime.minuteOnes == 10) {
          module.callTime.minuteOnes = 0;

          if (++module.callTime.minuteTens == 10) {
            module.callTime.minuteTens = 0;
            ++module.callTime.minuteHundreds;

            if (module.isDisplayUpdateEnabled) {
              HANDSET_PrintCharAt((module.callTime.minuteHundreds % 10) + '0', 5);
            }
          }

          if (module.isDisplayUpdateEnabled) {
            HANDSET_PrintCharAt(module.callTime.minuteTens + '0', 4);
          }
        }

        if (module.isDisplayUpdateEnabled) {
          HANDSET_PrintCharAt(module.callTime.minuteOnes + '0', 3);
        }
      }

      if (module.isDisplayUpdateEnabled) {
        HANDSET_PrintCharAt(module.callTime.secondTens + '0', 1);
      }

      if ((module.callTime.secondTens == 5) && STORAGE_GetOneMinuteBeepEnabled()) {
        playWarningBeep = true;
      }
    }

    if (module.isDisplayUpdateEnabled) {
      HANDSET_PrintCharAt(module.callTime.secondOnes + '0', 0);
    }  
    
    if (playWarningBeep) {
      SOUND_PlayStatusBeep();
    }
  }
}

void CALL_TIMER_Start(bool updateDisplay) {
  if (module.isRunning) {
    return;
  }
  
  module.callTime.minuteHundreds = 0;
  module.callTime.minuteTens = 0;
  module.callTime.minuteOnes = 0;
  module.callTime.secondTens = 0;
  module.callTime.secondOnes = 0;
  
  if (updateDisplay) {
    displayCallTime();
  }
  
  module.timerInterruptCount = 0;
  module.secondTimerExpired = false;
  module.isRunning = true;
  module.isDisplayUpdateEnabled = updateDisplay;
  
  TMR0_WriteTimer(0);
  TMR0_StartTimer();
}

void CALL_TIMER_Stop(void) {
  if (!module.isRunning) {
    return;
  }
  
  TMR0_StopTimer();
  module.isRunning = false;
  
  STORAGE_SetLastCallTime(
    module.callTime.minuteHundreds * 100 + module.callTime.minuteTens * 10 + module.callTime.minuteOnes,
    module.callTime.secondTens * 10 + module.callTime.secondOnes  
  );
}

void CALL_TIMER_SetCallWaitingIndicator(bool isCallWaiting) {
  if (isCallWaiting == module.isCallWaiting) {
    return;
  }
  
  module.isCallWaiting = isCallWaiting;
  
  if (module.isDisplayUpdateEnabled) {
    HANDSET_PrintCharAt(module.isCallWaiting ? 'W' : ' ', 6);
  }
}

void CALL_TIMER_EnableDisplayUpdate(void) {
  if (module.isDisplayUpdateEnabled) {
    return;
  }
  
  displayCallTime();
  module.isDisplayUpdateEnabled = true;
}

void CALL_TIMER_DisableDisplayUpdate(void) {
  module.isDisplayUpdateEnabled = false;
}

bool CALL_TIMER_IsDisplayEnabled(void) {
  return module.isDisplayUpdateEnabled;
}


