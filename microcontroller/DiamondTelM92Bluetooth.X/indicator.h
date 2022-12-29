/* 
 * File:   indicator.h
 * Author: Jeff
 *
 * Created on March 3, 2022, 10:10 PM
 */

#ifndef INDICATOR_H
#define	INDICATOR_H

#include "handset.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef	__cplusplus
extern "C" {
#endif

void INDICATOR_Initialize(void);
void INDICATOR_Task(void);
void INDICATOR_Timer10MS_Tick(void);

void INDICATOR_StartFlashing(HANDSET_Indicator indicator);
void INDICATOR_StopFlashing(HANDSET_Indicator indicator, bool isOn);

void INDICATOR_StartSignalStrengthSweep(void);
void INDICATOR_StopSignalStrengthSweep(uint8_t signalStrength);

#ifdef	__cplusplus
}
#endif

#endif	/* INDICATOR_H */

