#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include "app.h"
#include "eeprom.h"
#include "string.h"
#include "handset.h"
#include "transceiver.h"
#include "mcc_generated_files/pin_manager.h"
#include "tone.h"
#include "sound.h"
#include "storage.h"
#include "bt_command_decode.h"
#include "bt_command_send.h"
#include "call_timer.h"
#include "indicator.h"
#include "ringtone.h"
#include "ringtone_select.h"
#include "timeout.h"
#include "marquee.h"
#include "interval.h"
#include "atcmd.h"
#include "programming.h"
#include "snake_game.h"
#include "memory_game.h"
#include "tetris_game.h"
#include "security_code.h"

static enum {
  APP_CALL_IDLE,
  APP_CALL_SENDING,
  APP_CALL_ACCEPTING,
  APP_CALL_REJECTING,
  APP_CALL_ENDING
} APP_CallAction;

#define CALL_ACTION_TIMEOUT (150)
static timeout_t callActionTimeout;

static enum {
  BT_CALL_IDLE,
  BT_CALL_VOICE_DIAL,
  BT_CALL_INCOMING,
  BT_CALL_OUTGOING,
  BT_CALL_ACTIVE,
  BT_CALL_ACTIVE_WITH_CALL_WAITING,
  BT_CALL_ACTIVE_WITH_HOLD
} BT_CallStatus;

static const char* const BT_CallStatusLabel[] = {
  "Idle",
  "Voice Dial",
  "Incoming",
  "Outgoing",
  "Active",
  "Active (call waiting)",
  "Active (hold)"
};

static enum {
  APP_State_INIT_START,
  APP_State_INIT_SET_LCD_ANGLE,
  APP_State_INIT_ALL_DISPLAY_ON,
  APP_State_INIT_DISPLAY_PHONE_NUMBER,
  APP_State_INIT_CLEAR_PHONE_NUMBER,
  APP_State_NUMBER_INPUT,
  APP_State_PAIRING,    
  APP_State_INCOMING_CALL,
  APP_State_SELECT_RINGTONE,    
  APP_State_SELECT_SYSTEM_MODE,    
  APP_State_ADJUST_VIEW_ANGLE,
  APP_State_DISPLAY_DISMISSABLE_TEXT,
  APP_State_DISPLAY_BATTERY_LEVEL,
  APP_State_DISPLAY_PAIRED_BATTERY_LEVEL,
  APP_State_DISPLAY_NUMBER_OVERFLOW,
  APP_State_ADJUST_VOLUME,
  APP_State_BROWSE_DIRECTORY_UP,    
  APP_State_BROWSE_DIRECTORY_DOWN,    
  APP_State_BROWSE_DIRECTORY_IDLE,  
  APP_State_BROWSE_DIRECTORY_SHOW_OVERFLOW,    
  APP_State_DISPLAY_CREDIT_CARD_NUMBER,    
  APP_State_DISPLAY_CREDIT_CARD_NUMBER_SHOW_OVERFLOW,    
  APP_State_ALPHA_STO_NAME_INPUT,    
  APP_State_ALPHA_STO_NUMBER_INPUT,    
  APP_State_ALPHA_SCAN,    
  APP_State_ENTER_SECURITY_CODE,
  APP_State_RECALL_BT_DEVICE_NAME,
  APP_State_RECALL_PAIRED_DEVICE_NAME,
  APP_State_SNAKE_GAME,
  APP_State_MEMORY_GAME,
  APP_State_TETRIS_GAME,
  APP_State_PROGRAMMING,
  APP_State_REBOOT_AFTER_DELAY,
  APP_State_REBOOT
} appState;

static int lastAppState;

static const char* const appStateLabel[] = {
  "INIT_START",
  "INIT_SET_LCD_ANGLE",
  "INIT_ALL_DISPLAY_ON",
  "INIT_DISPLAY_PHONE_NUMBER",
  "INIT_CLEAR_PHONE_NUMBER",
  "NUMBER_INPUT",
  "PAIRING",    
  "INCOMING_CALL",
  "SELECT_RINGTONE",    
  "SELECT_SYSTEM_MODE",    
  "ADJUST_VIEW_ANGLE",
  "DISPLAY_DISMISSABLE_TEXT",
  "DISPLAY_BATTERY_LEVEL",
  "DISPLAY_PAIRED_BATTERY_LEVEL",
  "DISPLAY_NUMBER_OVERFLOW",
  "ADJUST_VOLUME",
  "BROWSE_DIRECTORY_UP",    
  "BROWSE_DIRECTORY_DOWN",    
  "BROWSE_DIRECTORY_IDLE",  
  "BROWSE_DIRECTORY_SHOW_OVERFLOW",    
  "DISPLAY_CREDIT_CARD_NUMBER",    
  "DISPLAY_CREDIT_CARD_NUMBER_SHOW_OVERFLOW",    
  "ALPHA_STO_NAME_INPUT",    
  "ALPHA_STO_NUMBER_INPUT",    
  "ALPHA_SCAN",    
  "ENTER_SECURITY_CODE",
  "RECALL_BT_DEVICE_NAME",
  "RECALL_PAIRED_DEVICE_NAME",
  "SNAKE_GAME",
  "MEMORY_GAME",
  "TETRIS_GAME",
  "PROGRAMMING",
  "REBOOT_AFTER_DELAY",
  "REBOOT"
};

static bool BT_isReady = false;

#define LINKBACK_RETRY_TIMEOUT (500)
static timeout_t linkbackRetryTimeout;

static timeout_t appStateTimeout;

static void reboot(void) {
  if (appState == APP_State_REBOOT) {
    return;
  }
  
  HANDSET_SetTextBlink(false);
  TIMEOUT_Start(&appStateTimeout, 10);
  IO_BT_RESET_SetLow();
  appState = APP_State_REBOOT;
}

static void rebootAfterDelay(uint8_t delay) {
  if (appState >= APP_State_REBOOT_AFTER_DELAY) {
    return;
  }

  TIMEOUT_Start(&appStateTimeout, delay);
  appState = APP_State_REBOOT_AFTER_DELAY;
}

#define INITIAL_BATTERY_LEVEL_REPORT_DELAY (100)

static struct {
  bool isConnected;
  bool hasService;
  uint8_t maxSignalStrength;
  uint8_t signalStrength;
  uint8_t maxBatteryLevel;
  uint8_t batteryLevel;
  timeout_t initialBatteryLevelReportTimeout;
} cellPhoneState;

static bool isMuted;

static bool isFcn;
static bool isExtendedFcn;

#define FCN_TIMEOUT (400)
static timeout_t fcnTimeout;
static timeout_t autoAnswerTimeout;

static bool isRclInputPending = false;
static bool isStoInputPending = false;
static bool isRclOrSto2ndDigitPending = false;

#define RCL_2ND_DIGIT_TIMEOUT (100)
static volatile uint8_t rcl2ndDigitTimer;
static volatile bool rcl2ndDigitTimerExpired;
static uint8_t rclOrStoAddr;

static void setFcn(void) {
  isFcn = true;
  isExtendedFcn = false;
  isRclInputPending = false;
  rcl2ndDigitTimer = 0;
  isStoInputPending = false;
  isRclOrSto2ndDigitPending = false;
  HANDSET_SetIndicator(HANDSET_Indicator_FCN, true);  
}

static void resetFcn(void) {
  isFcn = false;
  isExtendedFcn = false;
  TIMEOUT_Cancel(&fcnTimeout);
  HANDSET_SetIndicator(HANDSET_Indicator_FCN, false);
}

#define IDLE_TIMEOUT (1000)
static timeout_t idleTimeout;
static volatile bool isHandsetIdle;

static struct {
  bool isSending;
  char buffer[EXTENDED_PHONE_NUMBER_LENGTH];
  uint8_t head;
  uint8_t tail;
  uint8_t bufferSize;
} dtmfState;

static void cancelAllDtmfDigits(void) {
  dtmfState.tail = dtmfState.head;
  dtmfState.bufferSize = 0;
  dtmfState.isSending = false;
}

static void handleDtmfAtResponse(ATCMD_Response response, char const* result);

static void sendNextDtmfDigit(void) {
  if (!dtmfState.bufferSize) {
    dtmfState.isSending = false;
    return;
  }
  
  if (BT_CallStatus < BT_CALL_ACTIVE) {
    cancelAllDtmfDigits();
    return;
  }
  
  if (ATCMD_SendDTMFButtonPress(dtmfState.buffer[dtmfState.tail], &handleDtmfAtResponse)) {
    dtmfState.isSending = true;

    --dtmfState.bufferSize;
    if (++dtmfState.tail == EXTENDED_PHONE_NUMBER_LENGTH) {
      dtmfState.tail = 0;
    }
  } else {
    cancelAllDtmfDigits();
  }  
}

static void handleDtmfAtResponse(ATCMD_Response response, char const* result) {
  if (response != ATCMD_Response_OK) {
    cancelAllDtmfDigits();
  } else if (dtmfState.isSending) {
    sendNextDtmfDigit();
  }
}

static bool sendDtmfDigit(char digit) {
  if (!HANDSET_IsButtonPrintable(digit)) {
    return false;
  }

  if (dtmfState.bufferSize == EXTENDED_PHONE_NUMBER_LENGTH) {
    return false;
  }
  
  dtmfState.buffer[dtmfState.head] = digit;
  if (++dtmfState.head == EXTENDED_PHONE_NUMBER_LENGTH) {
    dtmfState.head = 0;
  }
  
  ++dtmfState.bufferSize;
  
  if (!dtmfState.isSending) {
    sendNextDtmfDigit();
  }
  
  return true;
}

static void sendDtmfString(char const* dtmfStr) {
  while(*dtmfStr) {
    sendDtmfDigit(*dtmfStr++);
  }
}

static void wakeUpHandset(bool backlight) {
  TIMEOUT_Start(&idleTimeout, IDLE_TIMEOUT);

  isHandsetIdle = false;
  SOUND_Enable();
  
  if (backlight) {
    HANDSET_SetBacklight(true);
  }
}

#define CALL_FAILED_TIMEOUT (3000)
static volatile uint16_t callFailedTimer;
static volatile bool callFailedTimerExpired;

#define LOW_BATTERY_BEEP_INTERVAL (2000)
interval_t lowBatteryBeepInterval;

#define BROWSE_DIRECTORY_SCAN_INTERVAL (100)
static bool isDirectoryScanNameMode;

#define NUMBER_INPUT_MAX_LENGTH (EXTENDED_PHONE_NUMBER_LENGTH)
static char numberInput[NUMBER_INPUT_MAX_LENGTH + 1];
static char dialedNumber[NUMBER_INPUT_MAX_LENGTH + 1];
static char* dialedNumberNextDigitsToDial;
static size_t numberInputLength;
static volatile bool numberInputIsStale;

static char tempNumberBuffer[NUMBER_INPUT_MAX_LENGTH + 1];

static char const alphaLookup[9][6] = {
  {'Q', 'Z', ' ', 'q', 'z', ' '},
  {'A', 'B', 'C', 'a', 'b', 'c'},
  {'D', 'E', 'F', 'd', 'e', 'f'},
  {'G', 'H', 'I', 'g', 'h', 'i'},
  {'J', 'K', 'L', 'j', 'k', 'l'},
  {'M', 'N', 'O', 'm', 'n', 'o'},
  {'P', 'R', 'S', 'p', 'r', 's'},
  {'T', 'U', 'V', 't', 'u', 'v'},
  {'W', 'X', 'Y', 'w', 'x', 'y'},
};

static char alphaInput[NUMBER_INPUT_MAX_LENGTH + 1];
static uint8_t alphaButton;
static uint8_t alphaCharIndex;
static bool isAlphaPromptDisplayed;
static bool isAlphaCharAccepted;
static bool isAlphaInput;
static bool isFcnInputPendingForAlphaSto;

typedef enum RecallOverflowType {
  RECALL_FROM_INPUT_MANUAL,
  RECALL_FROM_INPUT_AUTOMATIC,
  RECALL_FROM_MEMORY
} RecallOverflowType;

static uint8_t recallOverflowCurrentPage;
static RecallOverflowType recallOverflowType;

static void NumberInput_Overwrite(char const* str) {
  numberInputLength = strlen(str);
  memcpy(numberInput, str, numberInputLength + 1);
  numberInputIsStale = false;
}

static void NumberInput_PushDigit(char digit) {
  if (numberInputLength < NUMBER_INPUT_MAX_LENGTH) {
    numberInput[numberInputLength++] = digit;
    numberInput[numberInputLength] = 0;
  } else {
    memcpy(numberInput, numberInput + 1, NUMBER_INPUT_MAX_LENGTH - 1);
    numberInput[NUMBER_INPUT_MAX_LENGTH - 1] = digit;
  }
  numberInputIsStale = false;
}

static bool NumberInput_HasIncompleteCreditCardMemorySymbol(void) {
  return (numberInputLength > 1) 
      && (numberInput[numberInputLength - 1] == '*')
      && (numberInput[numberInputLength - 2] == 'M');
}

static void NumberInput_PopDigit(void) {
  if (numberInputLength) {
    // delete "M*" together as a single "digit" in number input mode
    if ((appState != APP_State_ALPHA_STO_NAME_INPUT) && NumberInput_HasIncompleteCreditCardMemorySymbol()) {
      numberInput[--numberInputLength] = 0;
    }

    numberInput[--numberInputLength] = 0;
    numberInputIsStale = false;
  }
}

static void NumberInput_PrintToHandset(void)  {
  if (numberInputLength == 0) {
    if (BT_CallStatus != BT_CALL_IDLE) {
      if (BT_CallStatus == BT_CALL_OUTGOING) {
        HANDSET_DisableTextDisplay();
        HANDSET_ClearText();
        HANDSET_PrintString("CALLING");
        HANDSET_EnableTextDisplay();
      } else {
        CALL_TIMER_EnableDisplayUpdate();
      }
    } else if (TRANSCEIVER_IsBatteryLevelLow()) {
      HANDSET_DisableTextDisplay();
      HANDSET_ClearText();
      HANDSET_PrintString("  LOW  BATTERY");
      HANDSET_EnableTextDisplay();
    } else {
      HANDSET_ClearText();
    }
  } else {
    CALL_TIMER_DisableDisplayUpdate();
    HANDSET_DisableTextDisplay();
    HANDSET_ClearText();

    if (numberInputLength > 14) {
      HANDSET_PrintString(numberInput + numberInputLength - 14);
    } else {
      HANDSET_PrintString(numberInput);
    }

    HANDSET_EnableTextDisplay();
  }
}

static void NumberInput_Clear(void) {
  numberInput[numberInputLength = 0] = 0;
  numberInputIsStale = false;
}

static uint8_t NumberInput_GetMemoryIndex(void) {
  uint8_t result;
  
  if ((numberInputLength < 1) || (numberInputLength > 2)) {
    return 0xFF;
  }
  
  if ((numberInputLength >= 1) && (numberInputLength <= 2)) {
    if (!isdigit(numberInput[0])) {
      return 0xFF;
    }
    
    result = (numberInput[0] & 0x0F);

    if (numberInputLength == 2) {
      if (!isdigit(numberInput[1])) {
        return 0xFF;
      } else {
        result = result * 10 + (numberInput[1] & 0x0F);
      }
    }
  }
 
  result -= 1;
  
  return (result < DIRECTORY_SIZE) ? result : 0xFF;
}

static void returnToNumberInput(bool withRecalledNumber);

