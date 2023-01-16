/** 
 * @file
 * @author Jeff Lau
 * 
 * See header file for module description.
 */

#include "storage.h"
#include "eeprom.h"
#include "../telephone/handset.h"
#include "../util/string.h"
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <ctype.h>

#define MARKER (0b10101100)
#define VERSION (25)

typedef struct {
  uint8_t number[MAX_EXTENDED_PHONE_NUMBER_LENGTH >> 1];
  char name[STORAGE_MAX_DIRECTORY_NAME_LENGTH];
} directory_entry_t;
  
typedef struct {
  bool statusBeepEnabled: 1;
  bool oneMinuteBeepEnabled: 1;
  bool vehicleModeEnabled: 1;
  bool showOwnNumberEnabled: 1;
  bool reserved: 1;
  bool dualNumbersEnabled: 1;
  bool cumulativeTimerResetEnabled: 1;
  bool autoAnswerEnabled: 1;
} toggles_t;

typedef struct {
  uint8_t lastCallMinutes;
  uint8_t lastCallSeconds;
  uint16_t accumulatedCallMinutes;
  uint8_t accumulatedCallSeconds;
  uint16_t totalCallMinutes;
  uint8_t totalCallSeconds;
} call_time_t;

typedef struct {
  uint8_t marker;
  uint8_t version;
  uint8_t lcdViewAngle;
  uint8_t volumeLevels[VOLUME_MODE_COUNT];
  uint8_t directoryIndex;
  uint8_t ringtone;
  call_time_t callTime;
  toggles_t toggles;
  uint8_t activeOwnNumberIndex;
  uint8_t programmingCount;
  uint16_t tetrisHighScore;
  char tetrisHighScoreInitials[3];
  char pairedDeviceName[STORAGE_MAX_DEVICE_NAME_LENGTH];
  uint8_t ownNumber[2][STANDARD_PHONE_NUMBER_LENGTH >> 1];
  uint8_t lastDialedNumber[MAX_EXTENDED_PHONE_NUMBER_LENGTH >> 1];
  uint8_t speedDial[3][MAX_EXTENDED_PHONE_NUMBER_LENGTH >> 1];
  uint8_t securityCode[SECURITY_CODE_LENGTH >> 1];
  uint8_t callerIdMode;
  uint8_t reserved[44];
  directory_entry_t directory[STORAGE_DIRECTORY_SIZE];
  uint8_t creditCardNumbers[STORAGE_CREDIT_CARD_COUNT][CREDIT_CARD_NUMBER_LENGTH >> 1];
} storage_t;

static storage_t storage;

static uint8_t sortedNameIndexes[STORAGE_DIRECTORY_SIZE];
static uint8_t sortedNameSize;

static int compareNameIndexes(void const* a, void const* b) {
  int result = strnicmp(
    storage.directory[*((uint8_t const*)a)].name,
    storage.directory[*((uint8_t const*)b)].name,
    STORAGE_MAX_DIRECTORY_NAME_LENGTH  
  );
  
  return result ? result : (*((int8_t const*)a) - *((int8_t const*)b));
}

static void initializeSortedNameIndexes(void) {
  sortedNameSize = 0;
  
  for (uint8_t i = 0; i < STORAGE_DIRECTORY_SIZE; ++i) {
    if (!STORAGE_IsDirectoryEntryEmpty(i) && storage.directory[i].name[0]) {
      sortedNameIndexes[sortedNameSize++] = i;
    }
  }
  
  qsort(sortedNameIndexes, sortedNameSize, 1, compareNameIndexes);
}

#define COMPRESSED_ASTERISK (0x0A)
#define COMPRESSED_POUND (0x0B)
#define COMPRESSED_PAUSE (0x0C)
#define COMPRESSED_CC_MEMORY (0x0D)
#define COMPRESSED_TERMINATOR (0x0F)

