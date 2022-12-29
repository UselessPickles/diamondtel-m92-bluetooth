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
#include <xc.h>
#include "mcc_generated_files/uart2.h"
#include "bt_command_decode.h"
#include "bt_command_send.h"
#include "app.h"
#include "atcmd.h"

#define BT_CMD_SIZE_MAX				200

//command decode state machine
typedef enum {
	RX_DECODE_CMD_SYNC_AA,
	RX_DECODE_CMD_SYNC_00,
	RX_DECODE_CMD_LENGTH,
	RX_DECODE_CMD_DATA,
	RX_DECODE_CMD_CHECKSUM
} RX_DECODE_MODE;
typedef enum
{
	APPS_GET_STATUS,
	APPS_SET_STATUS
}APPS_COMMAND;

//event ID
enum            
{
  ACK = 0x00,
  DEVICE_STATE = 0x01,
  CALL_STATUS = 0x02,
  CALL_ID = 0x03,
  SMS_INDICATION = 0x04,
  MISS_CALL_INDICATION = 0x05,
  PHONE_MAX_BATTERY_LEVEL = 0x06,
  PHONE_BATTERY_LEVEL = 0x07,
  PHONE_ROAMING_STATUS = 0x08,
  PHONE_MAX_SIGNAL_STRENGTH = 0x09,
  PHONE_SIGNAL_STRENGTH = 0x0A,
  PHONE_SERVICE_STATUS = 0x0B,
  BATTERY_LEVEL = 0x0C,
  CHARGER_STATUS = 0x0D,
  RESET_TO_DEFAULT = 0x0E,
  VOLUME_LEVEL = 0x0F,
  EQ_MODE = 0x10,
  MISS_CALL_HISTORY = 0x11,
  RECEIVED_CALL_HISTORY = 0x12,
  DIALED_CALL_HISTORY = 0x13,
  COMBINE_CALL_HISTORY = 0x14,
  PHONE_BOOK = 0x15,
  ACCESS_FINISH = 0x16,
  REPORT_LINKED_DEVICE_INFO = 0x17,
  UART_VERSION = 0x18,
  CALL_LIST_REPORT = 0x19,
  AVRCP_SPEC_RSP = 0x1A,
  BTM_UTILITY_REQ = 0x1B,
  VENDOR_AT_CMD_RSP = 0x1C,
  UNKNOW_AT_RESULT = 0x1D,
  REPORT_LINK_STATUS = 0x1E,
  REPORT_PAIRING_RECORD = 0x1F,
  REPORT_LOCAL_BD_ADDR = 0x20,
  REPORT_LOCAL_DEVICE_NAME = 0x21,
  REPORT_SPP_DATA = 0x22,
  REPORT_LINK_BACK_STATUS = 0x23,
  RINGTONE_FINISH_INDICATION = 0x24,
  USER_CONFIRM_SSP_REQ = 0x25,
  REPORT_AVRCP_VOL_CTRL = 0x26,
  REPORT_INPUT_SIGNAL_LEVEL = 0x27,
  REPORT_IAP_INFO = 0x28,
  REPORT_AVRCP_ABS_VOL_CTRL = 0x29,
  REPORT_VOICE_PROMPT_STATUS = 0x2A,
  REPORT_MAP_DATA = 0x2B,
  SECURITY_BONDLING_RES = 0x2C,
  REPORT_TYPE_CODEC = 0x2D,
  REPORT_TYPE_BTM_SETTING = 0x2E,
  REPORT_MCU_UPDATE_REPLY = 0x2F,
  REPORT_BTM_INITIAL_STATUS = 0x30,
  REPORT_LE_EVENT = 0x32,
  REPORT_nSPK_STATUS = 0x33,
  REPORT_nSPK_VENDOR_EVENT = 0x34,
  REPORT_CUSTOMER_GATT_ATTRIBUTE_DATA = 0x39,
  REPORT_LINK_MODE = 0x3A,
  REPORT_FEATURE_LIST = 0x40
};

