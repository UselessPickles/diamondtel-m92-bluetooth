/* 
 * File:   level_select.h
 * Author: Jeff
 *
 * Created on December 9, 2022, 11:45 PM
 */

#ifndef LEVEL_SELECT_H
#define	LEVEL_SELECT_H

#include "handset.h"

#ifdef	__cplusplus
extern "C" {
#endif

typedef void (*LEVEL_SELECT_Callback)(int8_t level);

void LEVEL_SELECT_Prompt(uint8_t levels, LEVEL_SELECT_Callback callback);

void LEVEL_SELECT_HANDSET_EventHandler(HANDSET_Event const* event);

#ifdef	__cplusplus
}
#endif

#endif	/* LEVEL_SELECT_H */