static void compressPhoneNumber(uint8_t* dest, char const* phoneNumber, uint8_t maxLength) {
  if (phoneNumber == NULL) {
    memset(dest, 0xFF, maxLength >> 1);
    return;
  }
  
  size_t len = strlen(phoneNumber);
  
  if (len > maxLength) {
    phoneNumber += len - maxLength;
  }
  
  uint8_t i = 0;
  uint8_t prevNibble;
  uint8_t nextNibble;
  
  while (*phoneNumber) {
    if (isdigit(*phoneNumber)) {
      nextNibble = *phoneNumber - '0';
    } else if (*phoneNumber == '*') {
      nextNibble = COMPRESSED_ASTERISK;
    } else if (*phoneNumber == '#') {
      nextNibble = COMPRESSED_POUND;
    } else if (*phoneNumber == 'P') {
      nextNibble = COMPRESSED_PAUSE;
    } else if (*phoneNumber == 'M') {
      nextNibble = COMPRESSED_CC_MEMORY;
    } else {
      nextNibble = COMPRESSED_TERMINATOR;
    }
    
    if (i & 1) {
      *dest++ = (uint8_t)(prevNibble << 4) | nextNibble;
    } else {
      prevNibble = nextNibble;
    }
    
    ++phoneNumber;
    ++i;
  }
  
  if (i & 1) {
    *dest++ = (uint8_t)(prevNibble << 4) | COMPRESSED_TERMINATOR;
    ++i;
  }
  
  memset(dest, 0xFF, (maxLength - i) >> 1);
}

static char uncompressPhoneNumberNibble(uint8_t nibble) {
  if (nibble < 10) {
    return '0' + nibble;
  } else if (nibble == COMPRESSED_ASTERISK) {
    return '*';
  } else if (nibble == COMPRESSED_POUND) {
    return '#';
  } else if (nibble == COMPRESSED_PAUSE) {
    return 'P';
  } else if (nibble == COMPRESSED_CC_MEMORY) {
    return 'M';
  } else {
    return 0;
  }
}

static char* uncompressPhoneNumber(char* dest, uint8_t const* compressedPhoneNumber, uint8_t maxCompressedLength) {
  uint8_t i = 0;
  
  while (i < maxCompressedLength) {
    uint8_t nextCompressedByte = *compressedPhoneNumber++;
    *dest++ = uncompressPhoneNumberNibble((nextCompressedByte & 0xF0) >> 4);
    *dest++ = uncompressPhoneNumberNibble(nextCompressedByte & 0x0F);
    ++i;
  }
  
  *dest = 0;
  
  return dest;
}

static void initializeDefaultStorageData(void) {
  storage.marker = MARKER;
  storage.version = VERSION;
  storage.lcdViewAngle = 0;
  storage.volumeLevels[VOLUME_Mode_ALERT] = VOLUME_Level_MID;
  storage.volumeLevels[VOLUME_Mode_HANDSET] = VOLUME_Level_MID;
  storage.volumeLevels[VOLUME_Mode_HANDS_FREE] = VOLUME_Level_MID;
  storage.volumeLevels[VOLUME_Mode_SPEAKER] = VOLUME_Level_MID;
  storage.volumeLevels[VOLUME_Mode_TONE] = VOLUME_Level_MID;
  storage.volumeLevels[VOLUME_Mode_GAME_MUSIC] = VOLUME_Level_MID;
  storage.directoryIndex = 0;
  storage.ringtone = 0;
  storage.callTime.lastCallMinutes = 0;
  storage.callTime.lastCallSeconds = 0;
  storage.callTime.accumulatedCallMinutes = 0;
  storage.callTime.accumulatedCallSeconds = 0;
  storage.callTime.totalCallMinutes = 0;
  storage.callTime.totalCallSeconds = 0;
  storage.toggles.statusBeepEnabled = true;
  storage.toggles.oneMinuteBeepEnabled = false;
  storage.toggles.vehicleModeEnabled = false;
  storage.toggles.showOwnNumberEnabled = true;
  storage.toggles.dualNumbersEnabled = false;
  storage.toggles.cumulativeTimerResetEnabled = true;
  storage.toggles.autoAnswerEnabled = false;
  storage.activeOwnNumberIndex = 0;
  storage.programmingCount = 0;
  storage.tetrisHighScore = 0;
  storage.callerIdMode = 0;
  memset(storage.tetrisHighScoreInitials, '?', 3);
  memset(storage.pairedDeviceName, 0, STORAGE_MAX_DEVICE_NAME_LENGTH);
  memset(storage.ownNumber, 0, (STANDARD_PHONE_NUMBER_LENGTH >> 1) * 2);
  memset(storage.lastDialedNumber, 0xFF, MAX_EXTENDED_PHONE_NUMBER_LENGTH >> 1);
  memset(storage.speedDial, 0xFF, (MAX_EXTENDED_PHONE_NUMBER_LENGTH >> 1) * 3);
  memset(storage.securityCode, 0, (SECURITY_CODE_LENGTH >> 1));

  // Store everything except for the directory to EEPROM
  EEPROM_WriteBytes(0, &storage, offsetof(storage_t, reserved));

  // Make all directory entries empty in EEPROM
  for (uint16_t i = 0; i < STORAGE_DIRECTORY_SIZE; ++i) {
    // Set first character of phone number to null
    storage.directory[i].number[0] = 0xFF;
    EEPROM_WriteByte(
        offsetof(storage_t, directory) + sizeof(directory_entry_t) * i + offsetof(directory_entry_t, number),
        0xFF
    );

    // Set first character of name to null
    EEPROM_WriteByte(
        offsetof(storage_t, directory) + sizeof(directory_entry_t) * i + offsetof(directory_entry_t, name),
        0
    );
  }

  STORAGE_SetDirectoryEntry(
      10,
      "3433620506",
      "Harvard Lines"
  );
  STORAGE_SetDirectoryEntry(
      11,
      "9147379938",
      "CPTA Announcemnt"
  );
  STORAGE_SetDirectoryEntry(
      12,
      "2027621401",
      "US Naval Time"
  );
  STORAGE_SetDirectoryEntry(
      13,
      "5055034455",
      "Call Saul"
  );
  STORAGE_SetDirectoryEntry(
      14,
      "7192662837",
      "Callin Oates"
  );
  STORAGE_SetDirectoryEntry(
      15,
      "8884732963",
      "HP Fax Me"
  );

  // Make all credit card entries empty in EEPROM
  for (uint16_t i = 0; i < STORAGE_CREDIT_CARD_COUNT; ++i) {
    // Set first character of phone number to null
    storage.creditCardNumbers[i][0] = 0xFF;
    EEPROM_WriteByte(
        offsetof(storage_t, creditCardNumbers) + (CREDIT_CARD_NUMBER_LENGTH >> 1) * i,
        0xFF
    );
  }
}

