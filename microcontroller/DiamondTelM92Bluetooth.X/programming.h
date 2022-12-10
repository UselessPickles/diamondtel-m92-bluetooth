/* 
 * File:   programming.h
 * Author: Jeff
 *
 * Created on November 21, 2022, 8:42 PM
 */

#ifndef PROGRAMMING_H
#define	PROGRAMMING_H

#include "handset.h"

#ifdef	__cplusplus
extern "C" {
#endif

typedef void (*PROGRAMMING_ReturnCallback)(void);
  
void PROGRAMMING_Start(PROGRAMMING_ReturnCallback returnCallback);

void PROGRAMMING_HANDSET_EventHandler(HANDSET_Event const* event);

#ifdef	__cplusplus
}
#endif

#endif	/* PROGRAMMING_H */

