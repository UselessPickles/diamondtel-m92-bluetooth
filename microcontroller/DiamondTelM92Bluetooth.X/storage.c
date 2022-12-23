#include "storage.h"
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <ctype.h>
#include "eeprom.h"
#include "string.h"

#define MARKER (0b10101100)
#define VERSION (23)

typedef struct {
  uint8_t number[EXTENDED_PHONE_NUMBER_LENGTH >> 1];
  char name[MAX_NAME_LENGTH];
} directory_entry_t;
  
typedef struct {
  bool statusBeepEnabled: 1;
  bool announceBeepEnabled: 1;
  bool vehicleModeEnabled: 1;
  bool showOwnNumberEnabled: 1;
  bool callerIdEnabled: 1;
  bool dualNumbersEnabled: 1;
  bool cumulativeTimerResetEnabled: 1;
  bool autoAnswerEnabled: 1;
} toggles_t;

typedef struct {
  uint8_t marker;
  uint8_t version;
  uint8_t lcdViewAngle;
  uint8_t volumeLevels[VOLUME_MODE_COUNT];
  uint8_t directoryIndex;
  uint8_t ringtone;
  uint8_t lastCallMinutes;
  uint8_t lastCallSeconds;
  uint16_t accumulatedCallMinutes;
  uint8_t accumulatedCallSeconds;
  uint16_t totalCallMinutes;
  uint8_t totalCallSeconds;
  toggles_t toggles;
  uint8_t activeNumberIndex;
  uint8_t programmingCount;
  uint16_t tetrisHighScore;
  char pairedDeviceName[MAX_DEVICE_NAME_LENGTH];
  uint8_t ownNumber[2][STANDARD_PHONE_NUMBER_LENGTH >> 1];
  uint8_t lastDialedNumber[EXTENDED_PHONE_NUMBER_LENGTH >> 1];
  uint8_t speedDial[3][EXTENDED_PHONE_NUMBER_LENGTH >> 1];
  uint8_t securityCode[SECURITY_CODE_LENGTH >> 1];
  directory_entry_t directory[DIRECTORY_SIZE];
  uint8_t creditCardNumbers[CREDIT_CARD_COUNT][CREDIT_CARD_LENGTH >> 1];
} storage_t;

static storage_t storage;

static uint8_t sortedNameIndexes[DIRECTORY_SIZE];
static uint8_t sortedNameSize;

static char securityCode[SECURITY_CODE_LENGTH + 1];

static int compareNameIndexes(void const* a, void const* b) {
  int result = strnicmp(
    storage.directory[*((uint8_t const*)a)].name,
    storage.directory[*((uint8_t const*)b)].name,
    MAX_NAME_LENGTH  
  );
  
  return result ? result : (*((int8_t const*)a) - *((int8_t const*)b));
}

