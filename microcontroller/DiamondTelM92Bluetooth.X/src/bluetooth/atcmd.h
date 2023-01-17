/** 
 * @file
 * @author Jeff Lau
 */

#ifndef ATCMD_H
#define	ATCMD_H

#include <stdint.h>
#include <stdbool.h>

#ifdef	__cplusplus
extern "C" {
#endif

typedef enum ATCMD_Response {
  ATCMD_Response_OK,
  ATCMD_Response_ERROR,
  ATCMD_Response_NO_RESPONSE,
  ATCMD_Response_FAILED,
  ATCMD_Response_RESULT
} ATCMD_Response;

typedef void (*ATCMD_ResponseCallback)(ATCMD_Response response, char const* result);

typedef void (*ATCMD_UnsolicitedResultHandler)(char const* result);

void ATCMD_Initialize(ATCMD_UnsolicitedResultHandler unsolicitedResultCallback);

void ATCMD_Task(void);

void ATCMD_Timer10ms_Interrupt(void);

bool ATCMD_Send(char const *cmd, ATCMD_ResponseCallback responseCallback);

bool ATCMD_SendDTMFDigit(char digit);

void ATCMD_SendDTMFDigitString(char const* dtmfString);

void ATCMD_CancelDTMFDigits(void);

void ATCMD_BT_ResultHandler(char const* result, uint8_t length);

void ATCMD_BT_ResponseHandler(ATCMD_Response response);

#ifdef	__cplusplus
}
#endif

#endif	/* ATCMD_H */

