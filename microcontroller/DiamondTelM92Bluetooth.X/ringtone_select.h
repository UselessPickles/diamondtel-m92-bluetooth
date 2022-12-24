/** 
 * @file
 * @author Jeff Lau
 *
 * UI module for ringtone selection.
 */

#ifndef RINGTONE_SELECT_H
#define	RINGTONE_SELECT_H

#include "handset.h"

#ifdef	__cplusplus
extern "C" {
#endif

/**
 * Callback function to exit ringtone selection.
 */
typedef void (*RINGTONE_SELECT_ReturnCallback)(void);

/**
 * Start ringtone selection.
 * 
 * After calling this function, the parent module is responsible for calling
 * RINGTONE_SELECT_Task(), RINGTONE_SELECT_Timer10MS_event(), and
 * RINGTONE_SELECT_HANDSET_EventHandler() appropriately until the
 * `returnCallback` has been called.
 *
 * When the `returnCallback`, the parent module is responsible for updating the 
 * display as desired.
 * 
 * @param returnCallback - Callback that is called when ringtone selection is complete.
 */
void RINGTONE_SELECT_Start(RINGTONE_SELECT_ReturnCallback returnCallback);

/**
 * Main task loop handler for this module.
 * 
 * The parent module must call this from its main task loop handler while 
 * ringtone selection is "active".
 */
void RINGTONE_SELECT_Task(void);

/**
 * 10 millisecond timer handler for this module.
 * 
 * The parent module must call this from its 10 millisecond timer handler while 
 * ringtone selection is "active".
 */
void RINGTONE_SELECT_Timer10MS_event(void);

/**
 * Handset event handler for this module.
 * 
 * The parent module must call this from its handset event handler while 
 * ringtone selection is "active".
 * 
 * @param event - The handset event.
 */
void RINGTONE_SELECT_HANDSET_EventHandler(HANDSET_Event const *event);

#ifdef	__cplusplus
}
#endif

#endif	/* RINGTONE_SELECT_H */

