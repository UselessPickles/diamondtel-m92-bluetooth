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

#ifndef TONE_H
#define	TONE_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/**
 * A value representing a tone to be output.
 * 
 * It is technically a fixed point integer value used to increment a fixed point
 * index through a sine lookup table at the sound sampling rate
 * 
 * Tone values can be calculated dynamically at runtime for integer frequency
 * values via TONE_CalculateToneFromFrequency.
 * 
 * Or you can pre-calculate tone values from any frequency value with the 
 * following formula:
 * 
 *   tone = round(freq * (1 << (indexIntegerBits + indexFractionBits)) / sampleRate);
 * 
 * Where:
 *   - freq = desired frequency (Hz).
 *   - indexIntegerBits = number of bits for the integer portion of the sine 
 *                        lookup table index.
 *   - indexFractionBits = number of bits for the fraction portion of the sine 
 *                         lookup table index.
 *   - sampleRate = sound output sample rate (Hz).
 * 
 * Current values for constants in the formula:
 *   - indexIntegerBits = 8
 *   - indexFractionBits = 8
 *   - sampleRate = 10000
 * 
 * NOTE a tone_t value of zero (a.k.a., TONE_OFF) produces no sound.
 */
typedef uint16_t tone_t;
    
#define TONE_OFF (0)

// Standard DTMF frequencies
// See: https://en.wikipedia.org/wiki/Dual-tone_multi-frequency_signaling#Keypad
#define TONE_DTMF_ROW1 (4568) // 697 Hz
#define TONE_DTMF_ROW2 (5046) // 770 Hz
#define TONE_DTMF_ROW3 (5584) // 852 Hz
#define TONE_DTMF_ROW4 (6167) // 941 Hz
#define TONE_DTMF_COL1 (7923) // 1209 Hz
#define TONE_DTMF_COL2 (8756) // 1336 Hz
#define TONE_DTMF_COL3 (9680) // 1477 Hz
#define TONE_DTMF_COL4 (10702) // 1633 Hz

// Standard Special Information Tone (SIT) frequencies
// See: https://en.wikipedia.org/wiki/Special_information_tone#AT&T/Bellcore_standard_composition
#define TONE_SIT_1_LOW  (5989) // 913.8 Hz
#define TONE_SIT_1_HIGH (6457) // 985.2 Hz
#define TONE_SIT_2_LOW  (8982) // 1370.6 Hz
#define TONE_SIT_2_HIGH (9362) // 1428.5 Hz
#define TONE_SIT_3_LOW  (11644) // 1776.7 Hz

// Standard Special Information Tone (SIT) durations in milliseconds
// See: https://en.wikipedia.org/wiki/Special_information_tone#AT&T/Bellcore_standard_composition
#define TONE_SIT_DURATION_SHORT_MS (276)
#define TONE_SIT_DURATION_LONG_MS (380)

// Common "low" and "high" tones used for various beeps/tones on the
// original DiamondTel phone
#define TONE_LOW (TONE_DTMF_ROW2)
#define TONE_HIGH (7550) // 1152 Hz

#define TONE_F3 (1144)  // 174.61 Hz
#define TONE_FS3 (1212) // 185.00 Hz
#define TONE_G3 (1285)  // 196.00 Hz
#define TONE_GS3 (1361) // 207.65 Hz
#define TONE_A3 (1442)  // 220.00 Hz
#define TONE_AS3 (1528) // 233.08 Hz
#define TONE_B3 (1618)  // 246.94 Hz

#define TONE_GF3 (TONE_FS3)
#define TONE_AF3 (TONE_GS3)
#define TONE_BF3 (TONE_AS3)

#define TONE_C4 (1715)  // 261.63 Hz
#define TONE_CS4 (1817) // 277.18 Hz
#define TONE_D4 (1925)  // 293.66 Hz
#define TONE_DS4 (2039) // 311.13 Hz
#define TONE_E4 (2160)  // 329.63 Hz
#define TONE_F4 (2289)  // 349.23 Hz
#define TONE_FS4 (2425) // 369.99 Hz
#define TONE_G4 (2569)  // 392.00 Hz
#define TONE_GS4 (2722) // 415.30 Hz
#define TONE_A4 (2884)  // 440.00 Hz
#define TONE_AS4 (3055) // 466.16 Hz
#define TONE_B4 (3237)  // 493.88 Hz

#define TONE_DF4 (TONE_CS4)
#define TONE_EF4 (TONE_DS4)
#define TONE_GF4 (TONE_FS4)
#define TONE_AF4 (TONE_GS4)
#define TONE_BF4 (TONE_AS4)