//Command decode: BTM_Utility_Req
enum
{
    BTM_CONTROL_AMP = 0x00,
    BTM_REPORT_LINE_IN_STATUS = 0x01,
    BTM_NOTIFY_HANDLE_BTM = 0x02,
    BTM_NOTIFY_EEPROM_UPDATE_FINISH = 0x03,
    BTM_REPORT_CODEC_STATUS = 0x04,
    NSPK_SYNC_POWER_OFF = 0x05,
    NSPK_SYNC_VOLUME_CTRL = 0x06,
    NSPK_SYNC_INTERNAL_GAIN = 0x07,
    NSPK_SYNC_A2DP_ABS_VOL = 0x08,
    NSPK_SYNC_CURRENT_CHANNEL_SETTING = 0x09,
    NSPK_NOTIFY_POWER_SYNCED = 0x0A,
    NSPK_NOTIFY_COMMAND_SUCCESS = 0x0B,
    NSPK_NOTIFY_COMMAND_FAIL = 0x0C,
    NSPK_NOTIFY_SLAVE_STATUS_CHANGED = 0x0D,
    NSPK_RESERVED = 0x0E,
    NSPK_NOTIFY_ADD_THIRD_SPEAKER = 0x0F
};
//command decode: BTM status
enum
{
  BT_OFF = 0x00,
  BT_DISCOVERABLE = 0x01,
  BT_ON = 0x02,
  BT_PAIRING_COMPLETE = 0x03,
  BT_PAIRING_FAIL = 0x04,
  BT_HFP_CONNECTED = 0x05,
  BT_A2DP_CONNECTED = 0x06,
  BT_HFP_DISCONNECTED = 0x07,
  BT_A2DP_DISCONNECTED = 0x08,
  BT_SCO_CONNECTED = 0x09,
  BT_SCO_DISCONNECTED = 0x0A,
  BT_ARVCP_CONNECTED = 0x0B,
  BT_AVRCP_DISCONNECTED = 0x0C,
  BT_SPP_CONNECTED = 0x0D,
  BT_SPP_DISCONNECTED = 0x0E,
  BT_STANDBY = 0x0F,
  BT_iAP_CONNECTED = 0x10,
  BT_ACL_DISCONNECTED = 0x11,
  BT_MAP_CONNECTED = 0x12,
  BT_MAP_FORBIDDEN = 0x13,
  BT_MAP_DISCONNECTED = 0x14,
  BT_ACL_CONNECTED = 0x15
};

#define INDIV_ACTION_CMD_LENGTH		8		// 4byte MAC address
//#define INDIV_ACTION_CMD_LENGTH		10		// 6byte MAC address



/*======================*/
/*  external variables  */
/*======================*/
uint8_t  BT_CmdDecodedFlag;
uint8_t  BT_CmdBuffer[BT_CMD_SIZE_MAX	];
uint8_t  BT_linkIndex = 0;

/*======================================*/
/*  internal variables          */
/*======================================*/
static RX_DECODE_MODE  BT_CmdDecodeState;
static uint8_t  BT_CmdDecodeCmdLength;
static uint8_t  BT_CmdDecodeChecksum;			
static uint8_t  BT_CmdDecodeDataCnt;                    //temporary variable in decoding
static unsigned short BT_CmdBufferPt;                    //

/*======================================*/
/*  function implemention       */
/*======================================*/
void BT_CommandDecode_Initialize(void)
{
    BT_CmdDecodedFlag = 0;
    BT_CmdDecodeState = RX_DECODE_CMD_SYNC_AA;
    while(uart2RxCount)
    {
        UART2_Read();
    }
    // BT_SPPBuffClear();
}

void BT_CommandDecode_Task(void)
{
	BT_CommandHandler();
	if(BT_CmdDecodedFlag)
	{
            BT_CommandDecode();
            BT_CmdDecodedFlag = 0;
	}
}

extern volatile uint8_t uart2RxCount;

