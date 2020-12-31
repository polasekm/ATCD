/*
 * usart.c
 *
 *  Created on: Apr 9, 2011
 *      Author: Martin
 */
//------------------------------------------------------------------------------
#include "atcd_gsm.h"
#include "atcd.h"

extern atcd_t atcd;

//------------------------------------------------------------------------------
void atcd_gsm_init()
{
  atcd.gsm.state     = ATCD_REG_STATE_OFF;
  atcd.gsm.cb_events = ATCD_GSM_EV_ALL;
  atcd.gsm.callback  = NULL;
}
//------------------------------------------------------------------------------
void atcd_gsm_reset()
{
  atcd.gsm.state     = ATCD_REG_STATE_OFF;
}
//------------------------------------------------------------------------------
void atcd_gsm_proc()
{


}
//------------------------------------------------------------------------------
uint8_t atcd_gsm_asc_msg()
{
  uint8_t val;
  uint8_t state_p;

  if(strncmp(atcd.parser.buff + atcd.parser.line_pos, "+CREG: ", strlen("+CREG: ")) == 0)
  {
    val = (uint8_t)atoi(atcd.parser.buff + atcd.parser.buff_pos - ATCD_RX_NL_LEN - 1);

    if(val <= 10)
    {
      ATCD_DBG_CREG
      state_p = atcd.gsm.state;

      atcd.gsm.state = val;

      //Prechod na roaming nemusi znamenat reset vsech spojeni - opravit.
      if(state_p != val)
      {
        atcd_conn_reset_all();
        if(atcd.gsm.callback != NULL && (atcd.gsm.cb_events & ATCD_GSM_EV_REG) != 0) atcd.gsm.callback(ATCD_GSM_EV_REG);
      }
    }
    else
    {
      ATCD_DBG_CREG_ERR
      atcd_conn_reset_all();
    }

    //+CREG nesmi prijit v tele jineho ATC... pokud se ukaze dukaz ze prisel i jindy, dospecifikovat i konkretni ATC
    //Zvazit zda nenapsat test, ktery by tyto situace vyhledaval...
    if(atcd.parser.at_cmd_top == NULL || atcd.parser.at_cmd_top->state != ATCD_ATC_STATE_W_END)
    {
        return 0;
    }

    atcd.parser.buff_pos = atcd.parser.line_pos;
    return 1;
  }

  return 0;
}
//------------------------------------------------------------------------------