static void startCallFailed(void) {
  SOUND_PlayEffect(
      SOUND_Channel_BACKGROUND, 
      SOUND_Target_EAR,
      VOLUME_Mode_TONE,
      SOUND_Effect_REORDER_TONE, 
      true
      );

  HANDSET_SetIndicator(HANDSET_Indicator_IN_USE, true);

  callFailedTimer = CALL_FAILED_TIMEOUT;

  if (
      (appState != APP_State_NUMBER_INPUT) &&
      (appState != APP_State_DISPLAY_NUMBER_OVERFLOW)
  ) {
    returnToNumberInput(false);
  }
}

static void NumberInput_SendCurrentNumberAsDtmf(void) {
  strcpy(dialedNumber, numberInput);
  char* firstPause = strbrk(dialedNumber, "PM");
  
  if (firstPause) {
    if (*firstPause == 'M') {
      firstPause[1] = '^';
    }
    *firstPause = 0;
    dialedNumberNextDigitsToDial = firstPause + 1;
  } else {
    dialedNumberNextDigitsToDial = NULL;
  }
  
  sendDtmfString(dialedNumber);
}

static void NumberInput_CallCurrentNumber(void) {
  STORAGE_SetLastDialedNumber(numberInput);
  numberInputIsStale = true;
  
  strcpy(dialedNumber, numberInput);
  char* firstPause = strbrk(dialedNumber, "PM");
  
  if (firstPause) {
    if (*firstPause == 'M') {
      firstPause[1] = '^';
    }
    *firstPause = 0;
    dialedNumberNextDigitsToDial = firstPause + 1;
  } else {
    dialedNumberNextDigitsToDial = NULL;
  }

  if (!cellPhoneState.hasService || !BT_MakeCall(dialedNumber)) {
    startCallFailed();
  } else {
    if (appState != APP_State_NUMBER_INPUT) {
      NumberInput_PrintToHandset();
      appState = APP_State_NUMBER_INPUT;
    }

    APP_CallAction = APP_CALL_SENDING;
    TIMEOUT_Start(&callActionTimeout, CALL_ACTION_TIMEOUT);
  }
}

static void recallNumberInputOverflow(RecallOverflowType type) {
  if (numberInputLength > 28) {
    recallOverflowCurrentPage = 2;
  } else if (numberInputLength > 14) {
    recallOverflowCurrentPage = 1;
  } else {
    recallOverflowCurrentPage = 0;
  }

  TIMEOUT_Cancel(&appStateTimeout);
  appState = APP_State_DISPLAY_NUMBER_OVERFLOW;  
  recallOverflowType = type;
}


#define CALLER_ID_MAX_LENGTH (20)
static char callerIdText[CALLER_ID_MAX_LENGTH + 1];

static void setCallerId(char const* text, size_t len) {
  if (!text) {
    callerIdText[0] = 0;
    return;
  }
  
  while (*text == ' ') {
    ++text;
    --len;
  }

  if (len > CALLER_ID_MAX_LENGTH) {
    len = CALLER_ID_MAX_LENGTH;
  }
  
  memcpy(callerIdText, text, len);
  callerIdText[len] = 0;
  
  while (callerIdText[len - 1] == ' ') {
    callerIdText[--len] = 0;
  }
  
  utf2ascii(callerIdText);
  
  //printf("[CALLER ID]: \"%s\"\r\n", callerIdText);
  
  if ((appState == APP_State_INCOMING_CALL) && len && STORAGE_GetCallerIdEnabled()) {
    HANDSET_SetTextBlink(false);
    MARQUEE_Start(callerIdText, MARQUEE_Row_TOP);
  }
}

static bool isCallFlashOn;

static void showIncomingCall(bool isMissedCall) {
  bool showCallerId = STORAGE_GetCallerIdEnabled() && callerIdText[0];
  
  CALL_TIMER_DisableDisplayUpdate();
  
  MARQUEE_Stop();
  HANDSET_DisableTextDisplay();
  HANDSET_ClearText();
  HANDSET_PrintString("CALL ");
  HANDSET_EnableTextDisplay();
  
  if (showCallerId) {
    MARQUEE_Start(callerIdText, MARQUEE_Row_TOP);
  }
 
  if (isMissedCall) {
    HANDSET_SetTextBlink(false);
    TIMEOUT_Cancel(&appStateTimeout);
    appState = APP_State_DISPLAY_DISMISSABLE_TEXT;
  } else {
    if (showCallerId) {
      isCallFlashOn = true;
      TIMEOUT_Start(&appStateTimeout, 50);
    } else  {
      HANDSET_SetTextBlink(true);
    }
    
    appState = APP_State_INCOMING_CALL;
  }
}

typedef enum NameInputReason {
  NAME_INPUT_Directory,
  NAME_INPUT_BluetoothName
} NameInputReason;

static NameInputReason nameInputReason;
static uint8_t nameInputMaxLength;

static void startNameInput(NameInputReason reason, bool reset) {
  nameInputReason = reason;
  nameInputMaxLength = (reason == NAME_INPUT_Directory) ? MAX_NAME_LENGTH : BT_MAX_DEVICE_NAME_LENGTH;
  
  if (reset) {
    NumberInput_Clear();
  }

  CALL_TIMER_DisableDisplayUpdate();
  HANDSET_DisableTextDisplay();

  if (numberInputLength) {
    NumberInput_PrintToHandset();

    if (numberInputLength < nameInputMaxLength) {
      HANDSET_PrintChar('_');
    }
    
    isAlphaPromptDisplayed = false;
  } else {
    HANDSET_ClearText();
    HANDSET_PrintString(reason == NAME_INPUT_BluetoothName ? "BT     NAME ? " : "NAME ?        ");
    isAlphaPromptDisplayed = true;
  }
  
  HANDSET_EnableTextDisplay();
  INDICATOR_StartFlashing(HANDSET_Indicator_FCN);  
  
  isAlphaInput = true;
  isFcnInputPendingForAlphaSto = false;
  alphaButton = 0;
  alphaCharIndex = 0;
  isAlphaCharAccepted = false;

  appState = APP_State_ALPHA_STO_NAME_INPUT;
  TIMEOUT_Cancel(&appStateTimeout);
}

static void startAlphaStoreNumberInput(bool reset) {
  if (reset) {
    NumberInput_Clear();
  }

  if (numberInputLength) {
    NumberInput_PrintToHandset();
    isAlphaPromptDisplayed = false;
  } else {
    HANDSET_DisableTextDisplay();
    HANDSET_PrintString("NUMBER?       ");
    HANDSET_EnableTextDisplay();
    isAlphaPromptDisplayed = true;
  }

  INDICATOR_StopFlashing(HANDSET_Indicator_FCN, false);
  
  appState = APP_State_ALPHA_STO_NUMBER_INPUT;
  isFcnInputPendingForAlphaSto = false;
  isStoInputPending = false;
}

static void startAlphaScan(bool isFromDirectoryScan) {
  CALL_TIMER_DisableDisplayUpdate();
  
  if (!isFromDirectoryScan) {
    HANDSET_DisableTextDisplay();
    HANDSET_PrintString("ALPHA  SCAN   ");
    HANDSET_EnableTextDisplay();
  }
  
  isAlphaPromptDisplayed = true;
  isAlphaInput = true;
  alphaButton = 0;
  alphaCharIndex = 0;
  isAlphaCharAccepted = false;

  INDICATOR_StartFlashing(HANDSET_Indicator_FCN);
  
  appState = APP_State_ALPHA_SCAN;
}

static void setBluetoothName(char const* name) {
  INDICATOR_StopFlashing(HANDSET_Indicator_FCN, false);
  BT_SetDeviceName(name);
  NumberInput_Clear();
  
  HANDSET_DisableTextDisplay();
  HANDSET_ClearText();
  HANDSET_PrintString("STORED BT NAME");
  HANDSET_EnableTextDisplay();
  appState = APP_State_DISPLAY_DISMISSABLE_TEXT;
}

static void displayBatteryLevel(bool showPairedBatteryLevel) {
  uint8_t batteryLevel = showPairedBatteryLevel 
      ? cellPhoneState.batteryLevel 
      : TRANSCEIVER_GetBatteryLevel();
  
  uint8_t targetAppState = showPairedBatteryLevel
      ? APP_State_DISPLAY_PAIRED_BATTERY_LEVEL
      : APP_State_DISPLAY_BATTERY_LEVEL;

  if (appState != targetAppState) {
    if (!showPairedBatteryLevel) {
      TRANSCEIVER_PollBatteryLevelNow();
    }
    
    CALL_TIMER_DisableDisplayUpdate();
    HANDSET_DisableTextDisplay();
    HANDSET_PrintString(showPairedBatteryLevel ? "CELLBATV:" : "BATTERYV:");
    HANDSET_PrintCharN('-', batteryLevel);
    HANDSET_PrintCharN(' ', 5 - batteryLevel);
    HANDSET_EnableTextDisplay();

    appState = targetAppState;
  } else {
    uint8_t pos = 4;
    uint8_t spaces = 5 - batteryLevel;
    
    while (batteryLevel) {
      HANDSET_PrintCharAt('-', pos--);
      --batteryLevel;
    }
    
    while (spaces) {
      HANDSET_PrintCharAt(' ', pos--);
      --spaces;
    }
  }
}

static void displayEmptyMessage(void) {
  CALL_TIMER_DisableDisplayUpdate();
  NumberInput_Clear();
  HANDSET_DisableTextDisplay();
  HANDSET_ClearText();
  HANDSET_PrintString("EMPTY ");
  HANDSET_EnableTextDisplay();
  
  numberInputIsStale = true;
  appState = APP_State_DISPLAY_DISMISSABLE_TEXT;
}

static void displayMemoryFullMessage(void) {
  HANDSET_DisableTextDisplay();
  HANDSET_ClearText();
  HANDSET_PrintString("MEMORY   FULL ");
  HANDSET_EnableTextDisplay();
  appState = APP_State_DISPLAY_DISMISSABLE_TEXT;
  numberInputIsStale = true;
}

static void displayAddressUpdatedMessage(uint8_t index, bool stored) {
  STORAGE_SetDirectoryIndex(index);
  CALL_TIMER_DisableDisplayUpdate();
  HANDSET_DisableTextDisplay();
  HANDSET_ClearText();
  HANDSET_PrintString(stored ? "STORED" : "ERASED");
  HANDSET_PrintString(" ADDR ");
  HANDSET_PrintString(uint2str(
      tempNumberBuffer,
      index + 1,
      2,
      2
      ));
  HANDSET_EnableTextDisplay();

  numberInputIsStale = true;
  appState = APP_State_DISPLAY_DISMISSABLE_TEXT;
}

static void recallNumber(char const* number) {
  NumberInput_Overwrite(number);

  if (number[0]) {
    returnToNumberInput(true);
  } else {
    displayEmptyMessage();
  }
}

static void recallAddress(uint8_t addr, bool isNameMode) {
  if (addr > DIRECTORY_SIZE) {
    displayEmptyMessage();
    return;
  }
  
  CALL_TIMER_DisableDisplayUpdate();
  INDICATOR_StopFlashing(HANDSET_Indicator_FCN, false);
  STORAGE_SetDirectoryIndex(addr);
  STORAGE_GetDirectoryNumber(addr, tempNumberBuffer);

  NumberInput_Overwrite(tempNumberBuffer);

  if (tempNumberBuffer[0]) {
    char buf[3];
    
    HANDSET_DisableTextDisplay();
    HANDSET_ClearText();
      HANDSET_PrintString(uint2str(
      buf,
      addr + 1,
      2,
      2
      ));
    HANDSET_PrintChar(':');

    if (isNameMode) {
      STORAGE_GetDirectoryName(addr, alphaInput);
      size_t length = strlen(alphaInput);
      
      if (length <= 11) {
        HANDSET_PrintCharN(' ', 11 - length);
        HANDSET_PrintString(alphaInput);
      } else {
        HANDSET_PrintStringN(alphaInput, 11);
      }
    } else {
      size_t length = strlen(tempNumberBuffer);
      
      if (length <= 11) {
        HANDSET_PrintCharN(' ', 11 - strlen(tempNumberBuffer));
        HANDSET_PrintString(tempNumberBuffer);
      } else {
        HANDSET_PrintString(tempNumberBuffer + length - 11);
      }
    }
    
    HANDSET_EnableTextDisplay();
    isDirectoryScanNameMode = isNameMode;
    appState = APP_State_BROWSE_DIRECTORY_IDLE;
  } else {
    displayEmptyMessage();
  }
}

static uint8_t creditCardIndex;

static void recallCreditCardNumber(uint8_t index) {
  creditCardIndex = index;
  STORAGE_GetCreditCardNumber(index, tempNumberBuffer);
  
  NumberInput_Clear();
  
  if (tempNumberBuffer[0]) {
    HANDSET_DisableTextDisplay();
    HANDSET_ClearText();
    HANDSET_PrintChar('*');
    HANDSET_PrintChar('1' + index);
    HANDSET_PrintChar(':');

    size_t length = strlen(tempNumberBuffer);

    if (length <= 11) {
      HANDSET_PrintCharN(' ', 11 - strlen(tempNumberBuffer));
      HANDSET_PrintString(tempNumberBuffer);
    } else {
      HANDSET_PrintString(tempNumberBuffer + length - 11);
    }

    HANDSET_EnableTextDisplay();
    appState = APP_State_DISPLAY_CREDIT_CARD_NUMBER;
  } else {
    displayEmptyMessage();
  }
}

static void showCreditCardNumberOverflow(void) {
  STORAGE_GetCreditCardNumber(creditCardIndex, tempNumberBuffer);
  size_t length = strlen(tempNumberBuffer);
  
  if (length > 11) {
    SOUND_PlayButtonBeep(HANDSET_Button_RCL, false);
    HANDSET_DisableTextDisplay();
    HANDSET_ClearText();
    HANDSET_PrintStringN(tempNumberBuffer, length - 11);
    HANDSET_EnableTextDisplay();
    
    appState = APP_State_DISPLAY_CREDIT_CARD_NUMBER_SHOW_OVERFLOW;
  }
}

static void recallSpeedDial(uint8_t index) {
  STORAGE_GetSpeedDial(index, tempNumberBuffer);
  recallNumber(tempNumberBuffer);
}

static void recallLastDialedNumber() {
  STORAGE_GetLastDialedNumber(tempNumberBuffer);
  recallNumber(tempNumberBuffer);
}

static void recallBtDeviceName(void) {
  CALL_TIMER_DisableDisplayUpdate();
  BT_ReadDeviceName();
  
  HANDSET_DisableTextDisplay();
  HANDSET_ClearText();
  HANDSET_PrintString("NAME:         ");
  HANDSET_EnableTextDisplay(); 

  appState = APP_State_RECALL_BT_DEVICE_NAME;
}

static void recallPairedDeviceName(bool requestDeviceName) {
  CALL_TIMER_DisableDisplayUpdate();
  
  if (appState != APP_State_RECALL_PAIRED_DEVICE_NAME) {
    HANDSET_DisableTextDisplay();
    HANDSET_ClearText();
    HANDSET_PrintString("PAIRED:       ");
    HANDSET_EnableTextDisplay(); 
  }

  if (requestDeviceName) {
    BT_ReadLinkedDeviceName();
  } else {
    char deviceName[MAX_DEVICE_NAME_LENGTH + 1];
    STORAGE_GetPairedDeviceName(deviceName);
    MARQUEE_Start(deviceName[0] ? deviceName : "[none]", MARQUEE_Row_BOTTOM);
  }
  
  appState = APP_State_RECALL_PAIRED_DEVICE_NAME;
}

