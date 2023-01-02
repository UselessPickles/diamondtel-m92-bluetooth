/** 
 * @file
 * @author Jeff Lau
 * 
 * A collection of string-related utilities.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include "string.h"
#include "../telephone/handset.h"

/**
 * Default ASCII character to use for UTF8 -> ASCII conversions when no
 * "best match" is known.
 */
#define DEFAULT_ASCII_CHAR ('?')

/** An entry for a UTF8 -> ASCII mapping. */
typedef struct {
  /** A 16-bit UTF8 code. */
  uint16_t utf8Code;
  /** The ASCII character that should be printed as a best match for the UTF8 code. */
  char asciiCode;
} utf2ascii_entry_t;

/**
 * UTF8 -> ASCII mapping for looking up the "best match" ASCII character
 * to be printed in place of the UTF8 code 
 * (because the phone handset only supports ASCII)
 *
 * The entries in this array MUST be in ascending UTF8 code order! 
 */
static utf2ascii_entry_t const utf2ascii_lookup[] = {
  { 0x00A5, HANDSET_Symbol_YEN_SIGN },
  { 0x03B1, HANDSET_Symbol_ALPHA },
  { 0x2018, '\'' },
  { 0x2019, '\'' },
  { 0x201C, '"' },
  { 0x201D, '"' },
  { 0x2192, HANDSET_Symbol_RIGHT_ARROW },
  { 0x21FE, HANDSET_Symbol_RIGHT_ARROW },
  { 0x25A0, HANDSET_Symbol_RECTANGLE },
  { 0x25A4, HANDSET_Symbol_RECTANGLE_STRIPED },
  { 0x25AE, HANDSET_Symbol_RECTANGLE },
  { 0x25B2, HANDSET_Symbol_LARGE_UP_ARROW },
  { 0x25B4, HANDSET_Symbol_SMALL_UP_ARROW },
  { 0x25B5, HANDSET_Symbol_SMALL_UP_ARROW_OUTLINE },
  { 0x25BC, HANDSET_Symbol_LARGE_DOWN_ARROW },
  { 0x25BE, HANDSET_Symbol_SMALL_DOWN_ARROW },
  { 0x25BF, HANDSET_Symbol_SMALL_DOWN_ARROW_OUTLINE },
};

#define UTF2ASCII_LOOKUP_LENGTH (sizeof(utf2ascii_lookup) / sizeof(utf2ascii_entry_t))

//#define USE_BINARY_SEARCH

#ifdef USE_BINARY_SEARCH
static int utf2ascii_compare(void const* key, void const* entry) {
  if (*(uint16_t const*)key < ((utf2ascii_entry_t const*)entry)->utf8Code) {
    return -1;
  } else if (*(uint16_t const*)key > ((utf2ascii_entry_t const*)entry)->utf8Code) {
    return 1;
  } else {
    return 0;
  }
}
#endif

