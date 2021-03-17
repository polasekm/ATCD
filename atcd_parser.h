/*-----------------------------------------------------------------------------*/
/*
 * usart.h
 *
 *  Created on: Apr 9, 2011
 *      Author: Martin Polasek
 */
/*-----------------------------------------------------------------------------*/
#ifndef ATCD_PARSER_H_INCLUDED
#define ATCD_PARSER_H_INCLUDED

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>     /* atoi */
#include <stdio.h>

#include "../rbuff/rbuff.h"

#include "atcd_config.h"

#include "atcd_atc.h"

/* Exported functions ------------------------------------------------------- */

/* Defines -------------------------------------------------------------------*/

// Mod parseru prichozich dat
#define ATCD_P_MODE_ATC             0
#define ATCD_P_MODE_IPD             1
#define ATCD_P_MODE_SMS             2
#define ATCD_P_MODE_TX_PEND         3
#define ATCD_P_MODE_PROMPT          4

// Opravdu - s ohledem na stav vyse asi smazat
#define ATCD_P_NO_TX_PENDING        0xFF

// Stav echa AT prikazu
#define ATCD_P_ECHO_OFF             0
#define ATCD_P_ECHO_ON              1

// Stav odesilani dat
#define ATCD_P_TX_COMPLETE          0
#define ATCD_P_TX_ONGOING           1

//------------------------------------------------------------------------------
typedef struct
{
  char buff[ATCD_P_BUFF_SIZE];      //rx data buffer
  
  uint16_t buff_pos;              //position in buffer
  uint16_t line_pos;              //last line position in buffer

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

// Functions -------------------------------------------------------------------
// ATC Device
void atcd_parser_init();         //init AT command device


//------------------------------------------------------------------------------
#endif /* ATCD_PARSER_H_INCLUDED */
