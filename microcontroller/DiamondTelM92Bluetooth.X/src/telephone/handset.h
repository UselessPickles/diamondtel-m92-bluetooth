/** 
 * @file
 * @author Jeff Lau
 * 
 * Tools for communicating with the handset of the DiamondTel Model 92 phone.
 */

#ifndef HANDSET_H
#define	HANDSET_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdbool.h>    
#include <stdint.h>
    
/**
 * The total length of the handset's text display.
 */  
#define HANDSET_TEXT_DISPLAY_LENGTH (14)

/**
 * The number of rows of characters on the handset's text display.
 */
#define HANDSET_TEXT_DISPLAY_ROWS (2)

/**
 * The number of columns of characters on the handset's text display.
 */
#define HANDSET_TEXT_DISPLAY_COLUMNS (7)
  
  
/**
 * Max value for the handset signal strength indicator.
 */  
#define HANDSET_MAX_SIGNAL_STRENGTH (6)  
  
/**
 * Max value for handset LCD viewing angle.
 */  
#define HANDSET_MAX_LCD_VIEW_ANGLE (7)  
  
/**
 * UART events that can be received from the handset.
 */  
typedef enum HANDSET_UartEvent {
  HANDSET_UartEvent_RELEASE = 0x04,

  HANDSET_UartEvent_POUND = 0x23,
  HANDSET_UartEvent_ASTERISK = 0x2A,

  HANDSET_UartEvent_0 = 0x30,
  HANDSET_UartEvent_1,
  HANDSET_UartEvent_2,
  HANDSET_UartEvent_3,
  HANDSET_UartEvent_4,
  HANDSET_UartEvent_5,
  HANDSET_UartEvent_6,
  HANDSET_UartEvent_7,
  HANDSET_UartEvent_8,
  HANDSET_UartEvent_9,

  HANDSET_UartEvent_UP,
  HANDSET_UartEvent_DOWN,

  HANDSET_UartEvent_CLR = 0x80,
  HANDSET_UartEvent_FCN,
  HANDSET_UartEvent_P1,
  HANDSET_UartEvent_P2,
  HANDSET_UartEvent_P3,
  HANDSET_UartEvent_STO,
  HANDSET_UartEvent_RCL,
  HANDSET_UartEvent_MUTE,
  HANDSET_UartEvent_END,
  HANDSET_UartEvent_SEND,

  HANDSET_UartEvent_ON_HOOK,
  HANDSET_UartEvent_OFF_HOOK,
      
  HANDSET_UartEvent_CLR_0 = 0x90,
  HANDSET_UartEvent_CLR_1,
  HANDSET_UartEvent_CLR_2,
  HANDSET_UartEvent_CLR_3,
  HANDSET_UartEvent_CLR_4,
  HANDSET_UartEvent_CLR_5,
  HANDSET_UartEvent_CLR_6,
  HANDSET_UartEvent_CLR_7,
  HANDSET_UartEvent_CLR_8,
  HANDSET_UartEvent_CLR_9,
  HANDSET_UartEvent_CLR_ASTERISK,
  HANDSET_UartEvent_CLR_POUND,
  HANDSET_UartEvent_CLR_P1,
  HANDSET_UartEvent_CLR_P2,
  HANDSET_UartEvent_CLR_P3,
  HANDSET_UartEvent_CLR_STO,
  HANDSET_UartEvent_CLR_RCL,
  HANDSET_UartEvent_CLR_MUTE,
  HANDSET_UartEvent_CLR_END,
  HANDSET_UartEvent_CLR_SEND,
  HANDSET_UartEvent_CLR_UP =0xAA,
  HANDSET_UartEvent_CLR_DOWN
} HANDSET_UartEvent;   

/**
 * UART commands that can be sent to the handset.
 */
