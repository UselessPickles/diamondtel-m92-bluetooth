/** 
 * @file
 * @author Jeff Lau
 */

#ifndef APP_H
#define	APP_H

#include<stdint.h>
#include "telephone/handset.h"

#ifdef	__cplusplus
extern "C" {
#endif

void APP_Initialize(void);

void APP_Task(void);

void APP_Timer1MS_Interrupt(void);

void APP_Timer10MS_Interrupt(void);

void APP_BT_EventHandler(uint8_t event, uint16_t para, uint8_t* para_full);

#ifdef	__cplusplus
}
#endif

#endif	/* APP_H */

