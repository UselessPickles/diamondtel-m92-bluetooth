/* 
 * File:   app.h
 * Author: Jeff
 *
 * Created on January 13, 2022, 12:06 AM
 */

#ifndef APP_H
#define	APP_H

#include<stdint.h>
#include "handset.h"
#include "transceiver.h"
#include "clr_codes.h"
#include "volume_adjust.h"

#ifdef	__cplusplus
extern "C" {
#endif

void APP_Initialize(void);

void APP_Task(void);

void APP_Timer10MS_event(void);

void APP_TRANSCEIVER_EventHandler(TRANSCEIVER_EventType event);

void APP_BT_EventHandler(uint8_t event, uint16_t para, uint8_t* para_full);

void APP_ATCMD_UnsolicitedResultHandler(char const* result);

#ifdef	__cplusplus
}
#endif

#endif	/* APP_H */

