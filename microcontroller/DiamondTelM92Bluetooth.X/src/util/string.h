/** 
 * @file
 * @author Jeff Lau
 * 
 * A collection of string-related utilities.
 */

#ifndef STRING_H
#define	STRING_H

#include <stdbool.h>

#ifdef	__cplusplus
extern "C" {
#endif

/**
 * Performs an in-place UTF8 -> ASCII conversion of a string.
 * 
 * All multi-byte UTF8 characters are replaced with "best match" ASCII 
 * characters, and a null terminator is written at the end of the shortened 
 * string.
 * 
 * A default ASCII character '?' will be used to replace any invalid UTF8 byte 
 * sequences and UTF8 characters that do not have a "best match" ASCII character.
 * 
 * @param str - A null-terminated string that may contain multi-byte UTF8 
 *        characters.
 * 
 * @return The same string pointer that was passed in (for convenience).
 */
char* utf2ascii(char* str);

/**
 * Parses a single field from the beginning of a CSV (comma-separated values) string.
 * 
 * If the field is quoted, then the resulting value will have the quotes
 * stripped away, and escaped quotes in the string ("") converted to quotes (").
 * 
 * @param dest - The destination buffer for the CSV field value.
 *        The caller is responsible for guaranteeing that the buffer is large 
 *        enough to contain the entirety of any possible value from the CSV 
 *        source, including a null terminator.
 * @param csv - A pointer to the beginning of a CSV field of a CSV string.
 * @return A pointer to the beginning of the next CSV field of the provided CSV string.
 *         This return value can be subsequently passed in as the `csv` parameter
 *         to this function to parse the next field.
 */
char const* parseNextCsvField(char* dest, char const* csv);

/**
 * Test if a string starts with a specified substring.
 * @param str - The string to be tested.
 * @param start - The string to look for at the start.
 * @return True if `str` starts with `start`.
 */
bool strstart(char const *str, char const* start);

/**
 * Case-insensitive version of strncmp().
 * 
 * @param a - A potentially-null-terminated string.
 * @param b - A potentially-null-terminated string.
 * @param len - Max length of the comparison.
 * @return An integer comparator result of the case-insensitive string comparison:
 *         - Result < 0 if a < b
 *         - Result == 0 if a == b
 *         - Result > 0 if a > 0
 */
int strnicmp(char const* a, char const* b, int len);

/**
 * Converts an unsigned int to a fixed length null-terminated string, formatted
 * in base 10.
 * 
 * The length of the resulting string will always be exactly the specified 
 * length. The number will be right-justified with in the output string.
 * 
 * @param dest - The destination string buffer. The size of the buffer must be 
 *        at least length + 1.
 * @param num - The number to be formatted as a string.
 * @param length - The length of the resulting string.
 * @param zeroPadLength - The number will be padded out to this length with 
 *        leading zeros.
 * @return A pointer to the destination buffer that was supplied.
 */
char* uint2str(char* dest, uint16_t num, uint8_t length, uint8_t zeroPadLength);

/**
 * Simplifies a phone number to be a valid phone number that could be entered
 * into the DiamondTel Model 92 handset, and with unnecessary US country code
 * removed if present.
 * 
 * If the number starts with a "+", then it is replaced with the numeric
 * international dialing code for dialing out of the US ("011").
 * 
 * If the number is formatted as international calling from the US and to the US
 * ("+1XXXXXXXXXX" or "0111XXXXXXXXXX") then the phone number is stripped down
 * to the simple 10-digit US phone number.
 * 
 * @param dest - Destination buffer for the simplified phone number.
 * @param number - A phone number.
 * @return A pointer to the destination buffer.
 */
char* simplifyPhoneNumber(char* dest, char const* number);

/**
 * Formats a phone number for display.
 * 
 * First, the provided phone number is simplified via simplifyPhoneNumber().
 * 
 * If the simplified number is 10 digits long, then it is formatted like
 * "(XXX) XXX-XXXX".
 * 
 * If the simplified number is 7 digits long, then it is formatted like
 * "XXX-XXXX".
 * 
 * Otherwise, the simplified is copied as-is. 
 * 
 * @param dest - Destination buffer for the formatted phone number.
 * @param number - A phone number.
 * @return A pointer to the destination buffer.
 */
char* formatPhoneNumber(char* dest, char const* number);

#ifdef	__cplusplus
}
#endif

#endif	/* STRING_H */

