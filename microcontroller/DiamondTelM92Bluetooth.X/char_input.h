/**
 * @file
 * @author Jeff Lau
 *
 * UI module for single alphabetic character input.
 * 
 * This module handles only buttons 1-9.
 * 
 * Pressing a button causes the first associated alpha character to be displayed
 * at display position 0. 
 * 
 * Releasing the button quickly causes a placeholder '_' character to be 
 * displayed at display position 0. 
 * 
 * Subsequent presses of the same button cycles through all alpha characters
 * associated with that button.
 * 
 * If the button is held for short time, then the displayed character is 
 * "selected", and will be passed to the return callback function.
 */

#ifndef CHAR_INPUT_H
#define	CHAR_INPUT_H

#include "handset.h"

#ifdef	__cplusplus
extern "C" {
#endif

/**
 * Return callback function for this module.
 * 
 * Called when the user has selected a character to enter.
 * 
 * @param c - The alpha character that the user has entered.
 */
typedef void (*CHAR_INPUT_ReturnCallback)(char c);

/**
 * Callback function used to indicate when the prompt text has been 
 * cleared.
 */
typedef void (*CHAR_INPUT_PromptClearedCallback)(void);

/**
 * Initializes/resets the alpha character input state.
 * 
 * Does NOT modify the contents of the display.
 * 
 * After calling this function, the parent module is responsible for calling
 * CHAR_INPUT_HANDSET_EventHandler() appropriately until the
 * `returnCallback` has been called.
 *
 * @param isPromptDisplayed - True if there is currently text displayed on the 
 *        screen that needs to be cleared out upon the first character input
 *        interaction.
 * @param allowLowercase - True to allow lowercase characters.
 *        False for uppercase characters only.
 * @param returnCallback - Callback function that is called when the user has
 *                         selected an alpha character.
 * @param promptClearedCallback - An optional callback that is called when
 *        the "prompt" is cleared (upon first interaction, if isPromptDisplayed
 *        is true). May be NULL if this feedback is unnecessary.
 */
void CHAR_INPUT_Start(
    bool isPromptDisplayed, 
    bool allowLowercase, 
    CHAR_INPUT_ReturnCallback returnCallback,
    CHAR_INPUT_PromptClearedCallback promptClearedCallback
);

/**
 * Handset event handler for this module.
 * 
 * The parent module must call this from its handset event handler while 
 * alpha character input is "active".
 * 
 * @param event - The handset event.
 */
void CHAR_INPUT_HANDSET_EventHandler(HANDSET_Event const* event);

#ifdef	__cplusplus
}
#endif

#endif	/* CHAR_INPUT_H */

