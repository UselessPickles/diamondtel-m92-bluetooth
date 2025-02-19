/** 
 * @file
 * @author Jeff Lau
 * 
 * This module provides an interface for the external vehicle horn alert
 * feature.
 */

#ifndef HORN_H
#define	HORN_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdbool.h>  

typedef void (*HORN_ShutdownCallback)(void);  

/**
 * Main task loop behavior for the HORN module.
 * 
 * Must be called from the main task loop of the app.
 */
void HORN_Task(void);

/**
 * Timer event handler for the HORN module.
 * 
 * Must be called from a timer interrupt every 10 milliseconds.
 */
void HORN_Timer10MS_Interrupt(void);

/**
 * Starts alerting with the vehicle horn. The horn will beep periodically.
 * The beeping will stop automatically after certain number of beeps.
 * 
 * This function does not validate that the horn alert feature is enabled.
 * The caller is responsible for calling this only if/when horn alerting 
 * should occur (e.g., if HORN_IsEnabled() is true and there is an incoming
 * call while the ignition is off). 
 */
void HORN_StartAlerting(void);

/**
 * Immediately stops any ongoing horn alerting.
 */
void HORN_StopAlerting(void);

/**
 * Check if the horn alert feature is enabled.
 * @return True if the horn alert feature is enabled.
 */
bool HORN_IsEnabled(void);

/**
 * Prints the label/description of the currently-selected horn alert mode
 * to the handset display.
 */
void HORN_PrintCurrentMode(void);

/**
 * Cycles to the next horn alert mode.
 * 
 * The "HORN" indicator on the handset is updated to indicate whether
 * the horn alert feature is enabled.
 */
void HORN_CycleMode(void);

/**
 * Disables the horn alert feature (i.e., switches to the "OFF" mode) and
 * turns the handset's "HORN" indicator off.
 */
void HORN_Disable(void);

/**
 * Starts the shutdown timer, but only if the horn alert feature is in the 
 * "ON with shutdown timer" mode.
 * 
 * Otherwise, this function does nothing.
 * 
 * This function should be called by the app when the ignition is turned off
 * while the horn alert feature is enabled, and implement the callback to
 * turn the phone off.
 * 
 * @param shutdownCallback - A callback that will be called when the shutdown
 *        timer expires.
 */
void HORN_StartShutdownTimer(HORN_ShutdownCallback shutdownCallback);

#ifdef	__cplusplus
}
#endif

#endif	/* HORN_H */

