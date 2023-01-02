/**
 * @file
 * @author Jeff Lau
 *
 * UI module for alphanumeric string input.
 */

#ifndef STRING_INPUT_H
#define	STRING_INPUT_H

#include "../telephone/handset.h"

#ifdef	__cplusplus
extern "C" {
#endif

/**
 * Return callback result types for this module.
 */
typedef enum {
  /**
   * The user canceled the string input.
   */
  STRING_INPUT_Result_CANCEL,
  /**
   * The user completed the string input.
   */
  STRING_INPUT_Result_APPLY
} STRING_INPUT_Result;

/**
 * Return callback function for this module.
 * 
 * @param result - The return result type.
 * @param buffer - A pointer to the same string input buffer that was provided
 *                 to STRING_INPUT_Start().
 */
typedef void (*STRING_INPUT_ReturnCallback)(STRING_INPUT_Result result, char const* buffer);

/**
 * Start alphanumeric string input.
 * 
 * After calling this function, the parent component is responsible for calling 
 * STRING_INPUT_HANDSET_EventHandler() appropriately until the `returnCallback` 
 * has been called.
 * 
 * NOTE: The current content of the provided string buffer is preserved as
 *       a starting point for the string input. If the string buffer is empty,
 *       then the provided prompt text will be displayed. If the string buffer
 *       is non-empty, then the current contents of the buffer will be displayed
 *       and the user my edit it (delete or append characters at the end).
 * 
 * @param buffer - The string buffer where the user's input will be maintained.
 *                 The size of this buffer must be at least `maxLength` + 1.
 * @param maxLength - The maximum allowed length of the string input.
 * @param prompt - The prompt text that will be displayed whenever the string
 *                 input is empty (provides the user context about what they are
 *                 entering).
 * @param allowLowercase - True to allow lowercase characters to be entered.
 *                         False to allow only uppercase characters.
 * @param allowNumericStart - True to allow the string to start with a numeric
 *                            character. False to requires the first character
 *                            to be alphabetic.
 * @param returnCallback - Callback function that is called when the user is 
 *                         done with string input.
 */
void STRING_INPUT_Start(
    char* buffer, 
    size_t maxLength, 
    char const* prompt, 
    bool allowLowercase, 
    bool allowNumericStart, 
    STRING_INPUT_ReturnCallback returnCallback
);

/**
 * Handset event handler for this module.
 * 
 * The parent module must call this from its handset event handler while 
 * string input is "active".
 * 
 * @param event - The handset event.
 */
void STRING_INPUT_HANDSET_EventHandler(HANDSET_Event const* event);

#ifdef	__cplusplus
}
#endif

#endif	/* STRING_INPUT_H */