typedef enum HANDSET_UartCmd {
  HANDSET_UartCmd_SET_PRINT_POS_0 = 0x01,
  HANDSET_UartCmd_SET_PRINT_POS_1,
  HANDSET_UartCmd_SET_PRINT_POS_2,
  HANDSET_UartCmd_SET_PRINT_POS_3,
  HANDSET_UartCmd_SET_PRINT_POS_4,
  HANDSET_UartCmd_SET_PRINT_POS_5,
  HANDSET_UartCmd_SET_PRINT_POS_6,
  HANDSET_UartCmd_SET_PRINT_POS_7,
  HANDSET_UartCmd_SET_PRINT_POS_8,
  HANDSET_UartCmd_SET_PRINT_POS_9,
  HANDSET_UartCmd_SET_PRINT_POS_10,
  HANDSET_UartCmd_SET_PRINT_POS_11,
  HANDSET_UartCmd_SET_PRINT_POS_12,
  HANDSET_UartCmd_SET_PRINT_POS_13,

  HANDSET_UartCmd_PRINT_SPACE = 0x20,
  HANDSET_UartCmd_PRINT_EXCLAMATION_MARK,
  HANDSET_UartCmd_PRINT_DOUBLE_QUOTE,
  HANDSET_UartCmd_PRINT_POUND,
  HANDSET_UartCmd_PRINT_DOLLAR_SIGN,
  HANDSET_UartCmd_PRINT_PERCENT_SIGN,
  HANDSET_UartCmd_PRINT_AMPERSAND,
  HANDSET_UartCmd_PRINT_APOSTROPHE,
  HANDSET_UartCmd_PRINT_LEFT_PAREN,
  HANDSET_UartCmd_PRINT_RIGHT_PAREN,
  HANDSET_UartCmd_PRINT_ASTERISK,
  HANDSET_UartCmd_PRINT_PLUS_SIGN,
  HANDSET_UartCmd_PRINT_COMMA,
  HANDSET_UartCmd_PRINT_HYPHEN,
  HANDSET_UartCmd_PRINT_PERIOD,
  HANDSET_UartCmd_PRINT_SLASH,
  HANDSET_UartCmd_PRINT_0,
  HANDSET_UartCmd_PRINT_1,
  HANDSET_UartCmd_PRINT_2,
  HANDSET_UartCmd_PRINT_3,
  HANDSET_UartCmd_PRINT_4,
  HANDSET_UartCmd_PRINT_5,
  HANDSET_UartCmd_PRINT_6,
  HANDSET_UartCmd_PRINT_7,
  HANDSET_UartCmd_PRINT_8,
  HANDSET_UartCmd_PRINT_9,
  HANDSET_UartCmd_PRINT_COLON,
  HANDSET_UartCmd_PRINT_SEMICOLON,
  HANDSET_UartCmd_PRINT_LESS_THAN_SIGN,
  HANDSET_UartCmd_PRINT_EQUAL_SIGN,
  HANDSET_UartCmd_PRINT_GREATER_THAN_SIGN,
  HANDSET_UartCmd_PRINT_QUESTION_MARK,
  HANDSET_UartCmd_PRINT_AT_SIGN,
  HANDSET_UartCmd_PRINT_A,
  HANDSET_UartCmd_PRINT_B,
  HANDSET_UartCmd_PRINT_C,
  HANDSET_UartCmd_PRINT_D,
  HANDSET_UartCmd_PRINT_E,
  HANDSET_UartCmd_PRINT_F,
  HANDSET_UartCmd_PRINT_G,
  HANDSET_UartCmd_PRINT_H,
  HANDSET_UartCmd_PRINT_I,
  HANDSET_UartCmd_PRINT_J,
  HANDSET_UartCmd_PRINT_K,
  HANDSET_UartCmd_PRINT_L,
  HANDSET_UartCmd_PRINT_M,
  HANDSET_UartCmd_PRINT_N,
  HANDSET_UartCmd_PRINT_O,
  HANDSET_UartCmd_PRINT_P,
  HANDSET_UartCmd_PRINT_Q,
  HANDSET_UartCmd_PRINT_R,
  HANDSET_UartCmd_PRINT_S,
  HANDSET_UartCmd_PRINT_T,
  HANDSET_UartCmd_PRINT_U,
  HANDSET_UartCmd_PRINT_V,
  HANDSET_UartCmd_PRINT_W,
  HANDSET_UartCmd_PRINT_X,
  HANDSET_UartCmd_PRINT_Y,
  HANDSET_UartCmd_PRINT_Z,
  HANDSET_UartCmd_PRINT_LEFT_BRACKET,
  HANDSET_UartCmd_PRINT_YEN_SIGN,
  HANDSET_UartCmd_PRINT_RIGHT_BRACKET,
  HANDSET_UartCmd_PRINT_CARET,
  HANDSET_UartCmd_PRINT_UNDERSCORE,
  HANDSET_UartCmd_PRINT_ALPHA,
  HANDSET_UartCmd_PRINT_a,
  HANDSET_UartCmd_PRINT_b,
  HANDSET_UartCmd_PRINT_c,
  HANDSET_UartCmd_PRINT_d,
  HANDSET_UartCmd_PRINT_e,
  HANDSET_UartCmd_PRINT_f,
  HANDSET_UartCmd_PRINT_g,
  HANDSET_UartCmd_PRINT_h,
  HANDSET_UartCmd_PRINT_i,
  HANDSET_UartCmd_PRINT_j,
  HANDSET_UartCmd_PRINT_k,
  HANDSET_UartCmd_PRINT_l,
  HANDSET_UartCmd_PRINT_m,
  HANDSET_UartCmd_PRINT_n,
  HANDSET_UartCmd_PRINT_o,
  HANDSET_UartCmd_PRINT_p,
  HANDSET_UartCmd_PRINT_q,
  HANDSET_UartCmd_PRINT_r,
  HANDSET_UartCmd_PRINT_s,
  HANDSET_UartCmd_PRINT_t,
  HANDSET_UartCmd_PRINT_u,
  HANDSET_UartCmd_PRINT_v,
  HANDSET_UartCmd_PRINT_w,
  HANDSET_UartCmd_PRINT_x,
  HANDSET_UartCmd_PRINT_y,
  HANDSET_UartCmd_PRINT_z,
  HANDSET_UartCmd_PRINT_LEFT_BRACE,
  HANDSET_UartCmd_PRINT_PIPE,
  HANDSET_UartCmd_PRINT_RIGHT_BRACE,
  HANDSET_UartCmd_PRINT_RIGHT_ARROW,
      
  HANDSET_UartCmd_DELETE_ALL_TEXT,
      
  HANDSET_UartCmd_INDICATOR_ROAM_ON,
  HANDSET_UartCmd_INDICATOR_ROAM_OFF,
  HANDSET_UartCmd_INDICATOR_PWR_ON,
  HANDSET_UartCmd_INDICATOR_PWR_OFF,
  HANDSET_UartCmd_INDICATOR_FCN_ON,
  HANDSET_UartCmd_INDICATOR_FCN_OFF,
  HANDSET_UartCmd_INDICATOR_HORN_ON,
  HANDSET_UartCmd_INDICATOR_HORN_OFF,
  HANDSET_UartCmd_INDICATOR_MUTE_ON,
  HANDSET_UartCmd_INDICATOR_MUTE_OFF,
  HANDSET_UartCmd_INDICATOR_NO_ON,
  HANDSET_UartCmd_INDICATOR_NO_OFF,

  HANDSET_UartCmd_INDICATOR_SVC_ON = 0x8E,
  HANDSET_UartCmd_INDICATOR_SVC_OFF,
      
  HANDSET_UartCmd_SIGNAL_STRENGTH_BAR_1_ON,
  HANDSET_UartCmd_SIGNAL_STRENGTH_BAR_1_OFF,
  HANDSET_UartCmd_SIGNAL_STRENGTH_BAR_2_ON,
  HANDSET_UartCmd_SIGNAL_STRENGTH_BAR_2_OFF,
  HANDSET_UartCmd_SIGNAL_STRENGTH_BAR_3_ON,
  HANDSET_UartCmd_SIGNAL_STRENGTH_BAR_3_OFF,
  HANDSET_UartCmd_SIGNAL_STRENGTH_BAR_4_ON,
  HANDSET_UartCmd_SIGNAL_STRENGTH_BAR_4_OFF,
  HANDSET_UartCmd_SIGNAL_STRENGTH_BAR_5_ON,
  HANDSET_UartCmd_SIGNAL_STRENGTH_BAR_5_OFF,
  HANDSET_UartCmd_SIGNAL_STRENGTH_BAR_6_ON,
  HANDSET_UartCmd_SIGNAL_STRENGTH_BAR_6_OFF,
      
  HANDSET_UartCmd_INDICATOR_IN_USE_ON,
  HANDSET_UartCmd_INDICATOR_IN_USE_OFF,

  HANDSET_UartCmd_BACKLIGHT_ON = 0xAC,
  HANDSET_UartCmd_BACKLIGHT_OFF,

  HANDSET_UartCmd_MASTER_AUDIO_ON = 0xB4,
  HANDSET_UartCmd_MASTER_AUDIO_OFF,
  HANDSET_UartCmd_TEXT_DISPLAY_ON,
  HANDSET_UartCmd_TEXT_DISPLAY_OFF,
  HANDSET_UartCmd_BLINKING_TEXT_ON,
  HANDSET_UartCmd_BLINKING_TEXT_OFF,
  HANDSET_UartCmd_ALL_TEXT_PIXELS_ON,
  HANDSET_UartCmd_ALL_TEXT_PIXELS_OFF,
  HANDSET_UartCmd_ALL_INDICATORS_ON,
  HANDSET_UartCmd_ALL_INDICATORS_OFF,
  HANDSET_UartCmd_MICROPHONE_ON,
  HANDSET_UartCmd_MICROPHONE_OFF,
      
  HANDSET_UartCmd_PRINT_SMALL_UP_ARROW_OUTLINE,
  HANDSET_UartCmd_PRINT_SMALL_DOWN_ARROW_OUTLINE,
  HANDSET_UartCmd_PRINT_SMALL_UP_ARROW,
  HANDSET_UartCmd_PRINT_SMALL_DOWN_ARROW,
  HANDSET_UartCmd_PRINT_LARGE_UP_ARROW,
  HANDSET_UartCmd_PRINT_LARGE_DOWN_ARROW,
  HANDSET_UartCmd_PRINT_RECTANGLE,
  HANDSET_UartCmd_PRINT_RECTANGLE_STRIPED,
      
  HANDSET_UartCmd_SET_LCD_ANGLE_0,
  HANDSET_UartCmd_SET_LCD_ANGLE_1,
  HANDSET_UartCmd_SET_LCD_ANGLE_2,
  HANDSET_UartCmd_SET_LCD_ANGLE_3,
  HANDSET_UartCmd_SET_LCD_ANGLE_4,
  HANDSET_UartCmd_SET_LCD_ANGLE_5,
  HANDSET_UartCmd_SET_LCD_ANGLE_6,
  HANDSET_UartCmd_SET_LCD_ANGLE_7,
      
  HANDSET_UartCmd_SHOW_CURSOR_POS_0,
  HANDSET_UartCmd_SHOW_CURSOR_POS_1,
  HANDSET_UartCmd_SHOW_CURSOR_POS_2,
  HANDSET_UartCmd_SHOW_CURSOR_POS_3,
  HANDSET_UartCmd_SHOW_CURSOR_POS_4,
  HANDSET_UartCmd_SHOW_CURSOR_POS_5,
  HANDSET_UartCmd_SHOW_CURSOR_POS_6,
  HANDSET_UartCmd_SHOW_CURSOR_POS_7,
  HANDSET_UartCmd_SHOW_CURSOR_POS_8,
  HANDSET_UartCmd_SHOW_CURSOR_POS_9,
  HANDSET_UartCmd_SHOW_CURSOR_POS_10,
  HANDSET_UartCmd_SHOW_CURSOR_POS_11,
  HANDSET_UartCmd_SHOW_CURSOR_POS_12,
  HANDSET_UartCmd_SHOW_CURSOR_POS_13,
  HANDSET_UartCmd_HIDE_CURSOR,
      
  HANDSET_UartCmd_REQUEST_HOOK_STATUS,
      
  HANDSET_UartCmd_SET_SIGNAL_STRENGTH_0,
  HANDSET_UartCmd_SET_SIGNAL_STRENGTH_1,
  HANDSET_UartCmd_SET_SIGNAL_STRENGTH_2,
  HANDSET_UartCmd_SET_SIGNAL_STRENGTH_3,
  HANDSET_UartCmd_SET_SIGNAL_STRENGTH_4,
  HANDSET_UartCmd_SET_SIGNAL_STRENGTH_5,
  HANDSET_UartCmd_SET_SIGNAL_STRENGTH_6,

  HANDSET_UartCmd_LOUD_SPEAKER_ON = 0xEB,
  HANDSET_UartCmd_LOUD_SPEAKER_OFF,
  HANDSET_UartCmd_EAR_SPEAKER_ON,
  HANDSET_UartCmd_EAR_SPEAKER_OFF,      
} HANDSET_UartCmd;  
  
