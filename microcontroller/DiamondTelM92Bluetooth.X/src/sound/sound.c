/** 
 * @file
 * @author Jeff Lau
 */

#include "sound.h"
#include "tone.h"
#include "volume.h"
#include "../util/timeout.h"
#include "../../mcc_generated_files/pin_manager.h"
#include <xc.h>
#include <stddef.h>

#define BUTTON_BEEP_LIMITED_DURATION (250)

typedef enum SpeakerMode {
  SpeakerMode_NONE,
  SpeakerMode_IDLE,
  SpeakerMode_EAR,
  SpeakerMode_SPEAKER    
} SpeakerMode;

typedef struct Note {
  tone_t tone1;
  tone_t tone2;
  uint16_t duration;
} Note;

typedef struct Repeat {
  uint8_t count;
  uint8_t afterIndex;
  uint8_t returnToIndex;
} Repeat;

typedef struct {
  Note const* notes;
  uint8_t length;
  Repeat repeat;
} SoundEffect;

extern SoundEffect const effects[];

typedef struct {
  bool on;
  tone_t tone1;
  tone_t tone2;
  SoundEffect const* effect;
  uint8_t noteIndex;
  uint8_t internalRepeatCount;
  volatile uint16_t noteTimer;
  volatile bool noteTimerExpired;
  bool repeatEffect;
  SOUND_Target target;
  VOLUME_Mode volumeMode;
} SoundEffectState;

static volatile bool isInitialized = false;
static SoundEffectState soundEffectState[2];
static SoundEffectState* backgroundEffectState = &soundEffectState[SOUND_Channel_BACKGROUND];
static SoundEffectState* foregroundEffectState = &soundEffectState[SOUND_Channel_FOREGROUND];
static SOUND_Target currentTarget;
static bool isOnHook;
static SpeakerMode currentSpeakerMode;
static VOLUME_Mode currentVolumeMode;
static timeout_t finishSetHandsetAudioOutputTimeout;
static bool isDisabled = true;
static SOUND_AudioSource defaultAudioSource;
static bool isButtonsMuted;
static HANDSET_Button currentButtonBeep;
static bool forceNextSetHandsetAudioOutput;

static bool isSoundEffectOnAndNotMuted(SoundEffectState const* soundEffectState) {
  if (!soundEffectState->on) {
    return false;
  }
  
  return VOLUME_GetLevel(soundEffectState->volumeMode) != VOLUME_Level_OFF;
}

static bool playCurrentSoundEffectStateTone(SoundEffectState const* soundEffectState) {
  if (isSoundEffectOnAndNotMuted(soundEffectState)) {
    if (soundEffectState->effect) {
      Note const* note = soundEffectState->effect->notes + soundEffectState->noteIndex;
      TONE_PlayDualTone(note->tone1, note->tone2);
    } else {
      TONE_PlayDualTone(soundEffectState->tone1, soundEffectState->tone2);
    }
    return true;
  }
  
  return false;
}

static void playCurrentTone(void) {
  if (playCurrentSoundEffectStateTone(foregroundEffectState)) {
    return;
  }

  if (playCurrentSoundEffectStateTone(backgroundEffectState)) {
    return;
  }
  
  TONE_Stop();
}