void BT_CommandHandler(void) {
    uint8_t current_byte;

    while (uart2RxCount) {
        current_byte = UART2_Read();

        switch (BT_CmdDecodeState) {
            case RX_DECODE_CMD_SYNC_AA:
                if (current_byte == 0xaa)
                    BT_CmdDecodeState = RX_DECODE_CMD_SYNC_00;
                break;

            case RX_DECODE_CMD_SYNC_00:
                if (current_byte == 0x00)
                    BT_CmdDecodeState = RX_DECODE_CMD_LENGTH;
                else
                    BT_CmdDecodeState = RX_DECODE_CMD_SYNC_AA;
                break;

            case RX_DECODE_CMD_LENGTH:
                BT_CmdDecodedFlag = 0; //command receive flag clear
                BT_CmdBufferPt = 0; //buffer reset for command parameter
                BT_CmdDecodeCmdLength = current_byte;
                BT_CmdDecodeChecksum = current_byte; //checksum calculation start!
                BT_CmdDecodeDataCnt = current_byte; //save bytes number, use to check where is command end
                BT_CmdDecodeState = RX_DECODE_CMD_DATA; //next state
                break;

            case RX_DECODE_CMD_DATA:
                BT_CmdDecodeChecksum += current_byte;
                BT_CmdDecodeDataCnt--;
                BT_CmdBuffer[BT_CmdBufferPt++] = current_byte;
                if (BT_CmdDecodeDataCnt == 0) //no data remained?
                    BT_CmdDecodeState = RX_DECODE_CMD_CHECKSUM; //yes, next mode: checksum
                break;

            case RX_DECODE_CMD_CHECKSUM:
                if ((uint8_t) (BT_CmdDecodeChecksum + current_byte) == 0) {
                    BT_CmdDecodedFlag = 1;
                } else {
                }
                BT_CmdDecodeState = RX_DECODE_CMD_SYNC_AA;
                break;
            default:
                break;
        }

        if (BT_CmdDecodedFlag) {
            break;
        }
    }
}

