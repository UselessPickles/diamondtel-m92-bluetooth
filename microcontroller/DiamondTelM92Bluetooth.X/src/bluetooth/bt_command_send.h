/******************************************************************************
 Software License Agreement:

 The software supplied herewith by Microchip Technology Incorporated
 (the "Company") for its PICmicro(r) Microcontroller is intended and
 supplied to you, the Company's customer, for use solely and
 exclusively on Microchip PICmicro Microcontroller products. The
 software is owned by the Company and/or its supplier, and is
 protected under applicable copyright laws. All rights are reserved.
 Any use in violation of the foregoing restrictions may subject the
 user to criminal sanctions under applicable laws, as well as to
 civil liability for the breach of the terms and conditions of this
 license.

 THIS SOFTWARE IS PROVIDED IN AN "AS IS" CONDITION. NO WARRANTIES,
 WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
 TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
 IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
 CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
********************************************************************/
#ifndef BT_COMMAND_SEND_H
#define BT_COMMAND_SEND_H

#include <stdbool.h>
#include <stdint.h>

enum
{
  MAKE_CALL = 0x00,
  MAKE_EXTENSION_CALL = 0x01,
  MMI_CMD = 0x02,
  REPORT_MASK = 0x03,
  MUSIC_CONTROL = 0x04,
  CHANGE_DEVICE_NAME = 0x05,
  CHANGE_PIN_CODE = 0x06,
  SET_PAIRING_TIMER = 0x07,
  GET_UART_VERSION = 0x08,
  GET_PB_BY_AT_CMD = 0x09,
  VENDOR_AT_CMD = 0x0A,
  AVRCP_SPEC_CMD = 0x0B,
  AVRCP_GROUP_NAVIAGE = 0x0C,
  READ_LINK_STATUS = 0x0D,
  READ_PAIRING_RECORD = 0x0E,
  READ_LOCAL_BD_ADDR = 0x0F,
  READ_LOCAL_DEVICE_NAME = 0x10,
  SET_ACCESS_PB_METHOD = 0x11,            //Item 18
  SEND_SPP_DATA = 0x12,
  BTM_UTILITY_FUNCTION = 0x13,
  MCU_SEND_EVENT_ACK = 0x14,
  ADDITIONAL_PROFILE_LINK_SETUP = 0x15,
  READ_LINKED_DEVICE_INFOR = 0x16,
  PROFILE_LINK_BACK = 0x17,
  DISCONNECT = 0x18,
  MCU_STATUS_INDICATION = 0x19,
  USER_CONFIRM_SPP_REQ_REPLY = 0x1A,
  SET_HF_GAIN_LEVEL = 0x1B,
  EQ_MODE_SETTING = 0x1C,
  DSP_NR_CTRL = 0x1D,
  GPIO_CTRL = 0x1E,
  BT_MCU_UART_RX_BUFF_SIZE = 0x1F,
  VOICE_PROMPT_CMD = 0x20,
  MAP_REQUEST = 0x21,
  SECURITY_BONDING_REQ = 0x22,
  SET_OVERALL_GAIN = 0x23,
  REPORT_BATTERY_CAPACITY = 0x27,
  LE_SIGNALING = 0x29,
  READ_NSPK_LINK_STATUS = 0x2B,
  GATT_CTRL = 0x2D,
  DISCOVERABLE = 0x33,
  READ_LINK_MODE = 0x34,
  CONFIGURE_VENDOR_PARAMETER = 0x35,
  READ_FEATURE_LIST = 0x39,
  MOBILE_APP_MCU_PROTOCOL = 0xCC
};                    // uart_cmdset_type_en;

