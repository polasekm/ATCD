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



//------------------------------------------------------------------------------

typedef struct
{
  uint8_t state;                  //phone state

  uint8_t cb_events;              //GPS events
  void (*callback)(uint8_t);      //events callback

} atcd_gps_t;

// Functions -------------------------------------------------------------------
void atcd_gps_init();
void atcd_gps_reset();
void atcd_gps_proc();
//------------------------------------------------------------------------------
#endif /* ATCD_GPS_H_INCLUDED */
