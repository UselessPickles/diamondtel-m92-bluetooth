/* 
 * File:   volume.h
 * Author: Jeff
 *
 * API for controlling the volume level of sound to the phone handset.
 * 
 * Separate volume levels are maintained for each of the volume "modes".
 * 
 * Volume can be enabled/disabled independently of current volume mode and 
 * volume level.
 */

#ifndef VOLUME_H
#define	VOLUME_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>    
  
typedef enum VOLUME_Mode {
  VOLUME_Mode_ALERT,
  VOLUME_Mode_SPEAKER,
  VOLUME_Mode_GAME_MUSIC,
  VOLUME_Mode_HANDSET,
  VOLUME_Mode_HANDS_FREE,
  VOLUME_Mode_TONE,
} VOLUME_Mode;
  
#define VOLUME_MODE_COUNT (6)

typedef enum VOLUME_Level {
  VOLUME_Level_OFF,
  VOLUME_Level_MIN,
  VOLUME_Level_1 = VOLUME_Level_MIN,
  VOLUME_Level_2,
  VOLUME_Level_3,
  VOLUME_Level_MID,    
  VOLUME_Level_4 = VOLUME_Level_MID,
  VOLUME_Level_5,
  VOLUME_Level_6,
  VOLUME_Level_7,
  VOLUME_Level_MAX = VOLUME_Level_7    
} VOLUME_Level;  

/**
 * Initializes volume control.
 * 
 * Initial state is:
 * - Disabled (no audio output)
 * - Mode = VOLUME_Mode_TONE
 */
void VOLUME_Initialize(void);

/**
 * Enable audio output to the handset.
 * 
 * Volume will be set to the current volume level of the current volume mode.
 */
void VOLUME_Enable(void);

/**
 * Disable audio output to the handset.
 * 
 * Volume mode and level is maintained and can be changed while output is 
 * disabled.
 */
void VOLUME_Disable(void);

/**
 * Set the current volume mode.
 * 
 * If volume is enabled, then the volume level will be set to the current 
 * volume level of the specified mode.
 * 
 * If volume is disabled, then audio output will remain disabled, but the 
 * mode change will persist.
 * 
 * @param mode - Desired volume mode.
 */
void VOLUME_SetMode(VOLUME_Mode mode);

/**
 * Get the current volume level for a specified volume mode.
 * 
 * @param mode - A volume mode.
 * @return The current volume level of the specified volume mode.
 */
VOLUME_Level VOLUME_GetLevel(VOLUME_Mode mode);

/**
 * Set the volume level of a specified volume mode.
 * 
 * The new volume level is persisted in storage.
 * 
 * If volume is enabled, and the specified mode is the current mode, then
 * output volume will be set to the specified level.
 * 
 * @param mode - A volume mode.
 * @param level - Desired volume level.
 */
void VOLUME_SetLevel(VOLUME_Mode mode, VOLUME_Level level);

#ifdef	__cplusplus
}
#endif

#endif	/* VOLUME_H */