enum
{
  ADD_REMOVE_SCO_LINK = 0x01,
  FORCE_END_CALL = 0x02,
  ACCEPT_CALL = 0x04,
  REJECT_CALL = 0x05,
  ENDCALL_OR_TRANSFER_TO_HEADSET = 0x06,
  MIC_MUTE_TOGGLE = 0x07,
  MUTE_MIC = 0x08,
  UNMUTE_MIC  = 0x09,
  VOICE_DIAL  = 0x0A,
  CANCEL_VOICE_DIAL = 0x0B,
  LAST_NUMBER_REDIAL = 0x0C,
  ACTIVE_CALL_HOLD_ACCEPT_HELD_CALL = 0x0D,
  VOICE_TRANSFER = 0x0E,
  QUERY_CALL_LIST = 0x0F,
  THREE_WAY_CALL = 0x10,
  RELEASE_CALL = 0x11,
  ACCEPT_WAITING_HOLD_CALL_RLS_ACTIVE_CALL = 0x12,
  INITIAL_HF_CONNECTION = 0x16,
  DISCONNECT_HF_LINK = 0x17,
  ENABLE_RX_NR = 0x18,
  DISABLE_RX_NR = 0x19,
  TOGGLE_RX_NR = 0x1A,
  ENABLE_TX_NR = 0x1B,
  DISABLE_TX_NR = 0x1C,
  TOGGLE_TX_NR = 0x1D,
  ENABLE_AEC = 0x1E,
  DISABLE_AEC = 0x1F,
  TOGGLE_AEC = 0x20,
  ENABLE_AEC_RX_NR = 0x21,
  DISABLE_AEC_RX_NR = 0x22,
  TOGGLE_AEC_RX_NR = 0x23,
  INC_MIC_GAIN = 0x24,
  DEC_MIC_GAIN = 0x25,
  SWITCH_PRIMARY_SECONDARY_HF_DEVICE = 0x26,
  INC_SPK_GAIN = 0x30,
  DEC_SPK_GAIN = 0x31,
  INITIAL_A2DP_CONNECT_PLAY_PAUSE = 0x32,
  STOP_MEDIA = 0x33,
  NEXT_SONG = 0x34,
  PREVIOUS_SONG = 0x35,
  DISCONNECT_A2DP = 0x3B,
  STANDBY_ENTERING_PAIRING = 0x50,
  POWERON_BUTTON_PRESS = 0x51,
  POWERON_BUTTON_RELEASE = 0x52,
  POWEROFF_BUTTON_PRESS = 0x53,
  POWEROFF_BUTTON_RELEASE = 0x54,
  RESET_EEPROM_SETTING = 0x56,
  ANY_MODE_ENTERING_PAIRING = 0x5D,
  POWEROFF_BT = 0x5E,
  BUZZER_TOGGLE = 0x60,
  DISABLE_BUZZER = 0x61,
  ENABLE_BUZZER = 0x62,
  TONE_CHANGE = 0x63,
  RETRIEVE_PHONE_BOOK = 0x64,
  RETRIEVE_MISS_CALL_HISTORY = 0x65,
  RETRIEVE_RECEIVED_CALL_HISTORY = 0x66,
  RETRIEVE_DIALED_CALL_HISTORY = 0x67,
  RETRIEVE_ALL_CALL_HISTORY = 0x68,
  CANCLE_RETRIEVE = 0x69,
  INDICATE_BATTERY_STATUS = 0x6A,
  EXIT_PAIRING_MODE = 0x6B,
  LINK_BACK_DEVICE = 0x6C,
  DISCONNECT_ALL_LINK = 0x6D,
  MASTERSPK_ENTER_CSB_PAGE = 0xE0,          //r3
  SLAVESPK_ENTER_CSB_PAGESCAN = 0xE1,          //r3
  NSPK_ADD_SPEAKER = 0xE2,
  MASTERSPK_TERMINAL_CSB = 0xE5,          //r3
  MASTERSPK_ENTER_AUXMODE = 0xE7,          //r3
  MASTERSPK_EXIT_AUXMODE = 0xE8,          //r3
  POWER_OFF_ALL_SPK = 0xED,
  MASTERSPK_REPAIR_TO_SLAVE = 0xF0
};            //uart_mmi_cmdset_type_en;

#define BT_MAX_DEVICE_NAME_LENGTH (23)

void BT_GiveUpThisCommand( void );
uint8_t BT_CalculateCmdChecksum(uint8_t const* startByte, uint8_t const* endByte);
void BT_SendBytesAsCompleteCommand(uint8_t const* command, uint8_t command_length);

void BT_ResetEEPROM(void);
void BT_SetEventMask(void);
void BT_ReadLinkedDeviceName(void);
void BT_ReadDeviceName(void);
bool BT_SetDeviceName(char const* name);
void BT_StartVoiceCommand(void);
void BT_CancelVoiceCommand(void);
bool BT_MakeCall(char const* number);
void BT_SetMicrohponeMuted(bool isMuted);
void BT_EndCall(void);
void BT_AcceptCall(void);
void BT_RejectCall(void);
void BT_SwapHoldOrWaitingCall(void);
void BT_SwapHoldOrWaitingCallAndEndActiveCall(void);
void BT_EndHoldOrWaitingCall(void);
void BT_EnterPairingMode(void);
void BT_ExitPairingMode(void);

void BT_MMI_ActionCommand(uint8_t MMI_ActionCode, uint8_t link_index);
void BT_SendAckToEvent(uint8_t ack_event);
void BT_LinkBackToLastDevice(void);
void BT_CancelLinkback(void);
void BT_DisconnectAllProfile(void);
void BT_SetHFPGain(uint8_t gain);

void BT_SetAecEnabled(bool isEnabled);

void BT_CommandSend_Initialize(void);
void BT_CommandSend_Task( void );
void BT_CommandSend_Timer1MS_Interrupt(void);

void UART_TransferFirstByte( void );
void UART_TransferNextByte( void );

//-----------------------------------------
typedef enum {
    STS_OK,
    ERROR_DISALLOWED,
    ERROR_COMMAND_UNKNOW,
    ERROR_PARA_ERROR,
    ERROR_BTM_BUSY,
    ERROR_OUT_OF_MEMORY,
    ERROR_NO_ACK = 0x10,
    IN_QUEUE = 0x20,
    IN_SENDING = 0x21,
} CMD_PROCESSING_STATUS;

enum
{
	CMD_INFO_IGNORE=0,
	CMD_INFO_MCU=1,
	CMD_INFO_APPS=2
};

uint8_t BT_UpdateAckStatusWhenReceived(uint8_t command_id, uint8_t ack_status) ;
void BT_UpdateAckStatusWhenSent(uint8_t command_id) ;
uint8_t BT_GetAckStatusByCommandID(uint8_t command_id);


void BT_Send_ActionCommand(uint8_t MMI_ActionCode, uint8_t link_index, uint8_t cmd_info);


#endif
