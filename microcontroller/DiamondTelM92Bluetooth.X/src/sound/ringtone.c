/** 
 * @file
 * @author Jeff Lau
 */

#include "ringtone.h"
#include "sound.h"

static const struct {
  char name[8];
  SOUND_Effect effect;
} ringtones[RINGTONE_COUNT] = {
  { "CLASSIC", SOUND_Effect_STANDARD_RINGTONE },
  { "NOKIA  ", SOUND_Effect_NOKIA },
  { "AXEL F ", SOUND_Effect_AXEL_F },
  { "SANS   ", SOUND_Effect_MEGALOVANIA },
  { "C.PHONE", SOUND_Effect_CARPHONE },
  { "TETRIS ", SOUND_Effect_TETRIS_MUSIC }
};

char const* RINGTONE_GetName(RINGTONE_Type ringtone) {
  return ringtones[ringtone].name;
}

void RINGTONE_Start(RINGTONE_Type ringtone) {
  SOUND_PlayEffect(
    SOUND_Channel_BACKGROUND, 
    SOUND_Target_SPEAKER,
    VOLUME_Mode_ALERT,
    ringtones[ringtone].effect, 
    true
  );  
}

void RINGTONE_Stop(void) {
  SOUND_Stop(SOUND_Channel_BACKGROUND);
}