void STORAGE_Initialize(void) {
  storage.marker = EEPROM_ReadByte(0);
  storage.version = EEPROM_ReadByte(1);
  
  if ((storage.marker != MARKER) || (storage.version != VERSION)) {
    initializeDefaultStorageData();
  } else {
    EEPROM_ReadBytes(0, &storage, sizeof(storage));
    
    if (storage.callerIdMode == 0xFF) {
      STORAGE_SetCallerIdMode(CALLER_ID_Mode_OFF);
    }
  }
  
  initializeSortedNameIndexes();
}

uint8_t STORAGE_GetLcdViewAngle(void) {
  if (storage.lcdViewAngle > 7) {
    return 0;
  }
  return storage.lcdViewAngle;
}

void STORAGE_SetLcdViewAngle(uint8_t lcdViewAngle) {
  if (lcdViewAngle > HANDSET_MAX_LCD_VIEW_ANGLE) {
    return;
  }
  
  if (lcdViewAngle == storage.lcdViewAngle) {
    return;
  }
  
  storage.lcdViewAngle = lcdViewAngle;
  EEPROM_AsyncWriteByte(offsetof(storage_t, lcdViewAngle), lcdViewAngle);
}

VOLUME_Level STORAGE_GetVolumeLevel(VOLUME_Mode mode) {
  if (mode >= VOLUME_MODE_COUNT) {
    return 0;
  }
  
  if (storage.volumeLevels[mode] > VOLUME_Level_MAX) {
    return VOLUME_Level_MAX;
  }
  
  return storage.volumeLevels[mode];
}

void STORAGE_SetVolumeLevel(VOLUME_Mode mode, VOLUME_Level level) {
  if ((mode >= VOLUME_MODE_COUNT) || (level > VOLUME_Level_MAX)) {
    return;
  }
  
  if (level == storage.volumeLevels[mode]) {
    return;
  }
  
  storage.volumeLevels[mode] = level;
  EEPROM_AsyncWriteByte(offsetof(storage_t, volumeLevels) + mode, level);
}

char* STORAGE_GetOwnNumber(uint8_t index, char* dest) {
  if (index > 1) {
    index = 0;
  }
  
  return uncompressPhoneNumber(dest, storage.ownNumber[index], STANDARD_PHONE_NUMBER_LENGTH >> 1);
}