void BT_CommandDecode( void )
{
    uint8_t spp_type, cmdInfo = CMD_INFO_IGNORE;
    uint16_t spp_total_length;
    uint16_t spp_payload_length;
    uint16_t para;

    // Send Ack to 5506 for the report event
    if (BT_CmdBuffer[0] == ACK)
    {
		cmdInfo = BT_UpdateAckStatusWhenReceived(BT_CmdBuffer[1],BT_CmdBuffer[2]);    
    }
	else
    {
        BT_SendAckToEvent(BT_CmdBuffer[0]);     //send ACK to this event
    }
    // Send all report event to SPP/BLE link except the SPP/BLE report event
/*    if (!((BT_CmdBuffer[0] == REPORT_SPP_DATA) ||
		  ((BT_CmdBuffer[0] == ACK) && 
		  (BT_CmdBuffer[1] == SEND_SPP_DATA) || (cmdInfo != CMD_INFO_APPS))
		  ))
*/
	if(BT_CmdBuffer[0] != REPORT_SPP_DATA)
    {
		if((BT_CmdBuffer[0] == ACK && cmdInfo != CMD_INFO_APPS) ||
			(BT_CmdBuffer[0] == ACK && BT_CmdBuffer[1] == SEND_SPP_DATA))
		;
		else
		{
	    	// All event passed to Audio Weidget 2.0
//			if(BT_isBLEConnected())
//				BT_SendSPPAllData(BT_CmdBuffer, (uint16_t)BT_CmdDecodeCmdLength, BT_linkIndex, (0-BT_CmdDecodeChecksum));
		}
    }
	
	if(BT_CmdBuffer[0] == REPORT_nSPK_VENDOR_EVENT &&
			BT_CmdBuffer[4] == APPS_SET_INDIV_MMI_ACTION)
	{
//		if(BT_CmpBDAddress(&BT_CmdBuffer[5]))
//		{
//			BT_Send_ActionCommand(BT_CmdBuffer[11], 0, CMD_INFO_MCU);
//		}
		return;		
	}

    switch(BT_CmdBuffer[0])
    {
      case PHONE_SERVICE_STATUS:
        APP_BT_EventHandler(BT_EVENT_PHONE_SERVICE_STATUS, BT_CmdBuffer[2], NULL);
        break;
        
      case PHONE_ROAMING_STATUS:
        APP_BT_EventHandler(BT_EVENT_PHONE_ROAMING_STATUS, BT_CmdBuffer[2], NULL);
        break;
        
      case PHONE_MAX_SIGNAL_STRENGTH:
        APP_BT_EventHandler(BT_EVENT_PHONE_MAX_SIGNAL_STRENGTH, BT_CmdBuffer[2], NULL);
        break;
        
      case PHONE_SIGNAL_STRENGTH:
        APP_BT_EventHandler(BT_EVENT_PHONE_SIGNAL_STRENGTH, BT_CmdBuffer[2], NULL);
        break;

      case PHONE_MAX_BATTERY_LEVEL:
        APP_BT_EventHandler(BT_EVENT_PHONE_MAX_BATTERY_LEVEL, BT_CmdBuffer[2], NULL);
        break;
        
      case PHONE_BATTERY_LEVEL:
        APP_BT_EventHandler(BT_EVENT_PHONE_BATTERY_LEVEL, BT_CmdBuffer[2], NULL);
        break;

      case CALL_ID:  
        APP_BT_EventHandler(BT_EVENT_CALLER_ID, BT_CmdDecodeCmdLength - 2, &BT_CmdBuffer[2]);
        break;
        
      case MISS_CALL_INDICATION:  
        APP_BT_EventHandler(BT_EVENT_MISSED_CALL, BT_CmdBuffer[2], NULL);
        break;
        
      case VENDOR_AT_CMD_RSP:
        ATCMD_BT_ResponseHandler(BT_CmdBuffer[2]);
        break;
      
      case UNKNOW_AT_RESULT:
        ATCMD_BT_ResultHandler(&BT_CmdBuffer[2], BT_CmdDecodeCmdLength - 2);
        break;
      
      case REPORT_LINKED_DEVICE_INFO:
        if (BT_CmdBuffer[2] == 0) {
          APP_BT_EventHandler(BT_EVENT_LINKED_NAME_RECEIVED, 0, &BT_CmdBuffer[3]);
        }
        break;
        
      case REPORT_nSPK_VENDOR_EVENT:
			APP_BT_EventHandler(NSPK_EVENT_VENDOR_CMD, 0, &BT_CmdBuffer[1]);			
			break;
        case UART_VERSION:
            break;
		case BATTERY_LEVEL:
			// BLE_SaveBatteryLevel(BT_CmdBuffer[2]);
			break;
        case DEVICE_STATE:
            switch(BT_CmdBuffer[1])
            {
                //power on, power off, standby etc.
                case BT_OFF:        //power off state
                    APP_BT_EventHandler(BT_EVENT_SYS_POWER_OFF, 0, &BT_CmdBuffer[1]);
                    break;
                case BT_ON:         //power on state
                    APP_BT_EventHandler(BT_EVENT_SYS_POWER_ON, 0, &BT_CmdBuffer[1]);
                    break;
                case BT_STANDBY:     //standby state
                    APP_BT_EventHandler(BT_EVENT_SYS_STANDBY, 0, &BT_CmdBuffer[1]);
                    break;

                //pairing etc
                case BT_DISCOVERABLE:       //pairing state
                    APP_BT_EventHandler(BT_EVENT_SYS_PAIRING_START, 0, &BT_CmdBuffer[1]);
                    break;
                case BT_PAIRING_COMPLETE:       //pairing ok
                    APP_BT_EventHandler(BT_EVENT_SYS_PAIRING_OK, 0, &BT_CmdBuffer[1]);
                    break;
                case BT_PAIRING_FAIL:           //pairing failed
                    APP_BT_EventHandler(BT_EVENT_SYS_PAIRING_FAILED, 0, &BT_CmdBuffer[1]);
                    break;

                    //HFP
                case BT_HFP_CONNECTED:
                    BT_linkIndex = BT_CmdBuffer[2]&0x0f;
                    APP_BT_EventHandler(BT_EVENT_HFP_CONNECTED, (uint16_t)BT_CmdBuffer[2], &BT_CmdBuffer[1]);
                    break;
                case BT_HFP_DISCONNECTED:
                    APP_BT_EventHandler(BT_EVENT_HFP_DISCONNECTED, (uint16_t)BT_CmdBuffer[2], &BT_CmdBuffer[1]);
                    break;
                    
                    //spp,iap etc
                case BT_SPP_CONNECTED:
                    APP_BT_EventHandler(BT_EVENT_SPP_CONNECTED, (uint16_t)BT_CmdBuffer[2], &BT_CmdBuffer[1]);
                    break;
                case BT_iAP_CONNECTED:
                    APP_BT_EventHandler(BT_EVENT_IAP_CONNETED, (uint16_t)BT_CmdBuffer[2], &BT_CmdBuffer[1]);
                    break;
                case BT_SPP_DISCONNECTED:
                    APP_BT_EventHandler(BT_EVENT_SPP_IAP_DISCONNECTED, (uint16_t)BT_CmdBuffer[2], &BT_CmdBuffer[1]);
                    break;

                    //a2dp etc
                case BT_A2DP_CONNECTED:
                    BT_linkIndex = BT_CmdBuffer[2]&0x0f;
                    APP_BT_EventHandler(BT_EVENT_A2DP_CONNECTED, (uint16_t)BT_CmdBuffer[2], &BT_CmdBuffer[1]);
                    break;
                case BT_A2DP_DISCONNECTED:
                    APP_BT_EventHandler(BT_EVENT_A2DP_DISCONNECTED, (uint16_t)BT_CmdBuffer[2], &BT_CmdBuffer[1]);
                    break;

                    //avrcp etc
                case BT_ARVCP_CONNECTED:
                    APP_BT_EventHandler(BT_EVENT_AVRCP_CONNECTED, (uint16_t)BT_CmdBuffer[2], &BT_CmdBuffer[1]);
                    break;
                case BT_AVRCP_DISCONNECTED:
                    APP_BT_EventHandler(BT_EVENT_AVRCP_DISCONNECTED, (uint16_t)BT_CmdBuffer[2], &BT_CmdBuffer[1]);
                    break;

                    //ACL, SCO etc
                case BT_ACL_CONNECTED:
                    APP_BT_EventHandler(BT_EVENT_ACL_CONNECTED, (uint16_t)BT_CmdBuffer[2], &BT_CmdBuffer[1]);
                    break;
                case BT_ACL_DISCONNECTED:
                    APP_BT_EventHandler(BT_EVENT_ACL_DISCONNECTED, (uint16_t)BT_CmdBuffer[2], &BT_CmdBuffer[1]);
                    break;
                case BT_SCO_CONNECTED:
                    APP_BT_EventHandler(BT_EVENT_SCO_CONNECTED, (uint16_t)BT_CmdBuffer[2], &BT_CmdBuffer[1]);
                    break;
                case BT_SCO_DISCONNECTED:
                    APP_BT_EventHandler(BT_EVENT_SCO_DISCONNECTED, (uint16_t)BT_CmdBuffer[2], &BT_CmdBuffer[1]);
                    break;

                case BT_MAP_CONNECTED:
                    APP_BT_EventHandler(BT_EVENT_MAP_CONNECTED, (uint16_t)BT_CmdBuffer[2], &BT_CmdBuffer[1]);
                    break;
                case BT_MAP_DISCONNECTED:
                    APP_BT_EventHandler(BT_EVENT_MAP_DISCONNECTED, (uint16_t)BT_CmdBuffer[2], &BT_CmdBuffer[1]);
                    break;

                case BT_MAP_FORBIDDEN:
                    break;    
            }
			//BLE_UpdateBTMState();
            break;

        case BTM_UTILITY_REQ:
            
            switch(BT_CmdBuffer[1])
            {
                case BTM_CONTROL_AMP:
                    break;
                case BTM_REPORT_LINE_IN_STATUS:
                    APP_BT_EventHandler(BT_EVENT_LINE_IN_STATUS,(uint16_t)BT_CmdBuffer[2], &BT_CmdBuffer[1]);
                    break;
                case BTM_NOTIFY_HANDLE_BTM:
                    break;
                case BTM_NOTIFY_EEPROM_UPDATE_FINISH:
                    break;
                case BTM_REPORT_CODEC_STATUS:
                    APP_BT_EventHandler(BT_EVENT_A2DP_STATUS,(uint16_t)BT_CmdBuffer[2], &BT_CmdBuffer[1]);
                    break;
                case NSPK_SYNC_POWER_OFF:
                    APP_BT_EventHandler(NSPK_EVENT_SYNC_POWER_OFF,0, &BT_CmdBuffer[1]);
                    break;
                case NSPK_SYNC_VOLUME_CTRL:
                    APP_BT_EventHandler(NSPK_EVENT_SYNC_VOL_CTRL,(uint16_t)BT_CmdBuffer[2], &BT_CmdBuffer[1]);
                    break;
                case NSPK_SYNC_INTERNAL_GAIN:
                    APP_BT_EventHandler(NSPK_EVENT_SYNC_INTERNAL_GAIN,(uint16_t)BT_CmdBuffer[2], &BT_CmdBuffer[1]);
                    break;
                case NSPK_SYNC_A2DP_ABS_VOL:
                    APP_BT_EventHandler(NSPK_EVENT_SYNC_ABS_VOL,(uint16_t)BT_CmdBuffer[2], &BT_CmdBuffer[1]);
                    break;
                case NSPK_SYNC_CURRENT_CHANNEL_SETTING:
                    APP_BT_EventHandler(NSPK_EVENT_CHANNEL_SETTING,(uint16_t)BT_CmdBuffer[2], &BT_CmdBuffer[1]);
                    break;
                case NSPK_NOTIFY_POWER_SYNCED:
                    break;
                case NSPK_NOTIFY_COMMAND_SUCCESS:
                    break;
                case NSPK_NOTIFY_COMMAND_FAIL:
                    break;
                case NSPK_NOTIFY_SLAVE_STATUS_CHANGED:
                    break;
                case NSPK_NOTIFY_ADD_THIRD_SPEAKER:
                    APP_BT_EventHandler(NSPK_EVENT_ADD_SPEAKER3,(uint16_t)BT_CmdBuffer[2], &BT_CmdBuffer[1]);
                    break;
            }
            break;
        
        case CALL_STATUS:
            
            APP_BT_EventHandler(BT_EVENT_CALL_STATUS_CHANGED, (uint16_t)BT_CmdBuffer[2], &BT_CmdBuffer[1]);
            break;
            
        case REPORT_nSPK_STATUS:
            
            para = BT_CmdBuffer[1];
            para <<= 8;
            para |= BT_CmdBuffer[2];
            APP_BT_EventHandler(BT_EVENT_NSPK_STATUS, para, &BT_CmdBuffer[1]);
            break;
            
        case REPORT_LINK_BACK_STATUS:
            
            if(BT_CmdBuffer[1] == 0 )       //ACL link-back
            {
                if(BT_CmdBuffer[2] == 0xFF)     //00 FF: ACL failed
                {
                    APP_BT_EventHandler(BT_EVENT_LINKBACK_FAILED, BT_CmdBuffer[1], &BT_CmdBuffer[1]);
                }
                else
                {
                    APP_BT_EventHandler(BT_EVENT_LINKBACK_SUCCESS, BT_CmdBuffer[1], &BT_CmdBuffer[1]);
                }
            }
            else if(BT_CmdBuffer[1] == 1 || BT_CmdBuffer[1] ==  2 || BT_CmdBuffer[1] == 3)  //01 00 or 02 00 or 03 00: HFP/A2DP/SPP success
            {
                if(BT_CmdBuffer[2] == 0x00)
                {
                    APP_BT_EventHandler(BT_EVENT_LINKBACK_SUCCESS, BT_CmdBuffer[1], &BT_CmdBuffer[1]);
                }
                else
                {
                    APP_BT_EventHandler(BT_EVENT_LINKBACK_FAILED, BT_CmdBuffer[1], &BT_CmdBuffer[1]);
                }
            }
            else
            {
                
            }
            break;
        
        case REPORT_LOCAL_BD_ADDR:            
            //BT_SaveLocalBDAddress(&BT_CmdBuffer[1]);
			//BLE_SaveLocalAddr(&BT_CmdBuffer[1]);
            APP_BT_EventHandler(BT_EVENT_BD_ADDR_RECEIVED, 0, &BT_CmdBuffer[1]);
            break;
		case REPORT_LOCAL_DEVICE_NAME:
            //BT_SaveDeviceName(&BT_CmdBuffer[1]);
            APP_BT_EventHandler(BT_EVENT_NAME_RECEIVED, 0, &BT_CmdBuffer[1]);
			break;
		case REPORT_FEATURE_LIST:
            //BT_SaveFeatureList(BT_CmdBuffer[1]);
            APP_BT_EventHandler(FEATURE_LIST_REPLY, 0, &BT_CmdBuffer[1]);
			break;
        case REPORT_PAIRING_RECORD:            
            APP_BT_EventHandler(BT_EVENT_PAIR_RECORD_RECEIVED, (uint16_t)(BT_CmdBuffer[1]), &BT_CmdBuffer[1]);
            break;

        case VOLUME_LEVEL:              //0x0f command: HFP Gain Level
            
            APP_BT_EventHandler(BT_EVENT_HFP_VOLUME_CHANGED, (uint16_t)(BT_CmdBuffer[2]), &BT_CmdBuffer[1]);
            break;
            
        case REPORT_AVRCP_VOL_CTRL:     //0x26 command
            
            APP_BT_EventHandler(BT_EVENT_AVRCP_VOLUME_CTRL, (uint16_t)(BT_CmdBuffer[2]), &BT_CmdBuffer[1]);
            break;
            
        case REPORT_AVRCP_ABS_VOL_CTRL: //0x29 command
            
            APP_BT_EventHandler(BT_EVENT_AVRCP_ABS_VOLUME_CHANGED, (uint16_t)(BT_CmdBuffer[2]), &BT_CmdBuffer[1]);
            break;

        case AVRCP_SPEC_RSP:
            
            //BT_CmdBuffer[0] = AVRCP_SPEC_RSP
            //BT_CmdBuffer[1] = database 0 or 1 link
            //BT_CmdBuffer[2] = 0x0F or 0x0D(INTRIM or CHANGED)
            //BT_CmdBuffer[3] = 0x48 fixed(Subunit_type + Subunit_ID)
            //BT_CmdBuffer[4] = 0x00 fixed(Opcode)
            //BT_CmdBuffer[5 ~ 7] = 00 19 58 fixed(company ID
            //BT_CmdBuffer[8] = 0x31 fixed(PDU ID)
            //BT_CmdBuffer[9] = 0x00 fixed(packet ID)
            //BT_CmdBuffer[10 - 11] = parameter length
            //BT_CmdBuffer[12] = EventID2 (EVENT_PLAYBACK_STATUS_CHANGED (0x01), EVENT_VOLUME_CHANGED (0x0d))
            //BT_CmdBuffer[13] = PLAYING STATUS
            if((BT_CmdBuffer[2] == 0x0D || BT_CmdBuffer[2] == 0x0F) &&
				BT_CmdBuffer[12] == 0x01)
			    //EVENT_PLAYBACK_STATUS_CHANGED (0x01)
            {
                APP_BT_EventHandler(BT_EVENT_PLAYBACK_STATUS_CHANGED, (uint16_t)(BT_CmdBuffer[13]), &BT_CmdBuffer[1]);
            }
            break;
                        
        case REPORT_SPP_DATA:
 
            spp_type = BT_CmdBuffer[2];

            spp_total_length = (uint16_t)BT_CmdBuffer[3];
            spp_total_length <<= 8;
            spp_total_length |= (uint16_t)BT_CmdBuffer[4];

            spp_payload_length = (uint16_t)BT_CmdBuffer[5];
            spp_payload_length <<= 8;
            spp_payload_length |= (uint16_t)BT_CmdBuffer[6];

            BT_SendAckToEvent(BT_CmdBuffer[0]);     //send ACK to this event
            //BT_AddBytesToSPPBuff(&BT_CmdBuffer[7], spp_payload_length);
            break;
        case REPORT_TYPE_CODEC:
            APP_BT_EventHandler(BT_EVENT_TYPE_CODEC, 0, &BT_CmdBuffer[1]);
			break;
        case REPORT_CUSTOMER_GATT_ATTRIBUTE_DATA:            
            APP_BT_EventHandler(GATT_ATTRIBUTE_DATA, (uint16_t)(BT_CmdBuffer[3]), &BT_CmdBuffer[1]);
            break;
            
        case REPORT_LINK_MODE:
            
            para = BT_CmdBuffer[2];     //second byte put to higher part
            para <<= 8;
            para |= BT_CmdBuffer[1];    //first byte put to lower part
            APP_BT_EventHandler(BT_EVENT_LINK_MODE_RECEIVED, para, &BT_CmdBuffer[1]);
            break;

        case REPORT_LE_EVENT:
            
            switch(BT_CmdBuffer[1])     //LE event sub command
            {
                case 0:
                    para = BT_CmdBuffer[2];
                    para <<= 8;
                    para |= BT_CmdBuffer[3];
                    APP_BT_EventHandler(LE_STATUS_CHANGED, para, &BT_CmdBuffer[1]);
                    break;
                case 1:
                    APP_BT_EventHandler(LE_ADV_CONTROL_REPORT, (uint16_t)(BT_CmdBuffer[2]), &BT_CmdBuffer[1]);
                    break;
                case 2:
                    APP_BT_EventHandler(LE_CONNECTION_PARA_REPORT, 0, &BT_CmdBuffer[1]);
                    break;
                case 3:
                    para = BT_CmdBuffer[2];
                    para <<= 8;
                    para |= BT_CmdBuffer[3];
                    APP_BT_EventHandler(LE_CONNECTION_PARA_UPDATE_RSP, para, &BT_CmdBuffer[1]);
                    break;
                default:
                    break;
            }
            break;
        
        case REPORT_INPUT_SIGNAL_LEVEL:
            
            BT_CmdBuffer[1] ^= 0xff;
            BT_CmdBuffer[2] ^= 0xff;
            BT_CmdBuffer[3] ^= 0xff;
            BT_CmdBuffer[4] ^= 0xff;
            if(BT_CmdBuffer[1])
            {
                para = BT_CmdBuffer[1];
                para <<= 8;
                para |= BT_CmdBuffer[5];
                APP_BT_EventHandler(PORT0_INPUT_CHANGED, para, &BT_CmdBuffer[1]);
            }
            if(BT_CmdBuffer[2])
            {
                para = BT_CmdBuffer[2];
                para <<= 8;
                para |= BT_CmdBuffer[6];
                APP_BT_EventHandler(PORT1_INPUT_CHANGED, para, &BT_CmdBuffer[1]);
            }
            if(BT_CmdBuffer[3])
            {
                para = BT_CmdBuffer[3];
                para <<= 8;
                para |= BT_CmdBuffer[7];
                APP_BT_EventHandler(PORT2_INPUT_CHANGED, para, &BT_CmdBuffer[1]);
            }
            if(BT_CmdBuffer[4])
            {
                para = BT_CmdBuffer[4];
                para <<= 8;
                para |= BT_CmdBuffer[8];
                APP_BT_EventHandler(PORT3_INPUT_CHANGED, para, &BT_CmdBuffer[1]);
            }
            break;
        default:
            break;
    }
}