static void returnToNumberInput(bool withRecalledNumber) {
  INDICATOR_StopFlashing(HANDSET_Indicator_FCN, false);
  // in case we are coming from incoming call state with blinking "CALL" text
  HANDSET_SetTextBlink(false);
  MARQUEE_Stop();
  
  if (numberInputLength > 14) {
    recallNumberInputOverflow(withRecalledNumber ? RECALL_FROM_MEMORY : RECALL_FROM_INPUT_AUTOMATIC);
  } else {
    NumberInput_PrintToHandset();
    appState = APP_State_NUMBER_INPUT;
  }
}

static void selectSystemMode(bool isVehicleMode) {
  STORAGE_SetVehicleModeEnabled(isVehicleMode);
  
  CALL_TIMER_DisableDisplayUpdate();
  HANDSET_DisableTextDisplay();
  HANDSET_ClearText();
  HANDSET_PrintString(isVehicleMode ? "VEHICLE" : "CARRIED");
  HANDSET_PrintString("MODE   ");
  HANDSET_EnableTextDisplay();
  
  appState = APP_State_SELECT_SYSTEM_MODE;
}

typedef enum SecurityAction {
  SecurityAction_CLEAR_TALK_TIME,
  SecurityAction_DISPLAY_TOTAL_TALK_TIME,
  SecurityAction_RCL_CARD_NUMBER,
  SecurityAction_STO_CARD_NUMBER,
  SecurityAction_RESET_PAIRED_DEVICES,
  SecurityAction_SET_BT_DEVICE_NAME,
  SecurityAction_TOGGLE_DUAL_NUMBER
} SecurityAction;

static void handle_SECURITY_CODE_Failure(void) {
  returnToNumberInput(false);
}

static void handle_SECURITY_CODE_Success_CLEAR_TALK_TIME(void) {
  STORAGE_ResetCallTime();
  returnToNumberInput(false);
}

static void handle_SECURITY_CODE_Success_DISPLAY_TOTAL_TALK_TIME(void) {
  HANDSET_DisableTextDisplay();
  HANDSET_ClearText();
  HANDSET_PrintString("OPERATE");

  HANDSET_PrintString("  ");
  HANDSET_PrintString(uint2str(
      tempNumberBuffer, 
      STORAGE_GetTotalCallMinutes(),
      5,
      5
      ));

  HANDSET_EnableTextDisplay();
  appState = APP_State_DISPLAY_DISMISSABLE_TEXT;
}

static void handle_SECURITY_CODE_Success_RCL_CARD_NUMBER(void) {
  recallCreditCardNumber(creditCardIndex);
}

static void handle_SECURITY_CODE_Success_CLR_CARD_NUMBER(void) {
  STORAGE_SetCreditCardNumber(creditCardIndex, numberInput);

  HANDSET_DisableTextDisplay();
  HANDSET_ClearText();
  HANDSET_PrintString(numberInputLength ? "STORED " : "ERASED ");
  HANDSET_PrintString("ADDR *");
  HANDSET_PrintChar('1' + creditCardIndex);
  HANDSET_EnableTextDisplay();
  appState = APP_State_DISPLAY_DISMISSABLE_TEXT;
  numberInputIsStale = true;
}

static void handle_SECURITY_CODE_Success_RESET_PAIRED_DEVICES(void) {
  BT_DisconnectAllProfile();
  BT_ResetEEPROM();
  STORAGE_SetPairedDeviceName("");
  returnToNumberInput(false);
}

static void handle_SECURITY_CODE_Success_SET_BT_DEVICE_NAME(void) {
  startNameInput(NAME_INPUT_BluetoothName, true);
}

static void handle_SECURITY_CODE_Success_TOGGLE_DUAL_NUMBER(void) {
  STORAGE_SetActiveNumberIndex(!STORAGE_GetActiveNumberIndex());
  reboot();
}

static SECURITY_CODE_Callback SECURITY_CODE_SUCCESS_CALLBACKS[] = {
  handle_SECURITY_CODE_Success_CLEAR_TALK_TIME,
  handle_SECURITY_CODE_Success_DISPLAY_TOTAL_TALK_TIME,
  handle_SECURITY_CODE_Success_RCL_CARD_NUMBER,
  handle_SECURITY_CODE_Success_CLR_CARD_NUMBER,
  handle_SECURITY_CODE_Success_RESET_PAIRED_DEVICES,
  handle_SECURITY_CODE_Success_SET_BT_DEVICE_NAME,
  handle_SECURITY_CODE_Success_TOGGLE_DUAL_NUMBER
};

static void startEnterSecurityCode(SecurityAction action, bool prompt) {
  if (prompt) {
    MARQUEE_Stop();
    CALL_TIMER_DisableDisplayUpdate();
  }

  SECURITY_CODE_Callback const successCallback = SECURITY_CODE_SUCCESS_CALLBACKS[action];
  
  if (prompt) {
    SECURITY_CODE_Prompt(successCallback, handle_SECURITY_CODE_Failure);
  } else {
    SECURITY_CODE_Watch(successCallback);
  }
  
  appState = APP_State_ENTER_SECURITY_CODE;
}

static void handleReturnFromSubModule(void) {
  HANDSET_CancelCurrentButtonHoldEvents();

  if (BT_CallStatus == BT_CALL_INCOMING) {
    showIncomingCall(false);
  } else {
    returnToNumberInput(false);
  }  
}

static void startVolumeAdjust(VOLUME_Mode volumeMode, bool up) {
  // In case we are adjusting volume during an incoming call state with blinking "CALL" text
  HANDSET_SetTextBlink(false);
  MARQUEE_Stop();
  CALL_TIMER_DisableDisplayUpdate();
  VOLUME_ADJUST_Start(volumeMode, up, handleReturnFromSubModule);
  appState = APP_State_ADJUST_VOLUME;
}

static char nameInput[MAX_NAME_LENGTH + 1] = {0};
static char nameInputLength = 0;


void handleCallListAtResponse(ATCMD_Response response, char const* result) {
  if (response == ATCMD_Response_RESULT) {
    char phoneNumber[24];
    char buffer[48];

    // id
    char const* nextField = parseNextCsvField(buffer, result + 7);
    // dir
    nextField = parseNextCsvField(buffer, nextField);

    // If is MT (incoming)...
    if (*buffer == '1') {
      // stat
      nextField = parseNextCsvField(buffer, nextField);

      // If is Incoming Call or Call Waiting...
      if ((*buffer == '4') || (*buffer == '5')) {
        // mode
        nextField = parseNextCsvField(buffer, nextField);
        // mpty
        nextField = parseNextCsvField(buffer, nextField);
        // number
        nextField = parseNextCsvField(phoneNumber, nextField);
        // type
        nextField = parseNextCsvField(buffer, nextField);
        // alpha
        nextField = parseNextCsvField(buffer, nextField);

        // If "alpha" is populated, then use it as Caller ID
        if (*buffer) {
          setCallerId(buffer, strlen(buffer));
        } else {
          setCallerId(phoneNumber, strlen(phoneNumber));
        }
      }
    }
  }
}

static int pendingCallStatus;
static timeout_t pendingCallStatusTimeout;

static void handleCallStatusChange(int newCallStatus) {
  if (newCallStatus == BT_CallStatus) {
    //printf("[handleCallStatusChange] Ignoring non-change: %s\r\n", BT_CallStatusLabel[newCallStatus]);
    return;
  }
  
  //printf("[handleCallStatusChange] %s -> %s\r\n", BT_CallStatusLabel[BT_CallStatus], BT_CallStatusLabel[newCallStatus]);
 
  if (BT_CallStatus == BT_CALL_INCOMING) {
    TIMEOUT_Cancel(&autoAnswerTimeout);
    RINGTONE_Stop();
  }

  if (callFailedTimer || callFailedTimerExpired) {
    SOUND_Stop(SOUND_Channel_BACKGROUND);
    HANDSET_SetIndicator(HANDSET_Indicator_IN_USE, false);
    callFailedTimerExpired = true;
    callFailedTimer = 0;
    callFailedTimerExpired = false;
  }

  int prevCallStatus = BT_CallStatus;
  BT_CallStatus = newCallStatus;

  switch (BT_CallStatus) {
    case BT_CALL_IDLE:
      TIMEOUT_Start(&idleTimeout, IDLE_TIMEOUT);
      dialedNumberNextDigitsToDial = NULL;
      CALL_TIMER_Stop();
      SOUND_SetDefaultAudioSource(SOUND_AudioSource_MCU);

      if ((appState == APP_State_INCOMING_CALL) && (APP_CallAction != APP_CALL_REJECTING)) {
        numberInputIsStale = true;
        showIncomingCall(true);
      } else { 
        if ((APP_CallAction != APP_CALL_ENDING) && (APP_CallAction != APP_CALL_REJECTING)) {
          SOUND_PlayEffect(
            SOUND_Channel_FOREGROUND, 
            SOUND_Target_EAR,
            HANDSET_IsOnHook() ? VOLUME_Mode_HANDS_FREE : VOLUME_Mode_HANDSET,
            SOUND_Effect_CALL_DISCONNECT, 
            false
          );
        }

        if ((appState == APP_State_NUMBER_INPUT) || (appState == APP_State_INCOMING_CALL)) {
          numberInputIsStale = true;
          returnToNumberInput(false);
        }
      }

      INDICATOR_StopFlashing(HANDSET_Indicator_IN_USE, false);
      HANDSET_SetIndicator(HANDSET_Indicator_MUTE, false);
      isMuted = false;
      SOUND_SetButtonsMuted(false);
      setCallerId(NULL, 0);
      break;

    case BT_CALL_ACTIVE:
      if ((prevCallStatus == BT_CALL_ACTIVE_WITH_HOLD) && (APP_CallAction != APP_CALL_ENDING) && (APP_CallAction != APP_CALL_REJECTING)) {
        SOUND_PlayEffect(
          SOUND_Channel_FOREGROUND, 
          SOUND_Target_EAR,
          HANDSET_IsOnHook() ? VOLUME_Mode_HANDS_FREE : VOLUME_Mode_HANDSET,
          SOUND_Effect_CALL_DISCONNECT, 
          false
        );
      }
      // Look out below!
      
    case BT_CALL_ACTIVE_WITH_HOLD:
      setCallerId(NULL, 0);
      CALL_TIMER_Start(false);
      // Look out below!

    case BT_CALL_OUTGOING:
      wakeUpHandset(true);
      BT_SetHFPGain(0x0F);
      SOUND_SetDefaultAudioSource(SOUND_AudioSource_BT);
      HANDSET_SetTextBlink(false);
      
      if (BT_CallStatus > BT_CALL_ACTIVE_WITH_CALL_WAITING) {
        INDICATOR_StartFlashing(HANDSET_Indicator_IN_USE);
      } else {
        INDICATOR_StopFlashing(HANDSET_Indicator_IN_USE, true);
      }

      if (APP_CallAction == APP_CALL_SENDING) {
        NumberInput_Clear();
      }

      if (appState == APP_State_NUMBER_INPUT) {
        NumberInput_PrintToHandset();
      } else if (appState == APP_State_INCOMING_CALL) {
        returnToNumberInput(false);
      }
      break;

    case BT_CALL_ACTIVE_WITH_CALL_WAITING:
    case BT_CALL_INCOMING:
      wakeUpHandset(true);
      showIncomingCall(false);

      INDICATOR_StopFlashing(HANDSET_Indicator_IN_USE, BT_CallStatus == BT_CALL_ACTIVE_WITH_CALL_WAITING);
      
      if (BT_CallStatus == BT_CALL_INCOMING) {
        RINGTONE_Start(STORAGE_GetRingtone());
        
        if (STORAGE_GetAutoAnswerEnabled()) {
          TIMEOUT_Start(&autoAnswerTimeout, 1200);
        }
      }

      resetFcn();
      isRclInputPending = false;
      isRclOrSto2ndDigitPending = false;
      isStoInputPending = false;
      
      // Request current call list to extract Caller ID
      if (STORAGE_GetCallerIdEnabled()) {
        ATCMD_Send("+CLCC", handleCallListAtResponse);
      }
      break;

    case BT_CALL_VOICE_DIAL:
      break;
  }

  APP_CallAction = APP_CALL_IDLE;
  TIMEOUT_Cancel(&callActionTimeout);
}

static void reportBatteryLevelToBT(void) {
  if (!cellPhoneState.isConnected || TIMEOUT_IsPending(&cellPhoneState.initialBatteryLevelReportTimeout)) {
    return;
  }
  
  uint8_t batteryLevel;
  
  if (TRANSCEIVER_IsBatteryLevelLow()) {
    batteryLevel = 0;
  } else {
    batteryLevel = TRANSCEIVER_GetBatteryLevel();
    
    if (batteryLevel == 0) {
      // Battery level not known yet, so don't report it
      return;
    }
    
    batteryLevel = (batteryLevel << 1) - 1;
  }
  
  char cmd[] = "+IPHONEACCEV=1,1, ";
  cmd[17] = '0' + batteryLevel;

  ATCMD_Send(cmd, NULL);
}

static void handle_CLR_CODES_Event(CLR_CODES_EventType event) {
  switch (event) {
    case CLR_CODES_EventType_PROGRAM: {
      uint8_t count = STORAGE_GetProgrammingCount();
      
      if (count < 3) {
        STORAGE_SetProgrammingCount(count + 1);
        appState = APP_State_PROGRAMMING;
        PROGRAMMING_Start(reboot);
      }
      break;
    }
    
    case CLR_CODES_EventType_PROGRAM_RESET: 
      STORAGE_SetProgrammingCount(0);
      appState = APP_State_PROGRAMMING;
      PROGRAMMING_Start(reboot);
      break;
    
    case CLR_CODES_EventType_FACTORY_RESET: 
      HANDSET_DisableTextDisplay();
      HANDSET_PrintString("FACTORY RESET ");
      HANDSET_EnableTextDisplay();
      // Reset the BT module (this erases all paired device info).
      BT_ResetEEPROM();
      // Reset the BT device name to the default.
      BT_SetDeviceName("DiamondTel Model 92");
      // Erase all MCU EEPROM data. After rebooting, STORAGE_Initialize()
      // will detect that the EEPROM data is not initialized, and defaults will
      // be restored.
      EEPROM_AsyncErase();
      // Allow time for BT module commands to complete before rebooting
      rebootAfterDelay(100);
      break;
  }
}

static bool isValidCreditCardMemoryButton(HANDSET_Button button) {
  return (button >= '1') && (button < '1' + CREDIT_CARD_COUNT);
}

static uint8_t getCreditCardMemoryIndexFromButton(HANDSET_Button button) {
  return button - '1';
}

void handle_HANDSET_Event(HANDSET_Event const* event);
void handle_TRANSCEIVER_Event(TRANSCEIVER_EventType event);
void handle_ATCMD_UnsolicitedResult(char const* result);

