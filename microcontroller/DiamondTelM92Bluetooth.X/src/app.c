/** 
 * @file
 * @author Jeff Lau
 */

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include "app.h"
#include "constants.h"
#include "../mcc_generated_files/pin_manager.h"
#include "telephone/handset.h"
#include "telephone/transceiver.h"
#include "sound/external_mic.h"
#include "sound/tone.h"
#include "sound/sound.h"
#include "sound/ringtone.h"
#include "storage/eeprom.h"
#include "storage/storage.h"
#include "bluetooth/bt_command_decode.h"
#include "bluetooth/bt_command_send.h"
#include "bluetooth/atcmd.h"
#include "ui/indicator.h"
#include "ui/call_timer.h"
#include "ui/ringtone_select.h"
#include "ui/marquee.h"
#include "ui/programming.h"
#include "ui/sound_test.h"
#include "ui/security_code.h"
#include "ui/view_adjust.h"
#include "ui/char_input.h"
#include "ui/string_input.h"
#include "ui/clr_codes.h"
#include "ui/volume_adjust.h"
#include "games/snake_game.h"
#include "games/memory_game.h"
#include "games/tetris_game.h"
#include "util/string.h"
#include "util/timeout.h"
#include "util/interval.h"

static enum {
  APP_CALL_IDLE,
  APP_CALL_SENDING,
  APP_CALL_ACCEPTING,
  APP_CALL_REJECTING,
  APP_CALL_ENDING,
  APP_CALL_VOICE_COMMAND,
  APP_CALL_CANCEL_VOICE_COMMAND
} APP_CallAction;

#define CALL_ACTION_TIMEOUT (200)
static timeout_t callActionTimeout;

static enum {
  BT_CALL_IDLE,
  BT_CALL_VOICE_COMMAND,
  BT_CALL_INCOMING,
  BT_CALL_OUTGOING,
  BT_CALL_ACTIVE,
  BT_CALL_ACTIVE_WITH_CALL_WAITING,
  BT_CALL_ACTIVE_WITH_HOLD
} BT_CallStatus;

static char const* const BT_CallStatusLabel[] = {
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
  APP_State_VOICE_COMMAND,
  APP_State_SELECT_RINGTONE,    
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
  APP_State_SET_BT_DEVICE_NAME,
  APP_State_RECALL_PAIRED_DEVICE_NAME,
  APP_State_SNAKE_GAME,
  APP_State_MEMORY_GAME,
  APP_State_TETRIS_GAME,
  APP_State_PROGRAMMING,
  APP_State_SOUND_TEST,
  APP_State_REBOOT_AFTER_DELAY,
  APP_State_REBOOT
} appState;

static int lastAppState;

static char const* const appStateLabel[] = {
  "INIT_START",
  "INIT_SET_LCD_ANGLE",
  "INIT_ALL_DISPLAY_ON",
  "INIT_DISPLAY_PHONE_NUMBER",
  "INIT_CLEAR_PHONE_NUMBER",
  "NUMBER_INPUT",
  "PAIRING",    
  "INCOMING_CALL",
  "VOICE_COMMAND",
  "SELECT_RINGTONE",    
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
  "SET_BT_DEVICE_NAME",
  "RECALL_PAIRED_DEVICE_NAME",
  "SNAKE_GAME",
  "MEMORY_GAME",
  "TETRIS_GAME",
  "PROGRAMMING",
  "SOUND_TEST",
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
  bool isScoConnected;
  uint8_t maxSignalStrength;
  uint8_t signalStrength;
  uint8_t maxBatteryLevel;
  uint8_t batteryLevel;
  timeout_t initialBatteryLevelReportTimeout;
} cellPhoneState;

static bool isMuted;

static bool isFcn;
static bool isExtendedFcn;
static bool isCallTimerDisplayedByDefault;

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

#define STATUS_BEEP_COOLDOWN_TIMEOUT (100)
static timeout_t statusBeepCooldownTimeout;

static void wakeUpHandset(bool backlight);

/**
 * Play a basic status beep, but only if status beeps are enabled and no other
 * status beep has been recently played.
 */
static void playStatusBeep(void) {
  if (
      // Don't play status beeps while the cell phone is not fully connected to 
      // Bluetooth HFP. This prevents status beeps before HFP connection and
      // after HFP disconnection.
      !cellPhoneState.isConnected || 
      !STORAGE_GetStatusBeepEnabled() || 
      TIMEOUT_IsPending(&statusBeepCooldownTimeout)
      ) {
    return;
  }
  
  TIMEOUT_Start(&statusBeepCooldownTimeout, STATUS_BEEP_COOLDOWN_TIMEOUT);

  wakeUpHandset(false);
  SOUND_PlayStatusBeep();
}

/**
 * Play a Bluetooth connect/disconnect status status beep, but only if status 
 * beeps are enabled.
 * 
 * @param isConnected - true if Bluetooth is now connected.
 */
static void playBluetoothConnectionStatusBeep(bool isConnected) {
  if (!STORAGE_GetStatusBeepEnabled()) {
    return;
  }
  
  TIMEOUT_Start(&statusBeepCooldownTimeout, STATUS_BEEP_COOLDOWN_TIMEOUT);

  wakeUpHandset(false);

  SOUND_PlayEffect(
        SOUND_Channel_FOREGROUND, 
        SOUND_Target_SPEAKER, 
        VOLUME_Mode_TONE, 
        isConnected ? SOUND_Effect_BT_CONNECT : SOUND_Effect_BT_DISCONNECT, 
        false
      );
}

static timeout_t deferredSetAecEnabledTimeout;

#define DEFERRED_SET_AEC_ENABLED_TIMEOUT (100)

static void setAecEnabled(void) {
  if (
      !TIMEOUT_IsPending(&deferredSetAecEnabledTimeout) && 
      cellPhoneState.isScoConnected && 
      ((BT_CallStatus == BT_CALL_VOICE_COMMAND) || (BT_CallStatus >= BT_CALL_ACTIVE))
      ) {
    BT_SetAecEnabled(HANDSET_IsOnHook());
  }
}

static void deferredSetAecEnabled(void) {
  TIMEOUT_Start(&deferredSetAecEnabledTimeout, DEFERRED_SET_AEC_ENABLED_TIMEOUT);
}

static void cancelDeferredSetAecEnabled(void) {
  TIMEOUT_Cancel(&deferredSetAecEnabledTimeout);
}

#define CALL_FAILED_TIMEOUT (3000)
static volatile uint16_t callFailedTimer;
static volatile bool callFailedTimerExpired;

#define IDLE_TIMEOUT (1000)
static timeout_t idleTimeout;
static volatile bool isHandsetIdle;

static void wakeUpHandset(bool backlight) {
  TIMEOUT_Start(&idleTimeout, IDLE_TIMEOUT);

  isHandsetIdle = false;
  SOUND_Enable();
  
  if (backlight) {
    HANDSET_SetBacklight(true);
  }
}

