/*-----------------------------------------------------------------------------*/
/*
 * usart.h
 *
 *  Created on: Apr 9, 2011
 *      Author: Martin Polasek
 */
/*-----------------------------------------------------------------------------*/
#ifndef ATCD_GPS_H_INCLUDED
#define ATCD_GPS_H_INCLUDED

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>     /* atoi */
#include <stdio.h>

#include "atcd_config.h"

#include "atcd_atc.h"

/* Exported functions ------------------------------------------------------- */

/* Defines -------------------------------------------------------------------*/

// GPS State
#define ATCD_GPS_STATE_OFF           0
#define ATCD_GPS_STATE_SEARCHING     1
#define ATCD_GPS_STATE_FIX           2

// GPS Events
#define ATCD_GPS_EV_NONE           0

//------------------------------------------------------------------------------

typedef struct
{
  uint8_t state;                  //GPS state
  uint32_t last_fix_time;

  char date[16];
  char time[16];

  float latitude;
  float longitude;

  float height;
  float speed;
  float course;

  float pdop;
  float hdop;
  float vdop;

  uint8_t cb_events;              //GPS events
  void (*callback)(uint8_t);      //events callback

} atcd_gps_t;

// Functions -------------------------------------------------------------------
void atcd_gps_init();
void atcd_gps_reset();
void atcd_gps_proc();

void atcd_gps_put_nmea(char *str);

uint8_t atcd_gps_asc_msg();
//------------------------------------------------------------------------------
#endif /* ATCD_GPS_H_INCLUDED */
