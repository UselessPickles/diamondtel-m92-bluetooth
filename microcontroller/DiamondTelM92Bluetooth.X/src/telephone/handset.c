/** 
 * @file
 * @author Jeff Lau
 */

#include "handset.h"
#include "../../mcc_generated_files/uart1.h"
#include "../../mcc_generated_files/uart3.h"
#include "../../mcc_generated_files/tmr2.h"
#include "../../mcc_generated_files/pin_manager.h"
#include <string.h>
#include <stdio.h>

/* 
 * Number of indicators defined by HANDSET_Indicator 
 * (not including the special HANDSET_Indicator_NO_SVC)
 */ 
#define INDICATOR_COUNT (14)

/*
 * Lookup table of HANDSET_Indicator -> UART command for turning the 
 * indicator on.
 * (not including the special HANDSET_Indicator_NO_SVC)
 * 
 * Add one (1) to the command to get the "off" command.
 */
static const HANDSET_UartCmd indicatorCmdLookup[INDICATOR_COUNT] = {
  HANDSET_UartCmd_INDICATOR_PWR_ON,
  HANDSET_UartCmd_INDICATOR_FCN_ON,
  HANDSET_UartCmd_INDICATOR_HORN_ON,
  HANDSET_UartCmd_INDICATOR_MUTE_ON,
  HANDSET_UartCmd_INDICATOR_IN_USE_ON,
  HANDSET_UartCmd_INDICATOR_ROAM_ON,
  HANDSET_UartCmd_INDICATOR_NO_ON,
  HANDSET_UartCmd_INDICATOR_SVC_ON,
  HANDSET_UartCmd_SIGNAL_STRENGTH_BAR_1_ON,
  HANDSET_UartCmd_SIGNAL_STRENGTH_BAR_2_ON,
  HANDSET_UartCmd_SIGNAL_STRENGTH_BAR_3_ON,
  HANDSET_UartCmd_SIGNAL_STRENGTH_BAR_4_ON,
  HANDSET_UartCmd_SIGNAL_STRENGTH_BAR_5_ON,
  HANDSET_UartCmd_SIGNAL_STRENGTH_BAR_6_ON,
};

#define BLANK_PRINTABLE_CHAR (' ')

/**
 * Module state.
 */
static struct {
  /**
   * Event handler function pointer.
   */
  HANDSET_EventHandler eventHandler;
  /**
   * The character that is currently printed at position 0.
   * 
   * This is needed to work around some bugs with positioning a
   * flashing cursor on the Handset.
   * 
   * See HANDSET_ShowFlashingCursorAt() and HANDSET_HideFlashingCursor()
   * implementations.
   */
  char charAtPos0;
  /**
   * True if the Handset is currently "on-hook".
   */
  bool isOnHook;
  /**
   * True if the PWR button is currently down.
   */
  bool isPwrButtonDown;
  /**
   * True if the FCN button is currently down.
   * 
   * (...to the best of our knowledge based on limited info from the Handset
   *  and assumptions about how users would make use of the FCN button
   *  in multi-button press situations)
   */
  bool isFcnButtonDown;
  /**
   * The current button that is down, aside from the PWR button.
   */
  HANDSET_Button currentNonPwrButtonDown;
  /**
   * The current button that is down, which was the last button pressed.
   * 
   * This is the button we are currently tracking duration for.
   */
  volatile HANDSET_Button currentButtonDown;
  /**
   * The current duration (in milliseconds) that the current button has been 
   * held down.
   * 
   * HANDSET_Timer1MS_Interrupt() event increments this value while 
   * `currentButtonDown` is populated.
   */
  volatile uint16_t currentButtonDownDuration;
  /**
   * The current button hold duration threshold that has been recently 
   * exceeded and is queued up for triggering the HANDSET_EventType_BUTTON_HOLD
   * event.
   * 
   * HANDSET_Timer1MS_Interrupt() sets this value as each HANDSET_HoldDuration 
   * threshold is met, and HANDSET_Task() triggers the event and clears out
   * the value.
   */
  volatile HANDSET_HoldDuration currentButtonHold;
  /**
   * If true, then several Handset commands will be optimized by maintaining
   * local state in sync with what the Handset state is believed to be, and
   * ignoring various HANDSET function calls that would not change the Handset 
   * state.
   * 
   * If true, then all commands will be unconditionally passed along to the 
   * Handset.
   */
  bool isCommandOptimizationEnabled;
  /**
   * A count of how many times HANDSET_DisableTextDisplay() has been called.
   * 
   * HANDSET_EnableTextDisplay() must be called this many times to enable
   * the text display.
   */
  uint8_t textDisplayDisableCount;
  /**
   * True if text blinking is currently enabled.
   */
  bool isTextBlinkOn;
  /**
   * True if the Handset backlight is currently on.
   */
  bool isBacklightOn;
  /**
   * True if the handset master audio circuit is on.
   */
  bool isMasterAudioOn;
  /**
   * True if the handset loud speaker is on.
   * 
   * NOTE: It is not truly on unless the master audio circuit is also on.
   */
  bool isLoudSpeakerOn;
  /**
   * True if the handset ear speaker is on.
   * 
   * NOTE: It is not truly on unless the master audio circuit is also on.
   */
  bool isEarSpeakerOn;
  /**
   * True if the handset microphone is on.
   * 
   * NOTE: It is not truly on unless the master audio circuit is also on.
   */
  bool isMicrophoneOn;
  /*
   * Mapping of HANDSET_Indicator -> current indicator "on" state.
   * (not including the special HANDSET_Indicator_NO_SVC)
   */
  bool indicatorState[INDICATOR_COUNT];
} handset;