void APP_Initialize(void) {
  IO_BT_RESET_SetLow();
  
  appState = APP_State_INIT_START;
  lastAppState = -1;
  BT_CallStatus = BT_CALL_IDLE;

  EEPROM_Initialize();
  TONE_Initialize();
  HANDSET_Initialize(handle_HANDSET_Event);
  TRANSCEIVER_Initialize(handle_TRANSCEIVER_Event);
  INDICATOR_Initialize();
  CALL_TIMER_Initialize();
  BT_CommandDecodeInit();
  BT_CommandSendInit();
  ATCMD_Initialize(handle_ATCMD_UnsolicitedResult);
  MARQUEE_Initialize();
  CLR_CODES_Initialize();
  INTERVAL_Init(&lowBatteryBeepInterval, LOW_BATTERY_BEEP_INTERVAL);
}

void APP_Task(void) {
  if (appState != lastAppState) {
    lastAppState = appState;
    printf("[App State] %s\r\n", appStateLabel[appState]);
  }
  
  EEPROM_Task();
  VOLUME_Task();
  SOUND_Task();
  CALL_TIMER_Task();
  INDICATOR_Task();
  MARQUEE_Task();
  ATCMD_Task();
  BT_CommandDecodeMain();
  BT_CommandSendTask();
  HANDSET_Task();
  TRANSCEIVER_Task();
  CLR_CODES_Task();
  
  TIMEOUT_Task(&appStateTimeout);
  
  if (
      TIMEOUT_Task(&idleTimeout) && 
      !STORAGE_GetVehicleModeEnabled() &&
      HANDSET_IsOnHook() && 
      !HANDSET_IsAnyButtonDown() &&
      (BT_CallStatus == BT_CALL_IDLE) &&
      (appState != APP_State_PAIRING) && 
      !callFailedTimer
  ) {
    isHandsetIdle = true;
    HANDSET_SetBacklight(false);
    SOUND_Disable();
  }
  
  if (TIMEOUT_Task(&fcnTimeout)) {
    resetFcn();
  }
  
  if (TIMEOUT_Task(&callActionTimeout)) {
    if (APP_CallAction == APP_CALL_SENDING) {
      startCallFailed();
    }
    
    APP_CallAction = APP_CALL_IDLE;
  }
  
  if (
      TIMEOUT_Task(&autoAnswerTimeout) && 
      (BT_CallStatus == BT_CALL_INCOMING) &&
      (APP_CallAction == APP_CALL_IDLE)
      ) {
    BT_AcceptCall();
    SOUND_PlayButtonBeep(HANDSET_Button_NONE, true);
    APP_CallAction = APP_CALL_ACCEPTING;
  }
  
  if (TIMEOUT_Task(&pendingCallStatusTimeout)) {
    handleCallStatusChange(pendingCallStatus);
  }
  
  if (TIMEOUT_Task(&linkbackRetryTimeout)) {
    BT_LinkBackToLastDevice();
  }
  
  if (TIMEOUT_Task(&cellPhoneState.initialBatteryLevelReportTimeout)) {
    reportBatteryLevelToBT();
  }
  
  if (INTERVAL_Task(&lowBatteryBeepInterval)) {
    wakeUpHandset(false);
    SOUND_PlaySingleTone(SOUND_Channel_FOREGROUND, SOUND_Target_SPEAKER, VOLUME_Mode_TONE, TONE_HIGH, 1000);
  }

  if (callFailedTimerExpired) {
    HANDSET_SetIndicator(HANDSET_Indicator_IN_USE, false);
    SOUND_Stop(SOUND_Channel_BACKGROUND);
    callFailedTimerExpired = false;
  }
  
  if (rcl2ndDigitTimerExpired) {
    if (rclOrStoAddr) {
      recallAddress(rclOrStoAddr - 1, false);
    }
    rcl2ndDigitTimerExpired = false;
    isRclInputPending = false;
  }
  
  switch(appState) {
    case APP_State_INIT_START: 
      TIMEOUT_Start(&appStateTimeout, 25);
      STORAGE_Initialize();
      VOLUME_Initialize();
      
      appState = APP_State_INIT_SET_LCD_ANGLE;
      break;
      
    case APP_State_INIT_SET_LCD_ANGLE: 
      if (!TIMEOUT_IsPending(&appStateTimeout)) {
        TIMEOUT_Start(&appStateTimeout, 25);
        HANDSET_SetLcdViewAngle(STORAGE_GetLcdViewAngle());
        IO_BT_RESET_SetHigh();
        
        appState = APP_State_INIT_ALL_DISPLAY_ON;
        break;
      }
      
    case APP_State_INIT_ALL_DISPLAY_ON:
      if (!TIMEOUT_IsPending(&appStateTimeout)) {
        TIMEOUT_Start(&appStateTimeout, 50);
        SOUND_Initialize();
        VOLUME_Enable();
        SOUND_PlayEffect(
            SOUND_Channel_FOREGROUND, 
            SOUND_Target_SPEAKER,
            VOLUME_Mode_SPEAKER,
            SOUND_Effect_TONE_DUAL_CONTINUOUS, 
            true
        );
        HANDSET_SetTextBlink(false);
        HANDSET_DisableTextDisplay();
        HANDSET_ClearText();
        HANDSET_PrintCharN('8', 14);
        HANDSET_EnableTextDisplay();
        HANDSET_SetAllIndicators(true);
        HANDSET_SetBacklight(true);
        
        appState = APP_State_INIT_DISPLAY_PHONE_NUMBER;
      }
      break;
      
    case APP_State_INIT_DISPLAY_PHONE_NUMBER:
      if (!TIMEOUT_IsPending(&appStateTimeout)) {
        HANDSET_ClearText();

        HANDSET_SetIndicator(HANDSET_Indicator_IN_USE, false);
        HANDSET_SetIndicator(HANDSET_Indicator_NO_SVC, false);
        HANDSET_SetIndicator(HANDSET_Indicator_ROAM, false);
        HANDSET_SetIndicator(HANDSET_Indicator_FCN, false);
        HANDSET_SetIndicator(HANDSET_Indicator_HORN, false);
        HANDSET_SetIndicator(HANDSET_Indicator_MUTE, false);

        if (STORAGE_GetShowOwnNumberEnabled()) {
          TIMEOUT_Start(&appStateTimeout, 150);
          STORAGE_GetOwnNumber(STORAGE_GetActiveNumberIndex(), tempNumberBuffer);
          HANDSET_DisableTextDisplay();
          HANDSET_PrintString(tempNumberBuffer);
          HANDSET_EnableTextDisplay();
        }

        SOUND_Stop(SOUND_Channel_FOREGROUND);

        appState = APP_State_INIT_CLEAR_PHONE_NUMBER;        
      }
      break;
      
    case APP_State_INIT_CLEAR_PHONE_NUMBER:
      if (!TIMEOUT_IsPending(&appStateTimeout) && BT_isReady) {
        BT_SetEventMask();
        BT_LinkBackToLastDevice();
        INDICATOR_StartFlashing(HANDSET_Indicator_NO_SVC);
        HANDSET_SetSignalStrength(0);
        
        HANDSET_ClearText();
        HANDSET_RequestHookStatus();
        HANDSET_EnableCommandOptimization();
        
        CLR_CODES_Start(handle_CLR_CODES_Event);
        
        appState = APP_State_NUMBER_INPUT;        
      }
      break;
      
    case APP_State_PAIRING:
      if (!TIMEOUT_IsPending(&appStateTimeout)) {
        wakeUpHandset(false);
        SOUND_PlayEffect(
            SOUND_Channel_FOREGROUND,
            SOUND_Target_SPEAKER,
            VOLUME_Mode_TONE,
            SOUND_Effect_CALL_DISCONNECT,
            false
        );
        BT_ExitPairingMode();
        INDICATOR_StopSignalStrengthSweep(0);
        INDICATOR_StartFlashing(HANDSET_Indicator_NO_SVC);
        
        HANDSET_DisableTextDisplay();
        HANDSET_PrintString("PAIRINGTIMEOUT");
        HANDSET_EnableTextDisplay();
        appState = APP_State_DISPLAY_DISMISSABLE_TEXT;
      }
      break;
      
    case APP_State_INCOMING_CALL:
      if (!TIMEOUT_IsPending(&appStateTimeout)) {
        if (STORAGE_GetCallerIdEnabled() && callerIdText[0]) {
          isCallFlashOn = !isCallFlashOn;
          HANDSET_PrintStringAt(isCallFlashOn ? "CALL" : "    ", 4);
          TIMEOUT_Start(&appStateTimeout, 50);
        }
      }
      break;
      
    case APP_State_DISPLAY_NUMBER_OVERFLOW:
      if (!TIMEOUT_IsPending(&appStateTimeout)) {
        switch (recallOverflowCurrentPage) {
          case 0: 
            if (recallOverflowType != RECALL_FROM_INPUT_MANUAL) {
              NumberInput_PrintToHandset();
              appState = APP_State_NUMBER_INPUT;
            }
            break;
            
          case 1: 
            HANDSET_DisableTextDisplay();
            HANDSET_ClearText();
            if (numberInputLength > 28) {
              HANDSET_PrintStringN(numberInput + numberInputLength - 28, 14);
            } else {
              HANDSET_PrintStringN(numberInput, numberInputLength - 14);
            }
            HANDSET_EnableTextDisplay();
            --recallOverflowCurrentPage;
            TIMEOUT_Start(&appStateTimeout, 100);
            break;
            
          case 2: 
            HANDSET_DisableTextDisplay();
            HANDSET_ClearText();
            HANDSET_PrintStringN(numberInput, numberInputLength - 28);
            HANDSET_EnableTextDisplay();
            --recallOverflowCurrentPage;
            TIMEOUT_Start(&appStateTimeout, 100);
            break;
        }
      }
      break;
      
    case APP_State_ADJUST_VOLUME: 
      VOLUME_ADJUST_Task();
      break;
      
    case APP_State_SELECT_RINGTONE: 
      RINGTONE_SELECT_Task();
      break;
      
    case APP_State_BROWSE_DIRECTORY_UP:
      if (!TIMEOUT_IsPending(&appStateTimeout)) {
        uint8_t index = STORAGE_GetDirectoryIndex();
        if (isDirectoryScanNameMode) {
          index = STORAGE_GetNextNamedDirectoryIndex(index, true);
        } else {
          index = STORAGE_GetNextPopulatedDirectoryIndex(index, true);
        }
        
        recallAddress(index, isDirectoryScanNameMode);
        appState = APP_State_BROWSE_DIRECTORY_UP;
        TIMEOUT_Start(&appStateTimeout, BROWSE_DIRECTORY_SCAN_INTERVAL);
      }
      break;
      
    case APP_State_BROWSE_DIRECTORY_DOWN:
      if (!TIMEOUT_IsPending(&appStateTimeout)) {
        uint8_t index = STORAGE_GetDirectoryIndex();
        if (isDirectoryScanNameMode) {
          index = STORAGE_GetNextNamedDirectoryIndex(index, false);
        } else {
          index = STORAGE_GetNextPopulatedDirectoryIndex(index, false);
        }
        
        recallAddress(index, isDirectoryScanNameMode);
        appState = APP_State_BROWSE_DIRECTORY_DOWN;
        TIMEOUT_Start(&appStateTimeout, BROWSE_DIRECTORY_SCAN_INTERVAL);
      }
      break;

    case APP_State_SNAKE_GAME:
      SNAKE_GAME_Task();
      break;
      
    case APP_State_MEMORY_GAME:
      MEMORY_GAME_Task();
      break;
      
    case APP_State_TETRIS_GAME:
      TETRIS_GAME_Task();
      break;
      
    case APP_State_REBOOT_AFTER_DELAY:
      if (!TIMEOUT_IsPending(&appStateTimeout)) {
        reboot();
      }
      break;
      
    case APP_State_REBOOT:
      if (!TIMEOUT_IsPending(&appStateTimeout) && EEPROM_IsDoneWriting()) {
        RESET();
      }
      break;
  }    
}

void APP_Timer10MS_event(void) {
  if (appState == APP_State_PROGRAMMING) {
    return;
  }

  TIMEOUT_Timer_event(&appStateTimeout);

  switch (appState) {
    case APP_State_REBOOT_AFTER_DELAY:
    case APP_State_REBOOT:
      return;
      
    case APP_State_ADJUST_VOLUME:
      VOLUME_ADJUST_Timer10MS_event();
      break;
      
    case APP_State_SELECT_RINGTONE:
      RINGTONE_SELECT_Timer10MS_event();
      break;
      
    case APP_State_SNAKE_GAME:
      SNAKE_GAME_Timer10MS_event();
      break;
      
    case APP_State_MEMORY_GAME:
      MEMORY_GAME_Timer10MS_event();
      break;
      
    case APP_State_TETRIS_GAME:
      TETRIS_GAME_Timer10MS_event();
      break;
  }
  
  CLR_CODES_Timer10MS_event();
  VOLUME_Timer10MS_event();
  
  TIMEOUT_Timer_event(&idleTimeout);
  TIMEOUT_Timer_event(&fcnTimeout);
  TIMEOUT_Timer_event(&callActionTimeout);
  TIMEOUT_Timer_event(&autoAnswerTimeout);
  TIMEOUT_Timer_event(&pendingCallStatusTimeout);
  TIMEOUT_Timer_event(&linkbackRetryTimeout);
  TIMEOUT_Timer_event(&cellPhoneState.initialBatteryLevelReportTimeout);
  INTERVAL_Timer_event(&lowBatteryBeepInterval);
  
  if (!callFailedTimerExpired && callFailedTimer) {
    if (!--callFailedTimer) {
      callFailedTimerExpired = true;
    }
  }
  
  if (rcl2ndDigitTimer) {
    if (!--rcl2ndDigitTimer) {
      rcl2ndDigitTimerExpired = true;
    }
  }
}