static void setHandsetAudioOutput(void) {
  SOUND_AudioSource newAudioSource = SOUND_AudioSource_MCU;
  bool isPlayingSound = false;
  
  if (isSoundEffectOnAndNotMuted(foregroundEffectState)) {
    isPlayingSound = true;
    currentTarget = foregroundEffectState->target;
    currentVolumeMode = foregroundEffectState->volumeMode;
  } else if (isSoundEffectOnAndNotMuted(backgroundEffectState)) {
    isPlayingSound = true;
    currentTarget = backgroundEffectState->target;
    currentVolumeMode = backgroundEffectState->volumeMode;
  } else {
    newAudioSource = defaultAudioSource;

    currentTarget = (defaultAudioSource == SOUND_AudioSource_BT) 
        ? SOUND_Target_EAR 
        : SOUND_Target_SPEAKER;
    
    if (defaultAudioSource == SOUND_AudioSource_BT) {
      currentVolumeMode = isOnHook 
          ? VOLUME_Mode_HANDS_FREE 
          : VOLUME_Mode_HANDSET;
    }
  }
  
  // Stop playing the current tone now if we're no longer playing sound.
  // This allows the tone to gracefully end (with zero-crossing detection)
  // by the time the handset receives processes the commands to change the
  // sound/speaker status, helping avoid an audio "click".
  if (!isPlayingSound) {
    TONE_Stop();
  }
  
  SpeakerMode newSpeakerMode = isDisabled 
      ? SpeakerMode_NONE
      : ((newAudioSource != SOUND_AudioSource_BT) && !isPlayingSound) 
      ? SpeakerMode_IDLE
      : ((currentTarget == SOUND_Target_SPEAKER) || isOnHook)
      ? SpeakerMode_SPEAKER
      : SpeakerMode_EAR;

  if (forceNextSetHandsetAudioOutput || (newSpeakerMode != currentSpeakerMode)) {
    // Only disable volume if the the speaker mode is believed to be changing
    // (not just for a forced update) AND if any sound is currently playing
    // (including if the default source is the Bluetooth module, meaning that
    // Bluetooth voice audio may be playing).
    // Disabling volume helps ensure a clean transition between sound sources
    // and speaker modes by avoiding cross-over of the old sound source playing 
    // through the new speaker mode or volume level, etc.
    if (
        newSpeakerMode != currentSpeakerMode && 
        (isPlayingSound || (defaultAudioSource == SOUND_AudioSource_BT))
        ) {
      VOLUME_Disable();
    }

    if (forceNextSetHandsetAudioOutput) {
      HANDSET_DisableCommandOptimization();
    }
    
    if (newSpeakerMode == SpeakerMode_SPEAKER) {
      HANDSET_SetMasterAudio(true);
      HANDSET_SetEarSpeaker(false);
      HANDSET_SetLoudSpeaker(true);
    } else if (newSpeakerMode == SpeakerMode_EAR) {
      HANDSET_SetMasterAudio(true);
      HANDSET_SetLoudSpeaker(false);
      HANDSET_SetEarSpeaker(true);
    } else if (newSpeakerMode == SpeakerMode_IDLE) {
      HANDSET_SetMasterAudio(true);
      HANDSET_SetLoudSpeaker(false);
      HANDSET_SetEarSpeaker(false);
    } else {
      HANDSET_SetEarSpeaker(false);
      HANDSET_SetLoudSpeaker(false);
      HANDSET_SetMasterAudio(false);
    }

    if (forceNextSetHandsetAudioOutput) {
      HANDSET_EnableCommandOptimization();
    }

    HANDSET_FlushWriteBuffer();

    // Handset microphone should only be on if the handset is off-hook
    // AND audio is coming from the Bluetooth module by default (e.g., in a call))
    HANDSET_SetMicrophone(!isOnHook && (defaultAudioSource == SOUND_AudioSource_BT));
    
    TIMEOUT_StartOrContinue(
        &finishSetHandsetAudioOutputTimeout,
        (newSpeakerMode == SpeakerMode_NONE) 
          ? 0 
          : (currentSpeakerMode == SpeakerMode_NONE) 
          ? 100
          : 20
    );
    
    currentSpeakerMode = newSpeakerMode;
  } else if (!TIMEOUT_IsPending(&finishSetHandsetAudioOutputTimeout)) {
    VOLUME_SetMode(currentVolumeMode);
    playCurrentTone();
  }

  if (newAudioSource == SOUND_AudioSource_BT) {
    IO_VOICE_IN_SetHigh();
  } else {
    IO_VOICE_IN_SetLow();
  }
  
  if (isOnHook) {
    // Handset is "on hook", so select the hands-free (external) microphone input.
    IO_MIC_HF_SELECT_SetHigh();
  } else {
    // Handset is "off hook", so select the handset microphone input.
    IO_MIC_HF_SELECT_SetLow();
  }
  
  if (isPlayingSound) {
    // Disconnect the microphone signal from the Bluetooth module when we're
    // playing a sound to prevent the sound from being picked up by the 
    // microphone and sent to the listener on the other end of the call.
    IO_MIC_OUT_DISABLE_SetHigh();
  } else {
    IO_MIC_OUT_DISABLE_SetLow();
  }
  
  forceNextSetHandsetAudioOutput = false;
}

static void finishSetHandsetAudioOutput(void) {
  VOLUME_SetMode(currentVolumeMode);
  VOLUME_Enable();
  playCurrentTone();
}

static bool soundEffectStateTask(SOUND_Channel channel) {
  SoundEffectState* effectState = &soundEffectState[channel];
  
  if (effectState->on && effectState->noteTimerExpired) {
    if (effectState->effect) {
      if (
          effectState->noteIndex != 0 &&
          (effectState->noteIndex == effectState->effect->repeat.afterIndex) &&
          (effectState->internalRepeatCount < effectState->effect->repeat.count)
          ) {
        effectState->noteIndex = effectState->effect->repeat.returnToIndex;
        ++effectState->internalRepeatCount;
      } else if (++effectState->noteIndex == effectState->effect->length) {
        if (
            (effectState->internalRepeatCount < effectState->effect->repeat.count) &&
            (effectState->effect->repeat.afterIndex == 0)
            ) {
          effectState->noteIndex = effectState->effect->repeat.returnToIndex;
          ++effectState->internalRepeatCount;
        } else if (effectState->repeatEffect) {
          effectState->noteIndex = 0;
          effectState->internalRepeatCount = 0;
        } else {
          effectState->on = false;
        }
      }
    } else {
      effectState->on = false;
    }

    if (effectState->on) {
      effectState->noteTimer = effectState->effect->notes[effectState->noteIndex].duration;
    } else if (channel == SOUND_Channel_FOREGROUND) {
      currentButtonBeep = HANDSET_Button_NONE;
    }
    
    effectState->noteTimerExpired = false;
    
    return true;
  }  
  
  return false;
}

void SOUND_Initialize(void) {
  defaultAudioSource = SOUND_AudioSource_MCU;
  currentTarget = SOUND_Target_SPEAKER;
  currentSpeakerMode = SpeakerMode_NONE;
  currentVolumeMode = VOLUME_Mode_SPEAKER;
  currentButtonBeep = HANDSET_Button_NONE;
  isOnHook = true;
  isInitialized = true;
  isDisabled = false;
  isButtonsMuted = false;
  forceNextSetHandsetAudioOutput = false;
  
  SOUND_PlayEffect(
      SOUND_Channel_FOREGROUND, 
      SOUND_Target_SPEAKER,
      VOLUME_Mode_SPEAKER,
      SOUND_Effect_TONE_DUAL_CONTINUOUS, 
      true
  );
}

