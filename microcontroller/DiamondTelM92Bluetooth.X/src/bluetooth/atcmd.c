/** 
 * @file
 * @author Jeff Lau
 */

#include "atcmd.h"
#include "bt_command_send.h"
#include "bt_command_decode.h"
#include "../util/string.h"
#include "../util/timeout.h"
#include "../constants.h"
#include "../telephone/handset.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>

typedef struct ATCMD_CmdInfo {
  uint16_t cmdBufferPos;
  uint8_t cmdLen;
  char resultPrefix[32];
  ATCMD_ResponseCallback responseCallback;
} ATCMD_CmdInfo;

#define COMMAND_INFO_BUFFER_SIZE (32)
#define COMMAND_BUFFER_SIZE (512)

static struct {
  ATCMD_UnsolicitedResultHandler unsolicitedResultHandler;
  
  struct {
    ATCMD_CmdInfo buffer[COMMAND_INFO_BUFFER_SIZE];
    uint8_t head;
    uint8_t tail;
    uint8_t remaining;
    ATCMD_CmdInfo const* pendingCmd;
  } cmdInfoBuffer;

  struct {
    char buffer[COMMAND_BUFFER_SIZE];
    uint16_t head;
    uint16_t remaining;
  } cmdBuffer;
  
  struct {
    bool isDigitPending;
    timeout_t nextDigitTimeout;
    char buffer[MAX_EXTENDED_PHONE_NUMBER_LENGTH];
    uint8_t head;
    uint8_t tail;
    uint8_t bufferSize;
  } dtmfState;
} module;

#define MAX_AT_CMD_LENGTH (128)

static void sendNextCommand(void) {
  if (module.cmdInfoBuffer.pendingCmd || (module.cmdInfoBuffer.remaining == COMMAND_INFO_BUFFER_SIZE)) {
    return;
  }
  
  ATCMD_CmdInfo const* cmdInfo = &module.cmdInfoBuffer.buffer[module.cmdInfoBuffer.tail];
  module.cmdInfoBuffer.pendingCmd = cmdInfo;
  
  uint16_t pos = cmdInfo->cmdBufferPos;
  uint8_t len = cmdInfo->cmdLen;

  uint8_t command[MAX_AT_CMD_LENGTH + 6];
  
  command[0] = 0xAA;                      //header byte 0
  command[1] = 0x00;                      //header byte 1
  command[2] = len + 2;                      //length
  command[3] = VENDOR_AT_CMD;        	//command ID
  command[4] = BT_linkIndex;                 	// link_index, set to 0

  char* nextChar = (char*)command + 5;
  while (len) {
    *nextChar++ = module.cmdBuffer.buffer[pos];
    
    if (++pos == COMMAND_BUFFER_SIZE) {
      pos = 0;
    }
    --len;
  }
  len = cmdInfo->cmdLen;

  *nextChar = 0;
  printf("[ATCMD] Sending: %s\r\n", command + 5);
  
  command[len + 5] = BT_CalculateCmdChecksum(&command[2], &command[len + 4]);
  BT_SendBytesAsCompleteCommand(command, len + 6);    
}

static void handleDTMFDigitAtResponse(ATCMD_Response response, char const* result) {
  if (response != ATCMD_Response_OK) {
    // If the command failed, then cancel all remaining pending DTMF digits.
    // This is most likely caused by no longer being in a call.
    ATCMD_CancelDTMFDigits();
  } else {
    module.dtmfState.isDigitPending = false;
    TIMEOUT_Start(&module.dtmfState.nextDigitTimeout, 10);
  }
}

static bool sendDTMFDigitAtCmd(char digit) {
  char command[] = "+VTS=?";
  command[5] = digit;
  return ATCMD_Send(command, handleDTMFDigitAtResponse);
}

static void sendNextDTMFStringDigit(void) {
  if (
      !module.dtmfState.bufferSize || 
      module.dtmfState.isDigitPending || 
      TIMEOUT_IsPending(&module.dtmfState.nextDigitTimeout)
      ) {
    return;
  }
  
  if (sendDTMFDigitAtCmd(module.dtmfState.buffer[module.dtmfState.tail])) {
    module.dtmfState.isDigitPending = true;

    --module.dtmfState.bufferSize;
    if (++module.dtmfState.tail == MAX_EXTENDED_PHONE_NUMBER_LENGTH) {
      module.dtmfState.tail = 0;
    }
  } else {
    ATCMD_CancelDTMFDigits();
  }
}

void ATCMD_Initialize(ATCMD_UnsolicitedResultHandler unsolicitedResultHandler) {
  module.unsolicitedResultHandler = unsolicitedResultHandler;

  module.cmdBuffer.head = 0;
  module.cmdBuffer.remaining = COMMAND_BUFFER_SIZE;

  module.cmdInfoBuffer.head = module.cmdInfoBuffer.tail = 0;
  module.cmdInfoBuffer.remaining = COMMAND_INFO_BUFFER_SIZE;

  ATCMD_CancelDTMFDigits();
}

