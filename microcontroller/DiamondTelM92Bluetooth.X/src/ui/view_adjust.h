/** 
 * @file
 * @author Jeff Lau
 *
 * UI module for adjusting the LCD viewing angle
 */

#ifndef VIEW_ADJUST_H
#define	VIEW_ADJUST_H

#include "../telephone/handset.h"

#ifdef	__cplusplus
extern "C" {
#endif

/**
 * Callback function to exit LCD view angle adjustment.
 */
typedef void (*VIEW_ADJUST_ReturnCallback)(void);

/**
 * Start LCD view angle adjustment.
 * 
 * After calling this function, the parent module is responsible for calling
 * VIEW_ADJUST_HANDSET_EventHandler() appropriately until the
 * `returnCallback` has been called.
 *
 * When the `returnCallback`, the parent module is responsible for updating the 
 * display as desired.
 * 
 * @param returnCallback - Callback that is called when LCD view angle adjustment is complete.
 */
void VIEW_ADJUST_Start(VIEW_ADJUST_ReturnCallback returnCallback);

/**
 * Handset event handler for this module.
 * 
 * The parent module must call this from its handset event handler while 
 * LCD view angle adjustment is "active".
 * 
 * @param event - The handset event.
 */
void VIEW_ADJUST_HANDSET_EventHandler(HANDSET_Event const* event);

#ifdef	__cplusplus
}
#endif

#endif	/* VIEW_ADJUST_H */

