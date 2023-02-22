/** 
 * @file
 * @author Jeff Lau
 * 
 * This module provides access to all persistent storage for this application.
 * 
 * A copy of the persisted EEPROM data is loaded into memory for more convenient
 * and quick access/processing.
 * 
 * All setter functions both update the in-memory data immediately, and 
 * asynchronously write the data to EEPROM. This prevents updates from 
 * impacting the performance of the application, but also does not guarantee
 * that the data is saved to EEPROM upon returning from the setter function. 
 * If necessary, use EEPROM_IsDoneWriting()  to confirm that all asynchronous 
 * EEPROM writes are complete.
 */

#ifndef STORAGE_H
#define	STORAGE_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "../sound/volume.h"  
#include "../sound/ringtone.h"  
#include "../constants.h"

/**
 * Number of credit cards that can be stored.
 */
#define STORAGE_CREDIT_CARD_COUNT (6)
  
/**
 * Max length of a name in the directory of stored names and numbers.
 */  
#define STORAGE_MAX_DIRECTORY_NAME_LENGTH (16)
/**
 * Number of entries available in the directory of stored names and numbers.
 */
#define STORAGE_DIRECTORY_SIZE (29)
  
/**
 * Max length of a paired Bluetooth device name that can be stored.
 */
#define STORAGE_MAX_DEVICE_NAME_LENGTH (32)
  
/**
 * The number of "own" phone numbers (the simulated phone number of the phone 
 * itself) that can be stored.
 */
#define STORAGE_OWN_PHONE_NUMBER_COUNT (2)
  
/**
 * The number of speed ial phone numbers that can be stored.
 */  
#define STORAGE_SPEED_DIAL_COUNT (3)

/**
 * Initializes storage data. Must be called before calling any other STORAGE_*
 * functions.
 * 
 * Loads EEPROM data into memory for quick access.
 * 
 * If EEPROM does not appear to be formatted properly, then factory defaults
 * will be applied.
 */
void STORAGE_Initialize(void);

/**
 * Get the stored handset LCD view angle.
 * 
 * Valid range is 0 - HANDSET_MAX_LCD_VIEW_ANGLE.
 * 
 * @return the stored handset LCD view angle.
 */
uint8_t STORAGE_GetLcdViewAngle(void);
/**
 * Set the stored handset LCD view angle.
 * 
 * Valid range is [0, HANDSET_MAX_LCD_VIEW_ANGLE].
 * 
 * @param lcdViewAngle - an LCD view angle.
 */
void STORAGE_SetLcdViewAngle(uint8_t lcdViewAngle);

/**
 * Get the stored volume level for a specified volume mode.
 * 
 * @param mode - A volume mode.
 * @return the stored volume level for the specified volume mode.
 */
VOLUME_Level STORAGE_GetVolumeLevel(VOLUME_Mode mode);
/**
 * Set the stored volume level for a specified volume mode.
 * 
 * @param mode - A volume mode.
 * @param level - A volume level.
 */
void STORAGE_SetVolumeLevel(VOLUME_Mode mode, VOLUME_Level level);

/**
 * Get the stored phone number that is the simulated phone number of this phone 
 * itself, copied into a provided char buffer with a null-terminator.
 * 
 * @param index - Specify which "own number" to get. Valid range is 
 *        [0, STORAGE_OWN_PHONE_NUMBER_COUNT).
 * @param dest - The destination char buffer. The buffer size must be at least 
 *        STANDARD_PHONE_NUMBER_LENGTH + 1.
 * @return The provided destination char buffer.
 */
char* STORAGE_GetOwnNumber(uint8_t index, char* dest);
/**
 * Set the stored phone number that is the simulated phone number of this phone 
 * itself.
 * 
 * An index is required to specify which of the two "dual numbers".
 * 
 * @param index - Specify which "own number" to get. Valid range is 
 *        [0, STORAGE_OWN_PHONE_NUMBER_COUNT).
 * @param ownNumber - A null-terminated phone number string. Will be truncated 
 *        to STANDARD_PHONE_NUMBER_LENGTH if longer by trimming off excess 
 *        length from the beginning of the string.
 */