static char const* getButtonName(HANDSET_Button button) {
  static char unknown[] = "Unknown (?)";
  
  switch (button) {
    case HANDSET_Button_NONE:
      return "[none]";
    case HANDSET_Button_MUTE:
      return "MUTE";
    case HANDSET_Button_STO:
      return "STO";
    case HANDSET_Button_FCN:
      return "FCN";
    case HANDSET_Button_RCL:
      return "RCL";
    case HANDSET_Button_CLR:
      return "CLR";
    case HANDSET_Button_SEND:
      return "SEND";
    case HANDSET_Button_END:
      return "END";
    case HANDSET_Button_0:
      return "0";
    case HANDSET_Button_1:
      return "1";
    case HANDSET_Button_2:
      return "2";
    case HANDSET_Button_3:
      return "3";
    case HANDSET_Button_4:
      return "4";
    case HANDSET_Button_5:
      return "5";
    case HANDSET_Button_6:
      return "6";
    case HANDSET_Button_7:
      return "7";
    case HANDSET_Button_8:
      return "8";
    case HANDSET_Button_9:
      return "9";
    case HANDSET_Button_ASTERISK:
      return "*";
    case HANDSET_Button_POUND:
      return "#";
    case HANDSET_Button_P1:
      return "P1";
    case HANDSET_Button_P2:
      return "P2";
    case HANDSET_Button_P3:
      return "P3";
    case HANDSET_Button_UP:
      return "UP";
    case HANDSET_Button_DOWN:
      return "DOWN";
    case HANDSET_Button_CLR_MUTE:
      return "CLR+MUTE";
    case HANDSET_Button_CLR_STO:
      return "CLR+STO";
    case HANDSET_Button_CLR_RCL:
      return "CLR+RCL";
    case HANDSET_Button_CLR_SEND:
      return "CLR+SEND";
    case HANDSET_Button_CLR_END:
      return "CLR+END";
    case HANDSET_Button_CLR_0:
      return "CLR+0";
    case HANDSET_Button_CLR_1:
      return "CLR+1";
    case HANDSET_Button_CLR_2:
      return "CLR+2";
    case HANDSET_Button_CLR_3:
      return "CLR+3";
    case HANDSET_Button_CLR_4:
      return "CLR+4";
    case HANDSET_Button_CLR_5:
      return "CLR+5";
    case HANDSET_Button_CLR_6:
      return "CLR+6";
    case HANDSET_Button_CLR_7:
      return "CLR+7";
    case HANDSET_Button_CLR_8:
      return "CLR+8";
    case HANDSET_Button_CLR_9:
      return "CLR+9";
    case HANDSET_Button_CLR_ASTERISK:
      return "CLR+*";
    case HANDSET_Button_CLR_POUND:
      return "CLR+#";
    case HANDSET_Button_CLR_P1:
      return "CLR+P1";
    case HANDSET_Button_CLR_P2:
      return "CLR+P2";
    case HANDSET_Button_CLR_P3:
      return "CLR+P3";
    case HANDSET_Button_CLR_UP:
      return "CLR+UP";
    case HANDSET_Button_CLR_DOWN:
      return "CLR+DOWN";
    case HANDSET_Button_PWR:
      return "PWR";
    default: 
      unknown[9] = button;
      return unknown;
  }
}

