#include "indicator.h"
#include "interval.h"
#include <string.h>

#define MAX_FLASHING_INDICATORS (5)
static HANDSET_Indicator flashingIndicators[MAX_FLASHING_INDICATORS];
static uint8_t flashingIndicatorsCount;
static bool isFlashOn;
static interval_t flashingInterval;

static bool isSweepingSignalStrength;
static uint8_t signalSweepIndex;
static interval_t signalSweepInterval;

void INDICATOR_Initialize(void) {
  INTERVAL_Init(&flashingInterval, 50);
  INTERVAL_Init(&signalSweepInterval, 10);
}

void INDICATOR_Task(void) {
  if (INTERVAL_Task(&flashingInterval)) {
    isFlashOn = !isFlashOn;

    for (uint8_t i = 0; i < flashingIndicatorsCount; ++i) {
      HANDSET_SetIndicator(flashingIndicators[i], isFlashOn);
    }
  }

  if (INTERVAL_Task(&signalSweepInterval)) {
    uint8_t currentSignalIndex = signalSweepIndex;
    
    if (currentSignalIndex > 5) {
      currentSignalIndex = 10 - currentSignalIndex;
    }
    
    if (++signalSweepIndex == 10) {
      signalSweepIndex = 0;
    }
    
    uint8_t nextSignalIndex = signalSweepIndex;

    if (nextSignalIndex > 5) {
      nextSignalIndex = 10 - nextSignalIndex;
    }
    
    HANDSET_SetSignalBarAtIndex(nextSignalIndex, true);
    HANDSET_SetSignalBarAtIndex(currentSignalIndex, false);
  }
}

void INDICATOR_Timer10MS_event(void) {
  INTERVAL_Timer_event(&flashingInterval);
  INTERVAL_Timer_event(&signalSweepInterval);
}

void INDICATOR_StartFlashing(HANDSET_Indicator indicator) {
  for (uint8_t i = 0; i < flashingIndicatorsCount; ++i) {
    if (flashingIndicators[i] == indicator) {
      return;
    }
  }
  
  if (!flashingIndicatorsCount) {
    INTERVAL_Start(&flashingInterval, false);
    isFlashOn = true;
  } else if (flashingIndicatorsCount == MAX_FLASHING_INDICATORS) {
    return;
  }
  
  HANDSET_SetIndicator(indicator, isFlashOn);
  
  flashingIndicators[flashingIndicatorsCount++] = indicator;
}

void INDICATOR_StopFlashing(HANDSET_Indicator indicator, bool isOn) {
  uint8_t i;
      
  for (i = 0; i < flashingIndicatorsCount; ++i) {
    if (flashingIndicators[i] == indicator) {
      break;
    }
  }
  
  HANDSET_SetIndicator(indicator, isOn);

  if (i == flashingIndicatorsCount) {
    return;
  }
  
  memcpy(flashingIndicators + i, flashingIndicators + i + 1, MAX_FLASHING_INDICATORS - i - 1);
  
  if (!--flashingIndicatorsCount) {
    INTERVAL_Cancel(&flashingInterval);
  }
}

void INDICATOR_StartSignalStrengthSweep(void) {
  if (isSweepingSignalStrength) {
    return;
  }
  
  HANDSET_SetSignalStrength(1);
  INTERVAL_Start(&signalSweepInterval, false);

  signalSweepIndex = 0;
  isSweepingSignalStrength = true;
}

void INDICATOR_StopSignalStrengthSweep(uint8_t signalStrength) {
  isSweepingSignalStrength = false;
  HANDSET_SetSignalStrength(signalStrength);
  INTERVAL_Cancel(&signalSweepInterval);
}