void STORAGE_SetOwnNumber(uint8_t index, char const* ownNumber);

/**
 * Get the stored "last dialed number", copied into a provided char buffer with
 * a null-terminator.
 * 
 * @param dest - The destination char buffer. The buffer size must be at least 
 *        MAX_EXTENDED_PHONE_NUMBER_LENGTH + 1.
 * @return The provided destination char buffer.
 */
char* STORAGE_GetLastDialedNumber(char* dest);
/**
 * Set the stored "last dialed number".
 * 
 * @param lastDialedNumber - A null-terminated phone number string. 
 *        Will be truncated to MAX_EXTENDED_PHONE_NUMBER_LENGTH if longer by 
 *        trimming off excess length from the beginning of the string.
 */
void STORAGE_SetLastDialedNumber(char const* lastDialedNumber);

/**
 * Get a stored speed dial phone number, copied into a provided char buffer with
 * a null-terminator.
 * 
 * The result will be a zero-length string if the speed dial entry is empty.
 * 
 * @param index - Specify which speed dial entry to get. Valid range is
 *        [0, STORAGE_SPEED_DIAL_COUNT).
 * @param dest - The destination char buffer. The buffer size must be at least 
 *        MAX_EXTENDED_PHONE_NUMBER_LENGTH + 1.
 * @return The provided destination char buffer.
 */
char* STORAGE_GetSpeedDial(uint8_t index, char* dest);
/**
 * Set a stored speed dial phone number.
 * 
 * A zero-length string or a NULL value will cause the speed dial entry to be
 * cleared out.
 * 
 * @param index - Specify which speed dial entry to set. Valid range is
 *        [0, STORAGE_SPEED_DIAL_COUNT).
 * @param number - A null-terminated phone number string. Will be truncated to 
 *        MAX_EXTENDED_PHONE_NUMBER_LENGTH if longer by trimming off excess 
 *        length from the beginning of the string.
 */
void STORAGE_SetSpeedDial(uint8_t index, char const* number);

/**
 * Get a stored phone number from the directory, copied into a provided char 
 * buffer with a null-terminator.
 * 
 * The result will be a zero-length string if the directory entry is empty.
 *
 * @param index - Specify which directory entry to get. Valid range is
 *        [0, STORAGE_DIRECTORY_SIZE).
 * @param dest - The destination char buffer. The buffer size must be at least 
 *        MAX_EXTENDED_PHONE_NUMBER_LENGTH + 1.
 * @return The provided destination char buffer.
 */
char* STORAGE_GetDirectoryNumber(uint8_t index, char* dest);
/**
 * Set a stored phone number directory entry.
 * 
 * A zero-length string or a NULL value will cause the directory entry to be
 * cleared out.
 * 
 * NOTE: This function will always clear out any previously existing name for
 *       the specified directory entry. Use STORAGE_SetDirectoryEntry() to set
 *       the number and name together.
 * 
 * @param index - Specify which directory entry to get. Valid range is
 *        [0, STORAGE_DIRECTORY_SIZE).
 * @param number - A null-terminated phone number string. Will be truncated to 
 *        MAX_EXTENDED_PHONE_NUMBER_LENGTH if longer by trimming off excess 
 *        length from the beginning of the string.
 */
void STORAGE_SetDirectoryNumber(uint8_t index, char const* number);

/**
 * Get a stored name from the directory, copied into a provided char 
 * buffer with a null-terminator.
 * 
 * The result will be a zero-length string if the directory entry is empty,
 * or if there is no name associated with the directory entry.
 *
 * @param index - Specify which directory entry to get. Valid range is
 *        [0, STORAGE_DIRECTORY_SIZE).
 * @param dest - The destination char buffer. The buffer size must be at least 
 *        STORAGE_MAX_DIRECTORY_NAME_LENGTH + 1.
 * @return The provided destination char buffer.
 */
