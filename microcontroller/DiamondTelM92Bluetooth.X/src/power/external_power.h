/** 
 * @file
 * @author Jeff Lau
 * 
 * This module provides access to the current state of the external power supply
 * connection.
 */

#ifndef EXTERNAL_POWER_H
#define	EXTERNAL_POWER_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdbool.h>  

/**
 * External power event types.
 */
typedef enum EXTERNAL_POWER_EventType {
  /**
   * External power has been disconnected.
   */
  EXTERNAL_POWER_EventType_DISCONNECTED,
  /**
   * External power has been connected.
   */
  EXTERNAL_POWER_EventType_CONNECTED
} EXTERNAL_POWER_EventType;
  
/**
 * EXTERNAL_POWER event handler function pointer.
 */
typedef void (*EXTERNAL_POWER_EventHandler)(EXTERNAL_POWER_EventType event);
  
/**
 * Initialize the EXTERNAL_POWER module and register an event handler.
 * 
 * NOTE: An event is NOT triggered for the initial connected status.
 *       Use EXTERNAL_POWER_IsConnected() to check initial status after
 *       initializing if needed.
 * 
 * @param eventHandler - Event handler function pointer.
 *        May be NULL to ignore events.
 */
void EXTERNAL_POWER_Initialize(EXTERNAL_POWER_EventHandler eventHandler);

/**
 * Main task loop behavior for the EXTERNAL_POWER module.
 * 
 * Must be called from the main task loop of the app.
 */
void EXTERNAL_POWER_Task(void);

/**
 * Timer event handler for the EXTERNAL_POWER module.
 * 
 * Must be called from a timer interrupt every 10 milliseconds.
 */
void EXTERNAL_POWER_Timer10MS_Interrupt(void);

/**
 * Test if external power is currently connected.
 * 
 * @return True if external power is currently connected.
 */
bool EXTERNAL_POWER_IsConnected(void);

#ifdef	__cplusplus
}
#endif

#endif	/* EXTERNAL_POWER_H */

