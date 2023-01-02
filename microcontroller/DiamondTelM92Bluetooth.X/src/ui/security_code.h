/** 
 * @file
 * @author Jeff Lau
 */

#ifndef SECURITY_CODE_H
#define	SECURITY_CODE_H

#include "../telephone/handset.h"

#ifdef	__cplusplus
extern "C" {
#endif

typedef void (*SECURITY_CODE_Callback)(void);

void SECURITY_CODE_Prompt(SECURITY_CODE_Callback successCallback, SECURITY_CODE_Callback failureCallback);

void SECURITY_CODE_Watch(SECURITY_CODE_Callback successCallback);

bool SECURITY_CODE_HANDSET_EventHandler(HANDSET_Event const* event);

#ifdef	__cplusplus
}
#endif

#endif	/* SECURITY_CODE_H */