char* STORAGE_GetDirectoryName(uint8_t index, char* dest);
/**
 * Set a stored phone number directory entry, with an associated name.
 * 
 * A zero-length string or a NULL `number` value will cause the directory entry 
 * to be cleared out (both name and number).
 * 
 * A zero-length string or a NULL `name` value will cause the name to be cleared
 * out, but the number will still be stored.
 * 
 * See also STORAGE_SetDirectoryNumber() for setting the number only and 
 * clearing out the name.
 * 
 * @param index - Specify which directory entry to get. Valid range is
 *        [0, STORAGE_DIRECTORY_SIZE).
 * @param number - A null-terminated phone number string. Will be truncated to 
 *        MAX_EXTENDED_PHONE_NUMBER_LENGTH if longer by trimming off excess 
 *        length from the beginning of the string.
 * @param name - A null-terminated name string. Will be truncated to 
 *        STORAGE_MAX_DIRECTORY_NAME_LENGTH if longer.
 */
void STORAGE_SetDirectoryEntry(uint8_t index, char const* number, char const* name);

/**
 * Get the index of the first empty directory entry.
 * 
 * Valid range of the result is [0, STORAGE_DIRECTORY_SIZE), or 0xFF if
 * the directory is full.
 * 
 * @return The index of the first empty directory entry, or 0xFF if the directory
 *         is full.
 */
uint8_t STORAGE_GetFirstEmptyDirectoryIndex(void);

/**
 * Get the index of the next populated directory entry, in directory index 
 * order, starting from a specified index, in either a forward (ascending index)
 * or reverse (descending index) direction.
 * 
 * A directory entry is considered to be populated if it has at least a phone
 * number. It may or may not not have a name.
 * 
 * The search for the next populated will wrap around to the beginning/end of 
 * the directory if necessary. If no populated entry is found, then the starting
 * index is returned.
 * 
 * @param startIndex - The directory index to start searching from. Must be a 
 *        valid directory index in the range [0, STORAGE_DIRECTORY_SIZE).
 * @param forward - True to search in the forward (ascending) direction. 
 *        False to search in the reverse (descending) direction.
 * @return The index of the next populated directory entry. Returns the starting 
 *         index if none is found. The result is always in the range 
 *         [0, STORAGE_DIRECTORY_SIZE).
 */
uint8_t STORAGE_GetNextPopulatedDirectoryIndex(uint8_t startIndex, bool forward);

/**
 * Get the index of the next populated directory entry with a name, in 
 * alphabetic name order, starting from a specified directory index, in either 
 * a forward (ascending alphabetic) or reverse (descending alphabetic) direction.
 * 
 * The search for the next named entry will wrap around to the beginning/end of 
 * the alphabetic list of named entries if necessary. If no named entry is 
 * found, then the starting index is returned.
 * 
 * @param startIndex - The directory index to start searching from. Must be a 
 *        valid directory index in the range [0, STORAGE_DIRECTORY_SIZE).
 * @param forward - True to search in the forward (ascending alphabetic) direction. 
 *        False to search in the reverse (descending alphabetic) direction.
 * @return The index of the next named directory entry. Returns the starting 
 *         index if none is found. The result is always in the range 
 *         [0, STORAGE_DIRECTORY_SIZE).
 */
uint8_t STORAGE_GetNextNamedDirectoryIndex(uint8_t startIndex, bool forward);

/**
 * Get the index of the first named directory entry (in alphabetic order of names) 
 * that starts with the specified letter (case insensitive), or a subsequent 
 * letter of the alphabet if there is no entry with a name starting with the 
 * specified letter.
 * 
 * If there is no such entry, then the index of the first named directory
 * entry (in alphabetic order) is returned.
 * 
 * If there are no named directory entries, then 0xFF is returned.
 * 
 * @param letter - A letter of the alphabet.
 * @return The index of the first named directory index for the specified letter
 *         (as described above). Return 0xFF if there are no named directory
 *         entries, otherwise the result is in the range [0, STORAGE_DIRECTORY_SIZE).
 */
uint8_t STORAGE_GetFirstNamedDirectoryIndexForLetter(char letter);