void SOUND_ForceNextSetHandsetAudioOutput(void) {
  forceNextSetHandsetAudioOutput = true;
}

void SOUND_Disable(void) {
  if (isDisabled) {
    return;
  }
  
  isDisabled = true;
  setHandsetAudioOutput();
}

void SOUND_Enable(void) {
  if (!isDisabled) {
    return;
  }
  
  isDisabled = false;
  setHandsetAudioOutput();
}

void SOUND_SetDefaultAudioSource(SOUND_AudioSource audioSource) {
  defaultAudioSource = audioSource;
  setHandsetAudioOutput();
}

void SOUND_Timer1MS_Interrupt(void) {
  if (!isInitialized) {
    return;
  }
  
  
  TIMEOUT_Timer_Interrupt(&finishSetHandsetAudioOutputTimeout);
  
  if (TIMEOUT_IsPending(&finishSetHandsetAudioOutputTimeout)) {
    return;
  }
  
  if (!backgroundEffectState->noteTimerExpired && backgroundEffectState->noteTimer) {
    if (!--backgroundEffectState->noteTimer) {
      backgroundEffectState->noteTimerExpired = true;
    }
  }

  if (!foregroundEffectState->noteTimerExpired && foregroundEffectState->noteTimer) {
    if (!--foregroundEffectState->noteTimer) {
      foregroundEffectState->noteTimerExpired = true;
    }
  }
}

void SOUND_Task(void) {
  if (!isInitialized) {
    return;
  }
  
  if (TIMEOUT_Task(&finishSetHandsetAudioOutputTimeout)) {
    finishSetHandsetAudioOutput();
    return;
  } 
  
  if (TIMEOUT_IsPending(&finishSetHandsetAudioOutputTimeout)) {
    return;
  }
  
  if (soundEffectStateTask(SOUND_Channel_BACKGROUND) || soundEffectStateTask(SOUND_Channel_FOREGROUND)) {
    setHandsetAudioOutput();
  }  
}

void SOUND_HANDSET_EventHandler(HANDSET_Event const* event) {
  if (event->type == HANDSET_EventType_BUTTON_UP) {
    if (event->button == currentButtonBeep) {
      SOUND_Stop(SOUND_Channel_FOREGROUND);
    }
  } else if (event->type == HANDSET_EventType_HOOK) {
    if (event->isOnHook != isOnHook) {
      isOnHook = event->isOnHook;
      setHandsetAudioOutput();
    }
  }
}

void SOUND_PlayEffect(SOUND_Channel channel, SOUND_Target target, VOLUME_Mode volumeMode, SOUND_Effect soundEffect, bool repeat) {
  if (soundEffect == SOUND_Effect_NONE) {
    SOUND_Stop(channel);
    return;
  }
  
  SoundEffectState* const state = &soundEffectState[channel];

  state->noteTimerExpired = true;
  state->on = true;
  state->effect = &effects[soundEffect];
  state->tone1 = TONE_OFF;
  state->tone2 = TONE_OFF;
  state->noteIndex = 0;
  state->internalRepeatCount = 0;
  state->noteTimer = state->effect->notes->duration;
  state->repeatEffect = repeat;
  state->target = target;
  state->volumeMode = volumeMode;

  setHandsetAudioOutput();
  
  if (currentButtonBeep && (channel == SOUND_Channel_FOREGROUND)) {
    currentButtonBeep = HANDSET_Button_NONE;
  }

  state->noteTimerExpired = false;
}

void SOUND_PlayDualTone(SOUND_Channel channel, SOUND_Target target, VOLUME_Mode volumeMode, tone_t tone1, tone_t tone2, uint16_t duration) {
  if (!tone1 && !tone2) {
    SOUND_Stop(channel);
    return;
  }
  
  SoundEffectState* const state = &soundEffectState[channel];
  
  state->noteTimerExpired = true;
  state->on = true;
  state->effect = NULL;
  state->tone1 = tone1;
  state->tone2 = tone2;
  state->noteIndex = 0;
  state->noteTimer = duration;
  state->repeatEffect = false;
  state->target = target;
  state->volumeMode = volumeMode;
  
  setHandsetAudioOutput();
  
  if (currentButtonBeep && (channel == SOUND_Channel_FOREGROUND)) {
    currentButtonBeep = HANDSET_Button_NONE;
  }
  
  state->noteTimerExpired = false;
}

void SOUND_PlaySingleTone(SOUND_Channel channel, SOUND_Target target, VOLUME_Mode volumeMode, tone_t tone, uint16_t duration) {
  SOUND_PlayDualTone(channel, target, volumeMode, tone, TONE_OFF, duration);
}

void SOUND_SetButtonsMuted(bool isMuted) {
  isButtonsMuted = isMuted;
}

bool SOUND_IsButtonsMuted(void) {
  return isButtonsMuted;
}