void STORAGE_SetOwnNumber(uint8_t index, char const* ownNumber) {
  if (index > 1) {
    index = 0;
  }
  
  compressPhoneNumber(storage.ownNumber[index], ownNumber, STANDARD_PHONE_NUMBER_LENGTH);
  EEPROM_AsyncWriteBytes(
      offsetof(storage_t, ownNumber) + index * (STANDARD_PHONE_NUMBER_LENGTH >> 1), 
      storage.ownNumber[index], 
      STANDARD_PHONE_NUMBER_LENGTH >> 1
  );
}

char* STORAGE_GetLastDialedNumber(char* dest) {
  return uncompressPhoneNumber(dest, storage.lastDialedNumber, MAX_EXTENDED_PHONE_NUMBER_LENGTH >> 1);
}

void STORAGE_SetLastDialedNumber(char const* lastDialedNumber) {
  compressPhoneNumber(storage.lastDialedNumber, lastDialedNumber, MAX_EXTENDED_PHONE_NUMBER_LENGTH);
  EEPROM_AsyncWriteBytes(
      offsetof(storage_t, lastDialedNumber), 
      storage.lastDialedNumber, 
      MAX_EXTENDED_PHONE_NUMBER_LENGTH >> 1
  );
}

char* STORAGE_GetSpeedDial(uint8_t index, char* dest) {
  if (index >= 3) {
    dest[0] = 0;
  } else {
    uncompressPhoneNumber(dest, storage.speedDial[index], MAX_EXTENDED_PHONE_NUMBER_LENGTH >> 1);
  }
  
  return dest;
}

void STORAGE_SetSpeedDial(uint8_t index, char const* number) {
  if (index >= 3) {
    return;
  }

  compressPhoneNumber(storage.speedDial[index], number, MAX_EXTENDED_PHONE_NUMBER_LENGTH);
  EEPROM_AsyncWriteBytes(
      offsetof(storage_t, speedDial) + (MAX_EXTENDED_PHONE_NUMBER_LENGTH >> 1) * index, 
      storage.speedDial[index], 
      MAX_EXTENDED_PHONE_NUMBER_LENGTH >> 1
  );
}

char* STORAGE_GetDirectoryNumber(uint8_t index, char* dest) {
  if (index >= STORAGE_DIRECTORY_SIZE) {
    dest[0] = 0;
  } else {
    uncompressPhoneNumber(dest, storage.directory[index].number, MAX_EXTENDED_PHONE_NUMBER_LENGTH >> 1);
    dest[MAX_EXTENDED_PHONE_NUMBER_LENGTH] = 0;
  }
  
  return dest;
}

void STORAGE_SetDirectoryNumber(uint8_t index, char const* number) {
  STORAGE_SetDirectoryEntry(index, number, NULL);
}

char* STORAGE_GetDirectoryName(uint8_t index, char* dest) {
  if (index >= STORAGE_DIRECTORY_SIZE) {
    dest[0] = 0;
  } else {
    strncpy(dest, storage.directory[index].name, STORAGE_MAX_DIRECTORY_NAME_LENGTH)[STORAGE_MAX_DIRECTORY_NAME_LENGTH] = 0;
  }
  
  return dest;
}

void STORAGE_SetDirectoryEntry(uint8_t index, char const* number, char const* name) {
  if (index >= STORAGE_DIRECTORY_SIZE) {
    return;
  }

  if (number) {
    compressPhoneNumber(storage.directory[index].number, number, MAX_EXTENDED_PHONE_NUMBER_LENGTH);
  } else {
    memset(storage.directory[index].number, 0xFF, MAX_EXTENDED_PHONE_NUMBER_LENGTH >> 1);
  }
  
  memset(storage.directory[index].name, 0xFF, STORAGE_MAX_DIRECTORY_NAME_LENGTH);
  
  if (name && number && number[0]) {
    strncpy(storage.directory[index].name, name, STORAGE_MAX_DIRECTORY_NAME_LENGTH);
  } else {
    storage.directory[index].name[0] = 0;
  }

  EEPROM_AsyncWriteBytes(
      offsetof(storage_t, directory) + sizeof(directory_entry_t) * index, 
      storage.directory + index, 
      sizeof(directory_entry_t)
  );
  
  initializeSortedNameIndexes();
}