/**
 * Handset button values.
 * 
 * These all match the corresponding HANDSET_UartEvent button press 
 * values, except for HANDSET_Button_NONE and HANDSET_Button_PWR.
 * 
 * HANDSET_Button_NONE represents no button. Its value is zero.
 * 
 * HANDSET_Button_PWR is special in that the PWR button does not trigger a UART 
 * event, but instead pulls a circuit low. This detail is abstracted away so 
 * that the PWR button can be handled just like any other button.
 * The value of HANDSET_Button_PWR is arbitrary and not conflicting with any
 * UART-based buttons.
 */
typedef enum HANDSET_Button {
  HANDSET_Button_NONE = 0,

  HANDSET_Button_POUND = HANDSET_UartEvent_POUND,
  HANDSET_Button_ASTERISK = HANDSET_UartEvent_ASTERISK,
  HANDSET_Button_0 = HANDSET_UartEvent_0,
  HANDSET_Button_1 = HANDSET_UartEvent_1,
  HANDSET_Button_2 = HANDSET_UartEvent_2,
  HANDSET_Button_3 = HANDSET_UartEvent_3,
  HANDSET_Button_4 = HANDSET_UartEvent_4,
  HANDSET_Button_5 = HANDSET_UartEvent_5,
  HANDSET_Button_6 = HANDSET_UartEvent_6,
  HANDSET_Button_7 = HANDSET_UartEvent_7,
  HANDSET_Button_8 = HANDSET_UartEvent_8,
  HANDSET_Button_9 = HANDSET_UartEvent_9,
  HANDSET_Button_UP = HANDSET_UartEvent_UP,
  HANDSET_Button_DOWN = HANDSET_UartEvent_DOWN,
  HANDSET_Button_CLR = HANDSET_UartEvent_CLR,
  HANDSET_Button_FCN = HANDSET_UartEvent_FCN,
  HANDSET_Button_P1 = HANDSET_UartEvent_P1,
  HANDSET_Button_P2 = HANDSET_UartEvent_P2,
  HANDSET_Button_P3 = HANDSET_UartEvent_P3,
  HANDSET_Button_STO = HANDSET_UartEvent_STO,
  HANDSET_Button_RCL = HANDSET_UartEvent_RCL,
  HANDSET_Button_MUTE = HANDSET_UartEvent_MUTE,
  HANDSET_Button_END = HANDSET_UartEvent_END,
  HANDSET_Button_SEND = HANDSET_UartEvent_SEND,

  HANDSET_Button_CLR_0 = HANDSET_UartEvent_CLR_0,
  HANDSET_Button_CLR_1 = HANDSET_UartEvent_CLR_1,
  HANDSET_Button_CLR_2 = HANDSET_UartEvent_CLR_2,
  HANDSET_Button_CLR_3 = HANDSET_UartEvent_CLR_3,
  HANDSET_Button_CLR_4 = HANDSET_UartEvent_CLR_4,
  HANDSET_Button_CLR_5 = HANDSET_UartEvent_CLR_5,
  HANDSET_Button_CLR_6 = HANDSET_UartEvent_CLR_6,
  HANDSET_Button_CLR_7 = HANDSET_UartEvent_CLR_7,
  HANDSET_Button_CLR_8 = HANDSET_UartEvent_CLR_8,
  HANDSET_Button_CLR_9 = HANDSET_UartEvent_CLR_9,
  HANDSET_Button_CLR_ASTERISK = HANDSET_UartEvent_CLR_ASTERISK,
  HANDSET_Button_CLR_POUND = HANDSET_UartEvent_CLR_POUND,
  HANDSET_Button_CLR_P1 = HANDSET_UartEvent_CLR_P1,
  HANDSET_Button_CLR_P2 = HANDSET_UartEvent_CLR_P2,
  HANDSET_Button_CLR_P3 = HANDSET_UartEvent_CLR_P3,
  HANDSET_Button_CLR_STO = HANDSET_UartEvent_CLR_STO,
  HANDSET_Button_CLR_RCL = HANDSET_UartEvent_CLR_RCL,
  HANDSET_Button_CLR_MUTE = HANDSET_UartEvent_CLR_MUTE,
  HANDSET_Button_CLR_END = HANDSET_UartEvent_CLR_END,
  HANDSET_Button_CLR_SEND = HANDSET_UartEvent_CLR_SEND,
  HANDSET_Button_CLR_UP = HANDSET_UartEvent_CLR_UP,
  HANDSET_Button_CLR_DOWN = HANDSET_UartEvent_CLR_DOWN,
      
  HANDSET_Button_PWR = 0xFF,
} HANDSET_Button;