void SOUND_PlayButtonBeep(HANDSET_Button button, bool limitDuration) {
  if (isButtonsMuted || (VOLUME_GetLevel(VOLUME_Mode_SPEAKER) == VOLUME_Level_OFF)) {
    return;
  }
  
  SOUND_PlaySingleTone(
      SOUND_Channel_FOREGROUND, 
      SOUND_Target_SPEAKER, 
      VOLUME_Mode_SPEAKER, 
      TONE_LOW, 
      limitDuration ? BUTTON_BEEP_LIMITED_DURATION : 0
      );
  
  currentButtonBeep = button;
}

void SOUND_PlayDTMFButtonBeep(HANDSET_Button button, bool limitDuration) {
  if (isButtonsMuted || (VOLUME_GetLevel(VOLUME_Mode_SPEAKER) == VOLUME_Level_OFF)) {
    return;
  }
  
  tone_t columnTone;
  tone_t rowTone;
  
  switch (button) {
    case HANDSET_Button_1:
    case HANDSET_Button_2:
    case HANDSET_Button_3:
      rowTone = TONE_DTMF_ROW1;
      break;
      
    case HANDSET_Button_4:
    case HANDSET_Button_5:
    case HANDSET_Button_6:
      rowTone = TONE_DTMF_ROW2;
      break;

    case HANDSET_Button_7:
    case HANDSET_Button_8:
    case HANDSET_Button_9:
      rowTone = TONE_DTMF_ROW3;
      break;

    case HANDSET_Button_ASTERISK:
    case HANDSET_Button_0:
    case HANDSET_Button_POUND:
      rowTone = TONE_DTMF_ROW4;
      break;
      
    default:
      rowTone = TONE_LOW;
      break;
}

  switch(button) {
    case HANDSET_Button_1:
    case HANDSET_Button_4:
    case HANDSET_Button_7:
    case HANDSET_Button_ASTERISK:
      columnTone = TONE_DTMF_COL1;
      break;
      
    case HANDSET_Button_2:
    case HANDSET_Button_5:
    case HANDSET_Button_8:
    case HANDSET_Button_0:
      columnTone = TONE_DTMF_COL2;
      break;

    case HANDSET_Button_3:
    case HANDSET_Button_6:
    case HANDSET_Button_9:
    case HANDSET_Button_POUND:
      columnTone = TONE_DTMF_COL3;
      break;
      
    default:
      columnTone = TONE_OFF;
      break;
  }
  
  SOUND_PlayDualTone(
      SOUND_Channel_FOREGROUND, 
      SOUND_Target_SPEAKER, 
      VOLUME_Mode_SPEAKER, 
      rowTone, 
      columnTone, 
      limitDuration ? BUTTON_BEEP_LIMITED_DURATION : 0
      );
  
  currentButtonBeep = button;
}

void SOUND_StopButtonBeep(void) {
  if (currentButtonBeep) {
    SOUND_Stop(SOUND_Channel_FOREGROUND);
  }
}

void SOUND_PlayStatusBeep(void) {
  SOUND_PlaySingleTone(
          SOUND_Channel_FOREGROUND, 
          SOUND_Target_SPEAKER, 
          VOLUME_Mode_TONE, 
          TONE_HIGH, 
          500
      );
}

void SOUND_Stop(SOUND_Channel channel) {
  if (!soundEffectState[channel].on) {
    return;
  }
  
  soundEffectState[channel].noteTimerExpired = true;
  soundEffectState[channel].on = false;  

  setHandsetAudioOutput();

  if (channel == SOUND_Channel_FOREGROUND) {
    currentButtonBeep = HANDSET_Button_NONE;
  }
}

VOLUME_Level SOUND_GetVolumeLevel(VOLUME_Mode mode) {
  return VOLUME_GetLevel(mode);
}

void SOUND_SetVolumeLevel(VOLUME_Mode mode, VOLUME_Level level) {
  VOLUME_SetLevel(mode, level);
  
  if (mode == currentVolumeMode) {
    setHandsetAudioOutput();
  }
}

static Note const TONE_LOW_CONTINUOUS_NOTES[] = {
  { TONE_LOW, TONE_OFF, 0 }    
};

static Note const TONE_HIGH_CONTINUOUS_NOTES[] = {
  { TONE_HIGH, TONE_OFF, 0 }    
};

static Note const TONE_DUAL_CONTINUOUS_NOTES[] = {
  { TONE_LOW, TONE_HIGH, 0 }    
};

static Note const ALERT_NOTES[] = {
  { TONE_HIGH, TONE_LOW, 25 },
  { TONE_OFF, TONE_OFF, 25 }    
};

static Note const REORDER_TONE_NOTES[] = {
  { TONE_HIGH, TONE_OFF, 250 },
  { TONE_LOW, TONE_OFF, 250 }    
};

static Note const BT_CONNECT_NOTES[] = {
  { TONE_LOW, TONE_OFF, 250 },
  { TONE_HIGH, TONE_OFF, 250 }
};

static Note const BT_DISCONNECT_NOTES[] = {
  { TONE_HIGH, TONE_OFF, 250 },
  { TONE_LOW, TONE_OFF, 250 }    
};

static Note const CALL_DISCONNECT_NOTES[] = {
  { TONE_LOW, TONE_OFF, 250 },
  { TONE_OFF, TONE_OFF, 250 },    
};

