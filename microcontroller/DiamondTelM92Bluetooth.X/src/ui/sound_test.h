/* 
 * File:   sound_test.h
 * Author: Jeff
 *
 * Created on March 12, 2023, 3:55 PM
 */

#ifndef SOUND_TEST_H
#define	SOUND_TEST_H

#include "../telephone/handset.h"

#ifdef	__cplusplus
extern "C" {
#endif

typedef void (*SOUND_TEST_ReturnCallback)(void);
  
void SOUND_TEST_Start(SOUND_TEST_ReturnCallback returnCallback);

void SOUND_TEST_Task(void);

void SOUND_TEST_HANDSET_EventHandler(HANDSET_Event const* event);

void SOUND_TEST_Timer10MS_Interrupt(void);


#ifdef	__cplusplus
}
#endif

#endif	/* SOUND_TEST_H */

