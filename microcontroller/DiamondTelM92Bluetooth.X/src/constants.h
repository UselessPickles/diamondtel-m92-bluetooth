/** 
 * @file
 * @author Jeff Lau
 *
 * General constants for this project.
 */

#ifndef CONSTANTS_H
#define	CONSTANTS_H

#ifdef	__cplusplus
extern "C" {
#endif

/**
 * Length of a standard phone number (including area code).
 * 
 * Example: 3138675309 (traditionally formatted as 313-867-5309)
 */
#define STANDARD_PHONE_NUMBER_LENGTH (10)

/**
 * Length of a short phone number (excluding area code).
 * 
 * Example: 8675309 (traditionally formatted as 867-5309)
 */
#define SHORT_PHONE_NUMBER_LENGTH (7)
  
/**
 * Maximum length of an "extended" phone number (to allow for pause characters, 
 * DTMF sequences, credit card memory location references).
 * 
 * Example: 3138675309P2#M*1#
 */
#define MAX_EXTENDED_PHONE_NUMBER_LENGTH (24)
  
/**
 * Length of a credit card number.
 */
#define CREDIT_CARD_NUMBER_LENGTH (16)

/**
 * Number of digits in a security code.
 */
#define SECURITY_CODE_LENGTH (4)
  
/**
 * Maximum length of a string containing player initials 
 * (for high scores in games).
 */  
#define MAX_PLAYER_INITIALS_LENGTH (3)

#ifdef	__cplusplus
}
#endif

#endif	/* CONSTANTS_H */

