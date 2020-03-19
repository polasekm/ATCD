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
void atcd_atc_cancell_all();              //cancel all AT commands in queue

//------------------------------------------------------------------------------
void atcd_atc_init(atcd_at_cmd_t *at_cmd)         //init AT command
{
  at_cmd->state  = ATCD_ATC_STATE_FREE;                  
  at_cmd->result = ATCD_ATC_RESULT_UNKNOWN;

  atcd_atc_set_default(at_cmd);

  at_cmd->next = NULL;
}
//------------------------------------------------------------------------------
void atcd_atc_set_default(atcd_at_cmd_t *at_cmd)  //set default AT commands values
{
  at_cmd->cmd = NULL;         
  
  at_cmd->resp           = NULL;
  at_cmd->resp_len       = 0;
  at_cmd->resp_buff_size = 0;
    
  /*at_cmd->state  = ATCD_ATC_STATE_FREE;                  
  at_cmd->result = ATCD_ATC_RESULT_UNKNOWN;*/

  at_cmd->data = NULL;
  at_cmd->data_len = 0;

  at_cmd->timeout = 5000;

  at_cmd->cb_events = ATCD_ATC_EV_ALL;
  at_cmd->callback = NULL;
  
  //at_cmd->next = NULL;
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
      atcd_atc_send_cmd();
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
 
  at_cmd = atcd.parser.at_cmd_top;

  // Pokud je nejaky prikaz na vrcholu fronty
  if(at_cmd != NULL)
  {             
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

        if(atcd.parser.at_cmd_top->data != NULL) atcd.parser.mode = ATCD_P_MODE_TX_PEND;
      }
    }

    // Kontrola, zda neni cekajici k odeslani
    //atcd_atc_queue_proc();

    if(at_cmd->state == ATCD_ATC_STATE_WAIT && atcd.parser.mode == ATCD_P_MODE_ATC)
    {
      atcd_dbg_inf("ATC: Ve fronte je cekajici AT prikaz - menim jeho stav a odesilam.\r\n");
      atcd_atc_send_cmd();
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
        atcd_hw_tx(NULL, at_cmd->data_len);
        atcd.parser.mode = ATCD_P_MODE_ATC;
      }

      atcd.parser.at_cmd_top = at_cmd->next;
      atcd_atc_queue_proc();

      if(at_cmd->callback != NULL && (at_cmd->cb_events & ATCD_ATC_EV_TIMEOUT) != 0) at_cmd->callback(ATCD_ATC_EV_TIMEOUT);
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
      atcd_atc_send_cmd();
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
void atcd_atc_send_cmd()                     //send AT command
{
  uint16_t len;

  atcd_dbg_inf("ATC: Odesilani prikazu bylo zahajeno.\r\n");
  atcd.parser.at_cmd_top->state = ATCD_ATC_STATE_TX;
  //atcd.parser.tx_state = ATCD_P_TX_ONGOING;
  atcd.parser.tx_state = ATCD_P_TX_COMPLETE;

  len = strlen(atcd.parser.at_cmd_top->cmd);

  /*atcd.parser.tx_rbuff.buff = (uint8_t*)atcd.parser.at_cmd_top->cmd;
  atcd.parser.tx_rbuff.buff_end = (uint8_t*)atcd.parser.at_cmd_top->cmd + len;
  atcd.parser.tx_rbuff.read = atcd.parser.tx_rbuff.buff;
  atcd.parser.tx_rbuff.write = atcd.parser.tx_rbuff.buff + len;
  atcd.parser.tx_rbuff.capacity = len;*/
  rbuff_lin_space(&atcd.parser.tx_rbuff, (uint8_t*)atcd.parser.at_cmd_top->cmd, len);

  atcd.parser.timer = atcd_get_ms();
  atcd_hw_tx(&atcd.parser.tx_rbuff, len);
}
//------------------------------------------------------------------------------
void atcd_atc_send_data()                     //send AT command data
{
  atcd_dbg_inf("ATC: Odesilani dat bylo zahajeno.\r\n");
  //atcd.parser.tx_state = ATCD_P_TX_ONGOING;
  atcd.parser.tx_state = ATCD_P_TX_COMPLETE;

  atcd.parser.tx_rbuff = *atcd.parser.at_cmd_top->data;
  atcd.parser.tx_data_len = atcd.parser.at_cmd_top->data_len;

  atcd_hw_tx(&atcd.parser.tx_rbuff, atcd.parser.tx_data_len);
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

      if(at_cmd->callback != NULL && (at_cmd->cb_events & ATCD_ATC_EV_DONE) != 0) at_cmd->callback(ATCD_ATC_EV_DONE);
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

    if(at_cmd->callback != NULL && (at_cmd->cb_events & ATCD_ATC_EV_DONE) != 0) at_cmd->callback(ATCD_ATC_EV_DONE);
  }

  atcd.parser.at_cmd_end = NULL;
}
//------------------------------------------------------------------------------
uint8_t atcd_atc_ln_proc()
{
  atcd_at_cmd_t *at_cmd;

  at_cmd = atcd.parser.at_cmd_top;
  // Pokud se zpracovava nejaky ATC
  if(at_cmd != NULL)                            
  {
    if(at_cmd->state == ATCD_ATC_STATE_W_ECHO) 
    {
      // Test echa
      //if(strncmp(atcd.buff + atcd.line_pos, at_cmd->cmd, strlen(at_cmd->cmd)) == 0)
      if(strncmp(atcd.buff + atcd.line_pos, at_cmd->cmd, strlen(at_cmd->cmd) - 1) == 0)
      {
        atcd_dbg_inf("ATC: ECHO detected.\r\n");
        at_cmd->state = ATCD_ATC_STATE_W_END;
        if(atcd.parser.at_cmd_top->data != NULL) atcd.parser.mode = ATCD_P_MODE_TX_PEND;

        atcd.buff_pos = 0;
        atcd.line_pos = 0;

        if(at_cmd->callback != NULL && (at_cmd->cb_events & ATCD_ATC_EV_ECHO) != 0) at_cmd->callback(ATCD_ATC_EV_ECHO);
        return 1;
      }
      else atcd_dbg_warn("ATC: ECHO test FAIL.\r\n");
    } 
    else if(at_cmd->state == ATCD_ATC_STATE_W_END) 
    {
      // Vymazani pocatecnich prazdnych radku
      if(at_cmd->resp_len == 0 && atcd.buff_pos == 2) 
      {
        atcd.buff_pos = 0;
        atcd.line_pos = 0;
        return 1;
      }
      // Test odpovedi a zpracovani dat
      if(strncmp(atcd.buff + atcd.line_pos, "OK\r\n", strlen("OK\r\n")) == 0)
      {
        atcd_dbg_inf("ATC: OK detected.\r\n");
        at_cmd->result = ATCD_ATC_RESULT_OK;
      }
      else if(strncmp(atcd.buff + atcd.line_pos, "ERROR\r\n", strlen("ERROR\r\n")) == 0)
      {
        atcd_dbg_inf("ATC: ERROR detected.\r\n");
        at_cmd->result = ATCD_ATC_RESULT_ERROR;
      }
      // Neni tohle nahodou asynchorinni zprava?
      else if(strncmp(atcd.buff + atcd.line_pos, "+CME ERROR:", strlen("+CME ERROR:")) == 0)
      {
        atcd_dbg_inf("ATC: ERROR detected.\r\n");
        at_cmd->result = ATCD_ATC_RESULT_ERROR;
      }
      else if(strncmp(atcd.buff + atcd.line_pos, "+CMS ERROR:", strlen("+CMS ERROR:")) == 0)
      {
        atcd_dbg_inf("ATC: ERROR detected.\r\n");
        at_cmd->result = ATCD_ATC_RESULT_ERROR;
      }
      else if(strncmp(atcd.buff + atcd.line_pos, "FAIL\r\n", strlen("FAIL\r\n")) == 0)
      {
        atcd_dbg_inf("ATC: FAIL detected.\r\n");
        at_cmd->result = ATCD_ATC_RESULT_FAIL;
      }

      if(at_cmd->result != ATCD_ATC_RESULT_UNKNOWN)
      {
        // AT prikaz byl v casti vyse dokoncen
        if(at_cmd->resp_len == 0)
          at_cmd->resp_len = 0;
        else
          at_cmd->resp_len -= 2;

        at_cmd->resp[at_cmd->resp_len] = 0;
        at_cmd->state  = ATCD_ATC_STATE_DONE;

        if(atcd.parser.mode == ATCD_P_MODE_TX_PEND) atcd.parser.mode = ATCD_P_MODE_ATC;

        atcd.buff_pos = 0;
        atcd.line_pos = 0;

        atcd.parser.at_cmd_top = at_cmd->next;
        atcd_atc_queue_proc(); 

        if(at_cmd->callback != NULL && (at_cmd->cb_events & ATCD_ATC_EV_DONE) != 0) at_cmd->callback(ATCD_ATC_EV_DONE);
        return 1;
      }
    }    
  }

  return 0;
}                
//------------------------------------------------------------------------------
uint8_t atcd_atc_prompt_tst()
{
  // "> " test
  if(atcd.parser.mode == ATCD_P_MODE_TX_PEND && strncmp(atcd.buff + atcd.line_pos, "> ", strlen("> ")) == 0)
  {
    atcd_dbg_inf("ATCD: Prompt \">\" detected.\r\n");
    atcd.parser.mode = ATCD_P_MODE_PROMPT;

    atcd.buff_pos = 0;
    atcd.line_pos = 0;

    // Nasypat data
    // eventualne budou nasypana z proc funkce
    
    // SEM pripsat dalsi promenou ve ktere bude pocet byte co se maji posilat
    // a moyna a ikazayatel na data
    // po yaniku spojeni je nutno minimalne vedet kolik e toho melo posilat, pokud
    // uz se prikaz zacal provadet...

    // tohle by se melo prepsat - bez dat k odeslani by nemel jit nastavit prompt, nejlepe jej zahodit a rozhodovat dle dat...

    if(atcd.parser.at_cmd_top->data != NULL)
    {
      atcd_atc_send_data(atcd.parser.at_cmd_top->data, atcd.parser.at_cmd_top->data_len);
    }
    else
    {
      atcd_dbg_err("ATCD: ATC nema zadna data k odeslani!\r\n");
    }

    atcd.parser.mode = ATCD_P_MODE_ATC;
    return 1;
  }

  return 0;
}
//------------------------------------------------------------------------------