/**
 * Pre-defined button hold durations (in milliseconds) that trigger the 
 * HANDSET_EventType_BUTTON_HOLD event.
 */
typedef enum HANDSET_HoldDuration {
  HANDSET_HoldDuration_NONE = 0,
  HANDSET_HoldDuration_SHORT = 500,
  HANDSET_HoldDuration_LONG = 1000,
  HANDSET_HoldDuration_VERY_LONG = 2000,
  HANDSET_HoldDuration_MAX = 6000    
} HANDSET_HoldDuration;

/**
 * Types of events that can be triggered by this HANDSET module.
 */
typedef enum HANDSET_EventType {
  /**
   * A button has been pressed.
   * 
   * The `button` field of the event indicates which button was pressed.
   * 
   * See documentation of HANDSET_IsButtonDown() for details about the 
   * limitations and quirks of button state tracking.
   */
  HANDSET_EventType_BUTTON_DOWN,
  /**
   * A button has remained held down one of the pre-defined HANDSET_HoldDuration
   * values.
   * 
   * The `button` field of the event indicates which button is held.
   * 
   * The `holdDuration` field of the event indicates how long the button has
   * been held.
   * 
   * Button hold duration tracking is limited to one button at a time. For the 
   * limited button combinations that allow correct multi-button press state to 
   * be accurately tracked (e.g., FCN + any other single button, PWR + any other 
   * single button, etc.), the button hold duration tracking of the first button
   * is abandoned as soon as the second button is pressed. Only the most
   * recently pressed button has its hold duration tracked (and can trigger 
   * BUTTON_HOLD events). After releasing the second button, when the first
   * button is finally released, the reported hold duration on its BUTTON_UP 
   * event will be zero (0).
   * 
   * See documentation of HANDSET_IsButtonDown() for details about the 
   * limitations and quirks of button state tracking.
   */
  HANDSET_EventType_BUTTON_HOLD,
  /**
   * A button has been released.
   * 
   * The `button` field of the event indicates which button was released.
   * 
   * See documentation of HANDSET_IsButtonDown() for details about the 
   * limitations and quirks of button state tracking.
   */
  HANDSET_EventType_BUTTON_UP,
  /**
   * The handset on/off hook status has changed.
   * 
   * The `isOnHook` field of the event will contain the new status.
   */
  HANDSET_EventType_HOOK,
  /**
   * An unknown UART event was received.
   * 
   * The `button` field of the event will contain the raw value of the command.
   */
  HANDSET_EventType_UNKNOWN    
} HANDSET_EventType;

/**
 * Details about an event triggered by this HANDSET module.
 * 
 * The APP_HANDSET_EventHandler() function in app.c is called to handle
 * these events.
 */
typedef struct HANDSET_Event {
  /**
   * The type of event.
   */
  HANDSET_EventType type;
  /**
   * The button that is relevant to this event (if any).
   * 
   * The value is HANDSET_Button_NONE (0) if this is not a button-related event.
   * 
   * See documentation of HANDSET_IsButtonDown() for details about the 
   * limitations and quirks of button state tracking.
   */
  HANDSET_Button button;
  /**
   * The duration (in milliseconds) that the button has been held at the time 
   * of this event.
   * 
   * For the HANDSET_EventType_BUTTON_UP event, the value is the exact
   * duration that the button was held, up to HANDSET_HoldDuration_MAX.
   * A button held for longer than HANDSET_HoldDuration_MAX will report
   * that it was held for exactly HANDSET_HoldDuration_MAX. The value may 
   * be HANDSET_HoldDuration_NONE (0) if button hold tracking had been
   * abandoned for this button due to a multi-button press situation.
   * See HANDSET_EventType_BUTTON_HOLD documentation for more explanation.
   *
   * For the HANDSET_EventType_BUTTON_HOLD event, the value will match one
   * of the HANDSET_HoldDuration values.
   * 
   * For all other events, the value is HANDSET_HoldDuration_NONE (0) because it
   * is irrelevant.
   * 
   * See documentation of HANDSET_IsButtonDown() for details about the 
   * limitations and quirks of button state tracking.
   */
  uint16_t holdDuration;
  /**
   * True if the FCN button is down at the time of this event.
   * 
   * NOTE: When the event is a press/release of the FCN button itself, this
   *       will match the new state of the FCN button.
   * 
   * See documentation of HANDSET_IsButtonDown() for details about the 
   * limitations and quirks of button state tracking.
   */
  bool isFcn;
  /**
   * True if the handset is on-hook at the time of the event.
   * 
   * NOTE: When the event is a HOOK event, this will match the new on-hook
   *       status.
   * 
   * NOTE: This can possibly be out of sync with the physical handset in some
   *       situations that require a forced sync. See HANDSET_IsOnHook()
   *       and HANDSET_RequestHookStatus() for more details.
   */
  bool isOnHook;
} HANDSET_Event;