char* utf2ascii(char* str) {
  /**
   * Possible states that this method can be in.
   */
  typedef enum UTFState {
    /**
     * Next character is a starting byte for a new UTF8 character.
     */
    UTF_StartByte,
    /**
     * There's one more character to process for the current UTF8 character.
     */    
    UTF_OneMore,
    /**
     * There's two more characters to process for the current UTF8 character.
     */    
    UTF_TwoMore,
    /**
     * There's three more characters to process for the current UTF8 character.
     */    
    UTF_ThreeMore,
    /**
     * The end of the string has been reached.
     */    
    UTF_Done
  } UTFState;

  /** The current state of this function. */
  UTFState utfState = UTF_StartByte;
  
  /** Pointer into `str` where the next character is to be written. */
  char *dest = str;
  /** Pointer into `str` where the next character is to be read for UTF8 processing. */
  char *src = str;
  
  /** 
   * True if the current UTF8 character is a 4-byte character  
   * 
   * NOTE: There are no mappings for 4-byte characters, so we take a short-cut
   *       and don't bother trying to build up the UTF8 code properly for these.
   */
  bool is4ByteChar = false;
  /**
   * The UTF8 code for the current UTF8 character.
   */
  uint16_t utf8Code = 0;
  
  while (utfState != UTF_Done) {
    switch(utfState) {
      case UTF_StartByte:
        if ((*src & 0b10000000) == 0) {
          /* This is a 1-byte UTF8 character, so copy it as-is */
          *dest++ = *src;
          
          if (!*src) {
            /* We reached the end of the string. We are done */
            utfState = UTF_Done;
          } else {
            /* 
             * Advance the source pointer. We're already in the "start byte"
             * state, so the next byte will be processed as a start byte.
             */
            ++src;
          }
        } else if ((*src & 0b11100000) == 0b11000000) {
          /* 
           * This is a 2-byte UTF8 character, so start building up the code 
           * and process one more byte next.
           */
          is4ByteChar = false;
          utf8Code = (uint16_t)(*src & 0b00011111) << 6;
          ++src;
          utfState = UTF_OneMore;
        } else if ((*src & 0b11110000) == 0b11100000) {
          /* 
           * This is a 3-byte UTF8 character, so start building up the code 
           * and process 2 more bytes next.
           */
          is4ByteChar = false;
          utf8Code = (uint16_t)(*src & 0b00001111) << 12;
          ++src;
          utfState = UTF_TwoMore;
        } else if ((*src & 0b11111000) == 0b11110000) {
          /* 
           * This is a 4-byte UTF8 character. 
           * We don't support mapping any 4-byte characters, so don't try to 
           * start building the code, but we do still need to consume the next
           * 3 bytes of the character
           */
          is4ByteChar = true;
          ++src;
          utfState = UTF_ThreeMore;
        } else {
          /* 
           * This is an invalid UTF8 byte. 
           * Write the default ASCII character to the result and process the next
           * byte as a starting byte.
           */
          *dest++ = DEFAULT_ASCII_CHAR;
          ++src;
          utfState = UTF_StartByte;
        }
        break;
        
      case UTF_ThreeMore:
        /* Verify that the next byte is valid */
        if ((*src & 0b11000000) == 0b10000000) {
          /* Add this byte to the UTF8 code and process 2 more bytes */
          utf8Code |= (uint16_t)(*src & 0b00111111) << 12;
          ++src;
          utfState = UTF_TwoMore;
        } else {
          /* 
           * This is an invalid UTF8 byte. 
           * Write the default ASCII character to the result and process the next
           * byte as a starting byte.
           */
          *dest++ = DEFAULT_ASCII_CHAR;
          utfState = UTF_StartByte;
        }
        break;

      case UTF_TwoMore:
        /* Verify that the next byte is valid */
        if ((*src & 0b11000000) == 0b10000000) {
          /* Add this byte to the UTF8 code and process 1 more byte */
          utf8Code |= (uint16_t)(*src & 0b00111111) << 6;
          ++src;
          utfState = UTF_OneMore;
        } else {
          /* 
           * This is an invalid UTF8 byte. 
           * Write the default ASCII character to the result and process the next
           * byte as a starting byte.
           */
          *dest++ = DEFAULT_ASCII_CHAR;
          utfState = UTF_StartByte;
        }
        break;

      case UTF_OneMore:
        /* Verify that the next byte is valid */
        if ((*src & 0b11000000) == 0b10000000) {
          /* Add this byte to the UTF8 code */
          utf8Code |= *src & 0b00111111;
          ++src;

          if (is4ByteChar) {
            /* This was a 4-byte character, so we don't care about the code*/
            *dest++ = DEFAULT_ASCII_CHAR;
          } else {
            /**
             * The resulting ASCII character that is the best match for the
             * current UTF8 code (initialized to the default ASCII character).
             */
            char c = DEFAULT_ASCII_CHAR;

#ifdef USE_BINARY_SEARCH
            /* Find the UTF8 -> ASCII mapping entry for the current UTF8 code */
            utf2ascii_entry_t const* utf2ascii_entry = bsearch(
              &utf8Code,
              utf2ascii_lookup,
              UTF2ASCII_LOOKUP_LENGTH,
              sizeof(utf2ascii_entry_t),
              &utf2ascii_compare  
            );

            if (utf2ascii_entry) {
              c = utf2ascii_entry->asciiCode;
            }
#else
            /** Pointer for performing a linear search through `utf2ascii_lookup` */
            utf2ascii_entry_t const* utf2ascii_entry = utf2ascii_lookup;
            
            /* Find the UTF8 -> ASCII mapping entry for the current UTF8 code */
            for (uint8_t i = 0; i < UTF2ASCII_LOOKUP_LENGTH; ++i) {
              if (utf2ascii_entry->utf8Code < utf8Code) {
                /* 
                 * Keep iterating until we reach the first entry whose UTF8 
                 * code is >= the code we are looking for...
                 */
                ++utf2ascii_entry;
                continue;
              }

              /*
               * Store the mapped ASCII character if we found the the entry
               * for the target UTF8 code.
               */
              if (utf2ascii_entry->utf8Code == utf8Code) {
                c = utf2ascii_entry->asciiCode;
              }
              
              /* 
               * Break out of the loop early because we either found, 
               * or will never find, the target
               */
              break;
            }
#endif            
            /* Write the best match ASCII character to the result. */
            *dest++ = c;
          }
        } else {
          /* 
           * This is an invalid UTF8 byte. 
           * Write the default ASCII character to the result and process the next
           * byte as a starting byte.
           */
          *dest++ = DEFAULT_ASCII_CHAR;
        }
        /*
         * We finished processing the last byte of a multi-byte character. 
         * Next byte is a starting byte.
         */
        utfState = UTF_StartByte;
        break;
    }
  }
  
  return str;
}