static char const* const eventTypeNames[] = {
  "Button Down",
  "Button Hold",
  "Button Up",
  "Hook On/Off",
  "Unknown"
};

static void printEvent(HANDSET_Event const* event) {
  printf(
      "[HANDSET Event] %s\tbtn: %s\tfn: %u\thook: %u\thold: %u\r\n", 
      eventTypeNames[event->type], 
      getButtonName(event->button),
      event->isFcn,
      event->isOnHook,
      event->holdDuration
      );
}

/**
 * Helper function for dispatching a Handset event. 
 * 
 * This function sets the common fields (isFcn, isOnHook) based on
 * the current Handset state.
 * 
 * The caller is responsible for setting all other fields of the event.
 * 
 * @param event - The event to dispatch.
 */
static void dispatchEvent(HANDSET_Event* event) {
  event->isFcn = handset.isFcnButtonDown;
  event->isOnHook = handset.isOnHook;
  
  handset.eventHandler(event);
  //printEvent(event);
}

/**
 * Use this to ensure that only printable characters are being sent to the 
 * Handset when intending to print text to the display.
 * 
 * Returns the provided character if it is printable by the Handset.
 * 
 * Otherwise, a blank space character is returned.
 * 
 * @param c - A potentially printable character.
 * @return A guaranteed printable character.
 */
static char ensurePrintableChar(char c) {
  if (HANDSET_IsCharPrintable(c)) {
    return c;
  } else {
    return BLANK_PRINTABLE_CHAR;
  }
}

/**
 * Validate that a display character is in the range of 0-13.
 * @param pos - A display character position.
 * @return True if the position is in the valid range of 0-13.
 */
static bool isValidCharPos(uint8_t pos) {
  return pos < HANDSET_TEXT_DISPLAY_LENGTH;
}

void HANDSET_Initialize(HANDSET_EventHandler eventHandler) {
  handset.eventHandler = eventHandler;
  handset.charAtPos0 = BLANK_PRINTABLE_CHAR;
  handset.isPwrButtonDown = true;
  handset.isOnHook = true;
}