void handle_HANDSET_Event(HANDSET_Event const* event) {
  bool wasDisplayingNonNumberInput = false;
  bool isButtonDown = event->type == HANDSET_EventType_BUTTON_DOWN;
  bool isButtonUp = event->type == HANDSET_EventType_BUTTON_UP;
  uint8_t button = event->button;

  if (isButtonDown && (button == HANDSET_Button_PWR) && event->isFcn) {
    reboot();
  }

  if (appState <= APP_State_INIT_CLEAR_PHONE_NUMBER) {
    return;
  }
  
  SOUND_HANDSET_EventHandler(event);
  
  if (isButtonUp || isButtonDown || (event->type == HANDSET_EventType_HOOK)) {
    wakeUpHandset(button != HANDSET_Button_PWR);
  }

  CLR_CODES_HANDSET_EventHandler(event);

  if (HANDSET_IsButtonClrCode(button)) {
    return;
  }
  
  if (button == HANDSET_Button_PWR) {
    if ( 
      (event->type == HANDSET_EventType_BUTTON_HOLD) &&
      (event->holdDuration == HANDSET_HoldDuration_VERY_SHORT)
      ) {
      SOUND_PlaySingleTone(
          SOUND_Channel_FOREGROUND,
          SOUND_Target_SPEAKER,
          VOLUME_Mode_TONE,
          TONE_HIGH,
          0
      );
    } else if (isButtonUp) {
      if (event->holdDuration >= HANDSET_HoldDuration_VERY_SHORT) {
        // Continue playing the high tone for a short time
        SOUND_PlaySingleTone(
            SOUND_Channel_FOREGROUND,
            SOUND_Target_SPEAKER,
            VOLUME_Mode_TONE,
            TONE_HIGH,
            300
        );
      } else {
        printf("[APP] Sending HANDSET Mystery Command EA\r\n");
        HANDSET_SendArbitraryCommand(0xEA);
      }
    }
  }
  
  if (event->type == HANDSET_EventType_HOOK) {
    HANDSET_SetMicrophone(!event->isOnHook);
  }

  if (event->button != HANDSET_Button_PWR) {
    switch (appState) {
      case APP_State_PROGRAMMING: 
        PROGRAMMING_HANDSET_EventHandler(event);
        return;

      case APP_State_ADJUST_VOLUME:
        VOLUME_ADJUST_HANDSET_EventHandler(event);
        return;
        
      case APP_State_SELECT_RINGTONE:
        RINGTONE_SELECT_HANDSET_EventHandler(event);
        return;
        
      case APP_State_SNAKE_GAME:
        SNAKE_GAME_HANDSET_EventHandler(event);
        return;
        
      case APP_State_MEMORY_GAME:
        MEMORY_GAME_HANDSET_EventHandler(event);
        return;
        
      case APP_State_TETRIS_GAME:
        TETRIS_GAME_HANDSET_EventHandler(event);
        return;
        
      case APP_State_ENTER_SECURITY_CODE:
        if (SECURITY_CODE_HANDSET_EventHandler(event)) {
          return;
        }
    }
  }

  // Process common escapes back to number input state
  if (
      (appState == APP_State_ENTER_SECURITY_CODE) || 
      (appState == APP_State_ADJUST_VIEW_ANGLE) || 
      (appState == APP_State_DISPLAY_DISMISSABLE_TEXT) ||
      (appState == APP_State_DISPLAY_BATTERY_LEVEL) ||
      (appState == APP_State_DISPLAY_PAIRED_BATTERY_LEVEL) ||
      (appState == APP_State_DISPLAY_CREDIT_CARD_NUMBER) || 
      (appState == APP_State_RECALL_BT_DEVICE_NAME) ||
      (appState == APP_State_RECALL_PAIRED_DEVICE_NAME)
  ) {
    if (isButtonDown) {
      switch(button) {
        case HANDSET_Button_RCL:
          if (appState == APP_State_DISPLAY_CREDIT_CARD_NUMBER) {
            // RCL does NOT escape from displaying credit card number
            break;
          }
          // Look out below!

        case HANDSET_Button_STO:
          if ((button == HANDSET_Button_STO) && 
              ((appState == APP_State_RECALL_BT_DEVICE_NAME) || (appState == APP_State_RECALL_PAIRED_DEVICE_NAME))) {
            // STO does NOT escape from BT device name or paired device name
            break;
          }
          // Look out below!
          
        case HANDSET_Button_FCN:
          if ((button == HANDSET_Button_FCN) 
              && ((appState == APP_State_DISPLAY_BATTERY_LEVEL) || (appState == APP_State_DISPLAY_PAIRED_BATTERY_LEVEL))
              ) {
            // FCN does NOT escape from displaying battery level
            break;
          }
          
          returnToNumberInput(false);
          // button will be processed below
          break;
          
        case HANDSET_Button_CLR:
          SOUND_PlayButtonBeep(button, false);
          returnToNumberInput(false);
          // Prevent short hold of CLR from clearing input
          HANDSET_CancelCurrentButtonHoldEvents();
          return;
          
        default:
          if (HANDSET_IsButtonPrintable(button)) {
            MARQUEE_Stop();
            wasDisplayingNonNumberInput = true;
            // button will be processed below
            appState = APP_State_NUMBER_INPUT;
          }
          break;
      }
    }
  }

  if (appState == APP_State_BROWSE_DIRECTORY_IDLE) {
    if (isButtonDown) {
      if (isDirectoryScanNameMode && (button >= '1' && button <= '9')) {
        startAlphaScan(true);
        // button will be processed below
      } else if ((!isDirectoryScanNameMode && HANDSET_IsButtonPrintable(button)) || (button == HANDSET_Button_STO)) {
        returnToNumberInput(false);
        // button will be processed below
      } else if(button == HANDSET_Button_CLR) {
        SOUND_PlayButtonBeep(button, false);
        returnToNumberInput(false);
        // Prevent short hold of CLR from clearing input
        HANDSET_CancelCurrentButtonHoldEvents();
        return;
      } else if(button == HANDSET_Button_SEND) {
        if ((BT_CallStatus == BT_CALL_IDLE) && (APP_CallAction == APP_CALL_IDLE) && !callFailedTimer) {
          SOUND_PlayButtonBeep(button, false);
          returnToNumberInput(false);
          NumberInput_CallCurrentNumber();
          return;
        } else if ((BT_CallStatus >= BT_CALL_ACTIVE) && (APP_CallAction == APP_CALL_IDLE)) {
          SOUND_PlayButtonBeep(button, false);
          returnToNumberInput(false);
          NumberInput_SendCurrentNumberAsDtmf();
          return;
        }
      }
    } else if (
        (event->type == HANDSET_EventType_BUTTON_HOLD) && 
        (event->holdDuration == HANDSET_HoldDuration_LONG) && 
        ((button == HANDSET_Button_UP) || (button == HANDSET_Button_DOWN))
        ) {
      SOUND_StopButtonBeep();
      appState = (button == HANDSET_Button_UP) ? APP_State_BROWSE_DIRECTORY_UP : APP_State_BROWSE_DIRECTORY_DOWN; 
      TIMEOUT_Cancel(&appStateTimeout);
    }
  }

  if (isButtonDown && ((button == HANDSET_Button_UP) || (button == HANDSET_Button_DOWN))) {
    bool up = (button == HANDSET_Button_UP);
    
    switch (appState) {
      case APP_State_ADJUST_VIEW_ANGLE: {
        bool isViewAngleChanged = false;
        uint8_t lcdViewAngle = STORAGE_GetLcdViewAngle();

        if (up && (lcdViewAngle > 0)) {
          --lcdViewAngle;
          isViewAngleChanged = true;
        } else if (!up && (lcdViewAngle < 7)) {
          ++lcdViewAngle;
          isViewAngleChanged = true;
        }

        if (isViewAngleChanged) {
          SOUND_PlayButtonBeep(button, false);
          HANDSET_SetLcdViewAngle(lcdViewAngle);
          HANDSET_PrintCharAt('0' + (8 - lcdViewAngle), 0);
          STORAGE_SetLcdViewAngle(lcdViewAngle);
        }         
        return;
      }
      
      case APP_State_DISPLAY_DISMISSABLE_TEXT: 
      case APP_State_DISPLAY_BATTERY_LEVEL:  
      case APP_State_DISPLAY_PAIRED_BATTERY_LEVEL:  
      case APP_State_DISPLAY_NUMBER_OVERFLOW:  
      case APP_State_NUMBER_INPUT: 
        if (isRclInputPending && !isRclOrSto2ndDigitPending) {
          SOUND_PlayButtonBeep(button, false);
          
          uint8_t directoryIndex = STORAGE_GetDirectoryIndex();
          if (STORAGE_IsDirectoryEntryEmpty(directoryIndex)) {
            directoryIndex = STORAGE_GetNextPopulatedDirectoryIndex(directoryIndex, up);
          }
          
          recallAddress(directoryIndex, false);
          isRclInputPending = false;
          return;
        }
        // Look out below!
        
      case APP_State_DISPLAY_CREDIT_CARD_NUMBER:  
      case APP_State_RECALL_BT_DEVICE_NAME:  
      case APP_State_RECALL_PAIRED_DEVICE_NAME:  
      case APP_State_INCOMING_CALL:
      {
        VOLUME_Mode volumeMode;

        if (isFcn) {
          TIMEOUT_Cancel(&fcnTimeout);
          resetFcn();
          volumeMode = VOLUME_Mode_SPEAKER;
        } else if ((appState == APP_State_INCOMING_CALL) && (BT_CallStatus == BT_CALL_INCOMING)) {
          volumeMode = VOLUME_Mode_ALERT;
        } else if (BT_CallStatus != BT_CALL_IDLE) {
          volumeMode = event->isOnHook ? VOLUME_Mode_HANDS_FREE : VOLUME_Mode_HANDSET;
        } else if (event->isOnHook || callFailedTimer) {
          volumeMode = VOLUME_Mode_TONE;
        } else {
          volumeMode = VOLUME_Mode_ALERT;
        }

        startVolumeAdjust(volumeMode, up);
      }
      return;
      
      case APP_State_BROWSE_DIRECTORY_IDLE: {
        SOUND_PlayButtonBeep(button, false);
        
        uint8_t index = STORAGE_GetDirectoryIndex();
        if (isDirectoryScanNameMode) {
          index = STORAGE_GetNextNamedDirectoryIndex(index, up);
        } else {
          index = STORAGE_GetNextPopulatedDirectoryIndex(index, up);
        }
        
        recallAddress(index, isDirectoryScanNameMode);
        return;
      }
    }
  }
  
  if (
      (event->type == HANDSET_EventType_BUTTON_HOLD) &&
      (event->holdDuration == HANDSET_HoldDuration_SHORT) &&
      (button >= HANDSET_Button_P1) &&
      (button <= HANDSET_Button_P3) &&
      !callFailedTimer && (BT_CallStatus == BT_CALL_IDLE) && (APP_CallAction == APP_CALL_IDLE)// &&
//      (
//        (appState == APP_State_NUMBER_INPUT) ||
//        (appState == APP_State_DISPLAY_DISMISSABLE_TEXT) ||
//        (appState == APP_State_DISPLAY_BATTERY_LEVEL) ||
//        (appState == APP_State_DISPLAY_PAIRED_BATTERY_LEVEL) ||
//        (appState == APP_State_DISPLAY_NUMBER_OVERFLOW) ||
//        (appState == APP_State_ADJUST_VIEW_ANGLE) || 
//        (appState == APP_State_BROWSE_DIRECTORY_IDLE)
//      )
  ) {
    STORAGE_GetSpeedDial(button - HANDSET_Button_P1, tempNumberBuffer);

    if (tempNumberBuffer[0]) {
      NumberInput_Overwrite(tempNumberBuffer);
      returnToNumberInput(true);
      NumberInput_CallCurrentNumber();
      return;
    }
  }
  
  switch (appState) {
    case APP_State_DISPLAY_NUMBER_OVERFLOW:
      if (recallOverflowType == RECALL_FROM_INPUT_MANUAL) {
        if (isButtonUp && button == HANDSET_Button_RCL) {
          NumberInput_PrintToHandset();
          appState = APP_State_NUMBER_INPUT;
        }
        break;
      } else {
        // Look out below...
      }
      
    case APP_State_NUMBER_INPUT: {
      bool isNumberInput = appState == APP_State_NUMBER_INPUT;
      bool canType = isNumberInput || (recallOverflowType == RECALL_FROM_INPUT_AUTOMATIC);

      switch(event->type) {
        case HANDSET_EventType_BUTTON_DOWN:
          if ((button == HANDSET_Button_FCN) && !isFcn) {
            SOUND_PlayButtonBeep(button, false);
            TIMEOUT_Start(&fcnTimeout, FCN_TIMEOUT);
            setFcn();
          } else if (isFcn && !isExtendedFcn && (button == HANDSET_Button_ASTERISK)) {
            SOUND_PlayDTMFButtonBeep(button, false);
            TIMEOUT_Start(&fcnTimeout, FCN_TIMEOUT);
            // fcnTimer = FCN_TIMEOUT;
            isExtendedFcn = true;
          } else if (isFcn) {
            if (isExtendedFcn) {
              if (button == HANDSET_Button_1) {
                SOUND_PlayDTMFButtonBeep(button, false);
                
                bool isAnnounceBeepEnabled = !STORAGE_GetAnnounceBeepEnabled();
                STORAGE_SetAnnounceBeepEnabled(isAnnounceBeepEnabled);
                
                CALL_TIMER_DisableDisplayUpdate();
                HANDSET_DisableTextDisplay();
                HANDSET_ClearText();
                HANDSET_PrintString("ONE MINBEEP O");
                HANDSET_PrintChar(isAnnounceBeepEnabled ? 'N' : 'F');
                HANDSET_EnableTextDisplay();
                
                appState = APP_State_DISPLAY_DISMISSABLE_TEXT;
              } else if (button == HANDSET_Button_2) {
                SOUND_PlayDTMFButtonBeep(button, false);
                
                bool isStatusBeepEnabled = !STORAGE_GetStatusBeepEnabled();
                STORAGE_SetStatusBeepEnabled(isStatusBeepEnabled);
                
                CALL_TIMER_DisableDisplayUpdate();
                HANDSET_DisableTextDisplay();
                HANDSET_ClearText();
                HANDSET_PrintString("STATUS BEEP O");
                HANDSET_PrintChar(isStatusBeepEnabled ? 'N' : 'F');
                HANDSET_EnableTextDisplay();
                
                appState = APP_State_DISPLAY_DISMISSABLE_TEXT;
              } else if (button == HANDSET_Button_5) {
                SOUND_PlayDTMFButtonBeep(button, false);
                
                displayBatteryLevel(false);
                numberInputIsStale = true;
              } else if (button == HANDSET_Button_7) {
                SOUND_PlayDTMFButtonBeep(button, false);
                
                bool isAutoAnswerEnabled = !STORAGE_GetAutoAnswerEnabled();
                STORAGE_SetAutoAnswerEnabled(isAutoAnswerEnabled);
                
                CALL_TIMER_DisableDisplayUpdate();
                HANDSET_DisableTextDisplay();
                HANDSET_ClearText();
                HANDSET_PrintString("AUTO   ANS ");
                HANDSET_PrintString(isAutoAnswerEnabled ? "ON " : "OFF");
                HANDSET_EnableTextDisplay();
                
                appState = APP_State_DISPLAY_DISMISSABLE_TEXT;
              } else if (button == HANDSET_Button_ASTERISK) {
                SOUND_PlayDTMFButtonBeep(button, false);
                CALL_TIMER_DisableDisplayUpdate();
                
                HANDSET_DisableTextDisplay();
                HANDSET_ClearText();
                HANDSET_PrintString("PAIRING");
                HANDSET_EnableTextDisplay();
                
                numberInputIsStale = true;
                appState = APP_State_PAIRING;
                TIMEOUT_Start(&appStateTimeout, 6000);

                TIMEOUT_Cancel(&linkbackRetryTimeout);
                BT_DisconnectAllProfile();
                BT_EnterPairingMode();
              }
            } else {
              if ((button == HANDSET_Button_0) 
                  && isNumberInput 
                  && numberInputLength 
                  && !NumberInput_HasIncompleteCreditCardMemorySymbol()
                  ) {
                SOUND_PlayDTMFButtonBeep(button, false);
                NumberInput_PushDigit('P');
                HANDSET_PrintChar('P');
              } else if ((button == HANDSET_Button_RCL) 
                  && isNumberInput 
                  && numberInputLength
                  && !NumberInput_HasIncompleteCreditCardMemorySymbol()
                  ) {
                SOUND_PlayButtonBeep(button, false);
                NumberInput_PushDigit('M');
                NumberInput_PushDigit('*');
                HANDSET_PrintChar('M');
                HANDSET_PrintChar('*');
              } else if (button == HANDSET_Button_STO) {
                if (numberInput[0]) {
                  SOUND_PlayButtonBeep(button, false);

                  uint8_t index = STORAGE_GetFirstAvailableDirectoryIndex();

                  if (index == 0xFF) {
                    displayMemoryFullMessage();
                  } else {
                    STORAGE_SetDirectoryNumber(index, numberInput);
                    displayAddressUpdatedMessage(index, numberInput[0]);
                  }
                }
              } else if (button == HANDSET_Button_CLR) {
                if (STORAGE_GetCumulativeTimerResetEnabled() && (BT_CallStatus == BT_CALL_IDLE)) {
                  SOUND_PlayButtonBeep(button, false);
                  startEnterSecurityCode(SecurityAction_CLEAR_TALK_TIME, true);
                }
              } else if (button == HANDSET_Button_MUTE) {
                if (BT_CallStatus != BT_CALL_IDLE) {
                  SOUND_PlayButtonBeep(button, false);
                  SOUND_SetButtonsMuted(!SOUND_IsButtonsMuted());
                  
                  CALL_TIMER_DisableDisplayUpdate();
                  HANDSET_DisableTextDisplay();
                  HANDSET_ClearText();
                  HANDSET_PrintString("SILENT  SP ");
                  HANDSET_PrintString(SOUND_IsButtonsMuted() ? "ON " : "OFF");
                  HANDSET_EnableTextDisplay();
                  appState = APP_State_DISPLAY_DISMISSABLE_TEXT;
                }
              } else if (button == HANDSET_Button_SEND) {
                if (BT_CallStatus >= BT_CALL_ACTIVE) {
                  NumberInput_SendCurrentNumberAsDtmf();
                }
              } else if (button == HANDSET_Button_END) {
                if (BT_CallStatus > BT_CALL_ACTIVE) {
                  SOUND_PlayButtonBeep(button, false);
                  BT_EndHoldOrWaitingCall();
                  APP_CallAction = APP_CALL_ENDING;
                }
              } else if (button == HANDSET_Button_1) {
                SOUND_PlayDTMFButtonBeep(button, false);
                selectSystemMode(STORAGE_GetVehicleModeEnabled());
              } else if (button == HANDSET_Button_3) {
                SOUND_PlayDTMFButtonBeep(button, false);
                CALL_TIMER_DisableDisplayUpdate();
                RINGTONE_SELECT_Start(handleReturnFromSubModule);
                appState = APP_State_SELECT_RINGTONE;
              } else if (button == HANDSET_Button_4) {
                if (BT_CallStatus == BT_CALL_IDLE) {
                  SOUND_PlayDTMFButtonBeep(button, false);

                  HANDSET_DisableTextDisplay();
                  HANDSET_ClearText();

                  HANDSET_PrintString("L");
                  HANDSET_PrintString(uint2str(
                      tempNumberBuffer,
                      STORAGE_GetLastCallMinutes(),
                      3,
                      2
                      ));
                  HANDSET_PrintChar(' ');
                  HANDSET_PrintString(uint2str(
                      tempNumberBuffer,
                      STORAGE_GetLastCallSeconds(),
                      2,
                      2
                      ));

                  HANDSET_PrintString("A ");
                  HANDSET_PrintString(uint2str(
                      tempNumberBuffer,
                      STORAGE_GetAccumulatedCallMinutes(),
                      5,
                      5
                      ));

                  HANDSET_EnableTextDisplay();
                  startEnterSecurityCode(SecurityAction_DISPLAY_TOTAL_TALK_TIME, false);
                }
              } else if (button == HANDSET_Button_8) {
                SOUND_PlayDTMFButtonBeep(button, false);
                startNameInput(NAME_INPUT_Directory, true);
              } else if (button == HANDSET_Button_9) {
                SOUND_PlayDTMFButtonBeep(button, false);
                startAlphaScan(false);
              } else if (button == HANDSET_Button_P1) {
                SOUND_PlayButtonBeep(button, false);
                CALL_TIMER_DisableDisplayUpdate();
                SNAKE_GAME_Start(handleReturnFromSubModule);
                appState = APP_State_SNAKE_GAME;
              } else if (button == HANDSET_Button_P2) {
                SOUND_PlayButtonBeep(button, false);
                CALL_TIMER_DisableDisplayUpdate();
                MEMORY_GAME_Start(handleReturnFromSubModule);
                appState = APP_State_MEMORY_GAME;
              } else if (button == HANDSET_Button_P3) {
                SOUND_PlayButtonBeep(button, false);
                CALL_TIMER_DisableDisplayUpdate();
                TETRIS_GAME_Start(handleReturnFromSubModule);
                appState = APP_State_TETRIS_GAME;
              }
            }
            
            TIMEOUT_Cancel(&fcnTimeout);
            resetFcn();
            return;
          } else if (button == HANDSET_Button_RCL) {
            SOUND_PlayButtonBeep(button, false);
            
            if (isRclInputPending) {
              recallLastDialedNumber();
              isRclInputPending = false;
            } else {
              isRclInputPending = true;
            }
            isStoInputPending = false;
            isRclOrSto2ndDigitPending = false;
          } else if (button == HANDSET_Button_STO) {
            SOUND_PlayButtonBeep(button, false);
            isStoInputPending = true;
            isRclOrSto2ndDigitPending = false;
            isRclInputPending = false;
            rcl2ndDigitTimer = 0;
          } else if (isStoInputPending && isRclOrSto2ndDigitPending) {
            if (HANDSET_IsButtonNumeric(button)) {
              if (rclOrStoAddr == 0xFF) {
                if (isValidCreditCardMemoryButton(button)) {
                  SOUND_PlayDTMFButtonBeep(button, false);
                  creditCardIndex = getCreditCardMemoryIndexFromButton(button);
                  startEnterSecurityCode(SecurityAction_STO_CARD_NUMBER, true);
                }
              } else {
                rclOrStoAddr = rclOrStoAddr * 10 + button - '0' - 1;

                if (rclOrStoAddr < DIRECTORY_SIZE) {
                  SOUND_PlayDTMFButtonBeep(button, false);
                  STORAGE_SetDirectoryNumber(rclOrStoAddr, numberInput);
                  displayAddressUpdatedMessage(rclOrStoAddr, numberInput[0]);
                }
              }
            }
            
            isStoInputPending = false;
            isRclOrSto2ndDigitPending = false;
          } else if (isStoInputPending) {
            if(button >= HANDSET_Button_P1 && button <= HANDSET_Button_P3) {
              SOUND_PlayButtonBeep(button, false);
              uint8_t index = button - HANDSET_Button_P1;
              STORAGE_SetSpeedDial(index, numberInput);
              
              CALL_TIMER_DisableDisplayUpdate();
              HANDSET_DisableTextDisplay();
              HANDSET_ClearText();
              HANDSET_PrintString(numberInput[0] ? "STORED" : "ERASED");
              HANDSET_PrintString("      P");
              HANDSET_PrintChar('1' + index);
              HANDSET_EnableTextDisplay();
              numberInputIsStale = true;
              isStoInputPending = false;
              appState = APP_State_DISPLAY_DISMISSABLE_TEXT;
            } else if ((button == HANDSET_Button_ASTERISK) && !strbrk(numberInput, "PM")) {
              SOUND_PlayDTMFButtonBeep(button, false);
              rclOrStoAddr = 0xFF;
              isRclOrSto2ndDigitPending = true;
            } else if (HANDSET_IsButtonNumeric(button) && (button < '3')) {
              SOUND_PlayDTMFButtonBeep(button, false);
              rclOrStoAddr = button - '0';
              isRclOrSto2ndDigitPending = true;
            } else {
              isStoInputPending = false;
            }
          } else if (isRclInputPending && isRclOrSto2ndDigitPending) {
            if (HANDSET_IsButtonNumeric(button)) {
              if (rclOrStoAddr == 0xFF) {
                if (isValidCreditCardMemoryButton(button)) {
                  SOUND_PlayDTMFButtonBeep(button, false);
                  creditCardIndex = getCreditCardMemoryIndexFromButton(button);
                  startEnterSecurityCode(SecurityAction_RCL_CARD_NUMBER, true);
                }
              } else {
                rclOrStoAddr = (rclOrStoAddr * 10 + button - '0') - 1;

                if (rclOrStoAddr < DIRECTORY_SIZE) {
                  SOUND_PlayDTMFButtonBeep(button, false);
                  recallAddress(rclOrStoAddr, false);
                }
              }
            } else if ((button == HANDSET_Button_ASTERISK) && (rclOrStoAddr == 0xFF)) {
              SOUND_PlayDTMFButtonBeep(button, false);
              recallBtDeviceName();
            } else if ((button == HANDSET_Button_POUND) && (rclOrStoAddr == 0xFF)) {
              SOUND_PlayDTMFButtonBeep(button, false);
              recallPairedDeviceName(false);
            }
            
            isRclInputPending = false;
            isRclOrSto2ndDigitPending = false;
            rcl2ndDigitTimer = 0;
          } else if (isRclInputPending) {
            if (button == HANDSET_Button_POUND) {
              SOUND_PlayDTMFButtonBeep(button, false);

              char ownNumber[STANDARD_PHONE_NUMBER_LENGTH + 1];
              STORAGE_GetOwnNumber(STORAGE_GetActiveNumberIndex(), ownNumber);
              
              numberInputIsStale = true;
              CALL_TIMER_DisableDisplayUpdate();
              HANDSET_DisableTextDisplay();
              HANDSET_ClearText();
              HANDSET_PrintString(ownNumber[0] ? ownNumber : "EMPTY");
              HANDSET_PrintStringAt("TEL ", 13);
              HANDSET_EnableTextDisplay();

              if (STORAGE_GetDualNumberEnabled()) {
                startEnterSecurityCode(SecurityAction_TOGGLE_DUAL_NUMBER, false);
              } else {
                appState = APP_State_DISPLAY_DISMISSABLE_TEXT;
              }

              isRclInputPending = false;
            } else if (button == HANDSET_Button_ASTERISK) {
              SOUND_PlayDTMFButtonBeep(button, false);
              rclOrStoAddr = 0xFF;
              isRclOrSto2ndDigitPending = true;
            } else if(button >= HANDSET_Button_P1 && button <= HANDSET_Button_P3){
              SOUND_PlayButtonBeep(button, false);
              recallSpeedDial(button - HANDSET_Button_P1);
              isRclInputPending = false;
            } else if (HANDSET_IsButtonNumeric(button)) {
              SOUND_PlayDTMFButtonBeep(button, false);
              rclOrStoAddr = button - '0';
              isRclOrSto2ndDigitPending = true;
              rcl2ndDigitTimer = rclOrStoAddr ? RCL_2ND_DIGIT_TIMEOUT : 0;
            } else {
              isRclInputPending = false;
            }
          } else if (button == HANDSET_Button_MUTE) {
            if (BT_CallStatus != BT_CALL_IDLE) {
              SOUND_PlayButtonBeep(button, false);
              isMuted = !isMuted;

              BT_SetMicrohponeMuted(isMuted);
              HANDSET_SetIndicator(HANDSET_Indicator_MUTE, isMuted);
            }
          } else if ((button == HANDSET_Button_CLR) && isNumberInput && numberInputLength) {
            SOUND_PlayButtonBeep(button, false);
            NumberInput_PopDigit();
            NumberInput_PrintToHandset();
            isRclInputPending = false;
          } else if (canType && HANDSET_IsButtonPrintable(button)) {
            if (NumberInput_HasIncompleteCreditCardMemorySymbol()) {
              if (!isValidCreditCardMemoryButton(button)) {
                // Only allow valid credit card memory numbers
                break;
              }
            }
            
            if (!SOUND_IsButtonsMuted()) {
              SOUND_PlayDTMFButtonBeep(button, false);
              
              if ((BT_CallStatus >= BT_CALL_ACTIVE) && (APP_CallAction == APP_CALL_IDLE)) {
                sendDtmfDigit(button);
              }
            }

            if (!isNumberInput) {
              appState = APP_State_NUMBER_INPUT;
            }
            
            if (numberInputIsStale) {
              NumberInput_Clear();
              HANDSET_ClearText();
            }
            
            NumberInput_PushDigit(button);
            
            if (numberInputLength == 1) {
              CALL_TIMER_DisableDisplayUpdate();
              HANDSET_ClearText();
            } else if (wasDisplayingNonNumberInput) {
              NumberInput_PrintToHandset();
            }
            
            HANDSET_PrintChar(button);
          } else if (button == HANDSET_Button_SEND) {
            if ((BT_CallStatus >= BT_CALL_ACTIVE) && (APP_CallAction == APP_CALL_IDLE)) {
              if (dialedNumberNextDigitsToDial) {
                bool isCreditCardRecall = false;
                while(*dialedNumberNextDigitsToDial) {
                  if (*dialedNumberNextDigitsToDial == 'P') {
                    ++dialedNumberNextDigitsToDial;
                    break;
                  } else if (*dialedNumberNextDigitsToDial == 'M') {
                    ++dialedNumberNextDigitsToDial;
                    *dialedNumberNextDigitsToDial = '^';
                    break;
                  } else if (*dialedNumberNextDigitsToDial == '^') {
                    isCreditCardRecall = true;
                    ++dialedNumberNextDigitsToDial;
                  } else if (isCreditCardRecall) {
                    char creditCardNumber[CREDIT_CARD_LENGTH + 1];
                    STORAGE_GetCreditCardNumber(*dialedNumberNextDigitsToDial++ - '0', creditCardNumber);
                    sendDtmfString(creditCardNumber);
                    isCreditCardRecall = false;
                  } else {
                    sendDtmfDigit(*dialedNumberNextDigitsToDial++);
                  }
                }
                
                if (!*dialedNumberNextDigitsToDial) {
                  dialedNumberNextDigitsToDial = NULL;
                }
                return;
              }
            } else if (!callFailedTimer && (BT_CallStatus == BT_CALL_IDLE) && (APP_CallAction == APP_CALL_IDLE) && numberInputLength) {
              SOUND_PlayButtonBeep(button, false);
              uint8_t memoryIndex = NumberInput_GetMemoryIndex();
              
              if (memoryIndex != 0xFF) {
                STORAGE_GetDirectoryNumber(memoryIndex, tempNumberBuffer);
                
                if (tempNumberBuffer[0]) {
                  NumberInput_Overwrite(tempNumberBuffer);
                  returnToNumberInput(true);
                  NumberInput_CallCurrentNumber();
                } else {
                  displayEmptyMessage();
                }
              } else {
                NumberInput_CallCurrentNumber();
              }
              return;
            }
          }
          break;

        case HANDSET_EventType_BUTTON_HOLD:
          if ((button == HANDSET_Button_CLR) && (event->holdDuration == HANDSET_HoldDuration_SHORT) && canType && numberInputLength) {
            SOUND_StopButtonBeep();
            NumberInput_Clear();
            NumberInput_PrintToHandset();
          } else if((button == HANDSET_Button_RCL) && (event->holdDuration == HANDSET_HoldDuration_LONG) && isRclInputPending) {
            isRclInputPending = false;
            recallNumberInputOverflow(RECALL_FROM_INPUT_MANUAL);
          } else if ((button == HANDSET_Button_FCN) && (event->holdDuration == HANDSET_HoldDuration_VERY_LONG) && isFcn) {
            SOUND_StopButtonBeep();
            numberInputIsStale = true;
            TIMEOUT_Cancel(&fcnTimeout);
            resetFcn();
            CALL_TIMER_DisableDisplayUpdate();
            HANDSET_DisableTextDisplay();
            HANDSET_ClearText();
            HANDSET_PrintString("VIEW   ANGLE ");
            HANDSET_PrintChar('0' + (8 - STORAGE_GetLcdViewAngle()));
            HANDSET_EnableTextDisplay();
            appState = APP_State_ADJUST_VIEW_ANGLE;
          }
          break; 
      }
      break;
    }
    
    case APP_State_SELECT_SYSTEM_MODE: 
      if (isButtonDown) {
        if (button == HANDSET_Button_CLR) {
          SOUND_PlayButtonBeep(button, false);
          returnToNumberInput(false);
          // Prevent short hold of CLR from clearing input
          HANDSET_CancelCurrentButtonHoldEvents();
        } else if (button == HANDSET_Button_1) {
          SOUND_PlayDTMFButtonBeep(button, false);
          selectSystemMode(false);
        } else if (button == HANDSET_Button_2) {
          SOUND_PlayDTMFButtonBeep(button, false);
          selectSystemMode(true);
        }
      }
      break;
    
    case APP_State_PAIRING:
      if (isButtonDown && (button == HANDSET_Button_CLR)) {
        SOUND_PlayButtonBeep(button, false);
        BT_ExitPairingMode();
        BT_LinkBackToLastDevice();
        INDICATOR_StopSignalStrengthSweep(0);
        INDICATOR_StartFlashing(HANDSET_Indicator_NO_SVC);
        returnToNumberInput(false);
        // Prevent short hold of CLR from clearing input
        HANDSET_CancelCurrentButtonHoldEvents();
      }
      break;
      
    case APP_State_INCOMING_CALL:
      if (isButtonDown) {
        if ((button == HANDSET_Button_FCN) && (BT_CallStatus == BT_CALL_ACTIVE_WITH_CALL_WAITING)) {
          SOUND_PlayButtonBeep(button, false);

          if (isFcn) {
            resetFcn();
          } else {
            setFcn();
          }
        } else if ((button == HANDSET_Button_SEND) && (APP_CallAction == APP_CALL_IDLE)) {
          SOUND_PlayButtonBeep(button, false);
          
          if (BT_CallStatus == BT_CALL_INCOMING) {
            TIMEOUT_Cancel(&autoAnswerTimeout);
            BT_AcceptCall();
          } else {
            BT_SwapHoldOrWaitingCall();
          }
          APP_CallAction = APP_CALL_ACCEPTING;
          return;
        } else if ((button == HANDSET_Button_END) && (APP_CallAction == APP_CALL_IDLE)) {
          if (isFcn) {
            if (BT_CallStatus == BT_CALL_ACTIVE_WITH_CALL_WAITING) {
              SOUND_PlayButtonBeep(button, false);
              BT_EndHoldOrWaitingCall();
              APP_CallAction = APP_CALL_REJECTING;
            }
          } else {
            SOUND_PlayButtonBeep(button, false);

            if (BT_CallStatus == BT_CALL_INCOMING) {
              TIMEOUT_Cancel(&autoAnswerTimeout);
              BT_RejectCall();
            } else  {
              BT_SwapHoldOrWaitingCallAndEndActiveCall();
            }
            APP_CallAction = APP_CALL_REJECTING;
          }
          return;
        }
      } else if ((event->type == HANDSET_EventType_HOOK) && !event->isOnHook && (BT_CallStatus == BT_CALL_INCOMING)) {
        TIMEOUT_Cancel(&autoAnswerTimeout);
        BT_AcceptCall();
        APP_CallAction = APP_CALL_ACCEPTING;
      }
      break;
      
    case APP_State_ALPHA_STO_NAME_INPUT:
      if (isButtonDown) {
        if (button == HANDSET_Button_FCN) {
          if (numberInputLength || (nameInputReason != NAME_INPUT_Directory)) {
            SOUND_PlayButtonBeep(button, false);
            isAlphaInput = !isAlphaInput;
            
            if (isAlphaInput) {
              INDICATOR_StartFlashing(HANDSET_Indicator_FCN);
            } else {
              INDICATOR_StopFlashing(HANDSET_Indicator_FCN, false);
            }
          }
        } else if (button == HANDSET_Button_STO)  {
          if (numberInputLength) {
            SOUND_PlayButtonBeep(button, false);
            strcpy(alphaInput, numberInput);
            
            switch (nameInputReason) {
              case NAME_INPUT_Directory:
                startAlphaStoreNumberInput(true);
                break;
              
              case NAME_INPUT_BluetoothName:
                setBluetoothName(alphaInput);
                break;
            }
          }
        } else if (button == HANDSET_Button_CLR) {
          SOUND_PlayButtonBeep(button, false);
          
          if (!numberInputLength) {
            HANDSET_SetIndicator(HANDSET_Indicator_FCN, false);
            returnToNumberInput(false);
            // Prevent short hold of CLR from clearing input
            HANDSET_CancelCurrentButtonHoldEvents();
          } else if (numberInputLength == 1) {
            startNameInput(nameInputReason, true);
            HANDSET_CancelCurrentButtonHoldEvents();
         } else {
            NumberInput_PopDigit();
            HANDSET_DisableTextDisplay();
            NumberInput_PrintToHandset();
            HANDSET_PrintChar('_');
            HANDSET_EnableTextDisplay();
          }
        } else if (numberInputLength < nameInputMaxLength) {
          if (isAlphaInput) {
            if ((button >= '1') && (button <= '9')) {
              SOUND_PlayDTMFButtonBeep(button, false);

              if (button != alphaButton) {
                alphaButton = button;
                alphaCharIndex = 0;
              }

              uint8_t const c = alphaLookup[button - '1'][alphaCharIndex];

              if (isAlphaPromptDisplayed) {
                HANDSET_ClearText();
                HANDSET_PrintChar(c);
                isAlphaPromptDisplayed = false;
              } else {
                HANDSET_PrintCharAt(c, 0);
              }
            }
          } else {
            if (HANDSET_IsButtonPrintable(button)) {
              SOUND_PlayDTMFButtonBeep(button, false);
              NumberInput_PushDigit(button);

              if (isAlphaPromptDisplayed) {
                HANDSET_ClearText();
                HANDSET_PrintChar(button);
                isAlphaPromptDisplayed = false;
              } else {
                HANDSET_PrintCharAt(button, 0);
              }
            }
          }
        }
      } else if (isButtonUp) {
        if (isAlphaInput && (button >= '1') && (button <= '9')) {
          if (isAlphaCharAccepted) {
            alphaCharIndex = 0;
            isAlphaCharAccepted = false;
          } else if (!isAlphaPromptDisplayed && (numberInputLength < nameInputMaxLength)) {
            if (++alphaCharIndex == 6) {
              alphaCharIndex = 0;
            }
            HANDSET_PrintCharAt('_', 0);
          }
        } else if (!isAlphaInput && (numberInputLength < nameInputMaxLength) && HANDSET_IsButtonPrintable(button)) {
          HANDSET_PrintChar('_');
        }
      } else if ((event->type == HANDSET_EventType_BUTTON_HOLD) && (event->holdDuration == HANDSET_HoldDuration_SHORT)) {
        if (isAlphaInput && (numberInputLength < nameInputMaxLength) && (button >= '1') && (button <= '9')) {
          SOUND_PlayEffect(SOUND_Channel_FOREGROUND, SOUND_Target_SPEAKER, VOLUME_Mode_SPEAKER, SOUND_Effect_REORDER_TONE, false);
          isAlphaCharAccepted = true;
          NumberInput_PushDigit(alphaLookup[button - '1'][alphaCharIndex]);
          
          if (numberInputLength < nameInputMaxLength) {
            HANDSET_PrintChar('_');
          }
        } else if (button == HANDSET_Button_CLR) {
          startNameInput(nameInputReason, true);
        }
      }
      break;
      
    case APP_State_ALPHA_STO_NUMBER_INPUT:
      if (isButtonDown) {
        if (button == HANDSET_Button_CLR) {
          if (isFcnInputPendingForAlphaSto || isStoInputPending) {
            SOUND_PlayButtonBeep(button, false);
            returnToNumberInput(false);
            // Prevent short hold of CLR from clearing input
            HANDSET_CancelCurrentButtonHoldEvents();
            isStoInputPending = false;
            isRclOrSto2ndDigitPending = false;
            isFcnInputPendingForAlphaSto = false;
          } else {
            SOUND_PlayButtonBeep(button, false);

            if (numberInputLength) {
              if (numberInputLength == 1) {
                startAlphaStoreNumberInput(true);
              } else {
                NumberInput_PopDigit();
                NumberInput_PrintToHandset();
              }
            } else {
              NumberInput_Overwrite(alphaInput);
              startNameInput(nameInputReason, false);
              // Prevent short hold of CLR from clearing input
              HANDSET_CancelCurrentButtonHoldEvents();
            }
          }
        } if (isFcnInputPendingForAlphaSto) {
          if (button == HANDSET_Button_0) {
            SOUND_PlayDTMFButtonBeep(button, false);
            NumberInput_PushDigit('P');
            HANDSET_PrintChar('P');
            isFcnInputPendingForAlphaSto = false;
          } else if (button == HANDSET_Button_RCL) {
            SOUND_PlayButtonBeep(button, false);
            NumberInput_PushDigit('M');
            NumberInput_PushDigit('*');
            HANDSET_PrintChar('M');
            HANDSET_PrintChar('*');
            isFcnInputPendingForAlphaSto = false;
          } else if (button == HANDSET_Button_STO) {
            SOUND_PlayButtonBeep(button, false);

            uint8_t index = STORAGE_GetFirstAvailableDirectoryIndex();

            if (index == 0xFF) {
              displayMemoryFullMessage();
            } else {
              STORAGE_SetDirectoryEntry(index, numberInput, alphaInput);
              displayAddressUpdatedMessage(index, true);
            }
            isFcnInputPendingForAlphaSto = false;
          }
        } else if (isStoInputPending) {
          if (HANDSET_IsButtonNumeric(button)) {
            if (isRclOrSto2ndDigitPending) {
              rclOrStoAddr = rclOrStoAddr * 10 + button - '0' - 1;

              if (rclOrStoAddr < DIRECTORY_SIZE) {
                SOUND_PlayDTMFButtonBeep(button, false);
                STORAGE_SetDirectoryEntry(rclOrStoAddr, numberInput, alphaInput);
                displayAddressUpdatedMessage(rclOrStoAddr, true);
              } else {
                appState = APP_State_NUMBER_INPUT;
                numberInputIsStale = true;
              }
              isRclOrSto2ndDigitPending = true;
              isStoInputPending = false;
            } else {
              SOUND_PlayDTMFButtonBeep(button, false);
              rclOrStoAddr = button - '0';
              isRclOrSto2ndDigitPending = true;
            }
          }
        } else if (button == HANDSET_Button_FCN) {
          if (numberInputLength && !isFcnInputPendingForAlphaSto){
            SOUND_PlayButtonBeep(button, false);
            isFcnInputPendingForAlphaSto = true;
          }
        } else if (button == HANDSET_Button_STO) {
          if (numberInputLength) {
            SOUND_PlayButtonBeep(button, false);
            isStoInputPending = true;
          }
        } else if (HANDSET_IsButtonPrintable(button)) {
          SOUND_PlayDTMFButtonBeep(button, false);
          
          if (isAlphaPromptDisplayed) {
            HANDSET_ClearText();
            isAlphaPromptDisplayed = false;
          }
          
          NumberInput_PushDigit(button);
          HANDSET_PrintChar(button);
        }
      } else if (
          (event->type == HANDSET_EventType_BUTTON_HOLD) && 
          (event->holdDuration == HANDSET_HoldDuration_SHORT) &&
          (button == HANDSET_Button_CLR)
          ) {
        startAlphaStoreNumberInput(true);
      }
      break;
      
    case APP_State_BROWSE_DIRECTORY_UP:
    case APP_State_BROWSE_DIRECTORY_DOWN:
      if (isButtonUp && ((button == HANDSET_Button_UP) || (button == HANDSET_Button_DOWN))) {
        appState = APP_State_BROWSE_DIRECTORY_IDLE;
      }
      break;
      
    case APP_State_BROWSE_DIRECTORY_SHOW_OVERFLOW: 
      if (isButtonUp && (button == HANDSET_Button_RCL)) {
        recallAddress(STORAGE_GetDirectoryIndex(), isDirectoryScanNameMode);
      }
      break;
      
    case APP_State_BROWSE_DIRECTORY_IDLE:
      if (isButtonDown) {
        if (button == HANDSET_Button_FCN) {
          uint8_t const index = STORAGE_GetDirectoryIndex();

          if (isDirectoryScanNameMode || !STORAGE_IsDirectoryNameEmpty(index)) {
            SOUND_PlayButtonBeep(button, false);
            recallAddress(index, !isDirectoryScanNameMode);
          }
        } else if (button == HANDSET_Button_RCL) {
          if (isDirectoryScanNameMode && strlen(alphaInput) > 11) {
            SOUND_PlayButtonBeep(button, false);
            HANDSET_DisableTextDisplay();
            HANDSET_ClearText();
            HANDSET_PrintStringAt(alphaInput + 11, 13);
            HANDSET_EnableTextDisplay();
            
            appState = APP_State_BROWSE_DIRECTORY_SHOW_OVERFLOW;
          } else if (!isDirectoryScanNameMode && strlen(numberInput) > 11) {
            SOUND_PlayButtonBeep(button, false);
            HANDSET_DisableTextDisplay();
            HANDSET_ClearText();
            HANDSET_PrintStringN(numberInput, strlen(numberInput) - 11);
            HANDSET_EnableTextDisplay();
            
            appState = APP_State_BROWSE_DIRECTORY_SHOW_OVERFLOW;
          }
        }
      }
      break;

    case APP_State_ALPHA_SCAN:
      if (isButtonDown) {
        if (button == HANDSET_Button_CLR) {
          SOUND_PlayButtonBeep(button, false);
          HANDSET_SetIndicator(HANDSET_Indicator_FCN, false);
          returnToNumberInput(false);
          // Prevent short hold of CLR from clearing input
          HANDSET_CancelCurrentButtonHoldEvents();
        } else if (button >= '1' && button <= '9') {
          SOUND_PlayDTMFButtonBeep(button, false);

          if (button != alphaButton) {
            alphaButton = button;
            alphaCharIndex = 0;
          }

          uint8_t const c = alphaLookup[button - '1'][alphaCharIndex];

          if (isAlphaPromptDisplayed) {
            HANDSET_ClearText();
            HANDSET_PrintChar(c);
            isAlphaPromptDisplayed = false;
          } else {
            HANDSET_PrintCharAt(c, 0);
          }
        }
      } else if (isButtonUp) {
        if (!isAlphaPromptDisplayed && (button >= '1' && button <= '9')) {
          HANDSET_PrintCharAt('_', 0);
          
          if (++alphaCharIndex == 3) {
            alphaCharIndex = 0;
          }
        }
      } else if (
          (event->type == HANDSET_EventType_BUTTON_HOLD) &&
          (event->holdDuration == HANDSET_HoldDuration_SHORT) &&
          (button >= '1' && button <= '9')
        ) {
        SOUND_PlayEffect(SOUND_Channel_FOREGROUND, SOUND_Target_SPEAKER, VOLUME_Mode_SPEAKER, SOUND_Effect_REORDER_TONE, false);
        HANDSET_SetIndicator(HANDSET_Indicator_FCN, false);
        HANDSET_CancelCurrentButtonHoldEvents();
        recallAddress(STORAGE_GetFirstNamedDirectoryIndexForLetter(alphaLookup[button - '1'][alphaCharIndex]), true);
      }
    
    case APP_State_DISPLAY_CREDIT_CARD_NUMBER:
      if (isButtonDown){
        if (button == HANDSET_Button_RCL) {
          showCreditCardNumberOverflow();
        } else if (button == HANDSET_Button_SEND) {
          if ((BT_CallStatus == BT_CALL_IDLE) && (APP_CallAction == APP_CALL_IDLE) && !callFailedTimer) {
            SOUND_PlayButtonBeep(button, false);
            STORAGE_GetCreditCardNumber(creditCardIndex, tempNumberBuffer);
            NumberInput_Overwrite(tempNumberBuffer);
            NumberInput_CallCurrentNumber();
            return;
          } else if ((BT_CallStatus >= BT_CALL_ACTIVE) && (APP_CallAction == APP_CALL_IDLE)) {
            SOUND_PlayButtonBeep(button, false);
            STORAGE_GetCreditCardNumber(creditCardIndex, tempNumberBuffer);
            sendDtmfString(tempNumberBuffer);
            returnToNumberInput(false);
            return;
          }
        }
      }
      break;
      
    case APP_State_DISPLAY_CREDIT_CARD_NUMBER_SHOW_OVERFLOW:
      if (isButtonUp && (button == HANDSET_Button_RCL)) {
        recallCreditCardNumber(creditCardIndex);
      }
      break;
      
    case APP_State_RECALL_PAIRED_DEVICE_NAME:
      if (isButtonDown) {
        if (button == HANDSET_Button_STO) {
          SOUND_PlayButtonBeep(button, false);
          startEnterSecurityCode(SecurityAction_RESET_PAIRED_DEVICES, true);
        }
      }
      break;
      
    case APP_State_RECALL_BT_DEVICE_NAME:
      if (isButtonDown) {
        if (button == HANDSET_Button_STO) {
          SOUND_PlayButtonBeep(button, false);
          startEnterSecurityCode(SecurityAction_SET_BT_DEVICE_NAME, true);
        }
      }
      break;
      
    case APP_State_DISPLAY_BATTERY_LEVEL:  
    case APP_State_DISPLAY_PAIRED_BATTERY_LEVEL:
      if (isButtonDown) {
        if (button == HANDSET_Button_FCN) {
          SOUND_PlayButtonBeep(button, false);
          displayBatteryLevel(appState == APP_State_DISPLAY_BATTERY_LEVEL);
        }
      }
      break;
  }

  if (isButtonDown && (button == HANDSET_Button_END) && !isFcn) {
    if (callFailedTimer) {
      SOUND_PlayButtonBeep(button, false);
      HANDSET_SetIndicator(HANDSET_Indicator_IN_USE, false);
      callFailedTimer = 0;
      callFailedTimerExpired = false;
      SOUND_Stop(SOUND_Channel_BACKGROUND);
      numberInputIsStale = true;
      returnToNumberInput(false);
    } else if ((BT_CallStatus != BT_CALL_IDLE) && (APP_CallAction == APP_CALL_IDLE)) {
      SOUND_PlayButtonBeep(button, false);
      
      if (BT_CallStatus > BT_CALL_ACTIVE) {
        BT_SwapHoldOrWaitingCallAndEndActiveCall();
      } else {
        BT_EndCall();
      }
      APP_CallAction = APP_CALL_ENDING;
    }
  }
  
  if (isButtonDown && (button == HANDSET_Button_SEND)) {
    if ((BT_CallStatus >= BT_CALL_ACTIVE) && (APP_CallAction == APP_CALL_IDLE)) {
      SOUND_PlayButtonBeep(button, false);
      BT_SwapHoldOrWaitingCall();
    }
  }
}

