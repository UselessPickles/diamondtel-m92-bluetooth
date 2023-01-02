/* 
 * File:   memory_game.h
 * Author: Jeff
 *
 * Created on November 27, 2022, 2:46 PM
 */

#ifndef MEMORY_GAME_H
#define	MEMORY_GAME_H

#include "../telephone/handset.h"

#ifdef	__cplusplus
extern "C" {
#endif

typedef void (*MEMORY_GAME_ReturnCallback)(void);
  
void MEMORY_GAME_Start(MEMORY_GAME_ReturnCallback returnCallback);

void MEMORY_GAME_Task(void);

void MEMORY_GAME_Timer10MS_Interrupt(void);

void MEMORY_GAME_HANDSET_EventHandler(HANDSET_Event const* event);

#ifdef	__cplusplus
}
#endif

#endif	/* MEMORY_GAME_H */