static Note const CLASSIC_RINGTONE_NOTES[] = {
  { TONE_HIGH, TONE_LOW, 25 },
  { TONE_OFF, TONE_OFF, 25 },    
  { TONE_OFF, TONE_OFF, 3000 },    
};

static Note const SMOOTH_RINGTONE_NOTES[] = {
  { TONE_HIGH, TONE_OFF, 25 },
  { TONE_LOW, TONE_OFF, 25 },    
  { TONE_OFF, TONE_OFF, 3000 },    
};


#define SPACE (50)
#define NOTE(note, duration) { TONE_##note, TONE_OFF, duration - SPACE }, { TONE_OFF, TONE_OFF, SPACE }
#define DUAL_NOTE(note1, note2, duration) { TONE_##note1, TONE_##note2, duration - SPACE }, { TONE_OFF, TONE_OFF, SPACE }
#define NOTE_SLUR(note, duration) { TONE_##note, TONE_OFF, duration }
#define DUAL_NOTE_SLUR(note1, note2, duration) { TONE_##note1, TONE_##note2, duration }
#define REST(duration) { TONE_OFF, TONE_OFF, duration }

static Note const AXEL_F_NOTES[] = {
  // Measure 1
  NOTE(F5, 500),
  NOTE(GS5, 250 + 125),
  NOTE(F5, 250),
  NOTE(F5, 125),
  NOTE(AS5, 250),
  NOTE(F5, 250),
  NOTE(DS5, 250),

  // Measure 2
  NOTE(F5, 500),
  NOTE(C6, 250 + 125),
  NOTE(F5, 250),
  NOTE(F5, 125),
  NOTE(CS6, 250),
  NOTE(C6, 250),
  NOTE(GS5, 250),

  // Measure 3
  NOTE(F5, 250),
  NOTE(C6, 250),
  NOTE(F6, 250),
  NOTE(F5, 125),
  NOTE(DS5, 250),
  NOTE(DS5, 125),
  NOTE(C5, 250),
  NOTE(G5, 250),
  NOTE_SLUR(F5, 250),
  
  // Measure 4
  NOTE(F5, 1000),
  REST(1000),
};

static Note const NOKIA_NOTES[] = {
  NOTE_SLUR(E6, 125),
  NOTE_SLUR(D6, 125),
  NOTE_SLUR(FS5, 250),
  NOTE_SLUR(GS5, 250),
  
  NOTE_SLUR(CS6, 125),
  NOTE_SLUR(B5, 125),
  NOTE_SLUR(D5, 250),
  NOTE_SLUR(E5, 250),

  NOTE_SLUR(B5, 125),
  NOTE_SLUR(A5, 125),
  NOTE_SLUR(CS5, 250),
  NOTE_SLUR(E5, 250),
  
  NOTE_SLUR(A5, 750),
  
  REST(750),
};

static Note const MEGALOVANIA_NOTES[] = {
  // Measure 1
  DUAL_NOTE(D4, D5, 125),
  DUAL_NOTE(D4, D5, 125),
  DUAL_NOTE(D5, D6, 125),
  REST(125),
  DUAL_NOTE(A4, A5, 125),
  REST(125),
  REST(125),
  DUAL_NOTE(GS4, GS5, 125),
  REST(125),
  DUAL_NOTE(G4, G5, 125),
  REST(125),
  DUAL_NOTE(F4, F5, 250),
  DUAL_NOTE(D4, D5, 125),
  DUAL_NOTE(F4, F5, 125),
  DUAL_NOTE(F4, F5, 125),

  // Measure 2
  DUAL_NOTE(C4, C5, 125),
  DUAL_NOTE(C4, C5, 125),
  DUAL_NOTE(D5, D6, 125),
  REST(125),
  DUAL_NOTE(A4, A5, 125),
  REST(125),
  REST(125),
  DUAL_NOTE(GS4, GS5, 125),
  REST(125),
  DUAL_NOTE(G4, G5, 125),
  REST(125),
  DUAL_NOTE(F4, F5, 250),
  DUAL_NOTE(D4, D5, 125),
  DUAL_NOTE(F4, F5, 125),
  DUAL_NOTE(F4, F5, 125),

  // Measure 3
  DUAL_NOTE(B3, B4, 125),
  DUAL_NOTE(B3, B4, 125),
  DUAL_NOTE(D5, D6, 125),
  REST(125),
  DUAL_NOTE(A4, A5, 125),
  REST(125),
  REST(125),
  DUAL_NOTE(GS4, GS5, 125),
  REST(125),
  DUAL_NOTE(G4, G5, 125),
  REST(125),
  DUAL_NOTE(F4, F5, 250),
  DUAL_NOTE(D4, D5, 125),
  DUAL_NOTE(F4, F5, 125),
  DUAL_NOTE(F4, F5, 125),

  // Measure 4
  DUAL_NOTE(AS3, AS4, 125),
  DUAL_NOTE(AS3, AS4, 125),
  DUAL_NOTE(D5, D6, 125),
  REST(125),
  DUAL_NOTE(A4, A5, 125),
  REST(125),
  REST(125),
  DUAL_NOTE(GS4, GS5, 125),
  REST(125),
  DUAL_NOTE(G4, G5, 125),
  REST(125),
  DUAL_NOTE(F4, F5, 250),
  DUAL_NOTE(D4, D5, 125),
  DUAL_NOTE(F4, F5, 125),
  DUAL_NOTE(F4, F5, 125),
};

