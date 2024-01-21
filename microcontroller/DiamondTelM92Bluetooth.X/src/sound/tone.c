/** 
 * @file
 * @author Jeff Lau
 *
 * Tools for producing basic sound tones, output through DAC1.
 * 
 * Produces pure sine wave tones at a wide range of frequencies, and
 * supports producing dual-tone sounds, or playing single tones on two 
 * independent channels without affecting each other.
 */

#include "tone.h"
#include "../../mcc_generated_files/interrupt_manager.h"
#include "../../mcc_generated_files/tmr6.h"
#include "../../mcc_generated_files/fvr.h"
#include "../../mcc_generated_files/dac1.h"
#include <xc.h>
#include <stddef.h>

/**
 * Sine waveform lookup table.
 * 
 * 256 samples for a single complete wave.
 * i.e., scaled so that an 8-bit index value of 256 (overflows to 0) is 2*PI.
 * 
 * Values are are scaled such that -1 is represented by 0, and +1 is 
 * represented by 255, with the midpoint 0 represented by 128.
 * 
 * JavaScript function for generating this table:
 * 
     function makeSineTable(samples, min, max) {
       const result = [];

       for (i = 0; i < samples; ++i) {
         result.push(Math.round((Math.sin(2* Math.PI * i / samples) + 1) * ((max - min) / 2) + min));
       }

       console.log(result.join(", "));
     }
 */
uint8_t const SINE_DATA[] = { 128, 131, 134, 137, 140, 143, 146, 149, 152, 155, 158, 162, 165, 167, 170, 173, 176, 179, 182, 185, 188, 190, 193, 196, 198, 201, 203, 206, 208, 211, 213, 215, 218, 220, 222, 224, 226, 228, 230, 232, 234, 235, 237, 238, 240, 241, 243, 244, 245, 246, 248, 249, 250, 250, 251, 252, 253, 253, 254, 254, 254, 255, 255, 255, 255, 255, 255, 255, 254, 254, 254, 253, 253, 252, 251, 250, 250, 249, 248, 246, 245, 244, 243, 241, 240, 238, 237, 235, 234, 232, 230, 228, 226, 224, 222, 220, 218, 215, 213, 211, 208, 206, 203, 201, 198, 196, 193, 190, 188, 185, 182, 179, 176, 173, 170, 167, 165, 162, 158, 155, 152, 149, 146, 143, 140, 137, 134, 131, 128, 124, 121, 118, 115, 112, 109, 106, 103, 100, 97, 93, 90, 88, 85, 82, 79, 76, 73, 70, 67, 65, 62, 59, 57, 54, 52, 49, 47, 44, 42, 40, 37, 35, 33, 31, 29, 27, 25, 23, 21, 20, 18, 17, 15, 14, 12, 11, 10, 9, 7, 6, 5, 5, 4, 3, 2, 2, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 2, 3, 4, 5, 5, 6, 7, 9, 10, 11, 12, 14, 15, 17, 18, 20, 21, 23, 25, 27, 29, 31, 33, 35, 37, 40, 42, 44, 47, 49, 52, 54, 57, 59, 62, 65, 67, 70, 73, 76, 79, 82, 85, 88, 90, 93, 97, 100, 103, 106, 109, 112, 115, 118, 121, 124 };

/**
 * The midpoint value of the sine waveform.
 * 
 * This is used to output "nothing" as tone output sample. 
 */
#define SINE_MIDPOINT_VALUE (128)

/**
 * The DAC output sample rate (Hz)
 */
#define OUTPUT_SAMPLE_RATE (10000)

/**
 * 16-bit unsigned fixed point integer index into the SINE_DATA table.
 * Used to keep track of fractional increments through the table.
 */
typedef union {
  /**
   * Complete fixed point value.
   */
  uint16_t value;
  
  struct {
    /**
     * Fractional portion.
     */
    uint8_t fraction: 8;
    /**
     * Integer portion.
     */
    uint8_t integer: 8;
  };
} sine_index_t;

/**
 * Used to set the next tone(s) to be output starting on the next
 * sound sample output.
 */
static volatile struct {
  /**
   * The tone to play on channel 1.
   */
  tone_t tone1;
  /**
   * The tone to play on channel 2.
   */
  tone_t tone2;
  /**
   * If true, then the tone1 and tone2 values will be picked up at the next
   * sound sample timer interrupt.
   * 
   * Set back to false after the values have been consumed.
   * 
   * Do not modify tone1 or tone2 while this is true!
   */
  bool isStaged;
} stagedTones;

typedef struct {
  /**
   * Which tone to generate.
   * 
   * Zero (0) if no sound is to be generated.
   */
  tone_t tone;
  /**
   * Current index into the sine lookup table this tone.
   */
  sine_index_t index;
  /**
   * True if the tone is on its way to stopping 
   * (when the waveform next crosses the mid-point).
   */
  bool isStopping;  
} toneState_t;

/**
 * State of the sound sample output generator.
 * This structure must ONLY be read/written by the timer interrupt handler.
 */
static struct {
  /**
   * The sound sample value to be output at the next timer interrupt.
   */
  uint8_t nextSample;  
  /**
   * The current tone being played on channel 1.
   */
  toneState_t tone1;
  /**
   * The current tone being played on channel 2.
   */
  toneState_t tone2;
} state;

/**
 * Initialize a tone state to begin playing a specified tone.
 * @param toneState - Pointer to a tone state.
 * @param tone - The tone to play.
 */