void HANDSET_Task(void) {
  HANDSET_Event event;

  PIE3bits.TMR2IE = 0;
  uint16_t currentButtonDownDuration = (handset.currentButtonDownDuration > HANDSET_HoldDuration_MAX)
         ? HANDSET_HoldDuration_NONE 
         : handset.currentButtonDownDuration;

  uint16_t currentButtonHold = handset.currentButtonHold;
  handset.currentButtonHold = 0;  
  PIE3bits.TMR2IE = 1;

  if (currentButtonHold && handset.currentButtonDown) {
    event.button =  handset.currentButtonDown;
    event.type = HANDSET_EventType_BUTTON_HOLD;
    event.holdDuration = currentButtonHold;
    dispatchEvent(&event);
  }
  
  bool isPwrButtonDown = !IO_PWR_GetValue();
  
  if (isPwrButtonDown != handset.isPwrButtonDown) {
    if (isPwrButtonDown) {
      PIE3bits.TMR2IE = 0;
      handset.currentButtonDown = HANDSET_Button_PWR;
      handset.currentButtonDownDuration = HANDSET_HoldDuration_NONE;
      handset.currentButtonHold = 0;
      PIE3bits.TMR2IE = 1;

      event.holdDuration = HANDSET_HoldDuration_NONE;
    } else {
      if (handset.currentButtonDown == HANDSET_Button_PWR) {
        PIE3bits.TMR2IE = 0;
        handset.currentButtonDown = handset.currentNonPwrButtonDown;
        handset.currentButtonDownDuration = HANDSET_HoldDuration_MAX + 1;
        handset.currentButtonHold = 0;
        PIE3bits.TMR2IE = 1;

        event.holdDuration = currentButtonDownDuration;
      } else {
        event.holdDuration = HANDSET_HoldDuration_NONE; 
      }
    }
    
    handset.isPwrButtonDown = isPwrButtonDown;    

    event.button = HANDSET_Button_PWR;
    event.type = isPwrButtonDown 
        ? HANDSET_EventType_BUTTON_DOWN 
        : HANDSET_EventType_BUTTON_UP;
    dispatchEvent(&event);

  }
  
  // UART handset command pass-through for testing
  if (UART3_is_rx_ready()) {
    uint8_t cmd = UART3_Read();
    
    switch(cmd) {
      case HANDSET_UartCmd_BLINKING_TEXT_ON:
        HANDSET_SetTextBlink(true);
        break;
      case HANDSET_UartCmd_BLINKING_TEXT_OFF:
        HANDSET_SetTextBlink(false);
        break;
      case HANDSET_UartCmd_TEXT_DISPLAY_ON:
        HANDSET_EnableTextDisplay();
        break;
      case HANDSET_UartCmd_TEXT_DISPLAY_OFF:
        HANDSET_DisableTextDisplay();
        break;
      case HANDSET_UartCmd_HIDE_CURSOR:
        HANDSET_HideFlashingCursor();
        break;
        
      default:
        if ((cmd >= HANDSET_UartCmd_SHOW_CURSOR_POS_0) && (cmd <= HANDSET_UartCmd_SHOW_CURSOR_POS_13)) {
          HANDSET_ShowFlashingCursorAt(cmd - HANDSET_UartCmd_SHOW_CURSOR_POS_0);
        } else {
          HANDSET_SendArbitraryCommand(cmd);
        }
    }
    
  }
  
  if (UART1_is_rx_ready()) {
    uint8_t input = UART1_Read();
    
    switch (input) {
      case HANDSET_UartEvent_ON_HOOK:
      case HANDSET_UartEvent_OFF_HOOK:
        handset.isOnHook = (input == HANDSET_UartEvent_ON_HOOK);
//        printf("[HANDSET] %s Hook\r\n", handset.isOnHook ? "On" : "Off");
//        return;
        event.type = HANDSET_EventType_HOOK;
        event.button = HANDSET_Button_NONE;
        event.holdDuration = HANDSET_HoldDuration_NONE;
        dispatchEvent(&event);
        break;

      case HANDSET_UartEvent_RELEASE: 
//        printf("[HANDSET] Button Release\r\n");
//        return;
        if (handset.currentNonPwrButtonDown) {
          if (handset.currentNonPwrButtonDown == HANDSET_Button_FCN) {
            handset.isFcnButtonDown = false;
          }
          
          event.type = HANDSET_EventType_BUTTON_UP;
          event.button = handset.currentNonPwrButtonDown;

          if (handset.currentNonPwrButtonDown == handset.currentButtonDown) {
            PIE3bits.TMR2IE = 0;
            if (handset.isFcnButtonDown) {
              handset.currentButtonDown = handset.currentNonPwrButtonDown = HANDSET_Button_FCN;
            } else if (handset.isPwrButtonDown) {
              handset.currentNonPwrButtonDown = HANDSET_Button_NONE;
              handset.currentButtonDown = HANDSET_Button_PWR;
            } else {
              handset.currentButtonDown = handset.currentNonPwrButtonDown = HANDSET_Button_NONE;
            }

            handset.currentButtonDownDuration = HANDSET_HoldDuration_MAX + 1;
            handset.currentButtonHold = 0;
            PIE3bits.TMR2IE = 1;
            
            event.holdDuration = currentButtonDownDuration;
          } else {
            handset.currentNonPwrButtonDown = handset.isFcnButtonDown 
                ? HANDSET_Button_FCN
                : HANDSET_Button_NONE;

            event.holdDuration = HANDSET_HoldDuration_NONE;
          }

          dispatchEvent(&event);
        }
        break;

        
      default:
        if (
            // If the input is a button press...
            input == HANDSET_UartEvent_POUND || 
            input == HANDSET_UartEvent_ASTERISK || 
            ((input >= HANDSET_UartEvent_0) && (input <= HANDSET_UartEvent_DOWN)) ||
            ((input >= HANDSET_UartEvent_CLR) && (input <= HANDSET_UartEvent_SEND)) ||
            ((input >= HANDSET_UartEvent_CLR_0) && (input <= HANDSET_UartEvent_CLR_SEND)) ||
            ((input == HANDSET_UartEvent_CLR_UP) || (input == HANDSET_UartEvent_CLR_DOWN))
        ) {
//          printf("[HANDSET] Button Down: %s\r\n", HANDSET_GetButtonName(input));
//          return;
          
          if (handset.currentNonPwrButtonDown && (handset.currentNonPwrButtonDown != HANDSET_Button_FCN)) {
            event.type = HANDSET_EventType_BUTTON_UP;
            event.button = handset.currentNonPwrButtonDown;
            
            if (handset.currentButtonDown == handset.currentNonPwrButtonDown) {
              event.holdDuration = currentButtonDownDuration;
            } else {
              event.holdDuration = HANDSET_HoldDuration_NONE;
            }

            dispatchEvent(&event);
          }
          
          if (input == HANDSET_Button_FCN) {
            handset.isFcnButtonDown = true;
          }

          event.type = HANDSET_EventType_BUTTON_DOWN;
          event.button = input;
          event.holdDuration = HANDSET_HoldDuration_NONE;

          PIE3bits.TMR2IE = 0;
          handset.currentNonPwrButtonDown = handset.currentButtonDown = input;
          handset.currentButtonDownDuration = HANDSET_HoldDuration_NONE;
          handset.currentButtonHold = 0;
          PIE3bits.TMR2IE = 1;

          dispatchEvent(&event);
        } else {
          printf("[HANDSET] Unknown Event: %c\r\n", input);
          return;

          event.type = HANDSET_EventType_UNKNOWN;
          event.button = input;
          event.holdDuration = HANDSET_HoldDuration_NONE;
          dispatchEvent(&event);
        }
        break;
    }
  }
}

