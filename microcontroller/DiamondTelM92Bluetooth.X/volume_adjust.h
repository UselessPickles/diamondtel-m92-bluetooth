/** 
 * @file
 * @author Jeff Lau
 * 
 * UI module for adjusting volume level.
 */

#ifndef VOLUME_ADJUST_H
#define	VOLUME_ADJUST_H

#include "handset.h"
#include "volume.h"

#ifdef	__cplusplus
extern "C" {
#endif

/**
 * Callback function to exit volume adjustment.
 */
typedef void (*VOLUME_ADJUST_ReturnCallback)(void);
  
/**
 * Start volume adjustment.
 * 
 * After calling this function, the parent module is responsible for calling
 * VOLUME_ADJUST_Task(), VOLUME_ADJUST_Timer10MS_event(), and
 * VOLUME_ADJUST_HANDSET_EventHandler() appropriately until the
 * `returnCallback` has been called.
 *
 * When the `returnCallback`, the parent module is responsible for updating the 
 * display as desired.
 * 
 * @param volumeMode - The volume mode to adjust.
 * @param isInitialAdjustUp - True if the initial adjustment is up. 
 *                            False if the initial adjustment is down. 
 * @param returnCallback - Callback that is called when volume adjustment is complete.
 */
void VOLUME_ADJUST_Start(VOLUME_Mode volumeMode, bool isInitialAdjustUp, VOLUME_ADJUST_ReturnCallback returnCallback);

/**
 * Main task loop handler for this module.
 * 
 * The parent module must call this from its main task loop handler while 
 * volume adjustment is "active".
 */
void VOLUME_ADJUST_Task(void);

/**
 * 10 millisecond timer handler for this module.
 * 
 * The parent module must call this from its 10 millisecond timer handler while 
 * volume adjustment is "active".
 */
void VOLUME_ADJUST_Timer10MS_event(void);

/**
 * Handset event handler for this module.
 * 
 * The parent module must call this from its handset event handler while 
 * volume adjustment is "active".
 * 
 * @param event - The handset event.
 */
void VOLUME_ADJUST_HANDSET_EventHandler(HANDSET_Event const* event);


#ifdef	__cplusplus
}
#endif

#endif	/* VOLUME_ADJUST_H */