uint8_t STORAGE_GetFirstEmptyDirectoryIndex(void) {
  for (uint8_t i = 0; i < STORAGE_DIRECTORY_SIZE; ++i) {
    if (STORAGE_IsDirectoryEntryEmpty(i)) {
      return i;
    }
  }
  
  return 0xFF;
}

uint8_t STORAGE_GetNextPopulatedDirectoryIndex(uint8_t startIndex, bool forward) {
  uint8_t i = startIndex;
  
  do {
    if (forward) {
      ++i;
    } else {
      --i;
    }
    
    if (i >= STORAGE_DIRECTORY_SIZE) {
      i = 0;
    } else if (i == 0xFF) {
      i = STORAGE_DIRECTORY_SIZE - 1;
    }
  } while((i != startIndex) && STORAGE_IsDirectoryEntryEmpty(i));
  
  return i;
}

uint8_t STORAGE_GetNextNamedDirectoryIndex(uint8_t startIndex, bool forward) {
  uint8_t nameIndex;
  
  for (nameIndex = 0; nameIndex < sortedNameSize; ++nameIndex) {
    if (sortedNameIndexes[nameIndex] == startIndex) {
      break;
    }
  }
  
  if (nameIndex == sortedNameSize) {
    return 0xFF;
  }
  
  if (forward) {
    if (++nameIndex == sortedNameSize) {
      nameIndex = 0;
    }
  } else {
    if (!nameIndex--) {
      nameIndex = sortedNameSize - 1;
    }
  }

  return sortedNameIndexes[nameIndex];
}

uint8_t STORAGE_GetFirstNamedDirectoryIndexForLetter(char letter) {
  if (!sortedNameSize) {
    return 0xFF;
  }
  
  uint8_t index;
  letter = (char)tolower(letter);
  
  for (index = 0; index < sortedNameSize; ++index) {
    if (tolower(storage.directory[sortedNameIndexes[index]].name[0]) >= letter) {
      break;
    }
  }
  
  if (index == sortedNameSize) {
    index = 0;
  }
  
  return sortedNameIndexes[index];
}

bool STORAGE_IsDirectoryEntryEmpty(uint8_t index) {
  if (index >= STORAGE_DIRECTORY_SIZE) {
    return true;
  }
  
  return storage.directory[index].number[0] == 0xFF;
}

bool STORAGE_IsDirectoryNameEmpty(uint8_t index) {
  if (index >= STORAGE_DIRECTORY_SIZE) {
    return true;
  }
  
  return !storage.directory[index].name[0];
}

uint8_t STORAGE_GetDirectoryIndex(void) {
  if (storage.directoryIndex >= STORAGE_DIRECTORY_SIZE) {
    return 0;
  }
  
  return storage.directoryIndex;
}

void STORAGE_SetDirectoryIndex(uint8_t index) {
  if ((index == storage.directoryIndex) || (index >= STORAGE_DIRECTORY_SIZE)) {
    return;
  }
  
  storage.directoryIndex = index;
  EEPROM_AsyncWriteByte(offsetof(storage_t, directoryIndex), index);
}

char* STORAGE_GetCreditCardNumber(uint8_t index, char* dest) {
  if (index >= STORAGE_CREDIT_CARD_COUNT) {
    dest[0] = 0;
  } else {
    uncompressPhoneNumber(dest, storage.creditCardNumbers[index], CREDIT_CARD_NUMBER_LENGTH >> 1);
  }
  
  return dest;
}

void STORAGE_SetCreditCardNumber(uint8_t index, char const* number) {
  if (index >= STORAGE_CREDIT_CARD_COUNT) {
    return;
  }

  compressPhoneNumber(storage.creditCardNumbers[index], number, CREDIT_CARD_NUMBER_LENGTH);
  
  EEPROM_AsyncWriteBytes(
      offsetof(storage_t, creditCardNumbers) + (CREDIT_CARD_NUMBER_LENGTH >> 1) * index, 
      storage.creditCardNumbers[index], 
      CREDIT_CARD_NUMBER_LENGTH >> 1
  );
}

RINGTONE_Type STORAGE_GetRingtone(void) {
  if (storage.ringtone >= RINGTONE_COUNT) {
    return 0;
  }
  
  return storage.ringtone;
}

