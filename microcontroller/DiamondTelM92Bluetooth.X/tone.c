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

#include <xc.h>
#include <stddef.h>
#include "tone.h"
#include "mcc_generated_files/interrupt_manager.h"
#include "mcc_generated_files/tmr6.h"
#include "mcc_generated_files/dac1.h"

/**
 * Sine waveform lookup table.
 * 
 * 256 samples for a single complete wave.
 * i.e., scaled so that an 8-bit index value of 256 (overflows to 0) is 2*PI.
 * 
 * Values are are scaled such that -1 is represented by 20, and +1 is 
 * represented by 235, with the midpoint 0 represented by 128.
 * 
 * NOTE: The full range of 0-255 is not used so that the output voltage stays 
 *       safely within the "full swing" voltage limit of the op amp.
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
uint8_t const SINE_DATA[] = { 128, 130, 133, 135, 138, 141, 143, 146, 148, 151, 154, 156, 159, 161, 164, 166, 169, 171, 173, 176, 178, 180, 183, 185, 187, 189, 192, 194, 196, 198, 200, 202, 204, 205, 207, 209, 211, 212, 214, 215, 217, 218, 220, 221, 222, 224, 225, 226, 227, 228, 229, 230, 230, 231, 232, 232, 233, 233, 234, 234, 234, 235, 235, 235, 235, 235, 235, 235, 234, 234, 234, 233, 233, 232, 232, 231, 230, 230, 229, 228, 227, 226, 225, 224, 222, 221, 220, 218, 217, 215, 214, 212, 211, 209, 207, 205, 204, 202, 200, 198, 196, 194, 192, 189, 187, 185, 183, 180, 178, 176, 173, 171, 169, 166, 164, 161, 159, 156, 154, 151, 148, 146, 143, 141, 138, 135, 133, 130, 128, 125, 122, 120, 117, 114, 112, 109, 107, 104, 101, 99, 96, 94, 91, 89, 86, 84, 82, 79, 77, 75, 72, 70, 68, 66, 63, 61, 59, 57, 55, 53, 51, 50, 48, 46, 44, 43, 41, 40, 38, 37, 35, 34, 33, 31, 30, 29, 28, 27, 26, 25, 25, 24, 23, 23, 22, 22, 21, 21, 21, 20, 20, 20, 20, 20, 20, 20, 21, 21, 21, 22, 22, 23, 23, 24, 25, 25, 26, 27, 28, 29, 30, 31, 33, 34, 35, 37, 38, 40, 41, 43, 44, 46, 48, 50, 51, 53, 55, 57, 59, 61, 63, 66, 68, 70, 72, 75, 77, 79, 82, 84, 86, 89, 91, 94, 96, 99, 101, 104, 107, 109, 112, 114, 117, 120, 122, 125 };

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
static uint16_t getNextToneSample(toneState_t* toneState, uint16_t* sampleAccumulator) {
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
  uint16_t nextOutputCalc = 
      (getNextToneSample(&state.tone1, &nextOutputCalc) +
      getNextToneSample(&state.tone2, &nextOutputCalc)) >> 1;

  state.nextSample = (uint8_t)nextOutputCalc;
}

void TONE_Initialize(void) {
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
