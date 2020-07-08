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
#define ATCD_STATE_SLEEP            1
#define ATCD_STATE_STARTING         2
#define ATCD_STATE_ON               3
#define ATCD_STATE_INIT             4
#define ATCD_STATE_READY            5

#define ATCD_EV_NONE                0x00
#define ATCD_EV_STATE               0b00000001
#define ATCD_EV_ASYNC_MSG           0b00000010
#define ATCD_EV_ALL                 0xFF

// Mod parseru prichozich dat
#define ATCD_P_MODE_ATC             0
#define ATCD_P_MODE_IPD             1
#define ATCD_P_MODE_TX_PEND         2
#define ATCD_P_MODE_PROMPT          3

// Opravdu - s ohledem na stav vyse asi smazat
#define ATCD_P_NO_TX_PENDING        0xFF

// Stav echa AT prikazu
#define ATCD_P_ECHO_OFF             0
#define ATCD_P_ECHO_ON              1

// Stav odesilani dat
#define ATCD_P_TX_COMPLETE          0
#define ATCD_P_TX_ONGOING           1

// Navratove hodnoty
#define ATCD_OK                     0
#define ATCD_ERR                    1
#define ATCD_ERR_LOCK               2

//------------------------------------------------------------------------------
typedef struct
{
  uint8_t mode;                   //parser mode
  uint8_t echo_en;                //AT cmd echo enable  

  uint32_t timer;                 //current operation timer

  struct atcd_at_cmd *at_cmd_top;      //AT command top queue
  struct atcd_at_cmd *at_cmd_end;      //AT command end queue

  uint8_t  tx_state;              //transmission state
  uint8_t  tx_conn_num;
  uint16_t tx_data_len;
  rbuff_t  tx_rbuff;
  
  uint8_t  rx_conn_num;          //connection number for +IPD
  uint16_t rx_data_len;          //+IPD data length
  uint16_t rx_data_pos;          //position in +IPD data 

} atcd_parser_t;
//------------------------------------------------------------------------------
typedef struct
{
  uint8_t state;                  //device state
  uint32_t timer;                 //current operation timer
  uint8_t at_cmd_seq;             //phase of initialization
  
  char buff[ATCD_BUFF_SIZE];      //rx data buffer
  
  uint16_t buff_pos;              //position in buffer
  uint16_t line_pos;              //last line position in buffer
  
  atcd_parser_t parser;           //AT cmd parser

  atcd_at_cmd_t at_cmd;           //AT cmd for internal usage
  char at_cmd_buff[40];           //buffer pro sestaveny AT prikaz

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
 
void atcd_rx_data(uint8_t *data, uint16_t len);
void atcd_rx_str(char *ch);
void atcd_rx_ch(char ch);
void atcd_tx_complete();         //call on tx data complete

void atcd_proc();                //data processing 
//--------------------------------------------------------------
// Implementace pro jednotlive modemy...
void atcd_init_seq();
void atcd_restart_seq();
void atcd_check_state_seq();
//--------------------------------------------
void atcd_gprs_init_seq();
void atcd_gprs_deinit_seq();
void atcd_gprs_check_state_seq();
//--------------------------------------------
void atcd_conns_check_state_seq();
//--------------------------------------------
void atcd_conn_open_seq(atcd_conn_t *conn);
void atcd_conn_close_seq(atcd_conn_t *conn);
void atcd_conn_check_state_seq(atcd_conn_t *conn);

void atcd_conn_read_seq(atcd_conn_t *conn);
void atcd_conn_write_seq(atcd_conn_t *conn);
//------------------------------------------------------------------------------
#endif /* ATCD_H_INCLUDED */