void HANDSET_Timer1MS_Interrupt(void) {
  // Increment the hold duration of the current button, but not beyond
  // the max duration.
  if (
      handset.currentButtonDown && 
      (handset.currentButtonDownDuration < HANDSET_HoldDuration_MAX)
    ) {
    ++handset.currentButtonDownDuration;
        
    // If the current duration matches one of the defined durations
    // for the BUTTON_HOLD event, then store it in handset.currentButtonHold
    // so that HANDSET_Task() knows to trigger the event. 
    switch(handset.currentButtonDownDuration) {
      case HANDSET_HoldDuration_VERY_SHORT:
      case HANDSET_HoldDuration_SHORT:
      case HANDSET_HoldDuration_LONG:
      case HANDSET_HoldDuration_VERY_LONG:
      case HANDSET_HoldDuration_MAX:
        handset.currentButtonHold = handset.currentButtonDownDuration;
        break;
    }
  }
}

uint8_t HANDSET_GetDisplayPos(int8_t x, int8_t y) {
  if (x < 0 || x >= HANDSET_TEXT_DISPLAY_COLUMNS) {
    return 0xFF;
  }
  
  if (y < 0 || y >= HANDSET_TEXT_DISPLAY_ROWS) {
    return 0xFF;
  }
  
  return (uint8_t)(
      (((HANDSET_TEXT_DISPLAY_ROWS - 1) - y) * HANDSET_TEXT_DISPLAY_COLUMNS) + 
      ((HANDSET_TEXT_DISPLAY_COLUMNS - 1) - x)
      );
}

void HANDSET_RequestHookStatus(void) {
  UART1_Write(HANDSET_UartCmd_REQUEST_HOOK_STATUS);
}

bool HANDSET_IsOnHook(void) {
  return handset.isOnHook;
}

bool HANDSET_IsOffHook(void) {
  return !handset.isOnHook;
}

bool HANDSET_IsButtonDown(HANDSET_Button button) {
  if (button == HANDSET_Button_FCN) {
    return handset.isFcnButtonDown;
  } else if (button == HANDSET_Button_PWR) {
    return handset.isPwrButtonDown;
  } else {
    return (button == handset.currentNonPwrButtonDown);
  }
}

bool HANDSET_IsAnyButtonDown(void) {
  return handset.currentButtonDown != HANDSET_Button_NONE;
}

void HANDSET_CancelCurrentButtonHoldEvents(void) {
  PIE3bits.TMR2IE = 0;
  handset.currentButtonDownDuration = HANDSET_HoldDuration_MAX + 1;
  PIE3bits.TMR2IE = 1;
}

bool HANDSET_IsButtonNumeric(HANDSET_Button button) {
  return (button >= HANDSET_Button_0) && (button <= HANDSET_Button_9);
}

bool HANDSET_IsButtonPrintable(HANDSET_Button button) {
  return button <= HANDSET_Button_9;
}

bool HANDSET_IsButtonClrCode(HANDSET_Button button) {
  return ((button >= HANDSET_Button_CLR_0) && (button <= HANDSET_Button_CLR_SEND)) ||
      ((button >= HANDSET_Button_CLR_UP) && (button <= HANDSET_Button_CLR_DOWN));
}

void HANDSET_SetBacklight(bool on) {
  if (handset.isCommandOptimizationEnabled && (on == handset.isBacklightOn)) {
    return;
  }
  
  handset.isBacklightOn = on;
  UART1_Write(on ? HANDSET_UartCmd_BACKLIGHT_ON : HANDSET_UartCmd_BACKLIGHT_OFF);
}