static void sleepHandset(void) {
  if (
      !TIMEOUT_IsPending(&idleTimeout) &&
      HANDSET_IsOnHook() && 
      !HANDSET_IsAnyButtonDown() &&
      (BT_CallStatus == BT_CALL_IDLE) &&
      (appState != APP_State_PAIRING) && 
      (appState != APP_State_TETRIS_GAME) && 
      (appState != APP_State_SNAKE_GAME) && 
      (appState != APP_State_MEMORY_GAME) && 
      (appState != APP_State_SELECT_RINGTONE) && 
      !callFailedTimer &&
      !TRANSCEIVER_IsConnectedToExternalPower()
      ) {
    isHandsetIdle = true;
    HANDSET_SetBacklight(false);
    SOUND_Disable();
  }
}

#define LOW_BATTERY_BEEP_INTERVAL (2000)
interval_t lowBatteryBeepInterval;

#define BROWSE_DIRECTORY_SCAN_INTERVAL (100)
static bool isDirectoryScanNameMode;

#define NUMBER_INPUT_MAX_LENGTH (MAX_EXTENDED_PHONE_NUMBER_LENGTH)
static char numberInput[NUMBER_INPUT_MAX_LENGTH + 1];
static size_t numberInputNextDtmfSendIndex;
static size_t numberInputLength;
static bool numberInputIsStale;

static char tempNumberBuffer[NUMBER_INPUT_MAX_LENGTH + 1];

static char alphaInput[STORAGE_MAX_DEVICE_NAME_LENGTH + 1];
static bool isAlphaPromptDisplayed;
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
  numberInputNextDtmfSendIndex = 0;
  isCallTimerDisplayedByDefault = false;
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
  isCallTimerDisplayedByDefault = false;
}

static bool NumberInput_HasIncompleteCreditCardMemorySymbol(void) {
  return (numberInputLength > 1) 
      && (numberInput[numberInputLength - 1] == '*')
      && (numberInput[numberInputLength - 2] == 'M');
}

static void NumberInput_PopDigit(void) {
  if (numberInputLength) {
    // delete "M*" together as a single "digit"
    if (NumberInput_HasIncompleteCreditCardMemorySymbol()) {
      numberInput[--numberInputLength] = 0;
    }

    numberInput[--numberInputLength] = 0;
    numberInputNextDtmfSendIndex = 0;
    numberInputIsStale = false;
  }
}

static void NumberInput_PrintToHandset(void)  {
  CALL_TIMER_DisableDisplayUpdate();

  if (numberInputLength == 0) {
    if (TRANSCEIVER_IsBatteryLevelLow()) {
      HANDSET_DisableTextDisplay();
      HANDSET_ClearText();
      HANDSET_PrintString("  LOW  BATTERY");
      HANDSET_EnableTextDisplay();
    } else {
      HANDSET_ClearText();
    }
  } else {
    HANDSET_DisableTextDisplay();
    HANDSET_ClearText();
    HANDSET_PrintString(numberInput);
    HANDSET_EnableTextDisplay();
  }
}

static void NumberInput_Clear(void) {
  numberInput[numberInputLength = 0] = 0;
  numberInputNextDtmfSendIndex = 0;
  numberInputIsStale = false;
}

static uint8_t NumberInput_GetMemoryIndex(void) {
  uint8_t result;
  
  if ((numberInputLength < 1) || (numberInputLength > 2)) {
    return 0xFF;
  }
  
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
 
  result -= 1;
  
  return (result < STORAGE_DIRECTORY_SIZE) ? result : 0xFF;
}

static void returnToNumberInput(bool withRecalledNumber);

static void hideCallTimer(void) {
  isCallTimerDisplayedByDefault = false;

  if (CALL_TIMER_IsDisplayEnabled()) {
    if (numberInputLength) {
      returnToNumberInput(false);
    } else {
      NumberInput_PrintToHandset();
    }
  }
}

static bool NumberInput_SendNextDTMFString(void) {
  if ((numberInputNextDtmfSendIndex == 0) || (numberInputNextDtmfSendIndex >= numberInputLength)) {
    char const* const firstPause = strpbrk(numberInput, "PM");
    if (firstPause) {
      numberInputNextDtmfSendIndex = firstPause - numberInput;
    } else {
      numberInputNextDtmfSendIndex = 0;
    }
  }

  if (numberInputNextDtmfSendIndex == 0) {
    return false;
  }

  hideCallTimer();
  numberInputIsStale = true;
  
  if (NumberInput_HasIncompleteCreditCardMemorySymbol() ||
    (numberInput[numberInputLength - 1] == 'P')) {
    return true;
  }

  if (numberInput[numberInputNextDtmfSendIndex] == 'M') {
    numberInputNextDtmfSendIndex += 2;
    char digit = numberInput[numberInputNextDtmfSendIndex];

    STORAGE_GetCreditCardNumber(digit - '1', tempNumberBuffer);
    ++numberInputNextDtmfSendIndex;
  } else {
    if (numberInput[numberInputNextDtmfSendIndex] == 'P') {
      ++numberInputNextDtmfSendIndex;
    }

    char const* const nextPause = strpbrk(numberInput + numberInputNextDtmfSendIndex, "PM");

    if (nextPause) {
      size_t const len = (nextPause - numberInput) - numberInputNextDtmfSendIndex;
      strncpy(tempNumberBuffer, numberInput + numberInputNextDtmfSendIndex, len)[len] = 0;
      numberInputNextDtmfSendIndex += len;
    } else {
      strcpy(tempNumberBuffer, numberInput + numberInputNextDtmfSendIndex);
      numberInputNextDtmfSendIndex = 0;
    }
  }

  ATCMD_SendDTMFDigitString(tempNumberBuffer);
  return true;
}

static void startCallFailed(void) {
  HANDSET_SetIndicator(HANDSET_Indicator_IN_USE, true);

  SOUND_PlayEffect(
      SOUND_Channel_BACKGROUND, 
      SOUND_Target_EAR,
      VOLUME_Mode_TONE,
      SOUND_Effect_REORDER_TONE, 
      true
      );

  callFailedTimer = CALL_FAILED_TIMEOUT;

  if (
      (appState != APP_State_NUMBER_INPUT) &&
      (appState != APP_State_DISPLAY_NUMBER_OVERFLOW)
  ) {
    returnToNumberInput(false);
  }
}

static void displayVoiceCommand(void) {
  HANDSET_DisableTextDisplay();
  HANDSET_ClearText();
  HANDSET_PrintString("VOICE  COMMAND");
  HANDSET_EnableTextDisplay();
  appState = APP_State_VOICE_COMMAND;
}