void STORAGE_SetRingtone(RINGTONE_Type ringtone) {
  if ((ringtone == storage.ringtone) || (ringtone >= RINGTONE_COUNT)) {
    return;
  }
  
  storage.ringtone = ringtone;
  EEPROM_AsyncWriteByte(offsetof(storage_t, ringtone), ringtone);
}

uint8_t STORAGE_GetLastCallMinutes(void) {
  return storage.callTime.lastCallMinutes;
}

uint8_t STORAGE_GetLastCallSeconds(void) {
  return storage.callTime.lastCallSeconds;
}


void STORAGE_SetLastCallTime(uint8_t minutes, uint8_t seconds) {
  storage.callTime.lastCallMinutes = minutes;
  storage.callTime.lastCallSeconds = seconds;

  storage.callTime.accumulatedCallMinutes += minutes;
  storage.callTime.accumulatedCallSeconds += seconds;
  
  if (storage.callTime.accumulatedCallSeconds >= 60) {
    storage.callTime.accumulatedCallSeconds -= 60;
    ++storage.callTime.accumulatedCallMinutes;
  }

  storage.callTime.totalCallMinutes += minutes;
  storage.callTime.totalCallSeconds += seconds;
  
  if (storage.callTime.totalCallSeconds >= 60) {
    storage.callTime.totalCallSeconds -= 60;
    ++storage.callTime.totalCallMinutes;
  }

  EEPROM_AsyncWriteBytes(offsetof(storage_t, callTime), &storage.callTime, sizeof(storage.callTime));
}

uint16_t STORAGE_GetAccumulatedCallMinutes(void) {
  return storage.callTime.accumulatedCallMinutes;
}

uint8_t STORAGE_GetAccumulatedCallSeconds(void) {
  return storage.callTime.accumulatedCallSeconds;
}

uint16_t STORAGE_GetTotalCallMinutes(void) {
  return storage.callTime.totalCallMinutes;
}

uint8_t STORAGE_GetTotalCallSeconds(void) {
  return storage.callTime.totalCallSeconds;
}

void STORAGE_ResetCallTime(void) {
  storage.callTime.lastCallMinutes = 0;
  storage.callTime.lastCallSeconds = 0;
  storage.callTime.accumulatedCallMinutes = 0;
  storage.callTime.accumulatedCallSeconds = 0;

  EEPROM_AsyncWriteBytes(offsetof(storage_t, callTime), &storage.callTime, sizeof(storage.callTime));
}

bool STORAGE_GetStatusBeepEnabled(void) {
  return storage.toggles.statusBeepEnabled;
}

void STORAGE_SetStatusBeepEnabled(bool enabled) {
  storage.toggles.statusBeepEnabled = enabled;
  EEPROM_AsyncWriteBytes(offsetof(storage_t, toggles), &storage.toggles, sizeof(toggles_t));
}

bool STORAGE_GetOneMinuteBeepEnabled(void) {
  return storage.toggles.oneMinuteBeepEnabled;
}

void STORAGE_SetOneMinuteBeepEnabled(bool enabled) {
  storage.toggles.oneMinuteBeepEnabled = enabled;
  EEPROM_AsyncWriteBytes(offsetof(storage_t, toggles), &storage.toggles, sizeof(toggles_t));
}

bool STORAGE_GetVehicleModeEnabled(void) {
  return storage.toggles.vehicleModeEnabled;
}

void STORAGE_SetVehicleModeEnabled(bool enabled) {
  storage.toggles.vehicleModeEnabled = enabled;
  EEPROM_AsyncWriteBytes(offsetof(storage_t, toggles), &storage.toggles, sizeof(toggles_t));
}

CALLER_ID_Mode STORAGE_GetCallerIdMode(void) {
  return storage.callerIdMode;
}

void STORAGE_SetCallerIdMode(CALLER_ID_Mode mode) {
  storage.callerIdMode = mode;
  EEPROM_AsyncWriteByte(offsetof(storage_t, callerIdMode), storage.callerIdMode);
}

bool STORAGE_GetShowOwnNumberEnabled(void) {
  return storage.toggles.showOwnNumberEnabled;
}

void STORAGE_SetShowOwnNumberEnabled(bool enabled) {
  storage.toggles.showOwnNumberEnabled = enabled;
  EEPROM_AsyncWriteBytes(offsetof(storage_t, toggles), &storage.toggles, sizeof(toggles_t));
}

