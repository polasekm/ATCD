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

  atcd_atc_init(&atcd.gprs.at_cmd);
  atcd.gprs.at_cmd_str[0] = 0;
  atcd.gprs.at_cmd_resp[0] = 0;

  atcd_atc_seq_init(&atcd.gprs.atc_seq);
  atcd.gprs.atc_seq.at_cmd    = &atcd.gprs.at_cmd;
  atcd.gprs.atc_seq.err_max   = 3;            //0 znamena neomezene - pozor, uint8, casem pretece - realne tedy 256, osetrit!!!!
  //atcd.gprs.atc_seq.make_step = &atcd_check_state_seq_step();    //mela by se nastavovat v init fce...

  //atcd.gprs.err_cnt = 0;
  atcd.gprs.timer = 0;

  atcd.gprs.apn = NULL;
  atcd.gprs.psswd = NULL;
  
  atcd.gprs.cb_events = ATCD_GPRS_EV_NONE;
  atcd.gprs.callback = NULL;
}
//------------------------------------------------------------------------------
void atcd_gprs_reset()                   //gprs state reset
{
  atcd.gprs.state = ATCD_GPRS_STATE_DISCONN;
  
  atcd_atc_init(&atcd.gprs.at_cmd);
  atcd.gprs.at_cmd_str[0] = 0;
  atcd.gprs.at_cmd_resp[0] = 0;

  //atcd.gprs.err_cnt = 0;
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

    if(atcd.gprs.atc_seq.state == ATCD_ATC_SEQ_STATE_ERROR)
    {
      // Pri zpracovani doslo k chybe
      atcd.gprs.state = ATCD_GPRS_STATE_DISCONNING;
      // Spustime sekvenci znovu - ne
      //atcd_atc_seq_run(&atcd.gprs.atc_seq);
    }

    //Cekame na pripojeni - test stavu a timeoutu...
    if(atcd_get_ms() - atcd.gprs.timer > 30000)
    {
      // Vyprsel timeout na spojeni
      atcd.gprs.timer = atcd_get_ms();
      atcd.gprs.state = ATCD_GPRS_STATE_DISCONNING;
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
  if(atcd.gprs.state != ATCD_GPRS_STATE_CONNECTING)
  {
    atcd_dbg_inf("GPRS: INIT: Zacina inicializace.\r\n");

    atcd.gprs.state = ATCD_GPRS_STATE_CONNECTING;
    atcd.gprs.timer = atcd_get_ms();
  }
}
//------------------------------------------------------------------------------
void atcd_gprs_disconnect()                //disconnect gprs
{
  if(atcd.gprs.state != ATCD_GPRS_STATE_DISCONNING)
  {
    atcd_dbg_inf("GPRS: DEINIT: Zacina deinicializace.\r\n");

    //atcd.gprs.atc_seq.make_step = &atcd_gprs_disconn_seq_step();
    atcd_atc_seq_run(&atcd.gprs.atc_seq);

    atcd.gprs.state = ATCD_GPRS_STATE_DISCONNING;
    atcd.gprs.timer = atcd_get_ms();
  }
}
//------------------------------------------------------------------------------