static Note const CARPHONE_NOTES[] = {
  // Measure 1
  DUAL_NOTE(DS5, DS6, 220), // Call-
  DUAL_NOTE(DS5, DS6, 220), // -ing
  DUAL_NOTE(DS5, DS6, 220), // an-
  DUAL_NOTE(DS5, DS6, 220), // -y-
  DUAL_NOTE(DS5, DS6, 440), // -one
  DUAL_NOTE(DS5, DS6, 220), // an-
  DUAL_NOTE(DS5, DS6, 220), // y-
  
  DUAL_NOTE_SLUR(DS5, DS6, 220), // -one
  DUAL_NOTE(D5, D6, 220),   // I
  DUAL_NOTE(D5, D6, 440),   // want
  DUAL_NOTE(D5, D6, 220),   // when
  DUAL_NOTE(D5, D6, 220),   // I'm
  DUAL_NOTE(D5, D6, 220),   // on
  DUAL_NOTE(D5, D6, 220),   // my
  
  DUAL_NOTE(DS5, DS6, 440), // car
  DUAL_NOTE(AS4, AS5, 440), // phone
  REST(220),
  DUAL_NOTE(AS4, AS5, 220), // my
  DUAL_NOTE(DS5, DS6, 440), // car
  
  DUAL_NOTE(AS4, AS5, 440), // phone
  REST(440),
  REST(880),

  // Measure 2
  DUAL_NOTE(DS5, DS6, 220), // I
  DUAL_NOTE(DS5, DS6, 220), // can
  DUAL_NOTE(DS5, DS6, 440), // call
  DUAL_NOTE(DS5, DS6, 220), // an-
  DUAL_NOTE(DS5, DS6, 220), // -y-
  DUAL_NOTE(DS5, DS6, 220), // -one
  DUAL_NOTE_SLUR(DS5, DS6, 220), // I
  
  DUAL_NOTE(D5, D6, 440),   // want
  REST(440),
  DUAL_NOTE(D5, D6, 220),   // when
  DUAL_NOTE(D5, D6, 220),   // I'm
  DUAL_NOTE(D5, D6, 220),   // on
  DUAL_NOTE(D5, D6, 220),   // my
  
  DUAL_NOTE(DS5, DS6, 440), // car
  DUAL_NOTE(AS4, AS5, 440), // phone
  REST(220),
  DUAL_NOTE(AS4, AS5, 220), // my
  DUAL_NOTE(DS5, DS6, 440), // car
  
  DUAL_NOTE(AS4, AS5, 440), // phone
  REST(440),
  REST(880),

  // Measure 3
  DUAL_NOTE(DS5, DS6, 220), // Call-
  DUAL_NOTE(DS5, DS6, 220), // -ing
  DUAL_NOTE(DS5, DS6, 220), // an-
  DUAL_NOTE(DS5, DS6, 220), // -y-
  DUAL_NOTE(DS5, DS6, 440), // -one
  DUAL_NOTE(DS5, DS6, 220), // an-
  DUAL_NOTE(DS5, DS6, 220), // y-
  
  DUAL_NOTE_SLUR(DS5, DS6, 220), // -one
  DUAL_NOTE(F5, F6, 220),   // I
  DUAL_NOTE(F5, F6, 440),   // want
  DUAL_NOTE(F5, F6, 220),   // when
  DUAL_NOTE(F5, F6, 220),   // I'm
  DUAL_NOTE(F5, F6, 220),   // on
  DUAL_NOTE(F5, F6, 220),   // my
  
  DUAL_NOTE(FS5, FS6, 440), // car
  DUAL_NOTE(DS5, DS6, 440), // phone
  REST(220),
  DUAL_NOTE(DS5, DS6, 220), // my
  DUAL_NOTE(F5, F6, 440),   // car
  
  DUAL_NOTE_SLUR(AS5, AS6, 660), // pho-
  DUAL_NOTE_SLUR(GS5, GS6, 220), // -o-
  DUAL_NOTE(FS5, FS6, 440),      // -one
  REST(440),
};

static Note const TETRIS_USER_MOVE_NOTES[] = {
  { TONE_A5, 0, 25 }
};

static Note const TETRIS_USER_ROTATE_NOTES[] = {
  { TONE_E6, TONE_E5, 20 },
  { 0, 0, 20 },
  { TONE_E6, TONE_E5, 20 }
};

static Note const TETRIS_PIECE_PLACED_NOTES[] = {
  { TONE_G5, TONE_G4, 25 },
  { TONE_F5, TONE_F4, 25 }
};

static Note const TETRIS_CLEAR_2_LINES_NOTES[] = {
  { TONE_G5, 0, 50 },
  { TONE_A5, 0, 50 },
  { TONE_D6, 0, 50 },
};

static Note const TETRIS_CLEAR_3_LINES_NOTES[] = {
  { TONE_F5, 0, 50 },
  { TONE_G5, 0, 50 },
  { TONE_A5, 0, 50 },
  { TONE_G5, 0, 50 },
  { TONE_A5, 0, 50 },
  { TONE_D6, 0, 50 },
};