/**
 * HANDSET event handler function pointer.
 */
typedef void (*HANDSET_EventHandler)(HANDSET_Event const* event);

/**
 * Identifiers for referencing the status indicators on the handset display.
 */
typedef enum HANDSET_Indicator {
  HANDSET_Indicator_PWR,
  HANDSET_Indicator_FCN,
  HANDSET_Indicator_HORN,
  HANDSET_Indicator_MUTE,
  HANDSET_Indicator_IN_USE,
  HANDSET_Indicator_ROAM,
  HANDSET_Indicator_NO,
  HANDSET_Indicator_SVC,
  HANDSET_Indicator_SIGNAL_BAR_1,
  HANDSET_Indicator_SIGNAL_BAR_2,
  HANDSET_Indicator_SIGNAL_BAR_3,
  HANDSET_Indicator_SIGNAL_BAR_4,
  HANDSET_Indicator_SIGNAL_BAR_5,
  HANDSET_Indicator_SIGNAL_BAR_6,
  /** 
   * Special "synthetic" indicator to conveniently control 
   * both NO + SVC indicators together    
   */
  HANDSET_Indicator_NO_SVC
} HANDSET_Indicator;

/**
 * A subset of HANDSET_UartCmd values used for printing special symbol 
 * characters to the display. These values can be used in strings or as 
 * character values printed to the handset.
 */
typedef enum HANDSET_Symbol {
  HANDSET_Symbol_YEN_SIGN = HANDSET_UartCmd_PRINT_YEN_SIGN,
  HANDSET_Symbol_ALPHA = HANDSET_UartCmd_PRINT_ALPHA,
  HANDSET_Symbol_RIGHT_ARROW = HANDSET_UartCmd_PRINT_RIGHT_ARROW,
  HANDSET_Symbol_SMALL_UP_ARROW_OUTLINE = HANDSET_UartCmd_PRINT_SMALL_UP_ARROW_OUTLINE,
  HANDSET_Symbol_SMALL_DOWN_ARROW_OUTLINE = HANDSET_UartCmd_PRINT_SMALL_DOWN_ARROW_OUTLINE,
  HANDSET_Symbol_SMALL_UP_ARROW = HANDSET_UartCmd_PRINT_SMALL_UP_ARROW,
  HANDSET_Symbol_SMALL_DOWN_ARROW = HANDSET_UartCmd_PRINT_SMALL_DOWN_ARROW,
  HANDSET_Symbol_LARGE_UP_ARROW = HANDSET_UartCmd_PRINT_LARGE_UP_ARROW,
  HANDSET_Symbol_LARGE_DOWN_ARROW = HANDSET_UartCmd_PRINT_LARGE_DOWN_ARROW,
  HANDSET_Symbol_RECTANGLE = HANDSET_UartCmd_PRINT_RECTANGLE,
  HANDSET_Symbol_RECTANGLE_STRIPED = HANDSET_UartCmd_PRINT_RECTANGLE_STRIPED
} HANDSET_Symbol;

/**
 * Initialize the HANDSET module.
 * 
 * @param eventHandler - Event handler function pointer.
 */
void HANDSET_Initialize(HANDSET_EventHandler eventHandler);

/**
 * Main task loop behavior for the HANDSET module.
 * 
 * Must be called from the main task loop of the app.
 */
void HANDSET_Task(void);

/**
 * Timer event handler for the HANDSET module.
 * 
 * Must be called from a timer interrupt every 1 millisecond.
 */
void HANDSET_Timer1MS_Interrupt(void);

/**
 * Get the raw one-dimensional display position that corresponds to a 
 * 2-dimensional (col, row) coordinate, where col=0 is the left-most column and
 * row=0 is the top-most row.
 * 
 * The one-dimensional display position starts with 0 at the bottom right, and
 * increases right to left, wrapping back around to the right of the next row
 * above (the same as how normally printed text flows onto the display).
 * 
 * Returns 0xFF if the specified coordinate is out of bounds.
 * 
 * @param col - A column index (left column is 0).
 * @param row - A row index (top row is 0).
 * @return The display position of the specified coordinate, or 0xFF if the 
 *         coordinate is out of bounds.
 */
uint8_t HANDSET_GetDisplayPos(int8_t col, int8_t row);

/**
 * Sends a command to the Handset to force it to report its current
 * on-hook status.
 * 
 * This may cause the HANDSET_EventType_HOOK event to subsequently trigger 
 * if the handset reports a hook status different from the current assumed
 * hook status.
 * 
 * Use this to force synchronization of the handset's hook status any time that
 * it could be out of sync (e.g., after initial power-on or reset of the MCU).
 */
void HANDSET_RequestHookStatus(void);

/**
 * Check if the handset is currently on-hook.
 * 
 * This returns the opposite of HANDSET_IsOffHook().
 * 
 * NOTE: This is based on the local assumed state of the handset, which could 
 *       possibly be out of sync in some situations. See 
 *       HANDSET_RequestHookStatus().
 * 
 * @return True if the handset is currently on-hook.
 */
bool HANDSET_IsOnHook(void);

/**
 * Check if the handset is currently off-hook.
 * 
 * This returns the opposite of HANDSET_IsOnHook().
 * 
 * NOTE: This is based on the local assumed state of the handset, which could 
 *       possibly be out of sync in some situations. See 
 *       HANDSET_RequestHookStatus().
 * 
 * @return True if the handset is currently off-hook.
 */
bool HANDSET_IsOffHook(void);

