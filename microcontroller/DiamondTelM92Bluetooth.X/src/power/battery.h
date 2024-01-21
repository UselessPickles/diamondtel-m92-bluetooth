/** 
 * @file
 * @author Jeff Lau
 * 
 * This module provides access to the current state of the battery.
 */

#ifndef BATTERY_H
#define	BATTERY_H

#include <stdint.h>
#include <stdbool.h>

#ifdef	__cplusplus
extern "C" {
#endif

/**
 * Types of events that can be triggered by the Battery module.
 */  
typedef enum BATTERY_EventType {
  /**
   * Battery level was OK, but is now low.
   * Phone will power off soon if external power is not connected.
   */
  BATTERY_EventType_BATTERY_LEVEL_IS_LOW,
  /**
   * Battery level was low, but is now OK 
   * (most likely because external power was connected).
   */    
  BATTERY_EventType_BATTERY_LEVEL_IS_OK,
  /**
   * The battery level has changed.
   * Use BATTERY_GetBatteryLevel() to get the new battery level.
   */    
  BATTERY_EventType_BATTERY_LEVEL_CHANGED,
} BATTERY_EventType;  

/**
 * BATTERY event handler function pointer.
 */
typedef void (*BATTERY_EventHandler)(BATTERY_EventType);

/**
 * Initialize the Battery module.
 * 
 * @param eventHandler - Event handler function pointer.
 */
void BATTERY_Initialize(BATTERY_EventHandler eventHandler);

/**
 * Main task loop behavior for the Battery module.
 * 
 * This must be called from the main task loop of the application.
 */
void BATTERY_Task(void);

/**
 * 10 millisecond timer event handler for the Battery module.
 * 
 * This must be called from a timer interrupt with a 10 millisecond period.
 */
void BATTERY_Timer10MS_Interrupt(void);

/**
 * Poll the current battery level right now.
 * 
 * If the battery level is different than the last recorded battery level, 
 * then the BATTERY_EventType_BATTERY_LEVEL_CHANGED event will be triggered
 * soon.
 */
void BATTERY_PollBatteryLevelNow(void);

/**
 * Resets the rolling average of the reported battery level. 
 * Call this when there's a need for the next battery level report to more
 * immediately change in relation to a battery level change (e.g., when
 * external power supply is connected/disconnected).
 */
void BATTERY_ResetRollingAverage(void);

/**
 * Get the current battery level as of the last time the battery 
 * level was polled. The battery level is automatically polled periodically,
 * or you can force it to be polled right now via 
 * BATTERY_PollBatteryLevelNow().
 * 
 * The returned value is a rolling average of a number of recent readings
 * to reduce "jittering" of the value.
 * 
 * The BATTERY_EventType_BATTERY_LEVEL_CHANGED event is triggered whenever 
 * a new value is available from this function.
 * 
 * The range of the battery level is 1-100, or 0 if BATTERY_Initialize()
 * has not been called yet.
 * 
 * @return The current battery level in the range of 1-100, or 0 if BATTERY_Initialize()
 *         has not been called yet.
 */
uint8_t BATTERY_GetBatteryLevel(void);

/**
 * Get the current battery level, but converted for reporting to the paired
 * cell phone via the Bluetooth HFP AT command "+IPHONEACCEV" in the range
 * of 0-9.
 * 
 * @return The current battery level, in the range of 0-9.
 */
uint8_t BATTERY_GetBatteryLevelForBT(void);

/**
 * Get the current battery level, but converted presenting on the car phone 
 * handset in the range of 1-5, or 0 if BATTERY_Initialize() has not been called yet.
 * 
 * @return The current battery level, in the range of 1-5, or 0 if BATTERY_Initialize()
 *         has not been called yet.
 */
uint8_t BATTERY_GetBatteryLevelForHandset(void);

/**
 * Check if the battery level is currently "low".
 * 
 * A low battery means that the phone will power itself off soon if external 
 * power is not connected.
 * 
 * The BATTERY_EventType_BATTERY_LEVEL_IS_LOW or 
 * BATTERY_EventType_BATTERY_LEVEL_IS_OK events are triggered when the 
 * "low battery" state changes.
 * 
 * @return True if the battery level is low.
 */
bool BATTERY_IsBatteryLevelLow(void);


#ifdef	__cplusplus
}
#endif

#endif	/* BATTERY_H */