void HANDSET_SetLcdViewAngle(uint8_t angle) {
  if (angle <= HANDSET_MAX_LCD_VIEW_ANGLE) {
    UART1_Write(HANDSET_UartCmd_SET_LCD_ANGLE_0 + angle);
  }
}

void HANDSET_DisableTextDisplay(void) {
  if (!handset.textDisplayDisableCount++ || !handset.isCommandOptimizationEnabled) {
    if (handset.isTextBlinkOn) {
      UART1_Write(HANDSET_UartCmd_BLINKING_TEXT_OFF);
    }
    UART1_Write(HANDSET_UartCmd_TEXT_DISPLAY_OFF);
  }
}

void HANDSET_EnableTextDisplay(void) {
  if ((handset.textDisplayDisableCount == 1) || !handset.isCommandOptimizationEnabled) {
    handset.textDisplayDisableCount = 0;
    UART1_Write(HANDSET_UartCmd_TEXT_DISPLAY_ON);
    if (handset.isTextBlinkOn) {
      UART1_Write(HANDSET_UartCmd_BLINKING_TEXT_ON);
    }
  } else if (handset.textDisplayDisableCount) {
    --handset.textDisplayDisableCount;
  }
}

void HANDSET_SetTextBlink(bool on) {
  if (
      ((handset.isTextBlinkOn != on) && !handset.textDisplayDisableCount) || 
      !handset.isCommandOptimizationEnabled
      ) {
    UART1_Write(on ? HANDSET_UartCmd_BLINKING_TEXT_ON : HANDSET_UartCmd_BLINKING_TEXT_OFF);
    // This command forces the text display to be enabled, so keep the
    // disabled count in sync if this was called with command optimization
    // disabled.
    handset.textDisplayDisableCount = 0;
  }
  handset.isTextBlinkOn = on;
}

void HANDSET_SetTextAllPixels(bool on) {
  UART1_Write(on ? HANDSET_UartCmd_ALL_TEXT_PIXELS_ON : HANDSET_UartCmd_ALL_TEXT_PIXELS_OFF);
  handset.charAtPos0 = on ? HANDSET_Symbol_RECTANGLE : BLANK_PRINTABLE_CHAR;
}

void HANDSET_SetAllIndicators(bool on) {
  UART1_Write(on ? HANDSET_UartCmd_ALL_INDICATORS_ON : HANDSET_UartCmd_ALL_INDICATORS_OFF); 
  memset(handset.indicatorState, on, INDICATOR_COUNT);
}

void HANDSET_SetIndicator(HANDSET_Indicator indicator, bool on) {
  uint8_t offset = on ? 0 : 1;
  
  if (indicator == HANDSET_Indicator_NO_SVC) {
    // The NO_SVC indicator is a special "synthetic" indicator ID that maps
    // to bot the NO and SVC indicators simultaneously, so it must be converted 
    // to individual commands to turn each of the two indicators on/off.

    if (!handset.isCommandOptimizationEnabled || (on != handset.indicatorState[HANDSET_Indicator_NO])) {
      UART1_Write(HANDSET_UartCmd_INDICATOR_NO_ON + offset);
      handset.indicatorState[HANDSET_Indicator_NO] = on;
    }
    
    if (!handset.isCommandOptimizationEnabled || (on != handset.indicatorState[HANDSET_Indicator_SVC])) {
      UART1_Write(HANDSET_UartCmd_INDICATOR_SVC_ON + offset);
      handset.indicatorState[HANDSET_Indicator_SVC] = on;
    }
  } else if (!handset.isCommandOptimizationEnabled || (on != handset.indicatorState[indicator])) {
    // All indicators aside from NO_SVC are individual indicators according
    // to the handset.
    
    UART1_Write(indicatorCmdLookup[indicator] + offset);
    handset.indicatorState[indicator] = on;
  }
}

void HANDSET_SetSignalStrength(uint8_t signalStrength) {
  if (signalStrength > HANDSET_MAX_SIGNAL_STRENGTH) {  
    // Don't exceed the max valid value
    signalStrength = HANDSET_MAX_SIGNAL_STRENGTH;
  }
  
  UART1_Write(HANDSET_UartCmd_SET_SIGNAL_STRENGTH_0 + signalStrength);

  memset(
      handset.indicatorState + HANDSET_Indicator_SIGNAL_BAR_1, 
      true, 
      signalStrength
      );
  
  memset(
      handset.indicatorState + HANDSET_Indicator_SIGNAL_BAR_1 + signalStrength, 
      false, 
      HANDSET_MAX_SIGNAL_STRENGTH - signalStrength
      );
}