static Note const TETRIS_LOSE_NOTES[] = {
  { TONE_G5, TONE_G4, 25 },
  { TONE_F5, TONE_F4, 25 }
};

static Note const TETRIS_MUSIC_NOTES[] = {
  // Measure 1
  DUAL_NOTE(B4, E5, 400),
  DUAL_NOTE(GS4, B4, 200),
  DUAL_NOTE(A4, C5, 200),
  DUAL_NOTE(B4, D5, 200),
  NOTE(E5, 100),
  NOTE(D5, 100),
  DUAL_NOTE(A4, C5, 200),
  DUAL_NOTE(GS4, B4, 200),
  
  // Measure 2
  DUAL_NOTE(E4, A4, 400),
  DUAL_NOTE(E4, A4, 200),
  DUAL_NOTE(A4, C5, 200),
  DUAL_NOTE(C5, E5, 400),
  DUAL_NOTE(B4, D5, 200),
  DUAL_NOTE(A4, C5, 200),
  
  // Measure 3
  DUAL_NOTE_SLUR(GS4, B4, 100),
  DUAL_NOTE_SLUR(GS4, B4, 100),
  DUAL_NOTE(E4, B4, 200),
  NOTE(GS4, 200),
  DUAL_NOTE(A4, C5, 200),
  DUAL_NOTE(B4, D5, 400),
  DUAL_NOTE(C5, E5, 400),
  
  // Measure 4
  DUAL_NOTE(A4, C5, 400),
  DUAL_NOTE(E4, A4, 400),
  DUAL_NOTE(E4, A4, 400),
  DUAL_NOTE(B3, B3, 200),
  DUAL_NOTE(C4, C4, 200),
  
  // Measure 5
  DUAL_NOTE(D4, D4, 200),
  DUAL_NOTE(F4, D5, 400),
  DUAL_NOTE(A4, F5, 200),
  DUAL_NOTE(C5, A5, 200),
  DUAL_NOTE(C5, A5, 100),
  DUAL_NOTE(C5, A5, 100),
  DUAL_NOTE(B4, G5, 200),
  DUAL_NOTE(A4, F5, 200),

  // Measure 6
  DUAL_NOTE(G4, E5, 400),
  REST(200),
  DUAL_NOTE(E4, C5, 200),
  DUAL_NOTE_SLUR(G4, E5, 200),
  DUAL_NOTE_SLUR(A4, E5, 100),
  DUAL_NOTE(G4, E5, 100),
  DUAL_NOTE(F4, D5, 200),
  DUAL_NOTE(E4, C5, 200),
  
  // Measure 7
  DUAL_NOTE_SLUR(GS4, B4, 200),
  DUAL_NOTE(E4, B4, 200),
  DUAL_NOTE(GS4, B4, 200),
  DUAL_NOTE(A4, C4, 200),
  DUAL_NOTE_SLUR(B4, D5, 200),
  DUAL_NOTE(GS4, D5, 200),
  DUAL_NOTE_SLUR(C5, E5, 200),
  DUAL_NOTE(GS4, E5, 200),
  
  // Measure 8
  DUAL_NOTE_SLUR(A4, C5, 100),
  DUAL_NOTE_SLUR(C5, C5, 100),
  DUAL_NOTE(E4, C5, 200),
  DUAL_NOTE(E4, A4, 400),
  DUAL_NOTE(E4, A4, 400),
  REST(400),
  
  // Measure 9
  DUAL_NOTE_SLUR(A3, E5, 200),
  DUAL_NOTE_SLUR(E4, E5, 200),
  DUAL_NOTE_SLUR(A3, E5, 200),
  DUAL_NOTE(E4, E5, 200),
  DUAL_NOTE_SLUR(A3, C5, 200),
  DUAL_NOTE_SLUR(E4, C5, 200),
  DUAL_NOTE_SLUR(A3, C5, 200),
  DUAL_NOTE(E4, C5, 200),
  
  // Measure 10
  DUAL_NOTE_SLUR(GS3, D5, 200),
  DUAL_NOTE_SLUR(E4, D5, 200),
  DUAL_NOTE_SLUR(GS3, D5, 200),
  DUAL_NOTE(E4, D5, 200),
  DUAL_NOTE_SLUR(GS3, B4, 200),
  DUAL_NOTE_SLUR(E4, B4, 200),
  DUAL_NOTE_SLUR(GS3, B4, 200),
  DUAL_NOTE(E4, B4, 200),
  
  // Measure 11
  DUAL_NOTE_SLUR(A3, C5, 200),
  DUAL_NOTE_SLUR(E4, C5, 200),
  DUAL_NOTE_SLUR(A3, C5, 200),
  DUAL_NOTE(E4, C5, 200),
  DUAL_NOTE_SLUR(A3, A4, 200),
  DUAL_NOTE_SLUR(E4, A4, 200),
  DUAL_NOTE_SLUR(A3, A4, 200),
  DUAL_NOTE(E4, A4, 200),
  
  // Measure 12
  DUAL_NOTE_SLUR(GS3, GS4, 200),
  DUAL_NOTE_SLUR(E4, GS4, 200),
  DUAL_NOTE_SLUR(GS3, GS4, 200),
  DUAL_NOTE(E4, GS4, 200),
  DUAL_NOTE_SLUR(GS3, B4, 400),
  REST(400),
  
  // Measure 13
  DUAL_NOTE_SLUR(A3, E5, 200),
  DUAL_NOTE_SLUR(E4, E5, 200),
  DUAL_NOTE_SLUR(A3, E5, 200),
  DUAL_NOTE(E4, E5, 200),
  DUAL_NOTE_SLUR(A3, C5, 200),
  DUAL_NOTE_SLUR(E4, C5, 200),
  DUAL_NOTE_SLUR(A3, C5, 200),
  DUAL_NOTE(E4, C5, 200),
  
  // Measure 14
  DUAL_NOTE_SLUR(GS3, D5, 200),
  DUAL_NOTE_SLUR(E4, D5, 200),
  DUAL_NOTE_SLUR(GS3, D5, 200),
  DUAL_NOTE(E4, D5, 200),
  DUAL_NOTE_SLUR(GS3, B4, 200),
  DUAL_NOTE_SLUR(E4, B4, 200),
  DUAL_NOTE_SLUR(GS3, B4, 200),
  DUAL_NOTE(E4, B4, 200),
  
  // Measure 15
  DUAL_NOTE_SLUR(A3, C5, 200),
  DUAL_NOTE(E4, C5, 200),
  DUAL_NOTE_SLUR(A3, E5, 200),
  DUAL_NOTE(E4, E5, 200),
  DUAL_NOTE_SLUR(A3, A5, 200),
  DUAL_NOTE_SLUR(E4, A5, 200),
  DUAL_NOTE_SLUR(A3, A5, 200),
  DUAL_NOTE(E4, A5, 200),
  
  // Measure 16
  DUAL_NOTE_SLUR(GS3, GS5, 200),
  DUAL_NOTE_SLUR(E4, GS5, 200),
  DUAL_NOTE_SLUR(GS3, GS5, 200),
  DUAL_NOTE(E4, GS5, 200),
  DUAL_NOTE(GS3, GS5, 400),
  REST(400)
};