void handle_TRANSCEIVER_Event(TRANSCEIVER_EventType event) {
  switch (event)  {
    case TRANSCEIVER_EventType_BATTERY_LEVEL_CHANGED:
      if (appState == APP_State_DISPLAY_BATTERY_LEVEL) {
        displayBatteryLevel(false);
      }
      reportBatteryLevelToBT();
      break;
      
    case TRANSCEIVER_EventType_BATTERY_LEVEL_IS_LOW:
      INDICATOR_StartFlashing(HANDSET_Indicator_PWR);
      INTERVAL_Start(&lowBatteryBeepInterval, true);
      
      if ((appState == APP_State_NUMBER_INPUT) && (numberInputLength == 0)) {
        NumberInput_PrintToHandset();
      }
      reportBatteryLevelToBT();
      break;

    case TRANSCEIVER_EventType_BATTERY_LEVEL_IS_OK:
      INDICATOR_StopFlashing(HANDSET_Indicator_PWR, true);
      INTERVAL_Cancel(&lowBatteryBeepInterval);

      if ((appState == APP_State_NUMBER_INPUT) && (numberInputLength == 0)) {
        NumberInput_PrintToHandset();
      }
      reportBatteryLevelToBT();
      break;
  }
}

static char const* linkbackTypeLabel[] = {
  "ACL",
  "HFP",
  "A2DP",
  "SPP"
};

