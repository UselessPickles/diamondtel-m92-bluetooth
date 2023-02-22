/** 
 * @file
 * @author Jeff Lau
 * 
 * Tools for monitoring the DiamondTel Model 92's Transceiver battery state.
 */

#ifndef TRANSCEIVER_H
#define	TRANSCEIVER_H

#include "handset.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef	__cplusplus
extern "C" {
#endif

/**
 * The maximum possible value reported by TRANSCEIVER_GetBatteryLevel()
 */
#define TRANSCEIVER_MAX_BATTERY_LEVEL (5)
  
/**
 * Types of events that can be triggered by the transceiver.
 */  
typedef enum TRANSCEIVER_EventType {
  /**
   * Battery level was OK, but is now low.
   * Phone will power off soon if external power is not connected.
   */
  TRANSCEIVER_EventType_BATTERY_LEVEL_IS_LOW,
  /**
   * Battery level was low, but is now OK 
   * (most likely because external power was connected).
   */    
  TRANSCEIVER_EventType_BATTERY_LEVEL_IS_OK,
  /**
   * The battery level has changed.
   * Use TRANSCEIVER_GetBatteryLevel() to get the new battery level.
   */    
  TRANSCEIVER_EventType_BATTERY_LEVEL_CHANGED,
  /**
   * The transceiver has been connected to external power.
   * 
   * NOTE: This event is not fired when the phone starts up with external power
   *       because external power is the default assumption until evidence of
   *       no external power is received.
   */
  TRANSCEIVER_EventType_CONNECTED_TO_EXTERNAL_POWER,
  /**
   * The transceiver has been disconnected from external power.
   * 
   * NOTE: This event is fired as a delayed result of absence of external
   *       power when the transceiver attempts to turn off the handset 
   *       back-light after a period of inactivity.
   */
  TRANSCEIVER_EventType_DISCONNECTED_FROM_EXTERNAL_POWER,
  /**
   * The PWR button power-off process has begun.
   * 
   * The user has held the PWR button long enough such that the phone
   * will power off upon release.
   */
  TRANSCEIVER_EventType_POWERING_OFF
} TRANSCEIVER_EventType;  

/**
 * TRANSCEIVER event handler function pointer.
 */
typedef void (*TRANSCEIVER_EventHandler)(TRANSCEIVER_EventType);

/**
 * Initialize the transceiver module.
 * 
 * @param eventHandler - Event handler function pointer.
 */
void TRANSCEIVER_Initialize(TRANSCEIVER_EventHandler eventHandler);

/**
 * Main task loop behavior for the Transceiver module.
 * 
 * This must be called from the main task loop of the application.
 */
void TRANSCEIVER_Task(void);

/**
 * 10 millisecond timer event handler for the Transceiver module.
 * 
 * This must be called from a timer interrupt with a 10 millisecond period.
 */
void TRANSCEIVER_Timer10MS_Interrupt(void);

/**
 * Handset event handler for this module.
 * 
 * The parent module must call this from its handset event handler.
 * 
 * @param event - The handset event.
 */
void TRANSCEIVER_HANDSET_EventHandler(HANDSET_Event const* event);

/**
 * Poll the transceiver for the current battery level right now.
 * 
 * If the battery level is different than the last recorded battery level, 
 * then the TRANSCEIVER_EventType_BATTERY_LEVEL_CHANGED event will be triggered
 * soon.
 */
void TRANSCEIVER_PollBatteryLevelNow(void);

/**
 * Get the current Transceiver battery level as of the last time the battery 
 * level was polled. The battery level is automatically polled periodically,
 * or you can force it to be polled right now via 
 * TRANSCEIVER_PollBatteryLevelNow().
 * 
 * The TRANSCEIVER_EventType_BATTERY_LEVEL_CHANGED event is triggered whenever 
 * a new value is available from this function.
 * 
 * The normal range of the battery level is 1-5, or 0 if unknown.
 * 
 * @return The current Transceiver battery level.
 */
uint8_t TRANSCEIVER_GetBatteryLevel(void);

/**
 * Check if the battery level is currently "low".
 * 
 * A low battery means that the phone will power itself off soon if external 
 * power is not connected.
 * 
 * The TRANSCEIVER_EventType_BATTERY_LEVEL_IS_LOW or 
 * TRANSCEIVER_EventType_BATTERY_LEVEL_IS_OK events are triggered when the 
 * "low battery" state changes.
 * 
 * @return True if the battery level is low.
 */
bool TRANSCEIVER_IsBatteryLevelLow(void);

/**
 * Check if the transceiver is currently connected to external power.
 * 
 * NOTE: The default value during initialization is `true` until there is 
 *       evidence of not having external power.
 * 
 * The TRANSCEIVER_EventType_CONNECTED_TO_EXTERNAL_POWER or
 * TRANSCEIVER_EventType_DISCONNECTED_FROM_EXTERNAL_POWER events are triggered
 * when the external power state changes.
 * 
 * @return True of the transceiver is currently connected to external power.
 */
bool TRANSCEIVER_IsConnectedToExternalPower(void);
#ifdef	__cplusplus
}
#endif

#endif	/* TRANSCEIVER_H */

