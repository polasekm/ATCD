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
  atcd.parser.buff[0]                  = 0;
  atcd.parser.buff[ATCD_P_BUFF_SIZE - 1] = 0;
  
  atcd.parser.buff_pos   = 0;
  atcd.parser.line_pos   = 0;
  
  atcd.parser.mode       = ATCD_P_MODE_SLEEP;
  atcd.parser.echo_en    = ATCD_P_ECHO_ON;

  atcd.parser.topat_state_timer = atcd_get_ms();        //opravdu nastavit na aktualni cas?
  atcd.parser.mode_timer = atcd_get_ms();
  
  atcd.parser.at_cmd_top = NULL;
  atcd.parser.at_cmd_end = NULL;
  
  atcd.parser.tx_state    = ATCD_P_TX_COMPLETE;
  atcd.parser.tx_conn_num = 0;
  atcd.parser.tx_data_len = 0;
  rbuff_init(&atcd.parser.tx_rbuff, NULL, 0);

  atcd.parser.rx_conn_num = 0;
  atcd.parser.rx_data_len = 0;
  atcd.parser.rx_data_pos = 0;

}
//------------------------------------------------------------------------------
