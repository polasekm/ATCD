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
  atcd.gsm.gsm_sig = -1;
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
  const char *str = atcd.parser.buff + atcd.parser.line_pos;

  if(strncmp(str, "+CREG: ", strlen("+CREG: ")) == 0)
  { //muze prijit +CREG: 0,1 ale taky +CREG: 2,1,"9664","3873" a nevyzadana +CREG: 1 nebo +CREG: 1,"9664","3873" ano i +CREG: (0-2)
    if ((str[8]=='\r') || (str[8]==0) || ((str[8]==',') && (str[9]=='\"')))
      val = (uint8_t)atoi(str+7); //+CREG: 1 nebo +CREG: 1,"966...
    else
      val = (uint8_t)atoi(str+9); //+CREG: 0,1 nebo +CREG: 2,1,"966...

    if(val <= ATCD_REG_STATE__MAX)
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
      { //!(s==0 && v==2 || s==2 && v==1) = (s!=0 || v!=2) && (s!=2 || v!=1)
        if ((state_p!=0 || val!=2) && (state_p!=2 || val!=1)) //z initu do 2 a z 2 do 1 je normalka, to nemusis logovat
        {
          char buf[10];
          snprintf(buf, sizeof(buf), "%d->%d", state_p, val);
          atcd_dbg_inf3("ATCD CREG", buf);
        };
        if ((state_p==1) ||
            (state_p==5) ||
            ((val!=1) && (val!=5)))
        {
          atcd_conn_reset_all();
          if(atcd.gsm.callback != NULL && (atcd.gsm.cb_events & ATCD_GSM_EV_REG) != 0) atcd.gsm.callback(ATCD_GSM_EV_REG);
        }
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
atcd_reg_state_e atcd_gsm_state()
{
  return atcd.gsm.state;
}
//------------------------------------------------------------------------------
int8_t atcd_gsm_sig()
{
  return atcd.gsm.gsm_sig;
}
//------------------------------------------------------------------------------