static void initToneState(toneState_t* toneState, tone_t tone) {
  if (tone) {
    toneState->tone = tone;
    toneState->isStopping = false;
  } else if (toneState->tone) {
    // The tone state is changing from a tone to no tone.
    // We don't want to immediately start outputting nothing, because this 
    // can cause a "pop" in the sound if the previous tone is near a
    // high or low point in the sine wave and suddenly jumps to the midpoint.
    //
    // Instead, we set a flag to indicate we want to stop this tone, and it 
    // will continue playing until it crosses the midpoint.
    toneState->isStopping = true;
  }
}

/**
 * Advances the index of a tone state and returns its next sine wave sample value.
 * 
 * @param toneState - Pointer to a tone state.
 * @return The next sine wave sample value for the tone.
 */
static uint16_t getNextToneSample(toneState_t* toneState) {
  if (!toneState->tone) {
    return SINE_MIDPOINT_VALUE;
  }
  
  /**
   * The index of the previous sample output for this tone.
   */
  uint8_t const lastIndex = toneState->index.integer;
  
  // Advance the index of this tone
  toneState->index.value += toneState->tone;
  
  /**
   * The index of the next sample to output for this tone.
   */
  uint8_t const newIndex = toneState->index.integer;

  // If the tone is in the process of stopping, then check to see if this sample 
  // would be crossing the midpoint, which is when we want to actually stop 
  // the tone (to minimize "popping" in the sound when stopping a tone).
  //
  // The sine lookup table has 256 indexes, and the sine wave crosses the 
  // midpoint and index 0 (0b00000000) and at index 128 (0b10000000). So if the 
  // MSB of the previous index is different than the MSB of the new index, then
  // this means that the tone sample is crossing the midpoint.
  if (toneState->isStopping 
      && ((lastIndex & 0b10000000) != (newIndex & 0b10000000))
      ) {
    toneState->tone = 0;
    toneState->index.value = 0;
    toneState->isStopping = false;

    return SINE_MIDPOINT_VALUE;
  } else {
    return SINE_DATA[newIndex];
  }
}

/**
 * Outputs the sound sample value to the DAC, then calculates the next
 * sound sample.
 * 
 * Used as a timer interrupt handler for a timer that triggers at the desired 
 * sound sampling rate.
 */
static void outputSoundSampleAndCalculateNextSample(void) {
  // Write the DAC output of the previous calculated sample.
  // This is done first to guarantee consistent sample rate of the output.
  DAC1_SetOutput(state.nextSample);
  
  if (stagedTones.isStaged) {
    // Load up the staged tones
    initToneState(&state.tone1, stagedTones.tone1);
    initToneState(&state.tone2, stagedTones.tone2);
    stagedTones.isStaged = false;
  }

  // Add up the sample from both tones and divide by 2 to guarantee the result
  // is in the range of 0-255.
  uint16_t const nextOutputCalc = 
      (getNextToneSample(&state.tone1) +
      getNextToneSample(&state.tone2)) >> 1;

  state.nextSample = (uint8_t)nextOutputCalc;
}

void TONE_Initialize(void) {
  PMD0bits.FVRMD = 0;
  FVR_Initialize();
  
  PMD3bits.DAC1MD = 0;
  DAC1_Initialize();
  
  PMD1bits.TMR6MD = 0;
  TMR6_Initialize();
  
  stagedTones.isStaged = false;
  stagedTones.tone1 = 0;
  stagedTones.tone2 = 0;

  state.tone1.tone = TONE_OFF;
  state.tone1.index.value = 0;
  state.tone1.isStopping = false;
  
  state.tone2.tone = TONE_OFF;
  state.tone2.index.value = 0;
  state.tone2.isStopping = false;
  
  state.nextSample = SINE_MIDPOINT_VALUE;

  TMR6_SetInterruptHandler(&outputSoundSampleAndCalculateNextSample);
  TMR6_StartTimer();
}

tone_t TONE_CalculateToneFromFrequency(uint16_t freq) {
  return (tone_t)((((uint32_t)freq) << 16) / OUTPUT_SAMPLE_RATE);
}

void TONE_PlayTone1(tone_t tone) {
  // This prevents corruption of tone state data if the timer interrupt
  // occurs in the middle up this function.
  while (stagedTones.isStaged);
  
  // NOTE: Leave stagedTones.tone2 unchanged so that the previously played tone
  //       on channel 2 continues playing as-is.
  stagedTones.tone1 = tone;
  stagedTones.isStaged = true;
}

void TONE_PlayTone2(tone_t tone) {
  // This prevents corruption of tone state data if the timer interrupt
  // occurs in the middle up this function.
  while (stagedTones.isStaged);
  
  // NOTE: Leave stagedTones.tone1 unchanged so that the previously played tone
  //       on channel 1 continues playing as-is.
  stagedTones.tone2 = tone;
  stagedTones.isStaged = true;
}

void TONE_PlayDualTone(tone_t tone1, tone_t tone2) {
  // This prevents corruption of tone state data if the timer interrupt
  // occurs in the middle up this function.
  while (stagedTones.isStaged);
  
  stagedTones.tone1 = tone1;
  stagedTones.tone2 = tone2;
  stagedTones.isStaged = true;
}

void TONE_PlaySingleTone(tone_t tone) {
  TONE_PlayDualTone(tone, TONE_OFF);
}

void TONE_StopTone1(void) {
  TONE_PlayTone1(TONE_OFF);
}

void TONE_StopTone2(void) {
  TONE_PlayTone2(TONE_OFF);
}

void TONE_Stop(void) {
  TONE_PlayDualTone(TONE_OFF, TONE_OFF);
}