static char const* atResponseStatusLabel[] = {
  "OK",
  "Error",
  "No Response"
};

void APP_BT_EventHandler(uint8_t event, uint16_t para, uint8_t* para_full) {
  switch (event) {
    case BT_EVENT_SYS_POWER_ON: 
      BT_isReady = true;
      break;
      
    case BT_EVENT_CMD_SENT_NO_ACK:
      printf("[NO_ACK] cmd=%X\r\n", para);
      BT_GiveUpThisCommand();
      // Look out below!

    case BT_EVENT_CMD_SENT_ACK_ERROR:
      if (event == BT_EVENT_CMD_SENT_ACK_ERROR) {
        printf("[ACK ERROR] cmd=%X; status=%X\r\n", para, (int)para_full[1]);
      }
      
      if ((appState == APP_State_PROGRAMMING) || (appState == APP_State_REBOOT)) {
        // Ignore BT errors, because we specifically turn BT off during 
        // programming, all BT commands are expected to fail.
        printf("[ACK ERROR] Ignoring during %s\r\n", appStateLabel[appState]);
        return;
      }
      
      switch(para) {
        case PROFILE_LINK_BACK:
          printf("[LINKBACK] Error\r\n");
          // Error is likely due to no paired device yet.
          // Ignore.
          break;
        
        case READ_LINKED_DEVICE_INFOR:
          if (appState == APP_State_RECALL_PAIRED_DEVICE_NAME) {
            MARQUEE_Start("[error]", MARQUEE_Row_BOTTOM);
          }
          break;
          
        case MAKE_CALL:
          APP_CallAction = APP_CALL_IDLE;
          startCallFailed();
          break;
          
        default:
          INDICATOR_StartFlashing(HANDSET_Indicator_HORN);
      }
      break;
   
    case BT_EVENT_LINKBACK_SUCCESS: 
      printf("[LINKBACK] Success: %s\r\n", linkbackTypeLabel[para]);
      break;
      
    case BT_EVENT_LINKBACK_FAILED: 
      printf("[LINKBACK] Failed: %s\r\n", linkbackTypeLabel[para]);

      //if (para == LINKBACK_Type_ACL) {
      BT_DisconnectAllProfile();
      TIMEOUT_Start(&linkbackRetryTimeout, LINKBACK_RETRY_TIMEOUT);
      //}
      break;
      
    case BT_EVENT_SYS_PAIRING_START:
      printf("[PAIR] Start\r\n");
      INDICATOR_StartSignalStrengthSweep();
      INDICATOR_StopFlashing(HANDSET_Indicator_NO_SVC, false);
      break;

    case BT_EVENT_SYS_PAIRING_FAILED:
      printf("[PAIR] Failed\r\n");
      INDICATOR_StartFlashing(HANDSET_Indicator_NO_SVC);
      TIMEOUT_Start(&idleTimeout, IDLE_TIMEOUT);
      INDICATOR_StopSignalStrengthSweep(0);
      
      if (appState == APP_State_PAIRING) {
        HANDSET_DisableTextDisplay();
        HANDSET_PrintString("PAIRING FAILED");
        HANDSET_EnableTextDisplay();
        appState = APP_State_DISPLAY_DISMISSABLE_TEXT;
      }
      break;

    case BT_EVENT_SYS_PAIRING_OK:
      printf("[PAIR] OK\r\n");
      TIMEOUT_Start(&idleTimeout, IDLE_TIMEOUT);
      INDICATOR_StopSignalStrengthSweep(cellPhoneState.signalStrength);

      if (appState == APP_State_PAIRING) {
        TIMEOUT_Cancel(&appStateTimeout);
        recallPairedDeviceName(true);
      }
      break;
      
    case BT_EVENT_ACL_CONNECTED:  
      printf("[ACL] Connected\r\n");
      TIMEOUT_Cancel(&linkbackRetryTimeout);
      break;

    case BT_EVENT_ACL_DISCONNECTED:  
      printf("[ACL] Disconnected\r\n");
      
      if ((appState != APP_State_PAIRING) && (appState < APP_State_REBOOT_AFTER_DELAY)) {
        BT_LinkBackToLastDevice();
      }
      break;
      
    case BT_EVENT_HFP_CONNECTED:
      printf("[HFP] Connected\r\n");

      BT_CancelLinkback();
      BT_ReadLinkedDeviceName();
      
      if (STORAGE_GetStatusBeepEnabled()) {
        wakeUpHandset(false);
        SOUND_PlayEffect(SOUND_Channel_FOREGROUND, SOUND_Target_SPEAKER, VOLUME_Mode_TONE, SOUND_Effect_BT_CONNECT, false);
      }
      
      INDICATOR_StopFlashing(HANDSET_Indicator_NO_SVC, !cellPhoneState.hasService);
      INDICATOR_StopSignalStrengthSweep(cellPhoneState.signalStrength);

      if (appState == APP_State_PAIRING) {
        BT_ExitPairingMode();
        TIMEOUT_Start(&idleTimeout, IDLE_TIMEOUT);
        TIMEOUT_Cancel(&appStateTimeout);
        recallPairedDeviceName(false);
      }
      
      cellPhoneState.isConnected = true;
      
      TIMEOUT_Start(&cellPhoneState.initialBatteryLevelReportTimeout, INITIAL_BATTERY_LEVEL_REPORT_DELAY);
      break;

    case BT_EVENT_HFP_DISCONNECTED:
      printf("[HFP] Disconnected\r\n");
      if (STORAGE_GetStatusBeepEnabled()) {
        wakeUpHandset(false);
        SOUND_PlayEffect(SOUND_Channel_FOREGROUND, SOUND_Target_SPEAKER, VOLUME_Mode_TONE, SOUND_Effect_BT_DISCONNECT, false);
      }
      
      INDICATOR_StartFlashing(HANDSET_Indicator_NO_SVC);
      HANDSET_SetIndicator(HANDSET_Indicator_ROAM, false);
      HANDSET_SetSignalStrength(0);
      cellPhoneState.isConnected = false;
      cellPhoneState.hasService = false;
      cellPhoneState.maxSignalStrength = 0;
      cellPhoneState.signalStrength = 0;
      cellPhoneState.maxBatteryLevel = 0;
      cellPhoneState.batteryLevel = 0;
      
      TIMEOUT_Cancel(&cellPhoneState.initialBatteryLevelReportTimeout);
      
      if (appState == APP_State_DISPLAY_PAIRED_BATTERY_LEVEL) {
        displayBatteryLevel(true);
      }
      break;
      
    case BT_EVENT_SCO_CONNECTED:
      printf("[SCO] Connected\r\n");
      break;
      
    case BT_EVENT_SCO_DISCONNECTED:
      printf("[SCO] Disconnected\r\n");
      break;
      
    case BT_EVENT_PHONE_SERVICE_STATUS:
      printf("[PHONE] %s\r\n", para ? "Has Service" : "No Service");
      cellPhoneState.hasService = (bool)para;
      INDICATOR_StopFlashing(HANDSET_Indicator_NO_SVC, !cellPhoneState.hasService);
      
      if (!cellPhoneState.hasService) {
        cellPhoneState.signalStrength = 0;
        HANDSET_SetSignalStrength(0);
      } else if (cellPhoneState.signalStrength) {
        HANDSET_SetSignalStrength(cellPhoneState.signalStrength);
      }
      break;
      
    case BT_EVENT_PHONE_ROAMING_STATUS:  
      printf("[PHONE] %s\r\n", para ? "Roaming" : "Not Roaming");
      HANDSET_SetIndicator(HANDSET_Indicator_ROAM, para);
      break;
      
    case BT_EVENT_PHONE_MAX_SIGNAL_STRENGTH:
      printf("[PHONE] Max signal strength: %d\r\n", para);
      cellPhoneState.maxSignalStrength = (uint8_t)para;
      break;
      
    case BT_EVENT_PHONE_SIGNAL_STRENGTH:
      printf("[PHONE] Signal strength: %d\r\n", para);
      if (cellPhoneState.maxSignalStrength) {
        cellPhoneState.signalStrength = (uint8_t)(((((para * 6) << 1) / cellPhoneState.maxSignalStrength) + 1) >> 1);
        
        if (cellPhoneState.hasService) {
          HANDSET_SetSignalStrength(cellPhoneState.signalStrength);
        }
      }
      break;
      
    case BT_EVENT_PHONE_MAX_BATTERY_LEVEL:
      printf("[PHONE] Max battery level: %d\r\n", para);
      cellPhoneState.maxBatteryLevel = para;
      break;
      
    case BT_EVENT_PHONE_BATTERY_LEVEL:
      printf("[PHONE] Battery level: %d\r\n", para);
      if (cellPhoneState.maxBatteryLevel) {
        cellPhoneState.batteryLevel = (uint8_t)(((((para * 5) << 1) / cellPhoneState.maxBatteryLevel) + 1) >> 1);
        
        if (appState == APP_State_DISPLAY_PAIRED_BATTERY_LEVEL) {
          displayBatteryLevel(true);
        }
      }
      break;
      
    case BT_EVENT_HFP_VOLUME_CHANGED:
      printf("[PHONE] Volume changed: %d\r\n", para);
      if (BT_CallStatus >= BT_CALL_OUTGOING) {
        BT_SetHFPGain(0x0F);
      }
      break;

    case BT_EVENT_CALL_STATUS_CHANGED:
      printf("[Call Status] %s\r\n", BT_CallStatusLabel[para]);
      
      pendingCallStatus = para;

      if (!TIMEOUT_IsPending(&pendingCallStatusTimeout)) {
        handleCallStatusChange(pendingCallStatus);
      }
      
      TIMEOUT_Start(&pendingCallStatusTimeout, 150);
      break;
      
    case BT_EVENT_NAME_RECEIVED:
      if (appState == APP_State_RECALL_BT_DEVICE_NAME) {
        char deviceName[33];
        strncpy(deviceName, (char*)para_full + 1, para_full[0])[para_full[0]] = 0;
        MARQUEE_Start(deviceName, MARQUEE_Row_BOTTOM);
      }
      break;
      
    case BT_EVENT_LINKED_NAME_RECEIVED:
      STORAGE_SetPairedDeviceName(para_full[0] ? utf2ascii((char*)para_full) : "[blank]");
      if (appState == APP_State_RECALL_PAIRED_DEVICE_NAME) {
        recallPairedDeviceName(false);
      }
      break;
      
    case BT_EVENT_CALLER_ID: 
      //setCallerId(para_full, para);
      break;
  }
}

void handle_ATCMD_UnsolicitedResult(char const* result) {
  printf("[UNSCOLICITED AT RESULT] %s\r\n", result);
}
