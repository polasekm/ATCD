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


}
//------------------------------------------------------------------------------
void atcd_gsm_reset()
{

}
//------------------------------------------------------------------------------
void atcd_gsm_proc()
{


}
//------------------------------------------------------------------------------
uint8_t atcd_gsm_asc_msg()
{
  uint16_t op;
  uint8_t val;
  uint8_t state_p;

  if(strncmp(atcd.buff + atcd.line_pos, "+CREG: ", strlen("+CREG: ")) == 0)
  {
    val = (uint8_t)atoi(atcd.buff + atcd.buff_pos - ATCD_RX_NL_LEN - 1);

    if(val >= 0 && val <= 10)
    {
      ATCD_DBG_CREG
      state_p = atcd.gsm.state;

      atcd.gsm.state = val;
      atcd.buff_pos  = atcd.line_pos;

      //Pokud probihal ATC, nemazat z odpovedi...

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
      atcd.buff_pos = atcd.line_pos;
    }
    
    return 1;
  }

  return 0;
}
//------------------------------------------------------------------------------