bool STORAGE_GetDualNumberEnabled(void) {
  return storage.toggles.dualNumbersEnabled;
}

void STORAGE_SetDualNumberEnabled(bool enabled) {
  if (!enabled) {
    STORAGE_SetActiveOwnNumberIndex(0);
  }
  
  storage.toggles.dualNumbersEnabled = enabled;
  EEPROM_AsyncWriteBytes(offsetof(storage_t, toggles), &storage.toggles, sizeof(toggles_t));
}

bool STORAGE_GetCumulativeTimerResetEnabled(void) {
  return storage.toggles.cumulativeTimerResetEnabled;
}

void STORAGE_SetCumulativeTimerResetEnabled(bool enabled) {
  storage.toggles.cumulativeTimerResetEnabled = enabled;
  EEPROM_AsyncWriteBytes(offsetof(storage_t, toggles), &storage.toggles, sizeof(toggles_t));
}

bool STORAGE_GetAutoAnswerEnabled(void) {
  return storage.toggles.autoAnswerEnabled;
}

void STORAGE_SetAutoAnswerEnabled(bool enabled) {
  storage.toggles.autoAnswerEnabled = enabled;
  EEPROM_AsyncWriteBytes(offsetof(storage_t, toggles), &storage.toggles, sizeof(toggles_t));
}

uint8_t STORAGE_GetActiveOwnNumberIndex(void) {
  return storage.activeOwnNumberIndex;
}

void STORAGE_SetActiveOwnNumberIndex(uint8_t index) {
  if (index > 1) {
    return;
  }
  
  storage.activeOwnNumberIndex = index;
  EEPROM_AsyncWriteByte(offsetof(storage_t, activeOwnNumberIndex), index);
}

uint8_t STORAGE_GetProgrammingCount(void) {
  return storage.programmingCount;
}

void STORAGE_SetProgrammingCount(uint8_t count) {
  storage.programmingCount = count;
  EEPROM_AsyncWriteByte(offsetof(storage_t, programmingCount), count);
}

uint16_t STORAGE_GetTetrisHighScore(void) {
  return storage.tetrisHighScore;
}

void STORAGE_SetTetrisHighScore(uint16_t score) {
  storage.tetrisHighScore = score;
  EEPROM_AsyncWriteBytes(offsetof(storage_t, tetrisHighScore), &storage.tetrisHighScore, sizeof(storage.tetrisHighScore));
}

char* STORAGE_GetTetrisHighScoreInitials(char* dest) {
  strncpy(dest, storage.tetrisHighScoreInitials, MAX_PLAYER_INITIALS_LENGTH)[MAX_PLAYER_INITIALS_LENGTH] = 0;
  return dest;
}

void STORAGE_SetTetrisHighScoreInitials(char const* initials) {
  strncpy(storage.tetrisHighScoreInitials, initials, MAX_PLAYER_INITIALS_LENGTH);
  EEPROM_AsyncWriteBytes(offsetof(storage_t, tetrisHighScoreInitials), storage.tetrisHighScoreInitials, MAX_PLAYER_INITIALS_LENGTH);
}

char* STORAGE_GetPairedDeviceName(char* dest){
  strncpy(dest, storage.pairedDeviceName, STORAGE_MAX_DEVICE_NAME_LENGTH)[STORAGE_MAX_DEVICE_NAME_LENGTH] = 0;
  return dest;
}

void STORAGE_SetPairedDeviceName(char const* deviceName) {
  strncpy(storage.pairedDeviceName, deviceName, STORAGE_MAX_DEVICE_NAME_LENGTH);
  EEPROM_AsyncWriteBytes(offsetof(storage_t, pairedDeviceName), storage.pairedDeviceName, STORAGE_MAX_DEVICE_NAME_LENGTH);
}

char* STORAGE_GetSecurityCode(char* dest) {
  uncompressPhoneNumber(dest, storage.securityCode, SECURITY_CODE_LENGTH >> 1);
  return dest;
}

void STORAGE_SetSecurityCode(char const* code) {
  compressPhoneNumber(storage.securityCode, code, SECURITY_CODE_LENGTH);
  EEPROM_AsyncWriteBytes(offsetof(storage_t, securityCode), storage.securityCode, SECURITY_CODE_LENGTH >> 1);
}
