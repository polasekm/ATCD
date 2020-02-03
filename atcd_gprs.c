/*
 * usart.c
 *
 *  Created on: Apr 9, 2011
 *      Author: Martin
 */
//------------------------------------------------------------------------------
#include "atcd_gprs.h"
#include "atcd.h"

extern atcd_t atcd;
//--------------------------------------------------------------
// GPRS

void atcd_gprs_start_connect();           //gprs start connect
void atcd_gprs_start_disconnect();        //gprs start disconnect

//------------------------------------------------------------------------------
void atcd_gprs_init(atcd_gprs_t *gprs)      //inializace gprs
{
  gprs->state = ATCD_GPRS_STATE_DISCONN; 
  gprs->mode = ATCD_GPRS_MODE_DISCONN;

  atcd_atc_init(&gprs->at_cmd);
  gprs->at_cmd_str[0] = 0;
  gprs->at_cmd_resp[0] = 0;

  gprs->at_cmd_seq = 0;
  gprs->err_cnt = 0;
  gprs->timer = 0;

  gprs->apn = NULL;
  gprs->psswd = NULL;
  
  gprs->events = ATCD_GPRS_EV_NONE;
  gprs->callback = NULL;
}
//------------------------------------------------------------------------------
void atcd_gprs_reset()                   //gprs state reset
{
  atcd.gprs.state = ATCD_GPRS_STATE_DISCONN;
  
  atcd_atc_init(&atcd.gprs.at_cmd);
  atcd.gprs.at_cmd_str[0] = 0;
  atcd.gprs.at_cmd_resp[0] = 0;

  atcd.gprs.at_cmd_seq = 0;
  atcd.gprs.err_cnt = 0;
}
//------------------------------------------------------------------------------
void atcd_gprs_proc()                    //gprs connection processing
{
  // Dle pozadovaneho a aktualniho stavu GPRS pripojeni volit cinnost
  switch(atcd.gprs.state)
  {
    //---------------------------------------------
    case ATCD_GPRS_STATE_DISCONN:
      if(atcd.gprs.mode == ATCD_GPRS_MODE_CONN) atcd_gprs_start_connect();
      else atcd_gprs_check_state_seq();
      break;
    //---------------------------------------------
    case ATCD_GPRS_STATE_CONN:
      if(atcd.gprs.mode == ATCD_GPRS_MODE_DISCONN) atcd_gprs_start_disconnect();
      else atcd_gprs_check_state_seq();
      break;
    //---------------------------------------------
    case ATCD_GPRS_STATE_CONNECTING:
      if(atcd.gprs.mode == ATCD_GPRS_MODE_DISCONN) atcd_gprs_start_disconnect();
      else
      {
        if(atcd_get_ms() - atcd.gprs.timer > 50000)
        {
          atcd_dbg_warn("GPRS: Vyprsel timeout na pripojeni.\r\n");
          atcd_gprs_start_connect();
        }
        else atcd_gprs_check_state_seq();
      }
      break;
    //---------------------------------------------
    case ATCD_GPRS_STATE_INIT:
      if(atcd.gprs.mode == ATCD_GPRS_MODE_DISCONN) atcd_gprs_start_disconnect();
      else atcd_gprs_init_seq();
      break;
    //---------------------------------------------
    case ATCD_GPRS_STATE_DEINIT:
      if(atcd.gprs.mode == ATCD_GPRS_MODE_CONN) atcd_gprs_start_connect();
      else atcd_gprs_deinit_seq();
      break;
    //---------------------------------------------
    default:
      atcd_dbg_err("GPRS: Stav GPRS je mimo rozah.\r\n");
      atcd_gprs_start_disconnect();
  }
}
//------------------------------------------------------------------------------
void atcd_gprs_start_connect()              //gprs start connect
{
  if(atcd.state == ATCD_STATE_READY && atcd.phone.state == ATCD_PHONE_STATE_REG_HOME)
  {
    atcd_dbg_inf("GPRS: INIT: Zacina inicializace.\r\n");
    atcd_atc_cancell(&atcd.gprs.at_cmd);
    atcd.gprs.state = ATCD_GPRS_STATE_INIT;
    atcd.gprs.at_cmd_seq = 0;
    atcd.gprs.err_cnt = 0;
  }
}
//------------------------------------------------------------------------------
void atcd_gprs_start_disconnect()           //gprs start disconnect
{
  atcd_dbg_inf("GPRS: DEINIT: Zacina deinicializace.\r\n");
  atcd_atc_cancell(&atcd.gprs.at_cmd);
  atcd.gprs.state = ATCD_GPRS_STATE_DEINIT;
  atcd.gprs.at_cmd_seq = 0;
  atcd.gprs.err_cnt = 0;
}
//------------------------------------------------------------------------------
void atcd_gprs_connect()                    //connect gprs
{
  atcd.gprs.mode = ATCD_GPRS_MODE_CONN;
}
//------------------------------------------------------------------------------
void atcd_gprs_disconnect()                //disconnect gprs
{
  atcd.gprs.mode = ATCD_GPRS_MODE_DISCONN;
}
//------------------------------------------------------------------------------
