/*
 * usart.c
 *
 *  Created on: Apr 9, 2011
 *      Author: Martin
 */
//------------------------------------------------------------------------------
#include "atcd_atc.h"
#include "atcd.h"

extern atcd_t atcd;

//------------------------------------------------------------------------------
// AT commands
void atcd_atc_proc();                     //AT commands processing 
void atcd_atc_queue_proc();               //AT commands queue processing 
void atcd_atc_send();                     //send AT command 
void atcd_atc_cancell_all();               //cancel all AT commands in queue

//------------------------------------------------------------------------------
void atcd_atc_init(atcd_at_cmd_t *at_cmd)         //init AT command
{
  at_cmd->cmd = NULL;         
  
  at_cmd->resp           = NULL;
  at_cmd->resp_len       = 0;
  at_cmd->resp_buff_size = 0;

  at_cmd->data           = NULL;
  at_cmd->data_len       = 0;
    
  at_cmd->state  = ATCD_ATC_STATE_FREE;                  
  at_cmd->result = ATCD_ATC_RESULT_UNKNOWN;

  at_cmd->prompt = ATCD_ATC_PROMPT_OFF;
  
  at_cmd->timeout = 5000;
  at_cmd->callback = NULL;
  
  at_cmd->next = NULL;
}
//------------------------------------------------------------------------------
void atcd_atc_exec(atcd_at_cmd_t *at_cmd)         //execute AT command
{
  at_cmd->next = NULL;
  at_cmd->result = ATCD_ATC_RESULT_UNKNOWN;

  if(at_cmd->resp == NULL) 
  {
    at_cmd->resp = atcd.buff;
    at_cmd->resp_buff_size = ATCD_BUFF_SIZE;
  }

  at_cmd->resp_len = 0;
  
  // If not any other at cmd in queue
  if(atcd.parser.at_cmd_end == NULL)
  {              
    atcd.parser.at_cmd_end = at_cmd;
    atcd.parser.at_cmd_top = at_cmd;
    
    // If is possible tx data now
    if(atcd.parser.mode == ATCD_P_MODE_ATC)
    {
      atcd_dbg_inf("ATC: Odesilam AT prikaz.\r\n");
      atcd_atc_send();
    }
    else
    {
      atcd_dbg_inf("ATC: Ve fronte neni zadny dalsi AT prikaz ale parser je zamestnan - cekam.\r\n");
      at_cmd->state = ATCD_ATC_STATE_WAIT;  
    }
  }
  else
  {
    atcd_dbg_inf("ATC: Je fronta - cekam na konci.\r\n");
    at_cmd->state  = ATCD_ATC_STATE_WAIT;                  
    
    atcd.parser.at_cmd_end->next = at_cmd;
    atcd.parser.at_cmd_end = at_cmd;
  }
}
//------------------------------------------------------------------------------
void atcd_atc_proc()                     //AT commands processing 
{
  atcd_at_cmd_t *at_cmd;
  uint16_t i;
 
  at_cmd = atcd.parser.at_cmd_top;

  // Pokud je nejaky prikaz na vrcholu fronty
  if(at_cmd != NULL)
  {           
    // Kontrola, zda bylo dokonceno jeho odesilani   
    if(atcd.parser.at_cmd_top->state == ATCD_ATC_STATE_TX && atcd.parser.tx_state == ATCD_P_TX_COMPLETE)
    {
      if(atcd.parser.echo_en == ATCD_P_ECHO_ON)
      {
        atcd_dbg_inf("ATC: Odesilani bylo dokoceno - prechazime na W_ECHO.\r\n");
        atcd.parser.at_cmd_top->state = ATCD_ATC_STATE_W_ECHO;
      }
      else
      {
        atcd_dbg_inf("ATC: Odesilani bylo dokoceno - prechazime na W_END.\r\n");
        atcd.parser.at_cmd_top->state = ATCD_ATC_STATE_W_END;

        if(atcd.parser.at_cmd_top->prompt == ATCD_ATC_PROMPT_ON) atcd.parser.mode = ATCD_P_MODE_TX_PEND;
      }
    }

    // Kontrola, zda neni cekajici k odeslani
    if(at_cmd->state == ATCD_ATC_STATE_WAIT && atcd.parser.mode == ATCD_P_MODE_ATC)
    {
      atcd_dbg_inf("ATC: Ve fronte je cekajici AT prikaz - menim jeho stav a odesilam.\r\n");
      atcd_atc_send();
    }
    // Kontola, zda nevyprsel timeout
    else if((at_cmd->state == ATCD_ATC_STATE_W_ECHO || at_cmd->state == ATCD_ATC_STATE_W_END || at_cmd->state == ATCD_ATC_STATE_TX) && (atcd_get_ms() - atcd.parser.timer > at_cmd->timeout))
    {
      atcd_dbg_warn("ATC: Probihajicimu AT prikazu vyprsel timeout.\r\n");
      at_cmd->result = ATCD_ATC_RESULT_TIMEOUT;
      at_cmd->resp_len = 0;

      at_cmd->state = ATCD_ATC_STATE_DONE;

      if(atcd.parser.mode == ATCD_P_MODE_TX_PEND) 
      {
        atcd_hw_tx(NULL, 512);
        atcd.parser.mode = ATCD_P_MODE_ATC;
      }

      atcd.parser.at_cmd_top = at_cmd->next;
      atcd_atc_queue_proc();

      if(at_cmd->callback != NULL && (at_cmd->events & ATCD_ATC_EV_TIMEOUT) != 0) at_cmd->callback(ATCD_ATC_EV_TIMEOUT);
    }
  }
}
//------------------------------------------------------------------------------
void atcd_atc_queue_proc()               //AT commands queue processing 
{
  if(atcd.parser.at_cmd_top != NULL)
  {
    if(atcd.parser.mode == ATCD_P_MODE_ATC && atcd.parser.at_cmd_top->state == ATCD_ATC_STATE_WAIT)
    {              
      atcd_dbg_inf("ATC: Ve fronte je cekajici AT prikaz - menim jeho stav a odesilam.\r\n");
      atcd_atc_send();
    }
  }
  else 
  {
    if(atcd.parser.at_cmd_end != NULL)
    {
      atcd_dbg_inf("ATC: Ve fronte neni zadny dalsi cekajici AT prikaz - aktualizuji konec fronty.\r\n");
      atcd.parser.at_cmd_end = NULL;
    }
  }
}
//------------------------------------------------------------------------------
void atcd_atc_send()                     //send AT command 
{
  // Nepripsat enter na kone?
  //
  //
  atcd_dbg_inf("ATC: Odesilani bylo zahajeno.\r\n");
  atcd.parser.at_cmd_top->state = ATCD_ATC_STATE_TX;
  atcd.parser.tx_state = ATCD_P_TX_ONGOING;

  atcd.parser.tx_rbuff.buff = (uint8_t*)atcd.parser.at_cmd_top->cmd;
  atcd.parser.tx_rbuff.buff_end = (uint8_t*)atcd.parser.at_cmd_top->cmd; + strlen(atcd.parser.at_cmd_top->cmd) - 1;
  atcd.parser.tx_rbuff.read = (uint8_t*)atcd.parser.at_cmd_top->cmd;
  atcd.parser.tx_rbuff.write = (uint8_t*)atcd.parser.at_cmd_top->cmd + strlen(atcd.parser.at_cmd_top->cmd);
  atcd.parser.tx_rbuff.capacity = strlen(atcd.parser.at_cmd_top->cmd);
  atcd.parser.tx_rbuff.size = strlen(atcd.parser.at_cmd_top->cmd);

  atcd.parser.timer = atcd_get_ms();
  atcd_hw_tx(&atcd.parser.tx_rbuff);
}
//------------------------------------------------------------------------------
void atcd_atc_cancell(atcd_at_cmd_t *at_cmd)       //cancell execute AT command
{
  atcd_at_cmd_t *at_cmd_p, *at_cmd_pp;

  at_cmd_p = atcd.parser.at_cmd_top;
  at_cmd_pp = at_cmd_p;

  while(at_cmd_p != NULL)
  {
    if(at_cmd_p == at_cmd)
    {
      if(at_cmd_p == atcd.parser.at_cmd_top)
      {
        if(at_cmd_p->state != ATCD_ATC_STATE_WAIT)
        {
          return;
        }

        at_cmd_pp = NULL;
        atcd.parser.at_cmd_top = at_cmd_p->next;
      }

      if(at_cmd_p == atcd.parser.at_cmd_end) atcd.parser.at_cmd_end = at_cmd_pp;
      if(at_cmd_pp != NULL) at_cmd_pp->next = at_cmd_p->next;

      atcd_dbg_inf("ATC: Rusim AT prikaz.\r\n");

      at_cmd->result = ATCD_ATC_RESULT_CANCELL;
      at_cmd->state = ATCD_ATC_STATE_DONE;

      if(at_cmd->callback != NULL && (at_cmd->events & ATCD_ATC_EV_DONE) != 0) at_cmd->callback(ATCD_ATC_EV_DONE);
      return;
    }

    at_cmd_pp = at_cmd_p;
    at_cmd_p = at_cmd_p->next;
  }

  atcd_dbg_warn("ATC: Ruseny AT prikaz neni ve fronte.\r\n");
}
//------------------------------------------------------------------------------
void atcd_atc_cancell_all()               //cancell all AT commands in queue
{
  atcd_at_cmd_t *at_cmd;

  atcd_dbg_inf("ATC: Budu zruseny vsechny AT prikazy fe fronte.\r\n");

  while(atcd.parser.at_cmd_top != NULL)
  {
    atcd_dbg_inf("ATC: Rusim AT prikaz.\r\n");

    at_cmd = atcd.parser.at_cmd_top;
    at_cmd->result = ATCD_ATC_RESULT_CANCELL;
    at_cmd->state = ATCD_ATC_STATE_DONE;
    atcd.parser.at_cmd_top = at_cmd->next;

    //testovat zda se netoci v kruhu a pripadne to vypsat...
    //
    //
    //

    if(at_cmd->callback != NULL && (at_cmd->events & ATCD_ATC_EV_DONE) != 0) at_cmd->callback(ATCD_ATC_EV_DONE);
  }

  atcd.parser.at_cmd_end = NULL;
}
//------------------------------------------------------------------------------
