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

//------------------------------------------------------------------------------
void atcd_gprs_init()      //inializace gprs
{
  atcd.gprs.state = ATCD_GPRS_STATE_DISCONN;
  atcd.gprs.timer = 0;

  atcd.gprs.apn = NULL;
  atcd.gprs.name = NULL;
  atcd.gprs.psswd = NULL;
  
  atcd.gprs.cb_events = ATCD_GPRS_EV_NONE;
  atcd.gprs.callback = NULL;
}
//------------------------------------------------------------------------------
void atcd_gprs_reset()                   //gprs state reset
{
  atcd.gprs.state = ATCD_GPRS_STATE_DISCONN;
  atcd.gprs.timer = 0;
}
//------------------------------------------------------------------------------
void atcd_gprs_proc()                    //gprs connection processing
{
  if(atcd.gprs.state == ATCD_GPRS_STATE_DISCONN)
  {
    
  }
  else if(atcd.gprs.state == ATCD_GPRS_STATE_DISCONNING)
  {
    // Opravdu?
    //atcd.gprs.state = ATCD_GPRS_STATE_DISCONN;
  }
  else if(atcd.gprs.state == ATCD_GPRS_STATE_CONNECTING)
  {
    //Cekame na pripojeni - test stavu a timeoutu...
    if(atcd_get_ms() - atcd.gprs.timer > 30000)
    {
      // Vyprsel timeout na spojeni
      ATCD_DBG_GPRS_TIMEOUT

      //atcd.gprs.timer = atcd_get_ms();
      //atcd.gprs.state = ATCD_GPRS_STATE_DISCONNING;
      atcd_gprs_disconnect();
    }
  }
  else if(atcd.gprs.state == ATCD_GPRS_STATE_CONN)
  {
    // Kontrola stavu?
  }
}
//------------------------------------------------------------------------------
void atcd_gprs_connect()                    //connect gprs
{
  if(atcd.gprs.state == ATCD_GPRS_STATE_DISCONN || atcd.gprs.state == ATCD_GPRS_STATE_DISCONNING)
  //if(atcd.gprs.stat == ATCD_GPRS_STATE_DISCONN)
  {
    ATCD_DBG_GPRS_CONN_SET

    atcd.gprs.state = ATCD_GPRS_STATE_CONNECTING;
    atcd.gprs.timer = atcd_get_ms();
  }
}
//------------------------------------------------------------------------------
void atcd_gprs_disconnect()                //disconnect gprs
{
  //if(atcd.gprs.state != ATCD_GPRS_STATE_DISCONN && atcd.gprs.state != ATCD_GPRS_STATE_DISCONNING)
  if(atcd.gprs.state == ATCD_GPRS_STATE_CONN || atcd.gprs.state == ATCD_GPRS_STATE_CONNECTING)
  //if(atcd.gprs.state == ATCD_GPRS_STATE_CONN)
  {
    ATCD_DBG_GPRS_DISCONN_SET

    atcd.gprs.state = ATCD_GPRS_STATE_DISCONNING;
    atcd.gprs.timer = atcd_get_ms();
  }
}
//------------------------------------------------------------------------------
void atcd_gprs_set_apn(char *apn, char *name, char *psswd)
{
  atcd.gprs.apn = apn;
  atcd.gprs.name = name;
  atcd.gprs.psswd = psswd;
}
//------------------------------------------------------------------------------
