/* 
 * File:   volume_adjust.h
 * Author: Jeff
 *
 * Created on November 24, 2022, 11:46 PM
 */

#ifndef VOLUME_ADJUST_H
#define	VOLUME_ADJUST_H

#include "handset.h"
#include "volume.h"

#ifdef	__cplusplus
extern "C" {
#endif

typedef void (*VOLUME_ADJUST_ReturnCallback)(void);
  
void VOLUME_ADJUST_Start(VOLUME_Mode volumeMode, bool isInitialAdjustUp, VOLUME_ADJUST_ReturnCallback returnCallback);
void VOLUME_ADJUST_Task(void);
void VOLUME_ADJUST_Timer10MS_event(void);
void VOLUME_ADJUST_HANDSET_EventHandler(HANDSET_Event const* event);


#ifdef	__cplusplus
}
#endif

#endif	/* VOLUME_ADJUST_H */

