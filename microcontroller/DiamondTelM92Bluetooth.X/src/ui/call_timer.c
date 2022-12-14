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

static struct {
  uint8_t minuteHundreds;
  uint8_t minuteTens;
  uint8_t minuteOnes;
  uint8_t secondTens;
  uint8_t secondOnes;
} callTime;

static bool isRunning;
static bool isDisplayUpdateEnabled;

static void displayCallTime(void) {
  HANDSET_DisableTextDisplay();
  HANDSET_ClearText();
  
  if (callTime.minuteHundreds) {
    HANDSET_PrintChar((callTime.minuteHundreds % 10) + '0');
  }
  
  HANDSET_PrintChar(callTime.minuteTens + '0');
  HANDSET_PrintChar(callTime.minuteOnes + '0');
  HANDSET_PrintChar(' ');
  HANDSET_PrintChar(callTime.secondTens + '0');
  HANDSET_PrintChar(callTime.secondOnes + '0');
  HANDSET_EnableTextDisplay();
}

static volatile uint8_t timerInterruptCount;
static volatile bool secondTimerExpired;

static void timer100MS_Interrupt(void) {
  if (++timerInterruptCount == 10) {
    secondTimerExpired = true;
    timerInterruptCount = 0;
  }
}

void CALL_TIMER_Initialize(void) {
  timerInterruptCount = 0;
  secondTimerExpired = false;
  isRunning = false;
  isDisplayUpdateEnabled = false;
  
  TMR0_SetInterruptHandler(&timer100MS_Interrupt);
}

void CALL_TIMER_Task(void) {
  if (!isRunning) {
    return;
  }
  
  bool playWarningBeep = false;
  
  if (secondTimerExpired) {
    secondTimerExpired = false;
    
    if (++callTime.secondOnes == 10) {
      callTime.secondOnes = 0;

      if (++callTime.secondTens == 6) {
        callTime.secondTens = 0;

        if (++callTime.minuteOnes == 10) {
          callTime.minuteOnes = 0;

          if (++callTime.minuteTens == 10) {
            callTime.minuteTens = 0;
            ++callTime.minuteHundreds;

            if (isDisplayUpdateEnabled) {
              HANDSET_PrintCharAt((callTime.minuteHundreds % 10) + '0', 5);
            }
          }

          if (isDisplayUpdateEnabled) {
            HANDSET_PrintCharAt(callTime.minuteTens + '0', 4);
          }
        }

        if (isDisplayUpdateEnabled) {
          HANDSET_PrintCharAt(callTime.minuteOnes + '0', 3);
        }
      }

      if (isDisplayUpdateEnabled) {
        HANDSET_PrintCharAt(callTime.secondTens + '0', 1);
      }

      if ((callTime.secondTens == 5) && STORAGE_GetOneMinuteBeepEnabled()) {
        playWarningBeep = true;
      }
    }

    if (isDisplayUpdateEnabled) {
      HANDSET_PrintCharAt(callTime.secondOnes + '0', 0);
    }  
    
    if (playWarningBeep) {
      SOUND_PlayStatusBeep();
    }
  }
}

void CALL_TIMER_Start(bool updateDisplay) {
  if (isRunning) {
    return;
  }
  
  callTime.minuteHundreds = 0;
  callTime.minuteTens = 0;
  callTime.minuteOnes = 0;
  callTime.secondTens = 0;
  callTime.secondOnes = 0;
  
  if (updateDisplay) {
    displayCallTime();
  }
  
  timerInterruptCount = 0;
  secondTimerExpired = false;
  isRunning = true;
  isDisplayUpdateEnabled = updateDisplay;
  
  TMR0_WriteTimer(0);
  TMR0_StartTimer();
}

void CALL_TIMER_Stop(void) {
  TMR0_StopTimer();
  isRunning = false;
  isDisplayUpdateEnabled = false;
  
  STORAGE_SetLastCallTime(
    callTime.minuteHundreds * 100 + callTime.minuteTens * 10 + callTime.minuteOnes,
    callTime.secondTens * 10 + callTime.secondOnes  
  );
}

void CALL_TIMER_EnableDisplayUpdate(void) {
  if (!isRunning || isDisplayUpdateEnabled) {
    return;
  }
  
  displayCallTime();
  isDisplayUpdateEnabled = true;
}

void CALL_TIMER_DisableDisplayUpdate(void) {
  isDisplayUpdateEnabled = false;
}


