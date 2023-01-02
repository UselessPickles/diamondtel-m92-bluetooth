/** 
 * @file
 * @author Jeff Lau
 */

#ifndef CALL_TIMER_H
#define	CALL_TIMER_H

#include <stdbool.h>

#ifdef	__cplusplus
extern "C" {
#endif

void CALL_TIMER_Initialize(void);

void CALL_TIMER_Task(void);

void CALL_TIMER_Start(bool updateDisplay);

void CALL_TIMER_Stop(void);

void CALL_TIMER_EnableDisplayUpdate(void);

void CALL_TIMER_DisableDisplayUpdate(void);

#ifdef	__cplusplus
}
#endif

#endif	/* CALL_TIMER_H */

