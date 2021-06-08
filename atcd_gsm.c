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
      //prev  new  ->              prev in [1,5]
      //init  0,2,3   (reset)         f
      //init  1,5     NO RESET        f
      //0,2,3 0,2,3   (reset)         f
      //0,2,3 1       no reset        f
      //0,2,3 5       no reset        f
      //1     0,2,3   reset           t
      //1     5       reset           t
      //5     0,2,3   reset           t
      //5     1       reset           t
      //prvni AT+CREG? se dela az po otevreni GPRS a po tomhle bych si zavolal atcd_gprs_disconnect
      if(state_p != val)
        if ((state_p==1) ||
            (state_p==5) ||
            ((val!=1) && (val!=5)))
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
uint8_t atcd_gsm_state()
{
  return atcd.gsm.state;
}
//------------------------------------------------------------------------------
