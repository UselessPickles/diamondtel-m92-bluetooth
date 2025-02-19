#include "horn.h"
#include "../util/timeout.h"
#include "../util/interval.h"
#include "../../mcc_generated_files/pin_manager.h"
#include "../telephone/handset.h"

typedef enum HORN_Mode {
  HORN_Mode_OFF,
  HORN_Mode_ON,
  HORN_Mode_ON_WITH_SHUTDOWN_TIMER    
} HORN_Mode;

static char const* const HORN_MODE_LABELS[] = {
  "HORN       OFF",
  "HORN       ON ",
  "HORN ON240 MIN"
};

typedef enum HORN_AlertingState {
  HORN_AlertingState_IDLE,
  HORN_AlertingState_ALERTING_ON,
  HORN_AlertingState_ALERTING_OFF
} HORN_AlertingState;

static struct {
  HORN_Mode mode;
  HORN_AlertingState alertingState;
  timeout_t alertingStateTimeout;
  uint8_t alertCount;
  HORN_ShutdownCallback shutdownCallback;
  uint8_t shutdownMinutesRemaining;
  interval_t shutdownMinutesInterval;
} module;

#define MAX_ALERT_COUNT (5)
#define ALERT_ON_DURATION (100)
#define ALERT_OFF_DURATION (300)
#define SHUTDOWN_TIMER_MINUTES (240)

void HORN_Task(void) {
  if (INTERVAL_Task(&module.shutdownMinutesInterval)) {
    if (--module.shutdownMinutesRemaining == 0) {
      HORN_Disable();
      module.shutdownCallback();
    }
  }
  
  TIMEOUT_Task(&module.alertingStateTimeout);
  
  switch (module.alertingState) {
    case HORN_AlertingState_ALERTING_ON:
      if (!TIMEOUT_IsPending(&module.alertingStateTimeout)) {
        IO_HORN_SetLow();
        
        if (++module.alertCount >= MAX_ALERT_COUNT) {
          module.alertingState = HORN_AlertingState_IDLE;
        } else {
          TIMEOUT_Start(&module.alertingStateTimeout, ALERT_OFF_DURATION);
          module.alertingState = HORN_AlertingState_ALERTING_OFF;
        }
      }
      break;
      
    case HORN_AlertingState_ALERTING_OFF:
      if (!TIMEOUT_IsPending(&module.alertingStateTimeout)) {
        IO_HORN_SetHigh();
        TIMEOUT_Start(&module.alertingStateTimeout, ALERT_ON_DURATION);
        module.alertingState = HORN_AlertingState_ALERTING_ON;
      }
      break;
  }
}

void HORN_Timer10MS_Interrupt(void) {
  INTERVAL_Timer_Interrupt(&module.shutdownMinutesInterval);
  TIMEOUT_Timer_Interrupt(&module.alertingStateTimeout);
}

void HORN_StartAlerting(void) {
  module.alertCount = 0;
  module.alertingState = HORN_AlertingState_ALERTING_OFF;
  // Ensure that the ALERTING_OFF state immediately ends and transitions
  // to ALERTING_ON to begin the first horn beep.
  TIMEOUT_Cancel(&module.alertingStateTimeout);
}

void HORN_StopAlerting(void) {
  TIMEOUT_Cancel(&module.alertingStateTimeout);
  IO_HORN_SetLow();
  module.alertingState = HORN_AlertingState_IDLE;
}

bool HORN_IsEnabled(void) {
  return module.mode != HORN_Mode_OFF;
}

void HORN_PrintCurrentMode(void) {
  HANDSET_DisableTextDisplay();
  HANDSET_ClearText();
  HANDSET_PrintString(HORN_MODE_LABELS[module.mode]);
  HANDSET_EnableTextDisplay();
}

void HORN_CycleMode(void) {
  module.mode = (module.mode + 1) % 3;
  HANDSET_SetIndicator(HANDSET_Indicator_HORN, HORN_IsEnabled());
}

void HORN_Disable(void) {
  module.mode = HORN_Mode_OFF;
  INTERVAL_Cancel(&module.shutdownMinutesInterval);
  HANDSET_SetIndicator(HANDSET_Indicator_HORN, false);
}

void HORN_StartShutdownTimer(HORN_ShutdownCallback shutdownCallback) {
  module.shutdownCallback = shutdownCallback;
  
  if (module.mode == HORN_Mode_ON_WITH_SHUTDOWN_TIMER) {
    module.shutdownMinutesRemaining = SHUTDOWN_TIMER_MINUTES;
    INTERVAL_Initialize(&module.shutdownMinutesInterval, 6000);
    INTERVAL_Start(&module.shutdownMinutesInterval, false);
  } else {
    // Ensure that the shutdown timer is NOT running
    INTERVAL_Cancel(&module.shutdownMinutesInterval);
  }
}