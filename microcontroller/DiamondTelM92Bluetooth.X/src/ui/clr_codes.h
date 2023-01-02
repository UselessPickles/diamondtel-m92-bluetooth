/** 
 * @file
 * @author Jeff Lau
 */

#ifndef CLR_CODES_H
#define	CLR_CODES_H

#include "../telephone/handset.h"

#ifdef	__cplusplus
extern "C" {
#endif

typedef enum CLR_CODES_EventType {
  CLR_CODES_EventType_PROGRAM,
  CLR_CODES_EventType_PROGRAM_RESET,
  CLR_CODES_EventType_SELF_DIAGNOSTICS,
  CLR_CODES_EventType_FACTORY_RESET,
} CLR_CODES_EventType;

typedef void (*CLR_CODES_EventHandler)(CLR_CODES_EventType);

void CLR_CODES_Initialize(void);

void CLR_CODES_Start(CLR_CODES_EventHandler eventHandler);

void CLR_CODES_Task(void);

void CLR_CODES_Timer10MS_Interrupt(void);

void CLR_CODES_HANDSET_EventHandler(HANDSET_Event const* event);

#ifdef	__cplusplus
}
#endif

#endif	/* CLR_CODES_H */