void HANDSET_SetSignalBarsAsBits(uint8_t bits) {
  for (uint8_t i = 0; i < HANDSET_MAX_SIGNAL_STRENGTH; ++i) {
    HANDSET_SetSignalBarAtIndex((HANDSET_MAX_SIGNAL_STRENGTH - 1) - i, bits & 1);
    bits >>= 1;
  }
}

void HANDSET_SetSignalBarAtIndex(uint8_t index, bool on) {
  if (index >= HANDSET_MAX_SIGNAL_STRENGTH) {
    // ignore out-of-range indexes
    return;
  }
  
  HANDSET_SetIndicator(HANDSET_Indicator_SIGNAL_BAR_1 + (HANDSET_Indicator)index, on);
}

void HANDSET_ClearText(void) {
  UART1_Write(HANDSET_UartCmd_DELETE_ALL_TEXT);
  handset.charAtPos0 = BLANK_PRINTABLE_CHAR;
}

bool HANDSET_IsCharPrintable(char c) {
  return       
      // This is the range of standard ASCII characters that the Handset 
      // can handle.
      ((c >= HANDSET_UartCmd_PRINT_SPACE) && (c <= HANDSET_UartCmd_PRINT_RIGHT_ARROW)) || 
      // This is the range of additional special symbol characters that the 
      // Handset can handle
      ((c >= HANDSET_UartCmd_PRINT_SMALL_UP_ARROW_OUTLINE) && (c <= HANDSET_UartCmd_PRINT_RECTANGLE_STRIPED));
}

void HANDSET_PrintChar(char c) {
  c = ensurePrintableChar(c);

  UART1_Write(c);
  handset.charAtPos0 = c;
}

void HANDSET_PrintCharN(char c, size_t n) {
  c = ensurePrintableChar(c);

  if (n > HANDSET_TEXT_DISPLAY_LENGTH) {
    n = HANDSET_TEXT_DISPLAY_LENGTH;
  }
  
  while(n > 0) {
    UART1_Write(c);
    --n;
  }
  
  handset.charAtPos0 = c;
}

void HANDSET_PrintString(char const* str) {
  size_t len = strlen(str);
  
  if (len > HANDSET_TEXT_DISPLAY_LENGTH) {
    // Only the last 14 characters will actually fit on the display, so
    // only print the last 14 to avoid unnecessary UART commands.
    str += (len - HANDSET_TEXT_DISPLAY_LENGTH);
  }
  
  while (*str) {
    HANDSET_PrintChar(*str++);
  }
}

void HANDSET_PrintStringN(char const* str, size_t n) {
  size_t len = strlen(str);
  
  if (len < n) {
    n = len;
  }
  
  if (n > HANDSET_TEXT_DISPLAY_LENGTH) {
    size_t excess = (n - HANDSET_TEXT_DISPLAY_LENGTH);
    n -= excess;
    str += excess;
  }
  
  while (*str && (n > 0)) {
    HANDSET_PrintChar(*str++);
    --n;
  }
}

void HANDSET_PrintCharAt(char c, uint8_t pos) {
  if (isValidCharPos(pos)) {
    c = ensurePrintableChar(c);

    UART1_Write(HANDSET_UartCmd_SET_PRINT_POS_0 + pos);
    UART1_Write(c);
    
    if (pos == 0) {
      handset.charAtPos0 = c;
    }
  }
}

void HANDSET_PrintCharNAt(char c, size_t n, uint8_t pos) {
  if (pos >=  HANDSET_TEXT_DISPLAY_LENGTH) {
    uint8_t excess = (pos - HANDSET_TEXT_DISPLAY_LENGTH) + 1;
    
    if (n <= excess) {
      return;
    }
    
    n -= excess;
    pos -= excess;
  }

  c = ensurePrintableChar(c);

  while(n > 0) {
    UART1_Write(HANDSET_UartCmd_SET_PRINT_POS_0 + pos);
    UART1_Write(c);
    
    if (pos == 0) {
      handset.charAtPos0 = c;
      // We can't print any more characters beyond this position, so break
      // out early.
      break;
    }
    
    --pos;
    --n;
  }
}

