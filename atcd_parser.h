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

#include "../Libs/rbuff/rbuff.h"

#include "atcd_config.h"

#include "atcd_atc.h"

/* Exported functions ------------------------------------------------------- */

/* Defines -------------------------------------------------------------------*/

// Mod parseru prichozich dat
typedef enum
{
  ATCD_P_MODE_SLEEP,   //modem je (mozna) v powersaving
  ATCD_P_MODE_WAKING,  //cekani na probuzeni
  ATCD_P_MODE_IDLE,    //cekani nez poslu modem spat
  ATCD_P_MODE_WAITOK,  //cekani na OK, muze si odskocit do TX_PEND a PROMPT
  ATCD_P_MODE_IPD,
  ATCD_P_MODE_IPD_WAITOK,
  ATCD_P_MODE_IPD_SLEEP,
  ATCD_P_MODE_SMS_,
  ATCD_P_MODE_SMS_WAITOK,
  ATCD_P_MODE_SMS_SLEEP,
  ATCD_P_MODE_TX_PEND, //cekani na prompt protoze cmd->data!=NULL
  ATCD_P_MODE_PROMPT  //jen uvnitr funkce; posilani cmd->data
} atcd_parser_mode_e;

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
  char buff[ATCD_P_BUFF_SIZE];    //rx data buffer
  
  uint16_t buff_pos;              //position in buffer
  uint16_t line_pos;              //last line position in buffer

  atcd_parser_mode_e mode;        //parser mode
  uint8_t echo_en;                //AT cmd echo enable
  uint32_t mode_timer;            //plati mimo ATCD_P_MODE_ATC; fix: kdyz neprijde text SMS, zasekne se to uplne vsechno naporad

  uint32_t topat_state_timer;     //current operation timer

  struct atcd_at_cmd *at_cmd_top;      //AT command top queue
  struct atcd_at_cmd *at_cmd_end;      //AT command end queue

  uint8_t  tx_state;              //transmission state
  uint8_t  tx_conn_num;
  uint16_t tx_data_len;
  rbuff_t  tx_rbuff;
  
  uint8_t  rx_conn_num;          //connection number for +IPD
  uint16_t rx_data_len;          //+IPD data length
  uint16_t rx_data_pos;          //position in +IPD data

  //->mode_time uint32_t sleep_timer; //ATCD_P_MODE_WAKING a ATCD_P_MODE_FADING, mozna by sel spojit s timer "current operation timer"
  //uint8_t atcd_sleep_state;
  //uint8_t atcd_tx_pending;

} atcd_parser_t;
//------------------------------------------------------------------------------

// Functions -------------------------------------------------------------------
// ATC Device
void atcd_parser_init();         //init AT command device


//------------------------------------------------------------------------------
#endif /* ATCD_PARSER_H_INCLUDED */
