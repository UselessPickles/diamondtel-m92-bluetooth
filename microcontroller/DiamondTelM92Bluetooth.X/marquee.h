/* 
 * File:   marquee.h
 * Author: Jeff
 *
 * Created on April 8, 2022, 9:42 PM
 */

#ifndef MARQUEE_H
#define	MARQUEE_H

#include <stdbool.h>

#ifdef	__cplusplus
extern "C" {
#endif

typedef enum MARQUEE_Row {
  MARQUEE_Row_BOTTOM,
  MARQUEE_Row_TOP
} MARQUEE_Row;  
  
void MARQUEE_Initialize(void);

void MARQUEE_Timer10MS_event(void);

void MARQUEE_Task(void);

void MARQUEE_Start(char const* text, MARQUEE_Row row);

void MARQUEE_Stop(void);

bool MARQUEE_IsRunning(char const* text, MARQUEE_Row row);

#ifdef	__cplusplus
}
#endif

#endif	/* MARQUEE_H */
