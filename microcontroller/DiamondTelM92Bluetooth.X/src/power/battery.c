/** 
 * @file
 * @author Jeff Lau
 * 
 * See header file for module description.
 */

#include "battery.h"
#include "../util/timeout.h"
#include "../util/interval.h"
#include "../../mcc_generated_files/adcc.h"
#include <stdio.h>
#include <string.h>

/**
 * Amount of time (hundredths of a second) between automatic polling
 * of the battery level.
 */
#define BATTERY_LEVEL_POLL_INTERVAL (500)

/**
 * Number of battery level samples to use for a rolling average.
 */
#define BATTERY_LEVEL_SAMPLES (6)

/**
 * Minimum amount of time (hundredths of a second) between a low battery level
 * warning and power-off due to minimum battery level.
 */
#define MINIMUM_LOW_BATTERY_WARNING_DURATION (1000)

/**
 * Module state.
 */
static struct {
  /**
   * Event handler function pointer.
   */
  BATTERY_EventHandler eventHandler;
  /**
   * Interval used for automatic periodic polling of the battery level.
   */
  interval_t batteryLevelPollInterval;
  /**
   * Multiple battery level samples used to calculate a rolling average.
   */
  uint8_t batteryLevelSamples[BATTERY_LEVEL_SAMPLES];
  /**
   * Index into batteryLevelSampleswhere the next sample should be stored.
   * If the current value at this index is zero, then it is assumed that only 
   * the previous indexes in the array contain valid samples to be used when 
   * calculating the rolling average.
   */
  uint8_t nextBatteryLevelSampleIndex;
  /**
   * If true, then the next time the battery level is sampled, the array of
   * samples will be reset (resetting the rolling average).
   */
  bool resetSamples;
  /**
   * The current battery level.
   * 
   * Normal range is 1-100, or 0 if unknown.
   */
  uint8_t batteryLevel;
  /**
   * True if the battery level is currently "low".
   */
  bool isBatteryLevelLow;
  /**
   * True if an ADC conversion is currently pending in the background to
   * get a battery voltage reading.
   */
  bool isAdcConversionPending;
  /**
   * A timeout that is started upon the battery reaching its "low" threshold
   * and used to ensure that there is a minimum amount of time between warning
   * the user of the low battery and firing the BATTERY_EventType_LOW_VOLTAGE_POWER_OFF
   * event, even if the battery level very quickly drops to/below minimum
   * (e.g., immediately after powering on when the battery level was just above
   * minimum, but then drops due to higher current draw)
   */
  timeout_t lowVoltagePowerOffTimeout;
} module;

/**
 * The raw ADC battery voltage reading that is to be considered the lowest
 * battery level. All values <= this are reported as a battery level of 1.
 */
#define MIN_ADC_BATTERY_LEVEL (2720)
/**
 * The raw ADC battery voltage reading that is to be considered the highest
 * battery level. All values >= this are reported as a battery level of 100.
 */
#define MAX_ADC_BATTERY_LEVEL (3418)

/**
 * The battery level (on a 1-100 scale) that is used as the threshold for 
 * considering the battery level to be "low".
 * @return 
 */
#define LOW_BATTERY_THRESHOLD (5)

/**
 * Convert a raw ADC battery voltage reading to a battery level in the range
 * of 1-100.
 * @param adcBatteryLevel - Raw ADC battery voltage reading.
 * @return A battery level in the range of 1-100.
 */
static uint8_t convertAdcToBatteryLevel(uint16_t adcBatteryLevel) {
  if (adcBatteryLevel <= MIN_ADC_BATTERY_LEVEL) {
    return 1;
  }
  
  uint8_t const result = ((((adcBatteryLevel - MIN_ADC_BATTERY_LEVEL) >> 1) * 99) / ((MAX_ADC_BATTERY_LEVEL - MIN_ADC_BATTERY_LEVEL) / 2)) + 1;
  
  if (result > 100) {
    return 100;
  }
  
  return result;
}

/**
 * Adds a battery level to the array of battery level samples and calculates
 * the new rolling average of the battery level.
 * @param batteryLevel - The new battery level in the range of 1-100.
 * @return The new rolling average of the battery level in the range of 1-100.
 */
static uint8_t addBatteryLevelSample(uint8_t batteryLevel) {
  module.batteryLevelSamples[module.nextBatteryLevelSampleIndex++] = batteryLevel;
 
  if (module.nextBatteryLevelSampleIndex == BATTERY_LEVEL_SAMPLES) {
    module.nextBatteryLevelSampleIndex = 0;
  }
  
  uint8_t sampleCount = module.batteryLevelSamples[module.nextBatteryLevelSampleIndex] 
          ? BATTERY_LEVEL_SAMPLES 
          : module.nextBatteryLevelSampleIndex;
  
  uint16_t accumulator = 0;
  for (uint8_t i = 0; i < sampleCount; ++i) {
    accumulator += module.batteryLevelSamples[i];
  }
  
  return accumulator / sampleCount;
}

