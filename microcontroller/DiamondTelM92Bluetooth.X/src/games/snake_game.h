/** 
 * @file
 * @author Jeff Lau
 */

#ifndef SNAKE_H
#define	SNAKE_H

#include "../telephone/handset.h"

#ifdef	__cplusplus
extern "C" {
#endif

typedef void (*SNAKE_GAME_ReturnCallback)(void);
  
void SNAKE_GAME_Start(SNAKE_GAME_ReturnCallback returnCallback);

void SNAKE_GAME_Task(void);

void SNAKE_GAME_Timer10MS_Interrupt(void);

void SNAKE_GAME_HANDSET_EventHandler(HANDSET_Event const* event);

#ifdef	__cplusplus
}
#endif

#endif	/* SNAKE_H */