SoundEffect const effects[] = {
  {
    NULL,
    0
  },
  {
    TONE_LOW_CONTINUOUS_NOTES,
    sizeof(TONE_LOW_CONTINUOUS_NOTES) / sizeof(Note)
  },
  {
    TONE_HIGH_CONTINUOUS_NOTES,
    sizeof(TONE_HIGH_CONTINUOUS_NOTES) / sizeof(Note)
  },
  {
    TONE_DUAL_CONTINUOUS_NOTES,
    sizeof(TONE_DUAL_CONTINUOUS_NOTES) / sizeof(Note)
  },
  {
    ALERT_NOTES,
    sizeof(ALERT_NOTES) / sizeof(Note),
  },
  {
    REORDER_TONE_NOTES,
    sizeof(REORDER_TONE_NOTES) / sizeof(Note)
  },
  {
    BT_CONNECT_NOTES,
    sizeof(BT_CONNECT_NOTES) / sizeof(Note)
  },
  {
    BT_DISCONNECT_NOTES,
    sizeof(BT_DISCONNECT_NOTES) / sizeof(Note)
  },
  {
    CALL_DISCONNECT_NOTES,
    sizeof(CALL_DISCONNECT_NOTES) / sizeof(Note),
    {
      2
    }
  },
  {
    CLASSIC_RINGTONE_NOTES,
    sizeof(CLASSIC_RINGTONE_NOTES) / sizeof(Note),
    {
      19,
      1
    }
  },
  {
    SMOOTH_RINGTONE_NOTES,
    sizeof(SMOOTH_RINGTONE_NOTES) / sizeof(Note),
    {
      19,
      1
    }
  },
  {
    AXEL_F_NOTES,
    sizeof(AXEL_F_NOTES) / sizeof(Note)
  },
  {
    NOKIA_NOTES,
    sizeof(NOKIA_NOTES) / sizeof(Note)
  },
  {
    MEGALOVANIA_NOTES,
    sizeof(MEGALOVANIA_NOTES) / sizeof(Note)
  },
  {
    CARPHONE_NOTES,
    sizeof(CARPHONE_NOTES) / sizeof(Note)
  },
  {
    TETRIS_USER_MOVE_NOTES,
    sizeof(TETRIS_USER_MOVE_NOTES) / sizeof(Note)
  },
  {
    TETRIS_USER_ROTATE_NOTES,
    sizeof(TETRIS_USER_ROTATE_NOTES) / sizeof(Note)
  },
  {
    TETRIS_PIECE_PLACED_NOTES,
    sizeof(TETRIS_PIECE_PLACED_NOTES) / sizeof(Note)
  },
  {
    TETRIS_CLEAR_2_LINES_NOTES,
    sizeof(TETRIS_CLEAR_2_LINES_NOTES) / sizeof(Note),
  },
  {
    TETRIS_CLEAR_3_LINES_NOTES,
    sizeof(TETRIS_CLEAR_3_LINES_NOTES) / sizeof(Note),
  },
  {
    TETRIS_LOSE_NOTES,
    sizeof(TETRIS_LOSE_NOTES) / sizeof(Note),
    {
      7
    }
  },
  {
    TETRIS_MUSIC_NOTES,
    sizeof(TETRIS_MUSIC_NOTES) / sizeof(Note),
    {
      1,
      100
    }
  }
};
