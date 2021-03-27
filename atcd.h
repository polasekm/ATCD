/*-----------------------------------------------------------------------------*/
/*
 * usart.h
 *
 *  Created on: Apr 9, 2011
 *      Author: Martin Polasek
 */
/*-----------------------------------------------------------------------------*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef ATCD_H_INCLUDED
#define ATCD_H_INCLUDED

#ifdef __cplusplus
 extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>     /* atoi */
#include <stdio.h>

#include "rbuff/rbuff.h"

#include "atcd_config.h"
#include "atcd_hw.h"

#include "atcd_atc.h"
#include "atcd_parser.h"

#include "atcd_sim.h"
#include "atcd_gsm.h"
#include "atcd_phone.h"
#include "atcd_gprs.h"
#include "atcd_wifi.h"
#include "atcd_gps.h"
#include "atcd_conn.h"
#include "atcd_conns.h"

/* Exported functions ------------------------------------------------------- */

/* Defines -------------------------------------------------------------------*/


//---------------------------------------
// Stav samotneho zarizeni. Proihlaseni do site, wifi, ci gprs je v prislusnych 
// modulech
#define ATCD_STATE_OFF              0
#define ATCD_STATE_STARTING         1
#define ATCD_STATE_NO_INIT          2
#define ATCD_STATE_SLEEP            3
#define ATCD_STATE_ON               4

#define ATCD_SM_OFF                 0
#define ATCD_SM_AUTO                1
#define ATCD_SM_MANUAL              2

#define ATCD_EV_NONE                0x00
#define ATCD_EV_STATE               0b00000001
#define ATCD_EV_ASYNC_MSG           0b00000010
#define ATCD_EV_ALL                 0xFF

// Navratove hodnoty
#define ATCD_OK                     0
#define ATCD_ERR                    1
#define ATCD_ERR_LOCK               2

// Switch base offsets
#define ATCD_SB_INIT                0
#define ATCD_SB_STAT                100

#define ATCD_SB_PHONE               200

#define ATCD_SB_GPRS_INIT           300
#define ATCD_SB_GPRS_DEINIT         400

#define ATCD_SB_CONN_OPEN           500
#define ATCD_SB_CONN_WRITE          600
#define ATCD_SB_CONN_READ           700
#define ATCD_SB_CONN_CLOSE          800

#define ATCD_SB_GPS_START           900
#define ATCD_SB_GPS_STOP            1000

#define ATCD_SB_END                 1100

#define ATCD_SO_ERR                 98
#define ATCD_SO_END                 99
//------------------------------------------------------------------------------
typedef struct
{
  uint8_t state;                  //device state
  uint8_t sleep_mode;             //device sleep mode
  uint32_t timer;                 //current operation timer

  uint16_t proc_step;
  uint16_t err_cnt;
  uint16_t err_max;
  
  atcd_parser_t parser;           //AT cmd parser

  atcd_at_cmd_t at_cmd;           //AT cmd for internal usage
  char at_cmd_buff[64];           //buffer pro sestaveny AT prikaz - pohlidat delku a mozne preteceni...

  atcd_conns_t conns;             //TCP/UDP conections

  // Podmineny preklad
  atcd_sim_t sim;                 //
  atcd_gsm_t gsm;                 //
  atcd_phone_t phone;             //
  atcd_gprs_t gprs;               //
  atcd_gps_t gps;                 //
  atcd_wifi_t wifi;               //

  uint8_t cb_events;              //device callback events
  void (*callback)(uint8_t);      //events callback
  
} atcd_t;

// Functions -------------------------------------------------------------------
// ATC Device
void atcd_init();                //init AT command device
void atcd_reset();               //reset AT command device
void atcd_start();               //start AT command device

void atcd_set_sleep_mode(uint8_t mode);          //set sleep mode
 
void atcd_rx_data(uint8_t *data, uint16_t len);  //zpracuje prijata data
void atcd_rx_str(char *ch);                      //zpracuje prijaty retezec
void atcd_rx_ch(char ch);                        //zpracuje prijaty znak

void atcd_tx_complete();         //call on tx data complete
void atcd_sw_reset();            //SW reset

void atcd_proc();                //data processing 

uint8_t atcd_state();
//--------------------------------------------------------------
// nemela by byt lokalni?
uint16_t atcd_proc_step();       //pruchod jednim krokem kolecka modemu
//------------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif /* ATCD_H_INCLUDED */