void BATTERY_Initialize(BATTERY_EventHandler eventHandler) {
  module.eventHandler = eventHandler;
  module.isAdcConversionPending = false;
  module.resetSamples = false;
  module.nextBatteryLevelSampleIndex = 0;
  memset(module.batteryLevelSamples, 0, sizeof(module.batteryLevelSamples));
  
  uint16_t const rawBatteryLevel = ADCC_GetSingleConversion(IO_BATT_VOLTAGE);
  
  module.batteryLevel = addBatteryLevelSample(convertAdcToBatteryLevel(rawBatteryLevel));
  module.isBatteryLevelLow = module.batteryLevel < LOW_BATTERY_THRESHOLD;
  printf("[BATTERY] Initial battery level: %u/4095; %u/100\r\n", rawBatteryLevel, module.batteryLevel);
  
  INTERVAL_Initialize(&module.batteryLevelPollInterval, BATTERY_LEVEL_POLL_INTERVAL);
  INTERVAL_Start(&module.batteryLevelPollInterval, false);
}

void BATTERY_Task(void) {
  // Start the periodic read of the raw battery voltage
  if (INTERVAL_Task(&module.batteryLevelPollInterval) && !module.isAdcConversionPending) {
    module.isAdcConversionPending = true;
    ADCC_StartConversion(IO_BATT_VOLTAGE);
  } 
  
  // Process a completed raw battery voltage reading
  if (module.isAdcConversionPending && ADCC_IsConversionDone()) {
    module.isAdcConversionPending = false;
    
    if (module.resetSamples) {
      module.resetSamples = false;
      module.nextBatteryLevelSampleIndex = 0;
      memset(module.batteryLevelSamples, 0, sizeof(module.batteryLevelSamples));
    }
    
    uint16_t const rawBatteryLevel = ADCC_GetConversionResult();
    uint8_t const normalizedBatteryLevel = convertAdcToBatteryLevel(rawBatteryLevel);
    uint8_t const newBatteryLevel = addBatteryLevelSample(normalizedBatteryLevel);
    
    printf("[BATTERY] Battery level: %u/4095; %u/100; avg: %u/100\r\n", rawBatteryLevel, normalizedBatteryLevel, newBatteryLevel);
    
    if (newBatteryLevel != module.batteryLevel) {
      bool const wasBatteryLevelLow = module.isBatteryLevelLow;
      
      module.batteryLevel = newBatteryLevel;
      module.isBatteryLevelLow = newBatteryLevel < (LOW_BATTERY_THRESHOLD + (wasBatteryLevelLow ? 2 : 0));
      
      if (module.eventHandler) {
        module.eventHandler(BATTERY_EventType_BATTERY_LEVEL_CHANGED);
      }
      
      if (module.isBatteryLevelLow != wasBatteryLevelLow) {
        BATTERY_ResetLowBatteryHandling();
      }
    }
  }
  
  if ((module.batteryLevel == 1) && TIMEOUT_IsExpired(&module.lowVoltagePowerOffTimeout)) {
    TIMEOUT_Cancel(&module.lowVoltagePowerOffTimeout);
    
    if (module.eventHandler) {
      module.eventHandler(BATTERY_EventType_LOW_VOLTAGE_POWER_OFF);
    }
  }
}

void BATTERY_Timer10MS_Interrupt(void) {
  INTERVAL_Timer_Interrupt(&module.batteryLevelPollInterval);
  TIMEOUT_Timer_Interrupt(&module.lowVoltagePowerOffTimeout);
}

void BATTERY_PollBatteryLevelNow(void) {
  INTERVAL_SkipAhead(&module.batteryLevelPollInterval);
}

void BATTERY_ResetRollingAverage(void) {
  module.resetSamples = true;
}

bool BATTERY_IsBatteryLevelKnown(void) {
  return module.batteryLevel > 0;
}

uint8_t BATTERY_GetBatteryLevel(void) {
  return module.batteryLevel;
}

uint8_t BATTERY_GetBatteryLevelForBT(void) {
  if (module.batteryLevel == 0) {
    return 0;
  }
  
  return (module.batteryLevel - 1) / 10;
}

uint8_t BATTERY_GetBatteryLevelForHandset(void) {
  if (module.batteryLevel == 0) {
    return 0;
  }

  return ((module.batteryLevel - 1) / 20) + 1;
}

bool BATTERY_IsBatteryLevelLow(void) {
  return module.isBatteryLevelLow;
}

void BATTERY_ResetLowBatteryHandling(void) {
  if (module.isBatteryLevelLow) {
    TIMEOUT_Start(&module.lowVoltagePowerOffTimeout, MINIMUM_LOW_BATTERY_WARNING_DURATION);
  } else {
    TIMEOUT_Cancel(&module.lowVoltagePowerOffTimeout);
  }
  
  if (module.eventHandler) {
    module.eventHandler(module.isBatteryLevelLow ? BATTERY_EventType_BATTERY_LEVEL_IS_LOW : BATTERY_EventType_BATTERY_LEVEL_IS_OK);
  }
}
