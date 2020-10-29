/*-----------------------------------------------------------------------------*/
/*
 * usart.h
 *
 *  Created on: Apr 9, 2011
 *      Author: Martin Polasek
 */
/*-----------------------------------------------------------------------------*/
#ifndef ATCD_H_INCLUDED
#define ATCD_H_INCLUDED

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>     /* atoi */
#include <stdio.h>

#include "../rbuff/rbuff.h"

#include "atcd_config.h"
#include "atcd_hw.h"

#include "atcd_atc.h"
#include "atcd_atc_seq.h"
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
#define ATCD_STATE_SLEEP            2
#define ATCD_STATE_ON               3

// Prekryva se se stavem inicializacni sekvence...
/*#define ATCD_INIT_NONE              0
#define ATCD_INIT_RUN               1
#define ATCD_INIT_DONE              2*/

#define ATCD_EV_NONE                0x00
#define ATCD_EV_STATE               0b00000001
#define ATCD_EV_ASYNC_MSG           0b00000010
#define ATCD_EV_ALL                 0xFF

// Navratove hodnoty
#define ATCD_OK                     0
#define ATCD_ERR                    1
#define ATCD_ERR_LOCK               2

//------------------------------------------------------------------------------
typedef struct
{
  uint8_t state;                  //device state
  //uint8_t init_state;             //device init state    // Prekryva se se stavem inicializacni sekvence...
  uint32_t timer;                 //current operation timer
  atcd_at_cmd_seq_t init_seq;     //inicializacni sekvence
  atcd_at_cmd_seq_t state_seq;    //sekvence kontroly stavu
  
  atcd_parser_t parser;           //AT cmd parser

  atcd_at_cmd_t at_cmd;           //AT cmd for internal usage
  char at_cmd_buff[64];           //buffer pro sestaveny AT prikaz

  atcd_conns_t conns;             //TCP/UDP conections

  // Podmineny preklad
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
 
void atcd_rx_data(uint8_t *data, uint16_t len);  //zpracuje prijata data
void atcd_rx_str(char *ch);                      //zpracuje prijaty retezec
void atcd_rx_ch(char ch);                        //zpracuje prijaty znak

void atcd_tx_complete();         //call on tx data complete

void atcd_proc();                //data processing 
//--------------------------------------------------------------
// Implementace budou pro jednotlive modemy...

void atcd_init_seq_step(atcd_at_cmd_seq_t *atc_seq);
void atcd_restart_seq_step(atcd_at_cmd_seq_t *atc_seq);
void atcd_check_state_seq_step(atcd_at_cmd_seq_t *atc_seq);
//--------------------------------------------
void atcd_gprs_init_seq_step(atcd_at_cmd_seq_t *atc_seq);
void atcd_gprs_deinit_seq_step(atcd_at_cmd_seq_t *atc_seq);
void atcd_gprs_check_state_seq_step(atcd_at_cmd_seq_t *atc_seq);
//--------------------------------------------
void atcd_conns_check_state_seq(atcd_at_cmd_seq_t *atc_seq);
//--------------------------------------------
void atcd_conn_open_seq(atcd_conn_t *conn);
void atcd_conn_close_seq(atcd_conn_t *conn);
void atcd_conn_check_state_seq(atcd_conn_t *conn);

void atcd_conn_read_seq(atcd_conn_t *conn);
void atcd_conn_write_seq(atcd_conn_t *conn);
//------------------------------------------------------------------------------
#endif /* ATCD_H_INCLUDED */
