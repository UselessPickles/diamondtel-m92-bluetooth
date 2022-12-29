/* 
 * File:   sound.h
 * Author: Jeff
 *
 * Created on January 22, 2022, 11:54 PM
 */
#ifndef SOUND_H
#define	SOUND_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdbool.h>    
#include <stdint.h>    
#include "tone.h"
#include "volume.h"
#include "handset.h"  

typedef enum SOUND_AudioSource {
  SOUND_AudioSource_MCU,
  SOUND_AudioSource_BT,
} SOUND_AudioSource;

typedef enum SOUND_Target {
  SOUND_Target_SPEAKER,
  SOUND_Target_EAR,   
} SOUND_Target;

typedef enum SOUND_Channel {
  SOUND_Channel_BACKGROUND,
  SOUND_Channel_FOREGROUND        
} SOUND_Channel;    
    
typedef enum SOUND_Effect {
  SOUND_Effect_NONE,  
  SOUND_Effect_TONE_LOW_CONTINUOUS,  
  SOUND_Effect_TONE_HIGH_CONTINUOUS,  
  SOUND_Effect_TONE_DUAL_CONTINUOUS,  
  SOUND_Effect_ALERT,  
  SOUND_Effect_REORDER_TONE,  
  SOUND_Effect_BT_CONNECT,    
  SOUND_Effect_BT_DISCONNECT,    
  SOUND_Effect_CALL_DISCONNECT,    
  SOUND_Effect_STANDARD_RINGTONE,  
  SOUND_Effect_AXEL_F,  
  SOUND_Effect_NOKIA,  
  SOUND_Effect_MEGALOVANIA,  
  SOUND_Effect_CARPHONE,
  SOUND_Effect_TETRIS_USER_MOVE,
  SOUND_Effect_TETRIS_USER_ROTATE,
  SOUND_Effect_TETRIS_PIECE_PLACED,
  SOUND_Effect_TETRIS_CLEAR_2_LINES,
  SOUND_Effect_TETRIS_CLEAR_3_LINES,
  SOUND_Effect_TETRIS_LOSE,
  SOUND_Effect_TETRIS_MUSIC
} SOUND_Effect;    

void SOUND_Initialize(void);

void SOUND_Disable(void);

void SOUND_Enable(void);

void SOUND_SetDefaultAudioSource(SOUND_AudioSource audioSource);

void SOUND_Timer1MS_Interrupt(void);

void SOUND_Task(void);

void SOUND_HANDSET_EventHandler(HANDSET_Event const* event);

void SOUND_PlayEffect(SOUND_Channel channel, SOUND_Target target, VOLUME_Mode volumeMode, SOUND_Effect soundEffect, bool repeat);

void SOUND_PlayDualTone(SOUND_Channel channel, SOUND_Target target, VOLUME_Mode volumeMode, tone_t tone1, const tone_t tone2, uint16_t duration);

void SOUND_PlaySingleTone(SOUND_Channel channel, SOUND_Target target, VOLUME_Mode volumeMode, tone_t tone, uint16_t duration);

void SOUND_SetButtonsMuted(bool isMuted);

bool SOUND_IsButtonsMuted(void);

void SOUND_PlayButtonBeep(HANDSET_Button button, bool limitDuration);

void SOUND_PlayDTMFButtonBeep(HANDSET_Button button, bool limitDuration);

void SOUND_Stop(SOUND_Channel channel);

void SOUND_StopButtonBeep(void);



#ifdef	__cplusplus
}
#endif

#endif	/* SOUND_H */

