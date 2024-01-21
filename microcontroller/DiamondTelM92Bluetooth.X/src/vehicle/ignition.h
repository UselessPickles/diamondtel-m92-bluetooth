/** 
 * @file
 * @author Jeff Lau
 * 
 * This module provides access to the current state of the vehicle ignition 
 * switch.
 */

#ifndef IGNITION_H
#define	IGNITION_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdbool.h>  

/**
 * IGNITION event types.
 */
typedef enum IGNITION_EventType {
  /**
   * Vehicle ignition has turned on.
   */
  IGNITION_EventType_ON,
  /**
   * Vehicle ignition has turned off.
   */
  IGNITION_EventType_OFF
} IGNITION_EventType;
  
/**
 * IGNITION event handler function pointer.
 */
typedef void (*IGNITION_EventHandler)(IGNITION_EventType event);
  
/**
 * Initialize the IGNITION module and register an event handler.
 * 
 * NOTE: An event is NOT triggered for the initial ignition status.
 *       Use IGNITION_IsOn() to check initial status after
 *       initializing if needed.
 * 
 * @param eventHandler - Event handler function pointer.
 *        May be NULL to ignore events.
 */
void IGNITION_Initialize(IGNITION_EventHandler eventHandler);

/**
 * Main task loop behavior for the IGNITION module.
 * 
 * Must be called from the main task loop of the app.
 */
void IGNITION_Task(void);

/**
 * Timer event handler for the IGNITION module.
 * 
 * Must be called from a timer interrupt every 10 milliseconds.
 */
void IGNITION_Timer10MS_Interrupt(void);

/**
 * Test if the vehicle ignition is currently ON.
 * 
 * @return True if the vehicle ignition is ON.
 */
bool IGNITION_IsOn(void);

#ifdef	__cplusplus
}
#endif

#endif	/* IGNITION_H */