/**
 * Test if a directory entry is empty.
 * 
 * @param index - A directory index in the range [0, STORAGE_DIRECTORY_SIZE).
 * @return True if the specified directory index is empty.
 */
bool STORAGE_IsDirectoryEntryEmpty(uint8_t index);

/**
 * Test if the name for a directory entry is empty.
 * 
 * @param index - A directory index in the range [0, STORAGE_DIRECTORY_SIZE).
 * @return True if the name for the specified directory index is empty.
 */
bool STORAGE_IsDirectoryNameEmpty(uint8_t index);

/**
 * Get the stored directory index (used to keep track of which directory index 
 * was last viewed).
 * 
 * @return The stored directory index, in the range [0, STORAGE_DIRECTORY_SIZE).
 */
uint8_t STORAGE_GetDirectoryIndex(void);
/**
 * Set the stored directory index (used to keep track of which directory index 
 * was last viewed).
 * 
 * @param index - A directory index, in the range [0, STORAGE_DIRECTORY_SIZE).
 */
void STORAGE_SetDirectoryIndex(uint8_t index);

/**
 * Get a stored credit card number, copied into a provided char buffer with
 * a null-terminator.
 * 
 * @param index - The index of a credit card entry, in the range 
 *        [0, STORAGE_CREDIT_CARD_COUNT).
 * @param dest - The destination buffer. Buffer size must be at least 
 *        CREDIT_CARD_NUMBER_LENGTH + 1.
 * @return The provided destination char buffer.
 */
char* STORAGE_GetCreditCardNumber(uint8_t index, char* dest);
/**
 * Set a stored credit card number.
 * 
 * A zero-length string or a NULL value will cause the credit card entry to be
 * cleared out.
 * 
 * @param index - The index of a credit card entry, in the range 
 *        [0, STORAGE_CREDIT_CARD_COUNT).
 * @param number - A null-terminated credit card number string. Will be truncated 
 *        to CREDIT_CARD_NUMBER_LENGTH if longer by trimming off excess 
 *        length from the beginning of the string.
 */
void STORAGE_SetCreditCardNumber(uint8_t index, char const* number);

/**
 * Get the stored ringtone type selection.
 * 
 * @return The stored ringtone type selection.
 */
RINGTONE_Type STORAGE_GetRingtone(void);
/**
 * Set the stored ringtone selection.
 * 
 * @param ringtone - A ringtone type value.
 */
void STORAGE_SetRingtone(RINGTONE_Type ringtone);

/**
 * Get the number of whole minutes for the stored duration of the last phone call.
 * @return The number of whole minutes for the stored duration of the last phone call.
 */
uint8_t STORAGE_GetLastCallMinutes(void);
/**
 * Get the number of remainder seconds for the stored duration of the last phone call.
 * @return The number of remainder seconds for the stored duration of the last phone call.
 */
uint8_t STORAGE_GetLastCallSeconds(void);

/**
 * Set the stored duration of the last phone call.
 * 
 * This duration is also automatically added to the accumulated call time, and 
 * the total call time.
 * 
 * @param minutes - Number of whole minutes.
 * @param seconds - Number of remainder seconds.
 */
void STORAGE_SetLastCallTime(uint8_t minutes, uint8_t seconds);

/**
 * Get the number of whole minutes for the stored accumulated phone call duration.
 * 
 * NOTE: This is the resettable accumulated call time.
 * 
 * @return The number of whole minutes for the stored accumulated phone call duration.
 */
uint16_t STORAGE_GetAccumulatedCallMinutes(void);
/**
 * Get the number of remainder seconds for the stored accumulated phone call duration.
 * 
 * NOTE: This is the resettable accumulated call time.
 * 
 * @return The number of remainder seconds for the stored accumulated phone call duration.
 */
uint8_t STORAGE_GetAccumulatedCallSeconds(void);

/**
 * Get the number of whole minutes for the stored total phone call duration.
 * 
 * NOTE: This is the NON-resettable total "lifetime" call time.
 * 
 * @return The number of whole minutes for the stored total phone call duration.
 */
