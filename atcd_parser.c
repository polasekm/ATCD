/*
 * usart.c
 *
 *  Created on: Apr 9, 2011
 *      Author: Martin
 */
//------------------------------------------------------------------------------
#include "atcd_parser.h"
#include "atcd.h"

extern atcd_t atcd;

//char *strs[NUMBER_OF_STRINGS] = {"foo", "bar", "bletch", ...};
//char *strs[4] = {"foo", "bar", "bletch", ""};

extern rbuff_t atcd_rx_ring_buff;         //kruhovy buffer pro prijimana data

//------------------------------------------------------------------------------
void atcd_parser_init()                  //state machine reset
{
  atcd.parser.buff[0]                    = 0;
  atcd.parser.buff[ATCD_P_BUFF_SIZE - 1] = 0;
  
  atcd.parser.buff_pos   = 0;
  atcd.parser.line_pos   = 0;
  
  atcd.parser.mode       = ATCD_P_MODE_IDLE;
  atcd.parser.echo_en    = ATCD_P_ECHO_ON;

  atcd.parser.at_cmd_timer = atcd_get_ms();
  atcd.parser.timer        = atcd_get_ms();
  
  atcd.parser.at_cmd_top = NULL;
  atcd.parser.at_cmd_end = NULL;

  atcd.parser.rx_conn_num = 0;
  atcd.parser.rx_data_len = 0;
  atcd.parser.rx_data_pos = 0;

  atcd.parser.stat.atc_cnt = 0;
  atcd.parser.stat.atc_err = 0;
  atcd.parser.stat.atc_fail = 0;
  atcd.parser.stat.atc_tim = 0;
  atcd.parser.stat.atc_echo = 0;
}
//------------------------------------------------------------------------------
void atcd_parser_proc()                //Parser processing
{
  if((atcd.parser.mode == ATCD_P_MODE_IPD || atcd.parser.mode == ATCD_P_MODE_SMS) && (atcd_get_ms() - atcd.parser.timer > 4000))
  {
    //Vyprsel timeout na IPD nebo SMS
    ATCD_DBG_IPD_SMS_TIM
    atcd.parser.mode = ATCD_P_MODE_IDLE;
    //TODO: osetrit spojeni kde dochazelo k prijmu dat...
  }

  //TODO: Idealne asi dopsat prodlouzeni casu na ATC pokud probihal IPD ci SMS...
  /*if(atcd.parser.mode != ATCD_P_MODE_IDLE)
  {
    atcd.parser.at_cmd_timer = atcd_get_ms();
  }*/

  // Kontrola, zda neni cekajici k odeslani
  //atcd_atc_queue_proc();
}
//------------------------------------------------------------------------------
