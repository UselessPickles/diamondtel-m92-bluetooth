/* 
 * File:   tetris_game.h
 * Author: Jeff
 *
 * Created on December 3, 2022, 9:08 PM
 */

#ifndef TETRIS_GAME_H
#define	TETRIS_GAME_H

#include "handset.h"

#ifdef	__cplusplus
extern "C" {
#endif

typedef void (*TETRIS_GAME_ReturnCallback)(void);
  
void TETRIS_GAME_Start(TETRIS_GAME_ReturnCallback returnCallback);

void TETRIS_GAME_Task(void);

void TETRIS_GAME_Timer10MS_Tick(void);

void TETRIS_GAME_HANDSET_EventHandler(HANDSET_Event const* event);

#ifdef	__cplusplus
}
#endif

#endif	/* TETRIS_GAME_H */