static bool startVoiceCommand(void) {
  if (!cellPhoneState.isConnected) {
    startCallFailed();
    return false;
  } else {
    APP_CallAction = APP_CALL_VOICE_COMMAND;
    TIMEOUT_Start(&callActionTimeout, CALL_ACTION_TIMEOUT);
    BT_StartVoiceCommand();
    displayVoiceCommand();
    return true;
  }
}

static void NumberInput_SendCurrentNumberAsDtmf(void) {
  hideCallTimer();
  numberInputIsStale = true;
  numberInputNextDtmfSendIndex = 0;
  ATCMD_SendDTMFDigitString(numberInput);
}

static void NumberInput_CallCurrentNumber(void) {
  STORAGE_SetLastDialedNumber(numberInput);
  numberInputIsStale = true;
  numberInputNextDtmfSendIndex = 0;
  
  strcpy(tempNumberBuffer, numberInput);
  char* firstPause = strpbrk(tempNumberBuffer, "PM");
  
  if (firstPause) {
    *firstPause = 0;
  }
  
  if ((strcmp(tempNumberBuffer, "0") == 0) || (strcmp(tempNumberBuffer, "411") == 0)) {
    // Handle special phone numbers that should initiate voice command instead
    // of actually calling the number.
    // This special way of initiating voice command allows it to be triggered
    // from the Hands-Free Controller Box, which is only able speed dial by 
    // simulating the handset button sequence to recall one of the first 3 
    // directory entries then "SEND" to call it. There's no way to directly
    // detect and process Hands-Free button presses to implement custom behavior.
    startVoiceCommand();
  } else if (!cellPhoneState.hasService || !BT_MakeCall(tempNumberBuffer)) {
    startCallFailed();
  } else {
    if (
        (appState != APP_State_NUMBER_INPUT) && 
        (appState != APP_State_DISPLAY_NUMBER_OVERFLOW)
        ) {
      returnToNumberInput(false);
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

static void setCallerId(char const* text) {
  // NOTE: Ignore Caller ID when a microphone is detected while OEM Hands-Free 
  //       integration is enabled.
  //       This is because Caller ID is incompatible with the Hands-Free Controller.  
  if (!text || 
      !STORAGE_GetCallerIdMode() ||
      (STORAGE_GetOemHandsFreeIntegrationEnabled() && EXTERNAL_MIC_IsConnected())
      ) {
    callerIdText[0] = 0;
    return;
  }
  
  size_t len = strlen(text);
  
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
  
  if ((appState == APP_State_INCOMING_CALL) && len) {
    HANDSET_SetTextBlink(false);
    MARQUEE_Start(callerIdText, MARQUEE_Row_TOP);
  }
}

static bool isCallFlashOn;

static void showIncomingCall(bool isMissedCall) {
  bool const showCallerId = callerIdText[0];
  
  CALL_TIMER_DisableDisplayUpdate();
  
  MARQUEE_Stop();
  HANDSET_ClearText();
  HANDSET_DisableTextDisplay();
  HANDSET_PrintString(
      BT_CallStatus == BT_CALL_ACTIVE_WITH_CALL_WAITING 
      ? "W CALL " 
      : "CALL "
  );
  HANDSET_EnableTextDisplay();
  
  if (isMissedCall) {
    TIMEOUT_Cancel(&appStateTimeout);
    appState = APP_State_DISPLAY_DISMISSABLE_TEXT;
  } else {
    if (showCallerId) {
      HANDSET_SetTextBlink(false);
      isCallFlashOn = true;
      TIMEOUT_Start(&appStateTimeout, 50);
    } else {
      // This sequence of commands immediately following the printing of "CALL "
      // to the display seems to be necessary to properly trigger the
      // Hands-Free Controller to believe that there is an incoming call.
      // NOTE: Any subsequent commands to print anything to the display seems
      //       to break the state of the Hands-Free Controller such that it
      //       still has control of audio, but does not believe there is an
      //       incoming call so the "H/F" button cannot be used to answer the
      //       call. This means there's no way to make caller ID compatible with
      //       the Hands-Free Controller
      HANDSET_DisableCommandOptimization();
      HANDSET_SetTextBlink(true);
      HANDSET_SetIndicator(HANDSET_Indicator_IN_USE, BT_CallStatus == BT_CALL_ACTIVE_WITH_CALL_WAITING);
      HANDSET_EnableCommandOptimization();
    }
    
    appState = APP_State_INCOMING_CALL;
  }

  if (showCallerId) {
    MARQUEE_Start(callerIdText, MARQUEE_Row_TOP);
  } 
}

static void startAlphaStoreNumberInput(bool reset);

static void handleAlphaStoreStringInputReturn(STRING_INPUT_Result result, char const* name) {
  if (result == STRING_INPUT_Result_APPLY) {
    startAlphaStoreNumberInput(true);
  } else {
    returnToNumberInput(false);
  }
}

static void startAlphaStoreNameInput(bool reset) {
  if (reset) {
    alphaInput[0] = 0;
  }

  CALL_TIMER_DisableDisplayUpdate();

  STRING_INPUT_Start(
      alphaInput,
      STORAGE_MAX_DIRECTORY_NAME_LENGTH,
      "NAME ?        ",
      true,
      false,
      handleAlphaStoreStringInputReturn
      );
  
  appState = APP_State_ALPHA_STO_NAME_INPUT;
  TIMEOUT_Cancel(&appStateTimeout);
}

static void recallAddress(uint8_t addr, bool isNameMode);

static void handleAlphaScanCharInput(char c) {
  HANDSET_SetIndicator(HANDSET_Indicator_FCN, false);
  recallAddress(STORAGE_GetFirstNamedDirectoryIndexForLetter(c), true);  
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
  
  INDICATOR_StartFlashing(HANDSET_Indicator_FCN);

  CHAR_INPUT_Start(true, false, handleAlphaScanCharInput, NULL);
  
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
  if (addr > STORAGE_DIRECTORY_SIZE) {
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
    char deviceName[STORAGE_MAX_DEVICE_NAME_LENGTH + 1];
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
  
  if ((BT_CallStatus >= BT_CALL_ACTIVE) && !numberInputLength) {
    isCallTimerDisplayedByDefault = true;
  }
  
  if (isCallTimerDisplayedByDefault) {
    CALL_TIMER_EnableDisplayUpdate();
    appState = APP_State_NUMBER_INPUT;
  } else {
    CALL_TIMER_DisableDisplayUpdate();

    if (numberInputLength > 14) {
      recallNumberInputOverflow(withRecalledNumber ? RECALL_FROM_MEMORY : RECALL_FROM_INPUT_AUTOMATIC);
    } else {
      NumberInput_PrintToHandset();
      appState = APP_State_NUMBER_INPUT;
    }
  }
}

static void setExternallyInitiatedOutgoingCallNumber(char const* number) {
  NumberInput_Overwrite(number);
  numberInputIsStale = true;
  returnToNumberInput(false);
  STORAGE_SetLastDialedNumber(number);
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
  recallPairedDeviceName(false);
}

static void handleBluetoothNameStringInputReturn(STRING_INPUT_Result result, char const* name) {
  if (result == STRING_INPUT_Result_APPLY) {
    setBluetoothName(name);
  } else {
    returnToNumberInput(false);
  }
}

static void handle_SECURITY_CODE_Success_SET_BT_DEVICE_NAME(void) {
  alphaInput[0] = 0;
  
  STRING_INPUT_Start(
      alphaInput, 
      STORAGE_MAX_DEVICE_NAME_LENGTH, 
      "BT     NAME ? ",
      true,
      true,
      handleBluetoothNameStringInputReturn
      );
  
  appState = APP_State_SET_BT_DEVICE_NAME;
}

static void handle_SECURITY_CODE_Success_TOGGLE_DUAL_NUMBER(void) {
  STORAGE_SetActiveOwnNumberIndex(!STORAGE_GetActiveOwnNumberIndex());
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

  if ((BT_CallStatus == BT_CALL_INCOMING) || (BT_CallStatus == BT_CALL_ACTIVE_WITH_CALL_WAITING)) {
    showIncomingCall(false);
  } else if ((BT_CallStatus == BT_CALL_VOICE_COMMAND) || (APP_CallAction == APP_CALL_VOICE_COMMAND)) {
    displayVoiceCommand();
  } else {
    returnToNumberInput(false);
  }  
}

static void startVolumeAdjust(VOLUME_Mode volumeMode, bool up) {
  // In case we are adjusting volume during an incoming call state with blinking "CALL" text
  HANDSET_SetTextBlink(false);
  MARQUEE_Stop();
  CALL_TIMER_DisableDisplayUpdate();
  
  bool const isSilent = ((volumeMode == VOLUME_Mode_TONE) && callFailedTimer) ||
      (volumeMode == VOLUME_Mode_HANDSET) ||
      (volumeMode == VOLUME_Mode_HANDS_FREE);
  
  VOLUME_ADJUST_Start(volumeMode, isSilent, up, handleReturnFromSubModule);
  appState = APP_State_ADJUST_VOLUME;
}

static char nameInput[STORAGE_MAX_DIRECTORY_NAME_LENGTH + 1] = {0};
static char nameInputLength = 0;


void handleCallListAtResponse(ATCMD_Response response, char const* result) {
  if (response == ATCMD_Response_RESULT) {
    char phoneNumber[24];
    char buffer[48];

    // id
    char const* nextField = parseNextCsvField(buffer, result + 7);
    // dir
    nextField = parseNextCsvField(buffer, nextField);
    // stat
    nextField = parseNextCsvField(buffer, nextField);
    char const stat = *buffer;
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

    switch (BT_CallStatus) {
      case BT_CALL_INCOMING:
      case BT_CALL_ACTIVE_WITH_CALL_WAITING:
        // Process incoming or call waiting result to get caller ID
        if ((stat == '4') || (stat == '5')) {
          // If "alpha" is populated, then use it as Caller ID
          if (*buffer && ((STORAGE_GetCallerIdMode() == CALLER_ID_Mode_NAME) || !*phoneNumber)) {
            setCallerId(buffer);
          } else if (*phoneNumber) {
            formatPhoneNumber(buffer, phoneNumber);
            setCallerId(buffer);
          } else {
            setCallerId("Unknown Caller");
          }
        }
        break;
        
      case BT_CALL_OUTGOING:
        // Process outgoing call result to get the called number.
        // ASSUMPTION: We only request this for outgoing calls that were
        //             initiated externally.
        if (stat == '2') {
          setExternallyInitiatedOutgoingCallNumber(simplifyPhoneNumber(buffer, phoneNumber));
        }
        break;
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
    HANDSET_SetIndicator(HANDSET_Indicator_IN_USE, false);
    SOUND_ForceNextSetHandsetAudioOutput();
    SOUND_Stop(SOUND_Channel_BACKGROUND);
    callFailedTimerExpired = true;
    callFailedTimer = 0;
    callFailedTimerExpired = false;
  }

  int prevCallStatus = BT_CallStatus;
  BT_CallStatus = newCallStatus;
  
  bool const isOemHandsFreeControllerConnected = 
        STORAGE_GetOemHandsFreeIntegrationEnabled() && EXTERNAL_MIC_IsConnected();
  
  if ((BT_CallStatus != BT_CALL_VOICE_COMMAND) && (BT_CallStatus < BT_CALL_ACTIVE)) {
    cancelDeferredSetAecEnabled();
  } else if (
      (BT_CallStatus == BT_CALL_VOICE_COMMAND) ||
      ((prevCallStatus < BT_CALL_ACTIVE) && (BT_CallStatus >= BT_CALL_ACTIVE))
      ) {
    deferredSetAecEnabled();
  }
  
  switch (BT_CallStatus) {
    case BT_CALL_IDLE:
      TIMEOUT_Start(&idleTimeout, IDLE_TIMEOUT);
      CALL_TIMER_Stop();
      
      // Perform extra steps only if the OEM Hands-Free Controller is connected...
      if (isOemHandsFreeControllerConnected) {
        // The following sequence of commands is what seems to be necessary
        // to reliably trigger the Hands-Free Controller to believe that the phone
        // is no longer in a call (works when coming from an active call, or
        // an incoming call that was missed or explicitly rejected).
        // This MUST be done before any SOUND commands to ensure that the SOUND
        // commands are not intercepted by the Hands-Free Controller.
        HANDSET_DisableCommandOptimization();
        HANDSET_SetLoudSpeaker(false);
        HANDSET_SetEarSpeaker(true);
        HANDSET_SetMicrophone(true);
      }

      // These commands are necessary regardless of whether the OEM Hands-Free
      // Controller is connected or not. But when connected, the exact order
      // of these commands (and relative to other commands) is important.
      HANDSET_SetIndicator(HANDSET_Indicator_IN_USE, false);
      HANDSET_SetTextBlink(false);
      HANDSET_SetIndicator(HANDSET_Indicator_MUTE, false);
      
      // Perform extra steps only if the OEM Hands-Free Controller is connected...
      if (isOemHandsFreeControllerConnected) {
        HANDSET_EnableCommandOptimization();

        // Force the next update to handset audio output to send commands to the 
        // handset regardless of whether we believe the handset is already in the
        // desired state or not. This is necessary for compatibility with the 
        // Hands-Free Controller Unit, which may disable sound output on the 
        // handset during a call to redirect sound to the car radio speakers,
        // which causes our assumptions about the handset audio state to be out 
        // of sync with the actual handset audio state.
        SOUND_ForceNextSetHandsetAudioOutput();
      }
      
      SOUND_SetDefaultAudioSource(SOUND_AudioSource_MCU);
      SOUND_SetButtonsMuted(false);

      isMuted = false;
      isCallTimerDisplayedByDefault = false;

      if ((appState == APP_State_INCOMING_CALL) && (APP_CallAction != APP_CALL_REJECTING)) {
        numberInputIsStale = true;
        showIncomingCall(true);
      } else {
        if (
            (APP_CallAction != APP_CALL_ENDING) && 
            (APP_CallAction != APP_CALL_REJECTING) && 
            (APP_CallAction != APP_CALL_CANCEL_VOICE_COMMAND)
            ) {
          SOUND_PlayEffect(
            SOUND_Channel_FOREGROUND, 
            SOUND_Target_SPEAKER,
            VOLUME_Mode_TONE,
            SOUND_Effect_CALL_DISCONNECT, 
            false
          );
        }

        if (
            (appState == APP_State_VOICE_COMMAND) || 
            (appState == APP_State_NUMBER_INPUT) ||
            (appState == APP_State_INCOMING_CALL)
            ) {
          numberInputIsStale = true;
          returnToNumberInput(false);
        }
      }

      setCallerId(NULL);
      break;
    
    case BT_CALL_ACTIVE:
      if (
          (prevCallStatus == BT_CALL_ACTIVE_WITH_HOLD) && 
          (APP_CallAction != APP_CALL_ENDING)
          ) {
        // A second call was ended externally, so play the "call disconnect"
        // sound.
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
      setCallerId(NULL);
      CALL_TIMER_Start(false);

      if (
          (prevCallStatus < BT_CALL_ACTIVE) || 
          (prevCallStatus == BT_CALL_ACTIVE_WITH_CALL_WAITING)
          ) {
        // Force the call timer to display any time a call first becomes active,
        // or when coming from the "incoming call waiting" state.
        isCallTimerDisplayedByDefault = true;
      }
      // Look out below!

    case BT_CALL_OUTGOING:
      wakeUpHandset(true);
      BT_SetHFPGain(0x0F);
      SOUND_SetDefaultAudioSource(SOUND_AudioSource_BT);
      
      HANDSET_SetTextBlink(false);

      // Send the following command unconditionally for compatibility
      // with the Hands-Free Controller. This is necessary to for it to 
      // properly believe the the phone is "IN USE" when transitioning out of 
      // BT_CALL_ACTIVE_WITH_CALL_WAITING state.
      HANDSET_DisableCommandOptimization();
      HANDSET_SetIndicator(HANDSET_Indicator_IN_USE, true);
      HANDSET_EnableCommandOptimization();

      CALL_TIMER_SetCallWaitingIndicator(BT_CallStatus > BT_CALL_ACTIVE_WITH_CALL_WAITING);

      if ((BT_CallStatus == BT_CALL_OUTGOING) && (APP_CallAction != APP_CALL_SENDING)) {
        // This is an outgoing call that was initiated externally, so we don't 
        // have the called number available. Request the list of calls to get
        // the called number.
        //
        // NOTE: Specifically NOT returning to number input in this case
        //       to continue displaying whatever is currently on the display 
        //       briefly until the called number is received and ready to be 
        //       displayed.
        ATCMD_Send("+CLCC", handleCallListAtResponse);
      } else if (
          (appState == APP_State_INCOMING_CALL) || 
          ((BT_CallStatus == BT_CALL_ACTIVE) && (prevCallStatus < BT_CALL_ACTIVE))
          ) {
        returnToNumberInput(false);
      }
      break;

    case BT_CALL_ACTIVE_WITH_CALL_WAITING:
    case BT_CALL_INCOMING:
      wakeUpHandset(true);

      resetFcn();
      isRclInputPending = false;
      isRclOrSto2ndDigitPending = false;
      isStoInputPending = false;

      // NOTE: Updating the IN USE indicator BEFORE showing the incoming call
      //       is necessary for compatibility with the Hands-Free Controller
      //       in the edge case where the call state transitioned from
      //       BT_CALL_ACTIVE_WITH_CALL_WAITING to BT_CALL_INCOMING 
      //       (active call was ended from the other end while a waiting call
      //       was still incoming).
      //       In this edge case, clearing out the IN USE indicator will exit 
      //       the Hands-Free "in use" mode so that the subsequent display of
      //       the incoming call will properly trigger the Hands-Free
      //       "incoming call" mode.
      HANDSET_SetIndicator(HANDSET_Indicator_IN_USE, BT_CallStatus == BT_CALL_ACTIVE_WITH_CALL_WAITING);
      showIncomingCall(false);

      if (BT_CallStatus == BT_CALL_INCOMING) {
        RINGTONE_Start(STORAGE_GetRingtone());
        
        if (STORAGE_GetAutoAnswerEnabled()) {
          TIMEOUT_Start(&autoAnswerTimeout, 900);
        }
      } else {
        BT_SetHFPGain(0x0F);
        SOUND_SetDefaultAudioSource(SOUND_AudioSource_BT);
      }

      // Request current call list to extract Caller ID
      // NOTE: Ignore Caller ID when a microphone is detected while OEM Hands-Free 
      //       integration is enabled.
      //       This is because Caller ID is incompatible with the Hands-Free Controller.  
      if (STORAGE_GetCallerIdMode() && !isOemHandsFreeControllerConnected) {
        ATCMD_Send("+CLCC", handleCallListAtResponse);
      }
      break;

    case BT_CALL_VOICE_COMMAND:
      HANDSET_SetIndicator(HANDSET_Indicator_IN_USE, true);
      wakeUpHandset(true);
      BT_SetHFPGain(0x0F);
      SOUND_SetDefaultAudioSource(SOUND_AudioSource_BT);
      isCallTimerDisplayedByDefault = false;
      
      if (appState != APP_State_VOICE_COMMAND) {
        displayVoiceCommand();
      }
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
    
    case CLR_CODES_EventType_SOUND_TEST: 
      appState = APP_State_SOUND_TEST;
      SOUND_TEST_Start(reboot);
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
  return (button >= '1') && (button < '1' + STORAGE_CREDIT_CARD_COUNT);
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
  BT_CommandDecode_Initialize();
  BT_CommandSend_Initialize();
  ATCMD_Initialize(handle_ATCMD_UnsolicitedResult);
  MARQUEE_Initialize();
  CLR_CODES_Initialize();
  INTERVAL_Initialize(&lowBatteryBeepInterval, LOW_BATTERY_BEEP_INTERVAL);
}

void APP_Task(void) {
  if (appState != lastAppState) {
    lastAppState = appState;
    printf("[App State] %s\r\n", appStateLabel[appState]);
  }
  
  EEPROM_Task();
  VOLUME_Task();
  SOUND_Task();
  INDICATOR_Task();
  MARQUEE_Task();
  BT_CommandDecode_Task();
  BT_CommandSend_Task();
  HANDSET_Task();
  TRANSCEIVER_Task();
  EXTERNAL_MIC_Task();
  
  switch (appState) {
    case APP_State_PROGRAMMING:
      return;
      
    case APP_State_SOUND_TEST:
      SOUND_TEST_Task();
      return;
  }

  CALL_TIMER_Task();
  ATCMD_Task();
  CLR_CODES_Task();
  
  TIMEOUT_Task(&appStateTimeout);
  TIMEOUT_Task(&statusBeepCooldownTimeout);
  
  if (TIMEOUT_Task(&deferredSetAecEnabledTimeout)) {
    setAecEnabled();
  }

  if (TIMEOUT_Task(&idleTimeout)) {
    sleepHandset();
  }
  
  if (TIMEOUT_Task(&fcnTimeout)) {
    resetFcn();
  }
  
  if (TIMEOUT_Task(&callActionTimeout)) {
    if ((APP_CallAction == APP_CALL_SENDING) || (APP_CallAction == APP_CALL_VOICE_COMMAND)) {
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
    // Play a "virtual" button beep of no button in particular, but with a 
    // limited beep duration. This simulates the sound of the user pressing the
    // "SEND" button manually to answer the call.
    SOUND_PlayButtonBeep(HANDSET_Button_NONE, true);
    APP_CallAction = APP_CALL_ACCEPTING;
    TIMEOUT_Start(&callActionTimeout, CALL_ACTION_TIMEOUT);
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
    // Needed to recover audio control from the Hands-Free Controller
    SOUND_ForceNextSetHandsetAudioOutput();
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
        TIMEOUT_Start(&appStateTimeout, 60);
        SOUND_Initialize();       
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
          STORAGE_GetOwnNumber(STORAGE_GetActiveOwnNumberIndex(), tempNumberBuffer);
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
        INDICATOR_StartFlashing(HANDSET_Indicator_SIGNAL_BARS);
        HANDSET_SetIndicator(HANDSET_Indicator_NO_SVC, true);
        
        HANDSET_ClearText();
        HANDSET_RequestHookStatus();
        HANDSET_EnableCommandOptimization();
        
        CLR_CODES_Start(handle_CLR_CODES_Event);
        EXTERNAL_MIC_Initialize(NULL);
  
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
        INDICATOR_StartFlashing(HANDSET_Indicator_SIGNAL_BARS);
        
        HANDSET_DisableTextDisplay();
        HANDSET_PrintString("PAIRINGTIMEOUT");
        HANDSET_EnableTextDisplay();
        appState = APP_State_DISPLAY_DISMISSABLE_TEXT;
      }
      break;
      
    case APP_State_INCOMING_CALL:
      if (!TIMEOUT_IsPending(&appStateTimeout)) {
        if (callerIdText[0]) {
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

void APP_Timer1MS_Interrupt(void) {
  SOUND_Timer1MS_Interrupt();
  BT_CommandSend_Timer1MS_Interrupt();
  HANDSET_Timer1MS_Interrupt();
}

void APP_Timer10MS_Interrupt(void) {
  switch (appState) {
    case APP_State_PROGRAMMING:
      return;
      
    case APP_State_SOUND_TEST:
      SOUND_TEST_Timer10MS_Interrupt();
      return;
  }

  TIMEOUT_Timer_Interrupt(&appStateTimeout);
  TIMEOUT_Timer_Interrupt(&statusBeepCooldownTimeout);
  TIMEOUT_Timer_Interrupt(&deferredSetAecEnabledTimeout);

  switch (appState) {
    case APP_State_REBOOT_AFTER_DELAY:
    case APP_State_REBOOT:
      return;
      
    case APP_State_ADJUST_VOLUME:
      VOLUME_ADJUST_Timer10MS_Interrupt();
      break;
      
    case APP_State_SELECT_RINGTONE:
      RINGTONE_SELECT_Timer10MS_Interrupt();
      break;
      
    case APP_State_SNAKE_GAME:
      SNAKE_GAME_Timer10MS_Interrupt();
      break;
      
    case APP_State_MEMORY_GAME:
      MEMORY_GAME_Timer10MS_Interrupt();
      break;
      
    case APP_State_TETRIS_GAME:
      TETRIS_GAME_Timer10MS_Interrupt();
      break;
  }

  ATCMD_Timer10ms_Interrupt();
  TRANSCEIVER_Timer10MS_Interrupt();
  EXTERNAL_MIC_Timer10MS_Interrupt();
  INDICATOR_Timer10MS_Interrupt();
  MARQUEE_Timer10MS_Interrupt();
  CLR_CODES_Timer10MS_Interrupt();
  VOLUME_Timer10MS_Interrupt();
  
  TIMEOUT_Timer_Interrupt(&idleTimeout);
  TIMEOUT_Timer_Interrupt(&fcnTimeout);
  TIMEOUT_Timer_Interrupt(&callActionTimeout);
  TIMEOUT_Timer_Interrupt(&autoAnswerTimeout);
  TIMEOUT_Timer_Interrupt(&pendingCallStatusTimeout);
  TIMEOUT_Timer_Interrupt(&linkbackRetryTimeout);
  TIMEOUT_Timer_Interrupt(&cellPhoneState.initialBatteryLevelReportTimeout);
  INTERVAL_Timer_Interrupt(&lowBatteryBeepInterval);
  
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
  
  if (event->type == HANDSET_EventType_HOOK) {
    setAecEnabled();
  }

  if (isButtonUp || isButtonDown || (event->type == HANDSET_EventType_HOOK)) {
    wakeUpHandset(button != HANDSET_Button_PWR);
  }
  
  CLR_CODES_HANDSET_EventHandler(event);

  // Nothing below here needs to handle CLR CODE button events.
  if (HANDSET_IsButtonClrCode(button)) {
    return;
  }

  TRANSCEIVER_HANDSET_EventHandler(event);  
  
  // Nothing below here needs to handle PWR button events.
  if (button == HANDSET_Button_PWR) {
    return;
  }
  
  SOUND_HANDSET_EventHandler(event);

  // Common END button behavior that needs to work no matter what the 
  // app state is (e.g., user can end a call or initiate voice command while 
  // browsing the directory or playing Tetris).
  if (button == HANDSET_Button_END) {
    if (isButtonDown) {
      if (callFailedTimer && !isFcn) {
        // End failed call
        SOUND_PlayButtonBeep(button, false);
        HANDSET_SetIndicator(HANDSET_Indicator_IN_USE, false);
        callFailedTimer = 0;
        callFailedTimerExpired = false;
        // Needed to recover audio control from the Hands-Free Controller
        SOUND_ForceNextSetHandsetAudioOutput();
        SOUND_Stop(SOUND_Channel_BACKGROUND);
        numberInputIsStale = true;
        returnToNumberInput(false);
        return;
      } else if ((BT_CallStatus > BT_CALL_IDLE) && (APP_CallAction == APP_CALL_IDLE)) {
        if (BT_CallStatus > BT_CALL_ACTIVE) {
          APP_CallAction = APP_CALL_ENDING;
          
          if (isFcn) {
            BT_EndHoldOrWaitingCall();
          } else {
            BT_SwapHoldOrWaitingCallAndEndActiveCall();
          }
        } else if ((BT_CallStatus > BT_CALL_INCOMING) && !isFcn) {
          APP_CallAction = APP_CALL_ENDING;
          BT_EndCall();
        } else if ((BT_CallStatus == BT_CALL_INCOMING) && !isFcn) {
          APP_CallAction = APP_CALL_REJECTING;
          BT_RejectCall();
          TIMEOUT_Cancel(&autoAnswerTimeout);
        } else if ((BT_CallStatus == BT_CALL_VOICE_COMMAND) && !isFcn) {
          APP_CallAction = APP_CALL_CANCEL_VOICE_COMMAND;
          BT_CancelVoiceCommand();
        }

        if (APP_CallAction != APP_CALL_IDLE) {
          SOUND_PlayButtonBeep(button, false);
          TIMEOUT_Start(&callActionTimeout, CALL_ACTION_TIMEOUT);
          // Prevent triggering voice command if END button is held while
          // rejecting a call.
          HANDSET_CancelCurrentButtonHoldEvents();
        }

        resetFcn();
        return;
      }
    } else if (
        (event->type == HANDSET_EventType_BUTTON_HOLD) &&
        (event->holdDuration == HANDSET_HoldDuration_SHORT) &&
        (BT_CallStatus == BT_CALL_IDLE) &&
        (APP_CallAction == APP_CALL_IDLE)
        ) {
      // Handle initiation of voice command
      SOUND_Stop(SOUND_Channel_BACKGROUND);
      resetFcn();

      if (startVoiceCommand()) {
        SOUND_PlayButtonBeep(button, false);
      }
      return;
    }
  }
  
  if (
      ((APP_CallAction == APP_CALL_SENDING) || (BT_CallStatus == BT_CALL_OUTGOING)) && 
      (button != HANDSET_Button_UP) && 
      (button != HANDSET_Button_DOWN)
      ) {
    
    // Most button handling is disabled while an outgoing call is pending,
    // except for the button handling above and button handling below for:
    // - UP/DOWN to adjust volume
    return;
  }

  if (event->button != HANDSET_Button_PWR) {
    switch (appState) {
      case APP_State_PROGRAMMING: 
        PROGRAMMING_HANDSET_EventHandler(event);
        return;

      case APP_State_SOUND_TEST: 
        SOUND_TEST_HANDSET_EventHandler(event);
        return;

      case APP_State_ADJUST_VOLUME:
        VOLUME_ADJUST_HANDSET_EventHandler(event);
        return;
        
      case APP_State_SELECT_RINGTONE:
        RINGTONE_SELECT_HANDSET_EventHandler(event);
        return;
      
      case APP_State_ADJUST_VIEW_ANGLE:
        VIEW_ADJUST_HANDSET_EventHandler(event);
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
        
      case APP_State_SET_BT_DEVICE_NAME:
        STRING_INPUT_HANDSET_EventHandler(event);
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
        // Immediately handle this button press as char input
        CHAR_INPUT_HANDSET_EventHandler(event);
        return;
      } else if ((!isDirectoryScanNameMode && HANDSET_IsButtonPrintable(button)) || (button == HANDSET_Button_STO)) {
        returnToNumberInput(false);
        // button will be processed below
      } else if(button == HANDSET_Button_CLR) {
        SOUND_PlayButtonBeep(button, false);
        // Prevent short hold of CLR from clearing input
        HANDSET_CancelCurrentButtonHoldEvents();
        returnToNumberInput(false);
        return;
      } else if(button == HANDSET_Button_SEND) {
        if ((BT_CallStatus == BT_CALL_IDLE) && (APP_CallAction == APP_CALL_IDLE) && !callFailedTimer) {
          SOUND_PlayButtonBeep(button, false);
          returnToNumberInput(false);
          NumberInput_CallCurrentNumber();
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
      case APP_State_VOICE_COMMAND:  
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
      !callFailedTimer && (BT_CallStatus == BT_CALL_IDLE) && (APP_CallAction == APP_CALL_IDLE)
  ) {
    STORAGE_GetSpeedDial(button - HANDSET_Button_P1, tempNumberBuffer);

    if (tempNumberBuffer[0]) {
      SOUND_PlayButtonBeep(button, false);
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
                
                bool isOneMinuteBeepEnabled = !STORAGE_GetOneMinuteBeepEnabled();
                STORAGE_SetOneMinuteBeepEnabled(isOneMinuteBeepEnabled);
                
                CALL_TIMER_DisableDisplayUpdate();
                HANDSET_DisableTextDisplay();
                HANDSET_ClearText();
                HANDSET_PrintString("ONE MINBEEP O");
                HANDSET_PrintChar(isOneMinuteBeepEnabled ? 'N' : 'F');
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

                  uint8_t index = STORAGE_GetFirstEmptyDirectoryIndex();

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
                startAlphaStoreNameInput(true);
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

                if (rclOrStoAddr < STORAGE_DIRECTORY_SIZE) {
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
            } else if ((button == HANDSET_Button_ASTERISK) && !strpbrk(numberInput, "PM")) {
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

                if (rclOrStoAddr < STORAGE_DIRECTORY_SIZE) {
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
              STORAGE_GetOwnNumber(STORAGE_GetActiveOwnNumberIndex(), ownNumber);
              
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
          } else if (button == HANDSET_Button_CLR) {
            isRclInputPending = false;

            if (CALL_TIMER_IsDisplayEnabled()) {
              SOUND_PlayButtonBeep(button, false);
              HANDSET_CancelCurrentButtonHoldEvents();
              hideCallTimer();
            } else if (isNumberInput && !numberInputLength && (BT_CallStatus >= BT_CALL_ACTIVE)) {
              SOUND_PlayButtonBeep(button, false);
              HANDSET_CancelCurrentButtonHoldEvents();
              isCallTimerDisplayedByDefault = true;
              CALL_TIMER_EnableDisplayUpdate();
            } else if (isNumberInput && numberInputLength) {
              SOUND_PlayButtonBeep(button, false);
              NumberInput_PopDigit();
              
              if (!numberInputLength && (BT_CallStatus >= BT_CALL_ACTIVE)) {
                isCallTimerDisplayedByDefault = true;
                CALL_TIMER_EnableDisplayUpdate();
              } else {
                NumberInput_PrintToHandset();
              }
            }
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
                ATCMD_SendDTMFDigit(button);
              }
            }

            if (!isNumberInput) {
              appState = APP_State_NUMBER_INPUT;
            }
            
            if (numberInputIsStale) {
              NumberInput_Clear();
            }
            
            bool const redraw = numberInputLength == 0 
                || wasDisplayingNonNumberInput 
                || CALL_TIMER_IsDisplayEnabled();

            NumberInput_PushDigit(button);
            
            if (redraw) {
              NumberInput_PrintToHandset();
            } else {
              HANDSET_PrintChar(button);
            }
          } else if (button == HANDSET_Button_SEND) {
            if ((BT_CallStatus >= BT_CALL_ACTIVE) && (APP_CallAction == APP_CALL_IDLE)) {
              if (NumberInput_SendNextDTMFString()) {
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
            NumberInput_Clear();
            
            if (BT_CallStatus >= BT_CALL_ACTIVE) {
              isCallTimerDisplayedByDefault = true;
              CALL_TIMER_EnableDisplayUpdate();
            } else {
              NumberInput_PrintToHandset();
            }
          } else if((button == HANDSET_Button_RCL) && (event->holdDuration == HANDSET_HoldDuration_LONG) && isRclInputPending) {
            isRclInputPending = false;
            recallNumberInputOverflow(RECALL_FROM_INPUT_MANUAL);
          } else if ((button == HANDSET_Button_FCN) && (event->holdDuration == HANDSET_HoldDuration_VERY_LONG) && isFcn) {
            SOUND_StopButtonBeep();
            numberInputIsStale = true;
            TIMEOUT_Cancel(&fcnTimeout);
            resetFcn();
            CALL_TIMER_DisableDisplayUpdate();
            VIEW_ADJUST_Start(handleReturnFromSubModule);
            appState = APP_State_ADJUST_VIEW_ANGLE;
          }
          break; 
      }
      break;
    }
    
    case APP_State_PAIRING:
      if (isButtonDown && (button == HANDSET_Button_CLR)) {
        SOUND_PlayButtonBeep(button, false);
        BT_ExitPairingMode();
        BT_LinkBackToLastDevice();
        INDICATOR_StopSignalStrengthSweep(0);
        INDICATOR_StartFlashing(HANDSET_Indicator_SIGNAL_BARS);
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
          TIMEOUT_Start(&callActionTimeout, CALL_ACTION_TIMEOUT);
          return;
        }
      } else if ((event->type == HANDSET_EventType_HOOK) && !event->isOnHook && (BT_CallStatus == BT_CALL_INCOMING)) {
        TIMEOUT_Cancel(&autoAnswerTimeout);
        BT_AcceptCall();
        APP_CallAction = APP_CALL_ACCEPTING;
        TIMEOUT_Start(&callActionTimeout, CALL_ACTION_TIMEOUT);
      }
      break;
      
    case APP_State_ALPHA_STO_NAME_INPUT:
      STRING_INPUT_HANDSET_EventHandler(event);
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
              startAlphaStoreNameInput(false);
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

            uint8_t index = STORAGE_GetFirstEmptyDirectoryIndex();

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

              if (rclOrStoAddr < STORAGE_DIRECTORY_SIZE) {
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
      if (isButtonDown && (button == HANDSET_Button_CLR)) {
        SOUND_PlayButtonBeep(button, false);
        HANDSET_SetIndicator(HANDSET_Indicator_FCN, false);
        // Prevent short hold of CLR from clearing input
        HANDSET_CancelCurrentButtonHoldEvents();
        returnToNumberInput(false);
      } else {
        CHAR_INPUT_HANDSET_EventHandler(event);
      }
      break;
    
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
            ATCMD_SendDTMFDigitString(tempNumberBuffer);
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
        returnToNumberInput(false);
      }
      reportBatteryLevelToBT();
      break;
      
    case TRANSCEIVER_EventType_CONNECTED_TO_EXTERNAL_POWER: 
      wakeUpHandset(true);
      break;
      
    case TRANSCEIVER_EventType_DISCONNECTED_FROM_EXTERNAL_POWER:
      sleepHandset();
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
      
      if ((appState == APP_State_PROGRAMMING) || (appState == APP_State_SOUND_TEST) || (appState == APP_State_REBOOT)) {
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
      INDICATOR_StopFlashing(HANDSET_Indicator_SIGNAL_BARS, false);
      INDICATOR_StartSignalStrengthSweep();
      break;

    case BT_EVENT_SYS_PAIRING_FAILED:
      printf("[PAIR] Failed\r\n");
      INDICATOR_StopSignalStrengthSweep(0);
      INDICATOR_StartFlashing(HANDSET_Indicator_SIGNAL_BARS);
      TIMEOUT_Start(&idleTimeout, IDLE_TIMEOUT);
      
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
      
      playBluetoothConnectionStatusBeep(true);
      
      INDICATOR_StopFlashing(HANDSET_Indicator_SIGNAL_BARS, false);
      INDICATOR_StopSignalStrengthSweep(cellPhoneState.signalStrength);
      HANDSET_SetIndicator(HANDSET_Indicator_NO_SVC, !cellPhoneState.hasService);

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
      playBluetoothConnectionStatusBeep(false);
     
      INDICATOR_StartFlashing(HANDSET_Indicator_SIGNAL_BARS);
      HANDSET_SetIndicator(HANDSET_Indicator_NO_SVC, true);
      HANDSET_SetIndicator(HANDSET_Indicator_ROAM, false);
      HANDSET_SetIndicator(HANDSET_Indicator_IN_USE, false);
      cellPhoneState.isConnected = false;
      cellPhoneState.isScoConnected = false;
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
      cellPhoneState.isScoConnected = true;
      deferredSetAecEnabled();
      break;
      
    case BT_EVENT_SCO_DISCONNECTED:
      printf("[SCO] Disconnected\r\n");
      cellPhoneState.isScoConnected = false;
      cancelDeferredSetAecEnabled();
      break;
      
    case BT_EVENT_PHONE_SERVICE_STATUS:
      printf("[PHONE] %s\r\n", para ? "Has Service" : "No Service");
      playStatusBeep();
      cellPhoneState.hasService = (bool)para;
      HANDSET_SetIndicator(HANDSET_Indicator_NO_SVC, !cellPhoneState.hasService);
      
      if (!cellPhoneState.hasService) {
        cellPhoneState.signalStrength = 0;
        HANDSET_SetSignalStrength(0);
      } else if (cellPhoneState.signalStrength) {
        HANDSET_SetSignalStrength(cellPhoneState.signalStrength);
      }
      break;
      
    case BT_EVENT_PHONE_ROAMING_STATUS:  
      printf("[PHONE] %s\r\n", para ? "Roaming" : "Not Roaming");
      playStatusBeep();
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
      if ((BT_CallStatus != BT_CALL_IDLE) && (BT_CallStatus != BT_CALL_INCOMING)) {
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
  }
}

void handle_ATCMD_UnsolicitedResult(char const* result) {
  printf("[UNSCOLICITED AT RESULT] %s\r\n", result);
}