uint16_t STORAGE_GetTotalCallMinutes(void);
/**
 * Get the number of remainder seconds for the stored total phone call duration.
 * 
 * NOTE: This is the NON-resettable total "lifetime" call time.
 * 
 * @return The number of remainder seconds for the stored total phone call duration.
 */
uint8_t STORAGE_GetTotalCallSeconds(void);

/**
 * Resets both the last call time and the resettable accumulated call time.
 * 
 * Does NOT reset the "total" call time.
 */
void STORAGE_ResetCallTime(void);

/**
 * Get the stored setting of whether status beeps are enabled.
 * 
 * If enabled, then changes in service/roaming status should cause a beep.
 * 
 * @return True if status beeps are enabled.
 */
bool STORAGE_GetStatusBeepEnabled(void);
/**
 * Set the stored setting of whether status beeps are enabled.
 * @param enabled - True if status beeps are enabled.
 */
void STORAGE_SetStatusBeepEnabled(bool enabled);

/**
 * Get the stored setting of whether one-minute warning beeps are enabled.
 * 
 * If enabled, a warning beep should play during calls 10 seconds before the 
 * end of each minute of call time.
 * 
 * @return True if one-minute warning beeps are enabled.
 */
bool STORAGE_GetOneMinuteBeepEnabled(void);
/**
 * Set the stored setting of whether one-minute warning beeps are enabled.
 * @param enabled - True if one-minute warning beeps are enabled.
 */
void STORAGE_SetOneMinuteBeepEnabled(bool enabled);

/**
 * Get the stored setting of whether OEM Hands-Free integration is enabled.
 * 
 * If enabled, certain behaviors are automatically altered when an external
 * microphone connection is detected to ensure compatibility with the OEM 
 * Hands-Free Controller. For example, Caller ID is disabled to avoid interfering
 * with the controller's ability to infer status changes during an incoming call.
 * 
 * @return True if OEM Hands-Free integration is enabled.
 */
bool STORAGE_GetOemHandsFreeIntegrationEnabled(void);
/**
 * Set the stored setting of whether OEM Hands-Free integration is enabled.
 * @param enabled - True if OEM Hands-Free integration is enabled.
 */
void STORAGE_SetOemHandsFreeIntegrationEnabled(bool enabled);

/**
 * Get the stored setting of the Caller ID mode.
 * @return The stored Caller ID mode.
 */
CALLER_ID_Mode STORAGE_GetCallerIdMode(void);
/**
 * Set the stored setting of the Caller ID mode.
 * @param mode - A Caller ID mode.
 */
void STORAGE_SetCallerIdMode(CALLER_ID_Mode mode);

/**
 * Get the stored setting of whether the phone's simulated "own phone number" 
 * should be displayed during the startup sequence.
 * @return True if the phone's simulated  "own phone number" should be displayed 
 *         during the startup sequence.
 */
bool STORAGE_GetShowOwnNumberEnabled(void);
/**
 * Set the stored setting of whether the phone's simulated "own phone number" 
 * should be displayed during the startup sequence.
 * @param enabled - True if the phone's simulated "own phone number" should be 
 *        displayed during the startup sequence.
 */
void STORAGE_SetShowOwnNumberEnabled(bool enabled);

/**
 * Get the stored setting of whether dual phone numbers is enabled.
 * 
 * If enabled, then the phone has two simulated "own phone numbers" that the
 * user can switch between. Otherwise the phone has only one simulated 
 * "own phone number"
 * 
 * @return True if dual phone numbers is enabled.
 */
bool STORAGE_GetDualNumberEnabled(void);
/**
 * Set the stored setting of whether dual phone numbers is enabled.
 * @param enabled - True if dual phone numbers is enabled.
 */
void STORAGE_SetDualNumberEnabled(bool enabled);

/**
 * Get the stored setting of whether cumulative timer reset is enabled.
 * 
 * If enabled, then the user is allowed to reset that accumulated call timer.
 * 
 * @return True if cumulative timer reset is enabled.
 */
