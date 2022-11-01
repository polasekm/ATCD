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
#include <string.h>

#include "atcd_config.h"

#include "atcd_atc.h"

/* Exported functions ------------------------------------------------------- */

/* Defines -------------------------------------------------------------------*/

// GPS State
typedef enum
{
  ATCD_GPS_STATE_OFF         = 0,
  ATCD_GPS_STATE_W_OFF       = 1,
  ATCD_GPS_STATE_SEARCHING   = 2,
  ATCD_GPS_STATE_W_SEARCH    = 3,
  ATCD_GPS_STATE_FIX         = 4,
  ATCD_GPS_STATE__COUNT
} atcd_gps_state_t;

#define ATCD_GPS_FIX_M_NO            0
#define ATCD_GPS_FIX_M_2D            1
#define ATCD_GPS_FIX_M_3D            2

// GPS Events
#define ATCD_GPS_EV_NONE             0x00
#define ATCD_GPS_EV_FIX              0b00000001
#define ATCD_GPS_EV_UPDATE           0b00000010 //RMC, GSA, GGA
#define ATCD_GPS_EV_ALL              0xFF

//------------------------------------------------------------------------------
typedef struct
{
  uint8_t first_search;
  uint32_t start_time;
  uint32_t time_to_fix;
  uint32_t run_time_acc;

  uint32_t cs_err;

} atcd_gps_stat_t;

typedef struct atcd_gps_ts atcd_gps_t;

struct atcd_gps_ts
{
  atcd_gps_state_t state;          //GPS state

  char date[16];
  char time[16];
  char time_fix[16];

  uint8_t sats;

  double latitude;
  double longitude;
  float altitude;
  float undulation; //geoid separation
  uint8_t fix_mode;

  float speed;
  float course;

  float pdop;
  float hdop;
  float vdop;
  float accuracy;

  uint32_t last_fix;              //atcd_get_ms() v dobe fixu
  atcd_gps_stat_t stat;
  uint32_t last_nmea_time;         //atcd_get_ms() kdyz prislo nejnovejsi cokoli NMEA (kontrola baudrate)

  uint8_t cb_events;              //GPS events
  void (*callback)(uint8_t event, const atcd_gps_t *gps);      //events callback $__RMC, GSA, GGA -> event=ATCD_GPS_EV_UPDATE
};

// Functions -------------------------------------------------------------------
void atcd_gps_init();
void atcd_gps_reset();
void atcd_gps_proc();

void atcd_gps_enable();                        //enable gps
void atcd_gps_disable();                       //disable gps

void atcd_gps_put_nmea(char *str);

uint8_t atcd_gps_asc_msg();

atcd_gps_state_t atcd_gps_state();
uint32_t atcd_gps_last_fix();
uint32_t atcd_gps_runtime();

void atcd_gps_set_callback(uint8_t events, void (*gps_callback)(uint8_t event, const atcd_gps_t *gps));
//------------------------------------------------------------------------------
#endif /* ATCD_GPS_H_INCLUDED */