char const* parseNextCsvField(char* dest, char const* csv) {
  bool isQuoted = (*csv == '"');
  
  if (isQuoted) {
    ++csv;
  }
  
  while (true) {
    if (!*csv) {
      break;
    }
    
    if (isQuoted && (*csv == '"')) {
      if (csv[1] == '"') {
        *dest++ = '"';
        csv += 2;
        continue;
      } else if (csv[1] == ',') {
        csv += 2;
        break;
      } else if (!csv[1]) {
        ++csv;
        break;
      }
    }
    
    if (!isQuoted && (*csv == ',')) {
      ++csv;
      break;
    }
    
    *dest++ = *csv++;
  }
  
  *dest = 0;
  
  return csv;
}

bool strstart(char const *str, char const* start) {
  while(*start) {
    if (*str != *start ) {
      return false;
    }
    
    ++str;
    ++start;
  }
  
  return true;
}

int strnicmp(char const* a, char const* b, int len) {
  for (; len; --len, ++a, ++b) {
    if (!*a || !*b) {
      return *a - *b;
    }
    
    int const cmp = tolower(*a) - tolower(*b);
    
    if (cmp) {
      return cmp;
    }
  }
  
  return 0;
}

char* strbrk(char* str, char const* chars) {
  while(*str) {
    if (strchr(chars, *str)) {
      return str;
    }
    
    ++str;
  }
  
  return NULL;
}

char* uint2str(char* buffer, uint16_t num, uint8_t length, uint8_t zeroPadLength) {
  if (zeroPadLength > length) {
    zeroPadLength = length;
  }
  
  char* p = buffer + length;
  
  *p-- = 0;
  if (length && !num) {
    *p-- = '0';
    --length;
    
    if  (zeroPadLength) {
      --zeroPadLength;
    }
  } else {
    while (length && num) {
      *p-- = (num % 10) + '0';
      num /= 10;
      --length;
      
      if  (zeroPadLength) {
        --zeroPadLength;
      }
    }
  }
  
  if (zeroPadLength) {
    memset(buffer + length - zeroPadLength, '0', zeroPadLength);
    length -= zeroPadLength;
  }
  
  if (length) {
    memset(buffer, ' ', length);
  }
  
  return buffer;
}