void HANDSET_PrintStringAt(char const* str, uint8_t pos) {
  while ((pos >= HANDSET_TEXT_DISPLAY_LENGTH) && *str) {
    --pos;
    ++str;
  }
  
  while (*str) {
    HANDSET_PrintCharAt(*str, pos);
    
    if (pos == 0) {
      // We can't print any more characters beyond this position, so break
      // out early.
      break;
    }
    
    --pos;
    ++str;
  }
}

void HANDSET_PrintStringNAt(char const* str, size_t n, uint8_t pos) {
  if (pos >=  HANDSET_TEXT_DISPLAY_LENGTH) {
    uint8_t excess = (pos - HANDSET_TEXT_DISPLAY_LENGTH) + 1;
    
    if (n <= excess) {
      return;
    }
    
    n -= excess;
    pos -= excess;
    
    while (excess && *str) {
      --excess;
      ++str;
    }
  }
  
  while(n && *str) {
    HANDSET_PrintCharAt(*str, pos);
    
    if (pos == 0) {
      // We can't print any more characters beyond this position, so break
      // out early.
      break;
    }
    
    --n;
    --pos;
    ++str;
  }
}

void HANDSET_ShowFlashingCursorAt(uint8_t pos) {
  if (
      !handset.isCommandOptimizationEnabled || 
      (!handset.textDisplayDisableCount && isValidCharPos(pos))
      ) {
    // Hack to work around bugs with commands for displaying/hiding the flashing 
    // cursor. It's unclear why, but first printing a character to the display,
    // then sending the command to hide the flashing cursor (in that order only),
    // somehow guarantees that the cursor can be reliably shown, then reliably 
    // repositioned or hidden later.
    UART1_Write(HANDSET_UartCmd_SET_PRINT_POS_0);
    UART1_Write(handset.charAtPos0);
    UART1_Write(HANDSET_UartCmd_HIDE_CURSOR);

    UART1_Write(HANDSET_UartCmd_SHOW_CURSOR_POS_0 + pos);
    // This command forces the text display to be enabled, so keep the
    // disabled count in sync if this was called with command optimization
    // disabled.
    handset.textDisplayDisableCount = 0;
  }
}

void HANDSET_HideFlashingCursor(void) {
  if (!handset.isCommandOptimizationEnabled || !handset.textDisplayDisableCount) {
    UART1_Write(HANDSET_UartCmd_HIDE_CURSOR);
    // This command forces the text display to be enabled, so keep the
    // disabled count in sync if this was called with command optimization
    // disabled.
    handset.textDisplayDisableCount = 0;
  }
}

void HANDSET_SetMasterAudio(bool on) {
  if (handset.isCommandOptimizationEnabled && (on == handset.isMasterAudioOn)) {
    return;
  }
  
  UART1_Write(on ? HANDSET_UartCmd_MASTER_AUDIO_ON : HANDSET_UartCmd_MASTER_AUDIO_OFF);
  handset.isMasterAudioOn = on;
}

void HANDSET_SetMicrophone(bool on) {
  if (handset.isCommandOptimizationEnabled && (on == handset.isMicrophoneOn)) {
    return;
  }
  
  UART1_Write(on ? HANDSET_UartCmd_MICROPHONE_ON : HANDSET_UartCmd_MICROPHONE_OFF);
  handset.isMicrophoneOn = on;
}

void HANDSET_SetLoudSpeaker(bool on) {
  if (handset.isCommandOptimizationEnabled && (on == handset.isLoudSpeakerOn)) {
    return;
  }
  
  UART1_Write(on ? HANDSET_UartCmd_LOUD_SPEAKER_ON : HANDSET_UartCmd_LOUD_SPEAKER_OFF);
  handset.isLoudSpeakerOn = on;
}

void HANDSET_SetEarSpeaker(bool on) {
  if (handset.isCommandOptimizationEnabled && (on == handset.isEarSpeakerOn)) {
    return;
  }
  
  UART1_Write(on ? HANDSET_UartCmd_EAR_SPEAKER_ON : HANDSET_UartCmd_EAR_SPEAKER_OFF);
  handset.isEarSpeakerOn = on;
}

void HANDSET_SendArbitraryCommand(uint8_t cmd) {
  UART1_Write(cmd);
}

void HANDSET_FlushWriteBuffer(void) {
  while ((uart1TxBufferRemaining != 64) && !UART1_is_tx_done());
}

void HANDSET_EnableCommandOptimization(void) {
  handset.isCommandOptimizationEnabled = true;
}

void HANDSET_DisableCommandOptimization(void) {
  handset.isCommandOptimizationEnabled = false;
}