/**
 * Test if a button is currently down/pressed.
 * 
 * BEWARE: Due to limitations in how the handset reports button 
 * presses/releases, our assumptions about the current state of the buttons at 
 * any given time may be inaccurate when simultaneous/overlapping button presses
 * occur.
 * 
 * The state of the PWR button is always correct, because it
 * is a separate direct hardware button that does not cause UART events.
 * 
 * The state of all other buttons is guaranteed to be correct when only one 
 * button is pressed at a time.
 * 
 * If the FCN button is pressed and held, then only one other non-PWR button
 * is pressed at a time, then button state is also guaranteed to be correct.
 * However, if the FCN button is released while another non-PWR button is held,
 * it will be incorrectly assumed that the other button was released and the 
 * FCN button is still pressed. 
 * 
 * If the CLR button is pressed and held, then only one other non-PWR button
 * is pressed at a time, special behavior occurs. Immediately upon pressing
 * the second button, a release of the CLR button is simulated and the CLR 
 * button is not considered to be down/held any more. Then, for every
 * non-PWR button that is pressed while the CLR button remains held, there
 * is a separate set of distinct button IDs (HANDSET_Button_CLR_*) that are 
 * considered to be pressed/released instead. This is because the handset 
 * actually sends unique events for these buttons being pressed while the 
 * CLR button is held. This is likely because holding CLR and pressing a button 
 * sequence was a way to access "secret"/maintenance modes/settings on the 
 * original phone, and having distinct UART event values for this situation 
 * guarantees that the button presses cannot be accidentally interpreted as
 * plain button presses. This same set of distinct button IDs is preserved for
 * this same use case in this implementation. 
 * 
 * When the CLR button is finally released after pressing/releasing other 
 * buttons, there will be no associated button event or change in button state 
 * because the CLR button had already previously be treated as if it were
 * released. If the CLR button is released first while still holding down 
 * another non-PWR button, then it will be incorrectly assumed that the other
 * button was released, and that no other buttons remain held down (because 
 * there was a previous simulated release of the CLR button).
 * 
 * The FCN button is the only non-PWR button that does not have a special
 * different button ID for when it is pressed while the CLR button is held. It
 * will still be treated as the FCN button itself being pressed/released.
 * 
 * Other combinations of multiple button presses produce very unusual and 
 * ambiguous UART events from the handset that make it impossible to know
 * which physical buttons are pressed/released at any given time, and can 
 * produce strange results until all buttons have been released.
 * 
 * A common use case of "two finger typing" on the keypad (where the key presses 
 * can briefly overlap) will generally produce correct overall behavior, despite
 * the exact button state being out of sync along the way. For example, if the
 * user performs the following:
 * - Press 1
 * - Press 2
 * - Release 1
 * - Release 2
 * It will cause the following sequence of button events:
 * - 1 Pressed
 * - 1 Released
 * - 2 Pressed
 * - 2 Released
 * That doesn't accurately match the physical button state, but succeeds at
 * typing a 1 followed by a 2 as the user intended. But if the user's actions 
 * change slightly to:
 * - Press 1
 * - Press 2
 * - Release 2
 * - Release 1
 * Then resulting sequence of button events is bizarre:
 * - 1 Pressed
 * - 1 Released
 * - 1 Pressed
 * - 1 Released
 * This is entirely a limitation of how the handset converts button 
 * presses/releases to UART events, and there's no way around it.
 * 
 * The UX of the phone app should be designed in a way that the user is never
 * tempted to press buttons in ways that cannot be processed properly.
 * 
 * @param button - The button to test.
 * @return True if the specified button is currently down/pressed.
 */
bool HANDSET_IsButtonDown(HANDSET_Button button);

/**
 * Test if any button is currently down/pressed.
 * 
 * See documentation of HANDSET_IsButtonDown() for details about the 
 * limitations and quirks of button state tracking.
 * 
 * @return True if any button is currently down/pressed.
 */
bool HANDSET_IsAnyButtonDown(void);

/**
 * Prevent any future BUTTON_HOLD events from triggering for the button
 * that is currently pressed.
 * 
 * When the button is released, its BUTTON_UP event will report a hold
 * duration of HANDSET_HoldDuration_NONE (0).
 */
void HANDSET_CancelCurrentButtonHoldEvents(void);

/**
 * Test if a button is a numeric button (0-9).
 * 
 * If true, then the specified button value is guaranteed to the ASCII
 * code for the corresponding number character.
 * 
 * NOTE: The special HANDSET_Button_CLR_* buttons are never considered to 
 * be "numeric", even if triggered by pressing  one of the numeric physical 
 * buttons while holding the CLR button.
 * 
 * @param button - The button to test.
 * @return True if the button is numeric (0-9).
 */
bool HANDSET_IsButtonNumeric(HANDSET_Button button);

/**
 * Test if a button is a printable character (0-9, *, #).
 * 
 * If true, then the specified button value is guaranteed to the ASCII
 * code for the corresponding character on the button.
 * 
 * NOTE: The special HANDSET_Button_CLR_* buttons are never considered to 
 * be "printable", even if triggered by pressing  one of the printable physical 
 * buttons while holding the CLR button.
 * 
 * @param button - The button to test.
 * @return True if the button is a printable character (0-9, *, #).
 */
bool HANDSET_IsButtonPrintable(HANDSET_Button button);

/**
 * Test if a button is one of the special CLR code buttons (a button that 
 * is pressed while the CLR button is held down). These button values are 
 * distinct from the normal button values for the same physical button when
 * the CLR button is not held down.
 * 
 * @param button - The button to test.
 * @return True if the button is one of the special CLR code buttons.
 */
bool HANDSET_IsButtonClrCode(HANDSET_Button button);

/**
 * Set the handset back-light on/off.
 * 
 * This command is optimized by HANDSET_EnableCommandOptimization().
 * 
 * @param on - True for "on". False for "off".
 */
void HANDSET_SetBacklight(bool on);

/**
 * Set the handset LCD display viewing angle.
 * 
 * Does nothing if the specified angle is out of range (0-7).
 * 
 * @param angle - The desired viewing angle (0-7).
 */
void HANDSET_SetLcdViewAngle(uint8_t angle);

/**
 * Disable the handset's text display.
 * 
 * This will hide all text on the display until re-enabled via 
 * HANDSET_EnableTextDisplay. Use this when making substantial updates to 
 * the displayed text so that the user does not see the text printing/updating 
 * one character at a time (printing characters is slow enough to be noticeable).
 * 
 * A counter is maintained of how many times this function is called, then 
 * HANDSET_EnableTextDisplay() must be called the same number of times to 
 * re-enable the text display (only command optimization is enabled
 * via HANDSET_EnableCommandOptimization()). This allows nested functions
 * in a call stack to each guarantee that the text display is disabled while 
 * making updates without prematurely re-enabling the text display.
 */
void HANDSET_DisableTextDisplay(void);

/**
 * Enabled the handset's text display.
 * 
 * If command optimization is enabled via HANDSET_EnableCommandOptimization(),
 * then this must be called as many times as HANDSET_DisableTextDisplay() was
 * called before the text display is actually re-enabled.
 * 
 * When command optimization is NOT enabled, this will unconditionally send the
 * command to the handset and reset the disabled counter.
 */