void ATCMD_Task(void) {
  TIMEOUT_Task(&module.dtmfState.nextDigitTimeout);
  
  sendNextDTMFStringDigit();
  sendNextCommand();
}

void ATCMD_Timer10ms_Interrupt(void) {
  TIMEOUT_Timer_Interrupt(&module.dtmfState.nextDigitTimeout);
}

bool ATCMD_Send(char const *cmd, ATCMD_ResponseCallback responseCallback) {
  printf("[ATCMD] Queuing: %s\r\n", cmd);
  
  if (module.cmdInfoBuffer.remaining == 0) {
    printf("[ATCMD] Command Info buffer overflow!\r\n");
    return false;
  }
  
  size_t len = strlen(cmd);
  
  if (len > MAX_AT_CMD_LENGTH) {
    printf("[ATCMD] Command too long!\r\n");
    return false;
  }
  
  if (module.cmdBuffer.remaining < len) {
    printf("[ATCMD] Command buffer overflow!\r\n");
    return false;
  }
  
  ATCMD_CmdInfo* cmdInfo = &module.cmdInfoBuffer.buffer[module.cmdInfoBuffer.head];
  cmdInfo->cmdBufferPos = module.cmdBuffer.head;
  cmdInfo->cmdLen = (uint8_t)len;
  cmdInfo->responseCallback = responseCallback;

  size_t prefixLen = strcspn(cmd, "?=");
  memcpy(cmdInfo->resultPrefix, cmd, prefixLen);
  cmdInfo->resultPrefix[prefixLen] = ':';
  cmdInfo->resultPrefix[prefixLen + 1] = 0;
  
  --module.cmdInfoBuffer.remaining;
  if (++module.cmdInfoBuffer.head == COMMAND_INFO_BUFFER_SIZE) {
    module.cmdInfoBuffer.head = 0;
  }
  
  module.cmdBuffer.remaining -= len;
  uint16_t pos = module.cmdBuffer.head;
  while (len) {
    module.cmdBuffer.buffer[pos] = *cmd++;
    
    if (++pos == COMMAND_BUFFER_SIZE) {
      pos = 0;
    }
    --len;
  }
  module.cmdBuffer.head = pos;
  
  return true;
}

bool ATCMD_SendDTMFDigit(char digit) {
  if (!HANDSET_IsButtonPrintable(digit)) {
    return false;
  }

  if (module.dtmfState.bufferSize == MAX_EXTENDED_PHONE_NUMBER_LENGTH) {
    return false;
  }
  
  module.dtmfState.buffer[module.dtmfState.head] = digit;
  if (++module.dtmfState.head == MAX_EXTENDED_PHONE_NUMBER_LENGTH) {
    module.dtmfState.head = 0;
  }
  
  ++module.dtmfState.bufferSize;
  
  return true;
}

void ATCMD_SendDTMFDigitString(char const* dtmfString) {
  while(*dtmfString) {
    ATCMD_SendDTMFDigit(*dtmfString++);
  }
}

void ATCMD_CancelDTMFDigits(void) {
  module.dtmfState.tail = module.dtmfState.head = 0;
  module.dtmfState.bufferSize = 0;
  module.dtmfState.isDigitPending = false;
  TIMEOUT_Cancel(&module.dtmfState.nextDigitTimeout);
}

void ATCMD_BT_ResultHandler(char const* result, uint8_t length) {
  ATCMD_CmdInfo const* pendingCmd = module.cmdInfoBuffer.pendingCmd;
  
  char resultBuffer[255];

  memcpy(resultBuffer, result, length);
  resultBuffer[length] = 0;
  
  printf("[ATCMD] Result: %s\r\n", resultBuffer);
  
  if (pendingCmd && strstart(resultBuffer, pendingCmd->resultPrefix)) {
    if (pendingCmd->responseCallback) {
      pendingCmd->responseCallback(ATCMD_Response_RESULT, resultBuffer);
    }
  } else {
    module.unsolicitedResultHandler(resultBuffer);
  }
}

static char const* const responseLabels[] = {
  "OK",
  "Error",
  "No Response",
  "Failed"
};

void ATCMD_BT_ResponseHandler(ATCMD_Response response) {
  printf("[ATCMD] Response: %s\r\n", responseLabels[response]);
  
  ATCMD_CmdInfo const* pendingCmd = module.cmdInfoBuffer.pendingCmd;

  if (!pendingCmd) {
    printf("[ATCMD] Ignoring response; no pending command!\r\n");
    // No pending command; ignore
    return;
  }
  
  ATCMD_ResponseCallback responseCallback = pendingCmd->responseCallback;
  
  if (responseCallback) {
    responseCallback(response, NULL);
  }
  
  module.cmdBuffer.remaining += pendingCmd->cmdLen;
  
  if (++module.cmdInfoBuffer.tail == COMMAND_INFO_BUFFER_SIZE) {
    module.cmdInfoBuffer.tail = 0;
  }
  
  ++module.cmdInfoBuffer.remaining;
  module.cmdInfoBuffer.pendingCmd = NULL;
}
