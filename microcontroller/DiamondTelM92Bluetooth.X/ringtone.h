/* 
 * File:   ringtone.h
 * Author: Jeff
 *
 * Created on March 6, 2022, 11:46 AM
 */

#ifndef RINGTONE_H
#define	RINGTONE_H

#ifdef	__cplusplus
extern "C" {
#endif
 
typedef enum RINGTONE_Type {
  RINGTONE_Type_CLASSIC,
  RINGTONE_Type_NOKIA,
  RINGTONE_Type_AXEL_F,
  RINGTONE_Type_MEGALOMANIA,
  RINGTONE_Type_CAR_PHONE_SONG,
  RINGTONE_Type_TETRIS,
} RINGTONE_Type;

#define RINGTONE_COUNT (6)

char const* RINGTONE_GetName(RINGTONE_Type ringtone);

void RINGTONE_Preview(RINGTONE_Type ringtone);

void RINGTONE_Start(RINGTONE_Type ringtone);

void RINGTONE_Stop(void);

#ifdef	__cplusplus
}
#endif

#endif	/* RINGTONE_H */
