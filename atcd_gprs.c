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
  atcd.gprs.autoclose_bearer = 0;

  atcd.gprs.apn = NULL;
  atcd.gprs.name = NULL;
  atcd.gprs.psswd = NULL;
  
  atcd.gprs.ip[0] = 0;

  atcd.gprs.cb_events = ATCD_GPRS_EV_NONE;
  atcd.gprs.callback = NULL;

  atcd.gprs.stat.bytes_sent=0;
  atcd.gprs.stat.bytes_recv=0;
}
//------------------------------------------------------------------------------
void atcd_gprs_reset()                   //gprs state reset
{
  atcd.gprs.state = ATCD_GPRS_STATE_DISCONN;
  atcd.gprs.timer = 0;

  atcd.gprs.ip[0] = 0;
}
//------------------------------------------------------------------------------
void atcd_gprs_reset_stat()
{
  atcd.gprs.stat.bytes_sent_base=atcd.gprs.stat.bytes_sent;
  atcd.gprs.stat.bytes_recv_base=atcd.gprs.stat.bytes_recv;
}
//------------------------------------------------------------------------------
void atcd_gprs_proc()                    //gprs connection processing
{
  switch (atcd.gprs.state)
  {
  case ATCD_GPRS_STATE_DISCONN:
    break;
  case ATCD_GPRS_STATE_DISCONNING:
    // Opravdu?
    //atcd.gprs.state = ATCD_GPRS_STATE_DISCONN;
    break;
  case ATCD_GPRS_STATE_CONNECTING:
    //Cekame na pripojeni - test stavu a timeoutu...
    if(atcd_get_ms() - atcd.gprs.timer > 90000) //jen prikaz AT+CIICR ma podle dokumentace az 85s
    {
      // Vyprsel timeout na spojeni
      ATCD_DBG_GPRS_TIMEOUT

      //atcd.gprs.timer = atcd_get_ms();
      //atcd.gprs.state = ATCD_GPRS_STATE_DISCONNING;
      atcd_gprs_disconnect(0);
    }
    break;
  case ATCD_GPRS_STATE_CONN:
    // Kontrola stavu?
    break;
  }
}
//------------------------------------------------------------------------------
void atcd_gprs_autoconn()
{
  /* Docasne zakazani autoodpojovani...
  Spojeni by se nemela otevirat pokud neni GPRS!!!
  TODO: Dodelat...
  if (atcd.gprs.state == ATCD_GPRS_STATE_CONN)
  {
    uint8_t i;
    for(i = 0; i < ATCD_CONN_MAX_NUMBER; i++)
    {
      if (atcd.conns.conn[i])
          return;
    }
    atcd_gprs_disconnect(0);  // 0 - false
  }
  else */if ((atcd.gprs.state == ATCD_GPRS_STATE_DISCONN) && (atcd.gprs.autoclose_bearer))
  {
    uint8_t i;
    for(i = 0; i < ATCD_CONN_MAX_NUMBER; i++)
    {
      if (atcd.conns.conn[i])
      {
        atcd_gprs_connect();
        return;
      };
    }
  }
}
//------------------------------------------------------------------------------
void atcd_gprs_autoclose_bearer(uint8_t autoclose_bearer)
{
  atcd.gprs.autoclose_bearer=autoclose_bearer;
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
void atcd_gprs_disconnect(uint8_t force)                //disconnect gprs
{
  //if(atcd.gprs.state != ATCD_GPRS_STATE_DISCONN && atcd.gprs.state != ATCD_GPRS_STATE_DISCONNING)
  if(force || atcd.gprs.state == ATCD_GPRS_STATE_CONN || atcd.gprs.state == ATCD_GPRS_STATE_CONNECTING)
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
atcd_gprs_state_t atcd_gprs_state()
{
  return atcd.gprs.state;
}
//------------------------------------------------------------------------------
