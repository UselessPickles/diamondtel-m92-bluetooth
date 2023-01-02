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

char* strbrk(char* str, char const* chars);

char* uint2str(char* buffer, uint16_t num, uint8_t length, uint8_t zeroPadLength);

#ifdef	__cplusplus
}
#endif

#endif	/* STRING_H */

