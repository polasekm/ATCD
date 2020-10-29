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
void atcd_gprs_init()      //inializace gprs
{
  atcd.gprs.state = ATCD_GPRS_STATE_DISCONN;
  atcd.gprs.mode = ATCD_GPRS_MODE_DISCONN;

  atcd_atc_init(&atcd.gprs.at_cmd);
  atcd.gprs.at_cmd_str[0] = 0;
  atcd.gprs.at_cmd_resp[0] = 0;

  atcd_atc_seq_init(&atcd.gprs.atc_seq);
  atcd.gprs.atc_seq.at_cmd    = &atcd.gprs.at_cmd;
  atcd.gprs.atc_seq.err_max   = 3;            //0 znamena neomezene - pozor, uint8, casem pretece - realne tedy 256, osetrit!!!!
  atcd.gprs.atc_seq.make_step = &atcd_check_state_seq_step();    //mela by se nastavovat v init fce...

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
  // Dle pozadovaneho a aktualniho stavu GPRS pripojeni volit cinnost
  switch(atcd.gprs.state)
  {
    //---------------------------------------------
    case ATCD_GPRS_STATE_DISCONN:
      if(atcd.gprs.mode == ATCD_GPRS_MODE_CONN) atcd_gprs_start_connect();
      break;
    //---------------------------------------------
    case ATCD_GPRS_STATE_CONN:
      if(atcd.gprs.mode == ATCD_GPRS_MODE_DISCONN) atcd_gprs_start_disconnect();
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
      }
      break;
    //---------------------------------------------
    case ATCD_GPRS_STATE_INIT:
      if(atcd.gprs.mode == ATCD_GPRS_MODE_DISCONN) atcd_gprs_start_disconnect();
      break;
    //---------------------------------------------
    case ATCD_GPRS_STATE_DEINIT:
      if(atcd.gprs.mode == ATCD_GPRS_MODE_CONN) atcd_gprs_start_connect();
      break;
    //---------------------------------------------
    default:
      ATCD_DBG_GPRS_STATE_ERR
      atcd_gprs_start_disconnect();
  }

  if(atcd.gprs.atc_seq.state == ATCD_ATC_SEQ_STATE_RUN)
  {
    // Provedeme dalsi krok inicializace
    //ATCD_DBG_GPRS_SEQ_STEP
    atcd_atc_seq_proc(&atcd.gprs.atc_seq);
  }
  else if(atcd.gprs.atc_seq.state == ATCD_ATC_SEQ_STATE_ERROR)
  {
    // Pri zpracovani doslo k chybe
    ATCD_DBG_GPRS_SEQ_ERR
    // Posuneme sekvenci zpet na zacatek
    // Inkrementujeme citac chyb

    // Pokus o opravu odpojenim a znovu pripojenim

    /*atcd_atc_init(&atcd.gprs.at_cmd);
    atcd.gprs.at_cmd.cmd = "AT+CGATT=0\r\n";
    atcd.gprs.at_cmd.timeout = 10000;
    atcd_atc_exec(&atcd.gprs.at_cmd);
    return;*/

    //Odpojit se od GPRS
    // === Doplnit restart po opakovanem selhani...

    atcd_atc_seq_run(&atcd.gprs.atc_seq);
  }
  else if(atcd.gprs.atc_seq.state == ATCD_ATC_SEQ_STATE_DONE)
  {
    ATCD_DBG_GPRS_SEQ_OK

    switch(atcd.gprs.state)
    {
      //---------------------------------------------
      case ATCD_GPRS_STATE_INIT:
        atcd.gprs.state = ATCD_GPRS_STATE_CONNECTING;
        break;
      //---------------------------------------------
      case ATCD_GPRS_STATE_DEINIT:
        atcd.gprs.state = ATCD_GPRS_STATE_DISCONN;
        break;
      //---------------------------------------------
      default:
        ATCD_DBG_GPRS_STATE_ERR
    }
  }
  //----------------------------------------------------------
  if(atcd_get_ms() - atcd.gprs.timer > 2000)
  {
    atcd.gprs.timer = atcd_get_ms();
    ATCD_DBG_GPRS_STAT_START

    atcd_dbg_inf("GPRS: STAT: AT prikaz byl dokoncen.\r\n");

    atcd_dbg_warn("GPRS: STAT: Pri zpracovani AT prikazu doslo k chybe.\r\n");

  }
}
//------------------------------------------------------------------------------
void atcd_gprs_start_connect()              //gprs start connect
{
  if(atcd.state == ATCD_STATE_READY && atcd.gsm.state == ATCD_GSM_STATE_HOME)
  {
    atcd_dbg_inf("GPRS: INIT: Zacina inicializace.\r\n");
    atcd_atc_cancell(&atcd.gprs.at_cmd);

    atcd.gprs.atc_seq.make_step = &atcd_gprs_init_seq_step();
    atcd_atc_seq_run(&atcd.gprs.atc_seq);

    atcd.gprs.state = ATCD_GPRS_STATE_INIT;
  }
}
//------------------------------------------------------------------------------
void atcd_gprs_start_disconnect()           //gprs start disconnect
{
  atcd_dbg_inf("GPRS: DEINIT: Zacina deinicializace.\r\n");
  atcd_atc_cancell(&atcd.gprs.at_cmd);

  atcd.gprs.atc_seq.make_step = &atcd_gprs_deinit_seq_step();
  atcd_atc_seq_run(&atcd.gprs.atc_seq);

  atcd.gprs.state = ATCD_GPRS_STATE_DEINIT;
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