bool STORAGE_GetCumulativeTimerResetEnabled(void);
/**
 * Set the stored setting of whether cumulative timer reset is enabled.
 * @param enabled - True if cumulative timer reset is enabled.
 */
void STORAGE_SetCumulativeTimerResetEnabled(bool enabled);

/**
 * Get the stored setting of whether auto-answer is enabled.
 * 
 * If enabled, the phone should auto-answer incoming calls the duration of
 * three "classic" ring-tone rings.
 * 
 * @return True if auto-answer is enabled.
 */
bool STORAGE_GetAutoAnswerEnabled(void);
/**
 * Set the stored setting of whether auto-answer is enabled.
 * @param enabled - True if auto-answer is enabled.
 */
void STORAGE_SetAutoAnswerEnabled(bool enabled);

/**
 * Get the stored index of the active simulated "own phone number".
 * @return The stored index of the active simulated "own phone number", in the 
 *         range [0, STORAGE_OWN_PHONE_NUMBER_COUNT).
 */
uint8_t STORAGE_GetActiveOwnNumberIndex(void);
/**
 * Set the stored index of the active simulated "own phone number".
 * @param index - The index of a simulated "own phone number", in the 
 *        range [0, STORAGE_OWN_PHONE_NUMBER_COUNT).
 */
void STORAGE_SetActiveOwnNumberIndex(uint8_t index);


/**
 * Get the stored number of times the phone has been "programmed".
 * @return Number of times the phone has been "programmed".
 */
uint8_t STORAGE_GetProgrammingCount(void);
/**
 * Set stored number of times the phone has been "programmed".
 * @param score - Number of times the phone has been "programmed".
 */
void STORAGE_SetProgrammingCount(uint8_t count);

/**
 * Get the stored high score for Tetris.
 * @return The high score for Tetris.
 */
uint16_t STORAGE_GetTetrisHighScore(void);
/**
 * Set the stored high score for Tetris.
 * @param score - The high score for Tetris.
 */
void STORAGE_SetTetrisHighScore(uint16_t score);

/**
 * Get the stored high score player initials for Tetris, copied into a provided char 
 * buffer with a null-terminator.
 * 
 * @param dest - The destination char buffer. The buffer size must be at least 
 *        PLAYER_INITIALS_MAX_LENGTH + 1.
 * @return The provided destination char buffer.
 */
char* STORAGE_GetTetrisHighScoreInitials(char* dest);
/**
 * Set the stored high score player initials for Tetris.
 * @param code - A null-terminated device name string. Will be truncated 
 *        to PLAYER_INITIALS_MAX_LENGTH if longer
 */
void STORAGE_SetTetrisHighScoreInitials(char const* initials);

/**
 * Get the stored paired Bluetooth device name, copied into a provided char 
 * buffer with a null-terminator.
 * 
 * This is used to remember the name of the most-recently connected Bluetooth
 * device so it can be recalled while not connected to a Bluetooth device.
 * 
 * @param dest - The destination char buffer. The buffer size must be at least 
 *        STORAGE_MAX_DEVICE_NAME_LENGTH + 1.
 * @return The provided destination char buffer.
 */
char* STORAGE_GetPairedDeviceName(char* dest);
/**
 * Set the stored paired Bluetooth device name.
 * @param code - A null-terminated device name string. Will be truncated 
 *        to STORAGE_MAX_DEVICE_NAME_LENGTH if longer
 */
void STORAGE_SetPairedDeviceName(char const* id);

/**
 * Get the stored security code, copied into a provided char buffer with
 * a null-terminator.
 * 
 * @param dest - The destination char buffer. The buffer size must be at least 
 *        SECURITY_CODE_LENGTH + 1.
 * @return The provided destination char buffer.
 */
char* STORAGE_GetSecurityCode(char* dest);
/**
 * Set the stored security code.
 * @param code - A null-terminated numeric security code string that is exactly
 *        SECURITY_CODE_LENGTH in length.
 */
void STORAGE_SetSecurityCode(char const* code);

#ifdef	__cplusplus
}
#endif

#endif	/* STORAGE_H */

