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
  //ATCD_P_MODE_SLEEP,          //modem je (mozna) v powersaving
  //ATCD_P_MODE_WAKING,         //cekani na probuzeni
  ATCD_P_MODE_IDLE,           //cekani nez poslu modem spat
  ATCD_P_MODE_ATC,            //cekani na OK, muze si odskocit do TX_PEND a PROMPT
  ATCD_P_MODE_IPD,
  ATCD_P_MODE_SMS

} atcd_parser_mode_t;

// Opravdu - s ohledem na stav vyse asi smazat
//#define ATCD_P_NO_TX_PENDING        0xFF

// Stav echa AT prikazu
#define ATCD_P_ECHO_OFF             0
#define ATCD_P_ECHO_ON              1

//------------------------------------------------------------------------------
typedef struct
{
  uint32_t atc_cnt;
  uint32_t atc_err;
  uint32_t atc_fail;
  uint32_t atc_tim;
  uint32_t atc_echo;

} atcd_parser_stat_t;

typedef struct
{
  char buff[ATCD_P_BUFF_SIZE];    //rx data buffer

  uint16_t buff_pos;              //position in buffer
  uint16_t line_pos;              //last line position in buffer

  atcd_parser_mode_t mode;        //parser mode
  uint8_t echo_en;                //AT cmd echo enable
  uint32_t timer;                 //plati mimo ATCD_P_MODE_ATC; fix: kdyz neprijde text SMS, zasekne se to uplne vsechno naporad

  uint32_t at_cmd_timer;          //timer provadeneho at prikazu

  struct atcd_at_cmd *at_cmd_top;      //AT command top queue
  struct atcd_at_cmd *at_cmd_end;      //AT command end queue

  uint8_t  rx_conn_num;          //connection number for +IPD
  uint16_t rx_data_len;          //+IPD data length
  uint16_t rx_data_pos;          //position in +IPD data

  atcd_parser_stat_t stat;       //statistiky

} atcd_parser_t;
//------------------------------------------------------------------------------

// Functions -------------------------------------------------------------------
// ATC Device
void atcd_parser_init();         //init AT command device
void atcd_parser_proc();         //Parser processing


//------------------------------------------------------------------------------
#endif /* ATCD_PARSER_H_INCLUDED */
