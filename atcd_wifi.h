/*-----------------------------------------------------------------------------*/
/*
 * usart.h
 *
 *  Created on: Apr 9, 2011
 *      Author: Martin Polasek
 */
/*-----------------------------------------------------------------------------*/
#ifndef ATCD_WIFI_H_INCLUDED
#define ATCD_WIFI_H_INCLUDED

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>     /* atoi */
#include <stdio.h>

#include "atcd_config.h"

#include "atcd_atc.h"
#include "atcd_conn.h"

/* Exported functions ------------------------------------------------------- */

/* Defines -------------------------------------------------------------------*/

// WIFI
#define ATCD_WIFI_STATE_DISCONN     0
#define ATCD_WIFI_STATE_W_DHCP      1
#define ATCD_WIFI_STATE_CONN        2

// Mody WIFI
#define ATCD_WIFI_MODE_OFF          0
#define ATCD_WIFI_MODE_CLIENT       1
#define ATCD_WIFI_MODE_AP           2
#define ATCD_WIFI_MODE_SOFT_AP      3

// Udalosti WIFI
#define ATCD_WIFI_EV_NONE           0
#define ATCD_WIFI_EV_CONN           0b00000001
#define ATCD_WIFI_EV_GOT_IP         0b00000010
#define ATCD_WIFI_EV_DISCONN        0b00000100
#define ATCD_WIFI_EV_STA_CONN       0b00001000
#define ATCD_WIFI_EV_STA_IP         0b00010000
#define ATCD_WIFI_EV_STA_DISCONN    0b00100000

//------------------------------------------------------------------------------
typedef struct
{
  uint8_t state;                  //wifi state
  uint8_t mode;                   //wifi mode

  atcd_at_cmd_t at_cmd;           //AT cmd for internal usage  
  char at_cmd_buff[40];           //buffer pro sestaveny AT prikaz

  char *ssid;                     //SSID
  char *psswd;                    //password
  
  uint8_t events;                 //wifi events
  void (*callback)(uint8_t);      //events callback

} atcd_wifi_t;


// Functions -------------------------------------------------------------------
void atcd_wifi_proc();
uint8_t atcd_wifi_asc_msg();

//------------------------------------------------------------------------------
#endif /* ATCD_WIFI_H_INCLUDED */