void HANDSET_EnableTextDisplay(void);

/**
 * Set text blinking on/off.
 * 
 * When on, all text on the handset's display will blink continuously, unless
 * the text display is disabled via HANDSET_DisableTextDisplay(). 
 * 
 * @param on - True for "on". False for "off".
 */
void HANDSET_SetTextBlink(bool on);

/**
 * Set or clear all pixels in the text display area.
 * 
 * @param on - True for "on". False for "off".
 */
void HANDSET_SetTextAllPixels(bool on);

/**
 * Set all status indicators on or off.
 * 
 * This command is optimized by HANDSET_EnableCommandOptimization().
 * 
 * @param on - True for "on". False for "off".
 */
void HANDSET_SetAllIndicators(bool on);

/**
 * Set a specified status indicator on or off.
 * 
 * This command is optimized by HANDSET_EnableCommandOptimization().
 * 
 * @param indicator - The indicator to set.
 * @param on - True for "on". False for "off".
 */
void HANDSET_SetIndicator(HANDSET_Indicator indicator, bool on);

/**
 * Set the signal strength indicators to indicate a specified
 * numeric strength (0-6).
 * 
 * A number of signal strength indicator bars matching the specified strength
 * will be turned on, from left to right. the remaining bars will be turned off.
 * 
 * If a value greater than the max signal strength (6) is provided, then this
 * will be treated as full signal strength (the same as 6).
 * 
 * @param signalStrength - The signal strength value (0-6).
 */
void HANDSET_SetSignalStrength(uint8_t signalStrength);

/**
 * Set all signal strength bars on/off to match the 6 least significant bits of 
 * the provided value. The least significant bit is represented by the 
 * right-most signal strength bar.
 * 
 * This command is optimized by HANDSET_EnableCommandOptimization().
 * 
 * @param bits - The value whose bits are to be presented via the signal 
 *        strength indicator.
 */
void HANDSET_SetSignalBarsAsBits(uint8_t bits);

/**
 * Set a single signal strength indicator bar on/off.
 * 
 * Index 0 maps to the left-most signal strength bar.
 * 
 * Index 5 maps to the right-most signal strength bar.
 * 
 * Out-of-range indexes are ignored.
 * 
 * This command is optimized by HANDSET_EnableCommandOptimization().
 * 
 * @param index - The index of the signal strength indicator bar to set.
 * @param on - True for "on". False for "off".
 */
void HANDSET_SetSignalBarAtIndex(uint8_t index, bool on);

/**
 * Clears all text on the display.
 */
void HANDSET_ClearText(void);

/**
 * Test if a character can be printed to the handset.
 * 
 * Most standard ASCII characters are printable.
 * 
 * There are some character values that are printable, but the printed 
 * character does not align with standard ASCII characters:
 * - '\' (5C) prints as ¥ (yen symbol)
 * - '`' (60) prints as ? (greek alpha)
 * - '~' (7E) prints as ? (right arrow)
 * - 'À' (C0) prints as a small up arrow (outline)
 * - 'Á' (C1) prints as a small down arrow (outline)
 * - 'Â' (C2) prints as a small up arrow (filled)
 * - 'Ã' (C3) prints as a small down arrow (filled)
 * - 'Ä' (C4) prints as a large up arrow (filled)
 * - 'Å' (C5) prints as a large down arrow (filled)
 * - 'Æ' (C6) prints as a solid rectangle
 * - 'Ç' (C7) prints as a horizontal striped rectangle
 * 
 * @param c - The character to test.
 * @return True if this character can be printed to the handset.
 */
bool HANDSET_IsCharPrintable(char c);

/**
 * Prints a single character to the display using "standard" printing.
 * 
 * The character is printed to the bottom right position (position 0) of the 
 * display, and all previously standard printed characters are pushed to the 
 * left, wrapping up to the top row, and eventually pushed off the top left of 
 * the display.
 * 
 * Any character that cannot be printed to the display will instead print
 * as a blank space character.
 * 
 * See HANDSET_IsCharPrintable() for more details about possible printable 
 * characters.
 * 
 * @param c - The character to print.
 */
void HANDSET_PrintChar(char c);

/**
 * Prints a single character multiple times to the display using "standard" 
 * printing.
 * 
 * The result is simular to calling HANDSET_PrintChar(c) `n` times (but with 
 * some optimizations).
 * 
 * See HANDSET_PrintChar() documentation for more details about "standard"
 * printing .
 * 
 * Any character that cannot be printed to the display will instead print
 * as a blank space character.
 * 
 * See HANDSET_IsCharPrintable() for more details about possible printable 
 * characters.
 * 
 * @param c - The character to print.
 * @param n - The number of times to print the character.
 */
void HANDSET_PrintCharN(char c, size_t n);

/**
 * Prints an entire null-terminated string to the display using "standard"
 * printing.
 * 
 * The result is simular to calling HANDSET_PrintChar(c) for each character in
 * the string (but with some optimizations).
 * 
 * See HANDSET_PrintChar() documentation for more details about "standard"
 * printing.
 * 
 * Any character that cannot be printed to the display will instead print
 * as a blank space character.
 * 
 * See HANDSET_IsCharPrintable() for more details about possible printable 
 * characters.
 * 
 * @param str - The string to print to the display.
 */
void HANDSET_PrintString(char const* str);

/**
 * Prints up to `n` characters of a null-terminated string to the display 
 * using "standard" printing.
 * 
 * The result is simular to calling HANDSET_PrintChar(c) for each character in
 * the string up to `n` characters, or the end of the string, whichever comes
 * first (but with some optimizations).
 * 
 * See HANDSET_PrintChar() documentation for more details about "standard"
 * printing.
 * 
 * Any character that cannot be printed to the display will instead print
 * as a blank space character.
 * 
 * See HANDSET_IsCharPrintable() for more details about possible printable 
 * characters.
 * 
 * @param str - The string to print to the display.
 * @param n - The max number of characters to print from the string.
 */
void HANDSET_PrintStringN(char const* str, size_t n);

/**
 * Prints a single character using "positional" printing, at a specific
 * position on the display.
 * 
 * The positions on display range from 0-13, starting at the bottom right,
 * increasing to the left, wrapping to the top right, then finally ending at
 * the top left.
 * 
 * If there is already a "standard" printed character at the same position,
 * then this new character will replace that standard printed character and 
 * further standard printing will push the character through display along with 
 * the surrounding standard printed text.
 * 
 * However, if standard printed text has not yet reached this position, then the
 * positional printed character will remain in its original position until
 * further standard printed text overwrites it.
 * 
 * Any character that cannot be printed to the display will instead print
 * as a blank space character.
 * 
 * See HANDSET_IsCharPrintable() for more details about possible printable 
 * characters.
 * 
 * @param c - The character to print.
 * @param pos - The position to print the character at.
 */
