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
  if((atcd.parser.mode == ATCD_P_MODE_IPD || atcd.parser.mode == ATCD_P_MODE_SMS || atcd.parser.mode == ATCD_P_MODE_BINARY) &&
     (atcd_get_ms() - atcd.parser.timer > 4000))
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
uint8_t atcd_parser_binary_proc(char ch)
{
  // If parser in binary receiving mode
  if(atcd.parser.mode == ATCD_P_MODE_BINARY)
  {
    // Pokud je v bufferu misto
    if(atcd.parser.buff_pos < ATCD_P_BUFF_SIZE - 1)
    {
      // Zapise prijaty byte do bufferu
      atcd.parser.buff[atcd.parser.buff_pos++] = ch;
    }
    else
    {
      //ATCD_DBG_PHONE_SMS_BUFF_E
      //conn->cb_events |=  ATCD_CONN_EV_OVERRUN;
    }

    // Pokud jsme dosahli konce bloku s textem SMS
    //if(atcd.buff_pos >= atcd.parser.ipd_len)
    if (atcd.parser.buff_pos - atcd.parser.line_pos>=11 &&
        atcd.parser.buff[atcd.parser.line_pos]=='+' &&
        strncmp(atcd.parser.buff+atcd.parser.line_pos, "+CME ERROR:", 11)==0)
    {
      atcd.parser.mode = ATCD_P_MODE_IDLE;
      atcd_dbg_inf2("ATCD: PARSER: ", "+CME ERROR in binary\r\n");
    };

    if(atcd.parser.buff_pos - atcd.parser.line_pos >= atcd.parser.binary_len)
    {
      //ATCD_DBG_PHONE_SMS_END
      atcd_at_cmd_t *at_cmd;
      at_cmd = atcd.parser.at_cmd_top;
      // Pokud se zpracovava nejaky ATC
      if(at_cmd != NULL)
      {
        uint16_t len=atcd.parser.binary_len;
        atcd_dbg_inf2("ATCD: PARSER: ", "end in binary, cmd is\r\n");
        if (at_cmd->resp_buff_size < len)
          len=at_cmd->resp_buff_size;
        memmove(at_cmd->resp, atcd.parser.buff+atcd.parser.line_pos, len);
        at_cmd->resp_len=len;
        //jakoby prepnu na AT prikazy ale jeste nekoncim, prijde OK
      }
      else
        atcd_dbg_inf2("ATCD: PARSER: ", "end in binary, cmd NULL\r\n");


      atcd.parser.mode = ATCD_P_MODE_IDLE;
      atcd.parser.timer = atcd_get_ms();

      //atcd.parser.buff_pos = 0;
      atcd.parser.line_pos = atcd.parser.buff_pos;

      atcd.parser.buff[atcd.parser.buff_pos] = 0;
    }

    return 1;
  }

  return 0;
}
//------------------------------------------------------------------------------
void atcd_parser_expect_binary(uint16_t len)
{
  atcd.parser.mode = ATCD_P_MODE_BINARY;
  atcd.parser.timer = atcd_get_ms();
  atcd.parser.buff_pos=0;
  atcd.parser.line_pos=0;
  atcd.parser.binary_len=len;
}
//------------------------------------------------------------------------------
