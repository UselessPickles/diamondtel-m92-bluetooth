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
#ifndef BT_COMMAND_DECODE_H
#define BT_COMMAND_DECODE_H

#include <stdbool.h>
#include <stdint.h>

// @ event define
enum {
    BT_EVENT_NONE = 0,

    BT_EVENT_CMD_SENT_ACK_OK,       //ACK OK
    BT_EVENT_CMD_SENT_ACK_ERROR,        //ACK error
    BT_EVENT_CMD_SENT_NO_ACK,           //no ACK
    
    BT_EVENT_PHONE_MAX_SIGNAL_STRENGTH,
    BT_EVENT_PHONE_SIGNAL_STRENGTH,
    
    BT_EVENT_PHONE_MAX_BATTERY_LEVEL,
    BT_EVENT_PHONE_BATTERY_LEVEL,
    
    BT_EVENT_PHONE_ROAMING_STATUS,
    BT_EVENT_PHONE_SERVICE_STATUS,
    
    BT_EVENT_CALLER_ID,
    BT_EVENT_MISSED_CALL,
    
    BT_EVENT_NSPK_STATUS,
    BT_EVENT_LINE_IN_STATUS,
    BT_EVENT_A2DP_STATUS,
    BT_EVENT_CALL_STATUS_CHANGED,

    BT_EVENT_HFP_CONNECTED,
    BT_EVENT_HFP_DISCONNECTED,
    BT_EVENT_A2DP_CONNECTED,
    BT_EVENT_A2DP_DISCONNECTED,
    BT_EVENT_AVRCP_CONNECTED,
    BT_EVENT_AVRCP_DISCONNECTED,
    BT_EVENT_SPP_CONNECTED,
    BT_EVENT_IAP_CONNETED,
    BT_EVENT_SPP_IAP_DISCONNECTED,
    BT_EVENT_ACL_CONNECTED,
    BT_EVENT_ACL_DISCONNECTED,
    BT_EVENT_SCO_CONNECTED,
    BT_EVENT_SCO_DISCONNECTED,
    BT_EVENT_MAP_CONNECTED,
    BT_EVENT_MAP_DISCONNECTED,

    BT_EVENT_SYS_POWER_ON,
    BT_EVENT_SYS_POWER_OFF,
    BT_EVENT_SYS_STANDBY,
    BT_EVENT_SYS_PAIRING_START,
    BT_EVENT_SYS_PAIRING_OK,
    BT_EVENT_SYS_PAIRING_FAILED,

    BT_EVENT_LINKBACK_SUCCESS,
    BT_EVENT_LINKBACK_FAILED,

    BT_EVENT_BD_ADDR_RECEIVED,
    BT_EVENT_LINKED_NAME_RECEIVED,
    BT_EVENT_NAME_RECEIVED,
    BT_EVENT_PAIR_RECORD_RECEIVED,
    BT_EVENT_LINK_MODE_RECEIVED,

    BT_EVENT_PLAYBACK_STATUS_CHANGED,
    BT_EVENT_AVRCP_VOLUME_CTRL,
    BT_EVENT_AVRCP_ABS_VOLUME_CHANGED,
    BT_EVENT_HFP_VOLUME_CHANGED,
	BT_EVENT_TYPE_CODEC,

    
    NSPK_EVENT_SYNC_POWER_OFF,
    NSPK_EVENT_SYNC_VOL_CTRL,
    NSPK_EVENT_SYNC_INTERNAL_GAIN,
    NSPK_EVENT_SYNC_ABS_VOL,
    NSPK_EVENT_CHANNEL_SETTING,
    NSPK_EVENT_ADD_SPEAKER3,
    
    LE_STATUS_CHANGED,
    LE_ADV_CONTROL_REPORT,
    LE_CONNECTION_PARA_REPORT,
    LE_CONNECTION_PARA_UPDATE_RSP,
    GATT_ATTRIBUTE_DATA,
    
    PORT0_INPUT_CHANGED,
    PORT1_INPUT_CHANGED,
    PORT2_INPUT_CHANGED,
    PORT3_INPUT_CHANGED,

	NSPK_EVENT_VENDOR_CMD,
	NSPK_EVENT_VOL_MODE,
	NSPK_EVENT_EQ_MODE,	
	FEATURE_LIST_REPLY

};

typedef enum LINKBACK_Type {
  LINKBACK_Type_ACL,
  LINKBACK_Type_HFP,
  LINKBACK_Type_A2DP,
  LINKBACK_Type_SPP
} LINKBACK_Type;

typedef enum
{
	APPS_GET_AUXIN_STATUS,		
	APPS_GET_POWER_STATUS,		
	APPS_SET_AUDIO_SRC,
	APPS_SET_INDIV_MMI_ACTION = 0x7A

}APPS_STATUS;

uint8_t BT_linkIndex;

void BT_CommandHandler(void);
void BT_CommandDecode( void );

void BT_CommandDecodeInit(void);
void BT_CommandDecodeMain(void);
void BT_CommandDecode1MS_event( void );

#ifdef DATABASE2_TEST       //test only
bool ParsePayloadAsCommand(uint8_t* command, uint8_t length);
#endif

#endif