void HANDSET_PrintCharAt(char c, uint8_t pos);

/**
 * Prints a single character multiple times to the display using "positional"
 * printing, starting at a specific position on the display.
 * 
 * The result is similar to calling HANDSET_PrintCharAt(c, pos) `n` times,
 * with a decreasing `pos` value for each call.
 * 
 * See HANDSET_PrintCharAt() documentation for more details about "positional"
 * printing.
 * 
 * Any character that cannot be printed to the display will instead print
 * as a blank space character.
 * 
 * See HANDSET_IsCharPrintable() for more details about possible printable 
 * characters.
 * 
 * @param c - The character to print.
 * @param n - The number of times to print the character.
 * @param pos - The position to start printing at.
 */
void HANDSET_PrintCharNAt(char c, size_t n, uint8_t pos);

/**
 * Prints an entire null-terminated string to the display using "positional"
 * printing, starting at a specific position on the display.
 * 
 * The result is simular to calling HANDSET_PrintCharAt(c, pos) for each 
 * character in the string, with decreasing `pos` for each character 
 * (but with some optimizations).
 * 
 * See HANDSET_PrintCharAt() documentation for more details about "positional"
 * printing.
 * 
 * Any character that cannot be printed to the display will instead print
 * as a blank space character.
 * 
 * See HANDSET_IsCharPrintable() for more details about possible printable 
 * characters.
 * 
 * @param str - The string to print to the display.
 * @param pos - The position to start printing at.
 */
void HANDSET_PrintStringAt(char const* str, uint8_t pos);

/**
 * Prints up to `n` characters of a null-terminated string to the display 
 * using "positional" printing, starting at a specific position on the display.
 * 
 * The result is simular to calling HANDSET_PrintCharAt(c, pos) for each 
 * character in the string up to `n` characters, or the end of the string, 
 * whichever comes first, with decreasing `pos` for each character 
 * (but with some optimizations).
 * 
 * See HANDSET_PrintCharAt() documentation for more details about "positional"
 * printing.
 * 
 * Any character that cannot be printed to the display will instead print
 * as a blank space character.
 * 
 * See HANDSET_IsCharPrintable() for more details about possible printable 
 * characters.
 * 
 * @param str - The string to print to the display.
 * @param n - The max number of characters to print from the string.
 * @param pos - The position to start printing at.
 */
void HANDSET_PrintStringNAt(char const* str, size_t n, uint8_t pos);

/**
 * Display a flashing cursor a the specified display position.
 * 
 * The cursor can only be displayed at one position at a time. Subsequent
 * calls with this function with different position values will move the 
 * flashing cursor.
 * 
 * The cursor can only be displayed when the text display is enabled.
 * Disabling the text display will hide the cursor. Blinking text will also
 * hide the cursor.
 * 
 * Use HANDSET_ShowFlashingCursorAt() to hide the cursor.
 * 
 * @param pos - The position to display the flashing cursor at.
 */
void HANDSET_ShowFlashingCursorAt(uint8_t pos);

/**
 * Hides the flashing cursor that was displayed by HANDSET_ShowFlashingCursorAt().
 */
void HANDSET_HideFlashingCursor(void);

/**
 * Turns the handset's "master" audio system on or off.
 * 
 * The master audio must be enabled in order for any individual audio component 
 * to be fully enabled.
 * 
 * @param on - True for "on". False for "off".
 */
void HANDSET_SetMasterAudio(bool on);

/**
 * Turns the handset's microphone on or off.
 * 
 * The "master" audio must also be enabled via HANDSET_SetMasterAudio() for 
 * the microphone to function.
 *  
 * The microphone state is retained while the master audio is off.
 * 
 * @param on - True for "on". False for "off".
 */
void HANDSET_SetMicrophone(bool on);

/**
 * Turns the handset's loud speaker on or off.
 * 
 * The "master" audio must also be enabled via HANDSET_SetMasterAudio() for 
 * the loud speaker to function.
 *  
 * The loud speaker state is retained while the master audio is off.
 * 
 * @param on - True for "on". False for "off".
 */
void HANDSET_SetLoudSpeaker(bool on);

/**
 * Turns the handset's ear speaker on or off.
 * 
 * The "master" audio must also be enabled via HANDSET_SetMasterAudio() for 
 * the ear speaker to function.
 *  
 * The ear speaker state is retained while the master audio is off.
 * 
 * @param on - True for "on". False for "off".
 */
void HANDSET_SetEarSpeaker(bool on);

/**
 * Sends a raw UART command directly to the handset.
 * @param cmd - The UART command to send to the handset.
 */
void HANDSET_SendArbitraryCommand(uint8_t cmd);

/**
 * Waits for all pending UART commands to be sent to the handset before 
 * returning.
 * 
 * This is useful when you need to be able to make assumptions about the next 
 * handset command being sent immediately for timing purposes.
 */
void HANDSET_FlushWriteBuffer(void);

/**
 * Enables optimization of UART commands to the handset by avoiding sending 
 * unnecessary commands that would not change the state of the handset.
 * 
 * Optimization is disabled by default upon initialization.
 * 
 * This HANDSET module maintains the a partial representation of the assumed
 * state of the handset (based on what commands it has previously sent to the 
 * handset). The optimization may lead to incorrect behavior if this module's
 * representation of the handset state diverges from the actual handset state,
 * so it is important avoid the use of HANDSET_SendArbitraryCommand() in a
 * way that bypasses any of the more specific HANDSET functions.
 * 
 * Before enabling optimization, the application should initialize the handset 
 * to a complete known state (e.g., the boot sequence of the original phone).
 */
void HANDSET_EnableCommandOptimization(void);

/**
 * Disables optimization of UART commands. When disabled, then all HANDSET
 * functions will unconditionally send relevant UART commands to the handset 
 * regardless of whether this module believes it may be a wasteful command.
 * 
 * This may be useful if there is a situation where the handset's actual state 
 * may diverge from this module's representation of the assumed handset stated,
 * and it is necessary to force some commands to go through to re-synchronize.
 */
void HANDSET_DisableCommandOptimization(void);

#ifdef	__cplusplus
}
#endif

#endif	/* HANDSET_H */

