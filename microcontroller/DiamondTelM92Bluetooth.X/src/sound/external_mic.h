/** 
 * @file
 * @author Jeff Lau
 * 
 * External microphone connection status monitoring.
 */

#ifndef EXTERNAL_MIC_H
#define	EXTERNAL_MIC_H

#ifdef	__cplusplus
extern "C" {
#endif
  
#include <stdbool.h>  

/**
 * External microphone event types.
 */
typedef enum EXTERNAL_MIC_EventType {
  /**
   * External microphone has been disconnected.
   */
  EXTERNAL_MIC_EventType_DISCONNECTED,
  /**
   * External microphone has been connected.
   */
  EXTERNAL_MIC_EventType_CONNECTED
} EXTERNAL_MIC_EventType;
  
/**
 * EXTERNAL_MIC event handler function pointer.
 */
typedef void (*EXTERNAL_MIC_EventHandler)(EXTERNAL_MIC_EventType event);
  
/**
 * Initialize the EXTERNAL_MIC module and register an event handler.
 * 
 * NOTE: An event is NOT triggered for the initial connected status.
 *       Use EXTERNAL_MIC_IsConnected() to check initial status after
 *       initializing if needed.
 * 
 * @param eventHandler - Event handler function pointer.
 *        May be NULL to ignore events.
 */
void EXTERNAL_MIC_Initialize(EXTERNAL_MIC_EventHandler eventHandler);

/**
 * Main task loop behavior for the EXTERNAL_MIC module.
 * 
 * Must be called from the main task loop of the app.
 */
void EXTERNAL_MIC_Task(void);

/**
 * Timer event handler for the EXTERNAL_MIC module.
 * 
 * Must be called from a timer interrupt every 10 milliseconds.
 */
void EXTERNAL_MIC_Timer10MS_Interrupt(void);

/**
 * Test if an external microphone is currently connected.
 * 
 * @return True if an external microphone is currently connected.
 */
bool EXTERNAL_MIC_IsConnected(void);

#ifdef	__cplusplus
}
#endif

#endif	/* EXTERNAL_MIC_H */

