/* 
 * File:   storage.h
 * Author: Jeff
 *
 * Created on January 27, 2022, 10:12 PM
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

#define CREDIT_CARD_LENGTH (16)
#define CREDIT_CARD_COUNT (6)
#define EXTENDED_PHONE_NUMBER_LENGTH (24)
#define STANDARD_PHONE_NUMBER_LENGTH (10)
#define SHORT_PHONE_NUMBER_LENGTH (7)
#define MAX_NAME_LENGTH (16)
#define DIRECTORY_SIZE (29)
#define SECURITY_CODE_LENGTH (4)
#define MAX_DEVICE_NAME_LENGTH (32)
  
void STORAGE_Initialize(void);

uint8_t STORAGE_GetLcdViewAngle(void);
void STORAGE_SetLcdViewAngle(uint8_t lcdViewAngle);

VOLUME_Level STORAGE_GetVolumeLevel(VOLUME_Mode mode);
void STORAGE_SetVolumeLevel(VOLUME_Mode mode, VOLUME_Level level);

char* STORAGE_GetOwnNumber(uint8_t index, char* dest);
void STORAGE_SetOwnNumber(uint8_t index, char const* ownNumber);

char* STORAGE_GetLastDialedNumber(char* dest);
void STORAGE_SetLastDialedNumber(char const* lastDialedNumber);

char* STORAGE_GetSpeedDial(uint8_t index, char* dest);
void STORAGE_SetSpeedDial(uint8_t index, char const* number);

char* STORAGE_GetDirectoryNumber(uint8_t index, char* dest);
void STORAGE_SetDirectoryNumber(uint8_t index, char const* number);

char* STORAGE_GetDirectoryName(uint8_t index, char* dest);
void STORAGE_SetDirectoryEntry(uint8_t index, char const* number, char const* name);

uint8_t STORAGE_GetFirstAvailableDirectoryIndex(void);
uint8_t STORAGE_GetNextPopulatedDirectoryIndex(uint8_t startIndex, bool forward);

uint8_t STORAGE_GetNextNamedDirectoryIndex(uint8_t startIndex, bool forward);
uint8_t STORAGE_GetFirstNamedDirectoryIndexForLetter(char letter);

bool STORAGE_IsDirectoryEntryEmpty(uint8_t index);
bool STORAGE_IsDirectoryNameEmpty(uint8_t index);

uint8_t STORAGE_GetDirectoryIndex(void);
void STORAGE_SetDirectoryIndex(uint8_t index);

char* STORAGE_GetCreditCardNumber(uint8_t index, char* dest);
void STORAGE_SetCreditCardNumber(uint8_t index, char const* number);

RINGTONE_Type STORAGE_GetRingtone(void);
void STORAGE_SetRingtone(RINGTONE_Type ringtone);

uint8_t STORAGE_GetLastCallMinutes(void);
uint8_t STORAGE_GetLastCallSeconds(void);

void STORAGE_SetLastCallTime(uint8_t minutes, uint8_t seconds);

uint16_t STORAGE_GetAccumulatedCallMinutes(void);
uint8_t STORAGE_GetAccumulatedCallSeconds(void);

uint16_t STORAGE_GetTotalCallMinutes(void);
uint8_t STORAGE_GetTotalCallSeconds(void);

void STORAGE_ResetCallTime(void);

bool STORAGE_GetStatusBeepEnabled(void);
void STORAGE_SetStatusBeepEnabled(bool enabled);

bool STORAGE_GetAnnounceBeepEnabled(void);
void STORAGE_SetAnnounceBeepEnabled(bool enabled);

bool STORAGE_GetVehicleModeEnabled(void);
void STORAGE_SetVehicleModeEnabled(bool enabled);

bool STORAGE_GetCallerIdEnabled(void);
void STORAGE_SetCallerIdEnabled(bool enabled);

bool STORAGE_GetShowOwnNumberEnabled(void);
void STORAGE_SetShowOwnNumberEnabled(bool enabled);

bool STORAGE_GetDualNumberEnabled(void);
void STORAGE_SetDualNumberEnabled(bool enabled);

bool STORAGE_GetCumulativeTimerResetEnabled(void);
void STORAGE_SetCumulativeTimerResetEnabled(bool enabled);

bool STORAGE_GetAutoAnswerEnabled(void);
void STORAGE_SetAutoAnswerEnabled(bool enabled);

uint8_t STORAGE_GetActiveNumberIndex(void);
void STORAGE_SetActiveNumberIndex(uint8_t index);

uint8_t STORAGE_GetProgrammingCount(void);
void STORAGE_SetProgrammingCount(uint8_t count);

uint16_t STORAGE_GetTetrisHighScore(void);
void STORAGE_SetTetrisHighScore(uint16_t score);

char* STORAGE_GetTetrisHighScoreInitials(char* dest);
void STORAGE_SetTetrisHighScoreInitials(char const* initials);

char* STORAGE_GetPairedDeviceName(char* dest);
void STORAGE_SetPairedDeviceName(char const* id);

char const* STORAGE_GetSecurityCode(void);
void STORAGE_SetSecurityCode(char const* code);

#ifdef	__cplusplus
}
#endif

#endif	/* STORAGE_H */