#define TONE_C5 (3429)  // 523.25 Hz
#define TONE_CS5 (3633) // 554.37 Hz
#define TONE_D5 (3849)  // 587.33 Hz
#define TONE_DS5 (4078) // 622.25 Hz
#define TONE_E5 (4320)  // 659.25 Hz
#define TONE_F5 (4577)  // 698.46 Hz
#define TONE_FS5 (4850) // 739.99 Hz
#define TONE_G5 (5138)  // 783.99 Hz
#define TONE_GS5 (5443) // 830.61 Hz
#define TONE_A5 (5767)  // 880.00 Hz
#define TONE_AS5 (6110) // 932.33 Hz
#define TONE_B5 (6473)  // 987.77 Hz

#define TONE_DF5 (TONE_CS5)
#define TONE_EF5 (TONE_DS5)
#define TONE_GF5 (TONE_FS5)
#define TONE_AF5 (TONE_GS5)
#define TONE_BF5 (TONE_AS5)

#define TONE_C6 (6858)   // 1046.50 Hz
#define TONE_CS6 (7266)  // 1108.73 Hz
#define TONE_D6 (7698)   // 1174.66 Hz
#define TONE_DS6 (8156)  // 1244.51 Hz
#define TONE_E6 (8641)   // 1318.51 Hz
#define TONE_F6 (9155)   // 1396.91 Hz
#define TONE_FS6 (9699)  // 1479.98 Hz
#define TONE_G6 (10276)  // 1567.98 Hz
#define TONE_GS6 (10887) // 1661.22 Hz
#define TONE_A6 (11534)  // 1760.00 Hz
#define TONE_AS6 (12220) // 1864.66 Hz
#define TONE_B6 (12947)  // 1975.53 Hz

#define TONE_DF6 (TONE_CS6)
#define TONE_EF6 (TONE_DS6)
#define TONE_GF6 (TONE_FS6)
#define TONE_AF6 (TONE_GS6)
#define TONE_BF6 (TONE_AS6)

#define TONE_C7 (13717)  // 2093.00 Hz
#define TONE_CS7 (14532) // 2217.46 Hz
#define TONE_D7 (15397)  // 2349.32 Hz
#define TONE_DS7 (16312) // 2489.02 Hz
#define TONE_E7 (17282)  // 2637.02 Hz
#define TONE_F7 (18310)  // 2793.83 Hz
#define TONE_FS7 (19398) // 2959.96 Hz
#define TONE_G7 (20552)  // 3135.96 Hz
#define TONE_GS7 (21774) // 3322.44 Hz
#define TONE_A7 (23069)  // 3520.00 Hz

#define TONE_DF7 (TONE_CS7)
#define TONE_EF7 (TONE_DS7)
#define TONE_GF7 (TONE_FS7)
#define TONE_AF7 (TONE_GS7)

/**
 * Initialize the tone producing engine.
 */
void TONE_Initialize(void);

/**
 * Calculate the tone_t value for a given tone frequency.
 * 
 * @param freq - A tone frequency.
 * @return The corresponding tone_t value.
 */
tone_t TONE_CalculateToneFromFrequency(uint16_t freq);

/**
 * Start playing a tone on channel 1.
 * 
 * Does not affect channel 2.
 * 
 * NOTE a tone_t value of zero (a.k.a., TONE_OFF) produces no sound.
 * 
 * @param tone - The tone to play.
 */
void TONE_PlayTone1(tone_t tone);

/**
 * Start playing a tone on channel 2.
 * 
 * Does not affect channel 1.
 * 
 * NOTE a tone_t value of zero (a.k.a., TONE_OFF) produces no sound.
 * 
 * @param tone - The tone to play.
 */
void TONE_PlayTone2(tone_t tone);

/**
 * Start playing two tones simultaneously, one on each of the two channels.
 * 
 * NOTE a tone_t value of zero (a.k.a., TONE_OFF) produces no sound.
 * 
 * @param tone1 - The tone to play on channel 1.
 * @param tone2 - The tone to play on channel 2.
 */
void TONE_PlayDualTone(tone_t tone1, tone_t tone2);

/**
 * Play a single tone on channel 1 only, while simultaneously stopping
 * any tone that may be currently playing on channel 2.
 * 
 * NOTE a tone_t value of zero (a.k.a., TONE_OFF) produces no sound.
 * 
 * @param tone - The tone to play.
 */
void TONE_PlaySingleTone(tone_t tone);

/**
 * Stop the current tone that is playing on channel 1.
 * 
 * Does not affect channel 2.
 */
void TONE_StopTone1(void);

/**
 * Stop the current tone that is playing on channel 2.
 * 
 * Does not affect channel 1.
 */
void TONE_StopTone2(void);

/**
 * Stop the current tones that are playing on both channels.
 */
void TONE_Stop(void);

#ifdef	__cplusplus
}
#endif

#endif	/* TONE_H */