static void initializeSortedNameIndexes(void) {
  sortedNameSize = 0;
  
  for (uint8_t i = 0; i < DIRECTORY_SIZE; ++i) {
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

static void uncompressPhoneNumber(char* dest, uint8_t const* compressedPhoneNumber, uint8_t maxCompressedLength) {
  uint8_t i = 0;
  
  while (i < maxCompressedLength) {
    uint8_t nextCompressedByte = *compressedPhoneNumber++;
    *dest++ = uncompressPhoneNumberNibble((nextCompressedByte & 0xF0) >> 4);
    *dest++ = uncompressPhoneNumberNibble(nextCompressedByte & 0x0F);
    ++i;
  }
  
  *dest = 0;
}

static void resetStorage(void) {
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
  storage.lastCallMinutes = 0;
  storage.lastCallSeconds = 0;
  storage.accumulatedCallMinutes = 0;
  storage.accumulatedCallSeconds = 0;
  storage.totalCallMinutes = 0;
  storage.totalCallSeconds = 0;
  storage.toggles.statusBeepEnabled = true;
  storage.toggles.announceBeepEnabled = false;
  storage.toggles.vehicleModeEnabled = false;
  storage.toggles.callerIdEnabled = false;
  storage.toggles.showOwnNumberEnabled = true;
  storage.toggles.dualNumbersEnabled = false;
  storage.toggles.cumulativeTimerResetEnabled = true;
  storage.toggles.autoAnswerEnabled = false;
  storage.activeNumberIndex = 0;
  storage.programmingCount = 0;
  storage.tetrisHighScore = 0;
  memset(storage.pairedDeviceName, 0, MAX_DEVICE_NAME_LENGTH);
  memset(storage.ownNumber, 0, (STANDARD_PHONE_NUMBER_LENGTH >> 1) * 2);
  memset(storage.lastDialedNumber, 0xFF, EXTENDED_PHONE_NUMBER_LENGTH >> 1);
  memset(storage.speedDial, 0xFF, (EXTENDED_PHONE_NUMBER_LENGTH >> 1) * 3);
  memset(storage.securityCode, 0, (SECURITY_CODE_LENGTH >> 1));

  // Store everything except for the directory to EEPROM
  EEPROM_WriteBytes(0, &storage, offsetof(storage_t, directory));

  // Make all directory entries empty in EEPROM
  for (uint16_t i = 0; i < DIRECTORY_SIZE; ++i) {
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
      1,
      "3433620506",
      "Harvard Lines"
  );
  STORAGE_SetDirectoryEntry(
      2,
      "9147379938",
      "CPTA Announcemnt"
  );
  STORAGE_SetDirectoryEntry(
      3,
      "2027621401",
      "US Naval Time"
  );
  STORAGE_SetDirectoryEntry(
      4,
      "5055034455",
      "Call Saul"
  );
  STORAGE_SetDirectoryEntry(
      5,
      "7192662837",
      "Callin Oates"
  );
  STORAGE_SetDirectoryEntry(
      6,
      "18884732963",
      "HP Fax Me"
  );

  // Make all credit card entries empty in EEPROM
  for (uint16_t i = 0; i < CREDIT_CARD_COUNT; ++i) {
    // Set first character of phone number to null
    storage.creditCardNumbers[i][0] = 0xFF;
    EEPROM_WriteByte(
        offsetof(storage_t, creditCardNumbers) + (CREDIT_CARD_LENGTH >> 1) * i,
        0xFF
    );
  }
}

void STORAGE_Initialize(void) {
  storage.marker = EEPROM_ReadByte(0);
  storage.version = EEPROM_ReadByte(1);
  
  if ((storage.marker != MARKER) || (storage.version != VERSION)) {
    resetStorage();
  } else {
    EEPROM_ReadBytes(0, &storage, sizeof(storage));
  }
  
  initializeSortedNameIndexes();
}

void STORAGE_ResetToDefaults(void) {
  resetStorage();
}

uint8_t STORAGE_GetLcdViewAngle(void) {
  if (storage.lcdViewAngle > 7) {
    return 0;
  }
  return storage.lcdViewAngle;
}

void STORAGE_SetLcdViewAngle(uint8_t lcdViewAngle) {
  if (lcdViewAngle > 7) {
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

void STORAGE_GetOwnNumber(uint8_t index, char* dest) {
  if (index > 1) {
    index = 0;
  }
  
  uncompressPhoneNumber(dest, storage.ownNumber[index], STANDARD_PHONE_NUMBER_LENGTH >> 1);
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

void STORAGE_GetLastDialedNumber(char* dest) {
  uncompressPhoneNumber(dest, storage.lastDialedNumber, EXTENDED_PHONE_NUMBER_LENGTH >> 1);
}

void STORAGE_SetLastDialedNumber(char const* lastDialedNumber) {
  compressPhoneNumber(storage.lastDialedNumber, lastDialedNumber, EXTENDED_PHONE_NUMBER_LENGTH);
  EEPROM_AsyncWriteBytes(
      offsetof(storage_t, lastDialedNumber), 
      storage.lastDialedNumber, 
      EXTENDED_PHONE_NUMBER_LENGTH >> 1
  );
}

void STORAGE_GetSpeedDial(uint8_t index, char* dest) {
  if (index >= 3) {
    dest[0] = 0;
    return;
  }
  
  uncompressPhoneNumber(dest, storage.speedDial[index], EXTENDED_PHONE_NUMBER_LENGTH >> 1);
}

void STORAGE_SetSpeedDial(uint8_t index, char const* number) {
  if (index >= 3) {
    return;
  }

  compressPhoneNumber(storage.speedDial[index], number, EXTENDED_PHONE_NUMBER_LENGTH);
  EEPROM_AsyncWriteBytes(
      offsetof(storage_t, speedDial) + (EXTENDED_PHONE_NUMBER_LENGTH >> 1) * index, 
      storage.speedDial[index], 
      EXTENDED_PHONE_NUMBER_LENGTH >> 1
  );
}

void STORAGE_GetDirectoryNumber(uint8_t index, char* dest) {
  if (index >= DIRECTORY_SIZE) {
    dest[0] = 0;
    return;
  }
  
  uncompressPhoneNumber(dest, storage.directory[index].number, EXTENDED_PHONE_NUMBER_LENGTH >> 1);
  dest[EXTENDED_PHONE_NUMBER_LENGTH] = 0;
}

void STORAGE_SetDirectoryNumber(uint8_t index, char const* number) {
  STORAGE_SetDirectoryEntry(index, number, NULL);
}

void STORAGE_GetDirectoryName(uint8_t index, char* dest) {
  if (index >= DIRECTORY_SIZE) {
    dest[0] = 0;
    return;
  }
  
  strncpy(dest, storage.directory[index].name, MAX_NAME_LENGTH)[MAX_NAME_LENGTH] = 0;
}

void STORAGE_SetDirectoryEntry(uint8_t index, char const* number, char const* name) {
  if (index >= DIRECTORY_SIZE) {
    return;
  }

  if (number) {
    compressPhoneNumber(storage.directory[index].number, number, EXTENDED_PHONE_NUMBER_LENGTH);
  } else {
    storage.directory[index].number[0] = 0xFF;
  }
  
  if (name) {
    strncpy(storage.directory[index].name, name, MAX_NAME_LENGTH);
  } else {
    storage.directory[index].name[0] = 0;
  }

  EEPROM_AsyncWriteBytes(
      offsetof(storage_t, directory) + sizeof(directory_entry_t) * index + offsetof(directory_entry_t, number), 
      storage.directory[index].number, 
      EXTENDED_PHONE_NUMBER_LENGTH >> 1
  );
  
  EEPROM_AsyncWriteBytes(
      offsetof(storage_t, directory) + sizeof(directory_entry_t) * index + offsetof(directory_entry_t, name), 
      name, 
      MAX_NAME_LENGTH
  );
  
  initializeSortedNameIndexes();
}

uint8_t STORAGE_GetFirstAvailableDirectoryIndex(void) {
  for (uint8_t i = 0; i < DIRECTORY_SIZE; ++i) {
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
    
    if (i == DIRECTORY_SIZE) {
      i = 0;
    } else if (i == 255) {
      i = DIRECTORY_SIZE - 1;
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
  if (index >= DIRECTORY_SIZE) {
    return true;
  }
  
  return storage.directory[index].number[0] == 0xFF;
}

bool STORAGE_IsDirectoryNameEmpty(uint8_t index) {
  if (index >= DIRECTORY_SIZE) {
    return true;
  }
  
  return !storage.directory[index].name[0];
}

uint8_t STORAGE_GetDirectoryIndex(void) {
  if (storage.directoryIndex >= DIRECTORY_SIZE) {
    return 0;
  }
  
  return storage.directoryIndex;
}

void STORAGE_SetDirectoryIndex(uint8_t index) {
  if ((index == storage.directoryIndex) || (index >= DIRECTORY_SIZE)) {
    return;
  }
  
  storage.directoryIndex = index;
  EEPROM_AsyncWriteByte(offsetof(storage_t, directoryIndex), index);
}

void STORAGE_GetCreditCardNumber(uint8_t index, char* dest) {
  if (index >= CREDIT_CARD_COUNT) {
    dest[0] = 0;
    return;
  }
  
  uncompressPhoneNumber(dest, storage.creditCardNumbers[index], CREDIT_CARD_LENGTH >> 1);
}

void STORAGE_SetCreditCardNumber(uint8_t index, char const* number) {
  if (index >= CREDIT_CARD_COUNT) {
    return;
  }

  compressPhoneNumber(storage.creditCardNumbers[index], number, CREDIT_CARD_LENGTH);
  
  EEPROM_AsyncWriteBytes(
      offsetof(storage_t, creditCardNumbers) + (CREDIT_CARD_LENGTH >> 1) * index, 
      storage.creditCardNumbers[index], 
      CREDIT_CARD_LENGTH >> 1
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
  return storage.lastCallMinutes;
}

uint8_t STORAGE_GetLastCallSeconds(void) {
  return storage.lastCallSeconds;
}


void STORAGE_SetLastCallTime(uint8_t minutes, uint8_t seconds) {
  storage.lastCallMinutes = minutes;
  storage.lastCallSeconds = seconds;

  storage.accumulatedCallMinutes += minutes;
  storage.accumulatedCallSeconds += seconds;
  
  if (storage.accumulatedCallSeconds >= 60) {
    storage.accumulatedCallSeconds -= 60;
    ++storage.accumulatedCallMinutes;
  }

  storage.totalCallMinutes += minutes;
  storage.totalCallSeconds += seconds;
  
  if (storage.totalCallSeconds >= 60) {
    storage.totalCallSeconds -= 60;
    ++storage.totalCallMinutes;
  }

  EEPROM_AsyncWriteBytes(offsetof(storage_t, lastCallMinutes), &storage.lastCallMinutes, 8);
}

uint16_t STORAGE_GetAccumulatedCallMinutes(void) {
  return storage.accumulatedCallMinutes;
}

uint8_t STORAGE_GetAccumulatedCallSeconds(void) {
  return storage.accumulatedCallSeconds;
}

uint16_t STORAGE_GetTotalCallMinutes(void) {
  return storage.totalCallMinutes;
}

uint8_t STORAGE_GetTotalCallSeconds(void) {
  return storage.totalCallSeconds;
}

void STORAGE_ResetCallTime(void) {
  storage.lastCallMinutes = 0;
  storage.lastCallSeconds = 0;
  storage.accumulatedCallMinutes = 0;
  storage.accumulatedCallSeconds = 0;

  EEPROM_AsyncWriteBytes(offsetof(storage_t, lastCallMinutes), &storage.lastCallMinutes, 5);
}

bool STORAGE_GetStatusBeepEnabled(void) {
  return storage.toggles.statusBeepEnabled;
}

void STORAGE_SetStatusBeepEnabled(bool enabled) {
  storage.toggles.statusBeepEnabled = enabled;
  EEPROM_AsyncWriteBytes(offsetof(storage_t, toggles), &storage.toggles, sizeof(toggles_t));
}

bool STORAGE_GetAnnounceBeepEnabled(void) {
  return storage.toggles.announceBeepEnabled;
}

void STORAGE_SetAnnounceBeepEnabled(bool enabled) {
  storage.toggles.announceBeepEnabled = enabled;
  EEPROM_AsyncWriteBytes(offsetof(storage_t, toggles), &storage.toggles, sizeof(toggles_t));
}

bool STORAGE_GetVehicleModeEnabled(void) {
  return storage.toggles.vehicleModeEnabled;
}

void STORAGE_SetVehicleModeEnabled(bool enabled) {
  storage.toggles.vehicleModeEnabled = enabled;
  EEPROM_AsyncWriteBytes(offsetof(storage_t, toggles), &storage.toggles, sizeof(toggles_t));
}

bool STORAGE_GetCallerIdEnabled(void) {
  return storage.toggles.callerIdEnabled;
}

void STORAGE_SetCallerIdEnabled(bool enabled) {
  storage.toggles.callerIdEnabled = enabled;
  EEPROM_AsyncWriteBytes(offsetof(storage_t, toggles), &storage.toggles, sizeof(toggles_t));
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
    STORAGE_SetActiveNumberIndex(0);
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

uint8_t STORAGE_GetActiveNumberIndex(void) {
  return storage.activeNumberIndex;
}

void STORAGE_SetActiveNumberIndex(uint8_t index) {
  if (index > 1) {
    return;
  }
  
  storage.activeNumberIndex = index;
  EEPROM_AsyncWriteByte(offsetof(storage_t, activeNumberIndex), index);
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

void STORAGE_GetPairedDeviceName(char* dest){
  // If using strncpy here, I get this error:
  // C:\Program Files\Microchip\xc8\v2.35\pic\sources\c99\common\strncpy.c:9:: error: (1466) registers unavailable for code generation of this expression
  memcpy(dest, storage.pairedDeviceName, MAX_DEVICE_NAME_LENGTH);
  dest[MAX_DEVICE_NAME_LENGTH] = 0;
}

void STORAGE_SetPairedDeviceName(char const* deviceName) {
  strncpy(storage.pairedDeviceName, deviceName, MAX_DEVICE_NAME_LENGTH);
  EEPROM_AsyncWriteBytes(offsetof(storage_t, pairedDeviceName), storage.pairedDeviceName, MAX_DEVICE_NAME_LENGTH);
}

char const* STORAGE_GetSecurityCode(void) {
  uncompressPhoneNumber(securityCode, storage.securityCode, SECURITY_CODE_LENGTH >> 1);
  return securityCode;
}

void STORAGE_SetSecurityCode(char const* code) {
  compressPhoneNumber(storage.securityCode, code, SECURITY_CODE_LENGTH);
  EEPROM_AsyncWriteBytes(offsetof(storage_t, securityCode), storage.securityCode, SECURITY_CODE_LENGTH >> 1);
}
