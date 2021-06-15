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
void atcd_atc_queue_proc();                   //AT commands queue processing 
void atcd_atc_cancell_all();                  //cancel all AT commands in queue

void atcd_atc_check(atcd_at_cmd_t *at_cmd);    //check AT command

//------------------------------------------------------------------------------
void atcd_atc_init(atcd_at_cmd_t *at_cmd)     //init AT command
{
  // napred test zda neni ve fronte...
  
  at_cmd->state  = ATCD_ATC_STATE_DONE;                  
  at_cmd->result = ATCD_ATC_RESULT_UNKNOWN;
  at_cmd->next   = NULL;

  atcd_atc_set_defaults(at_cmd);
}
//------------------------------------------------------------------------------
void atcd_atc_check_queue(atcd_at_cmd_t *at_cmd)    //check AT command in queue
{

}
//------------------------------------------------------------------------------
uint8_t atcd_atc_set_defaults(atcd_at_cmd_t *at_cmd)  //set default AT commands values
{
  atcd_atc_check(at_cmd);
  if(at_cmd->state != ATCD_ATC_STATE_DONE)
  {
    return ATCD_ERR_LOCK;
  }

  at_cmd->cmd     = NULL;
  at_cmd->res_str = NULL;
  
  at_cmd->resp           = NULL;
  at_cmd->resp_len       = 0;
  at_cmd->resp_buff_size = 0;

  at_cmd->data = NULL;
  at_cmd->data_len = 0;

  at_cmd->timeout = 5000;

  at_cmd->cb_events = ATCD_ATC_EV_ALL;
  at_cmd->callback = NULL;
  
  //at_cmd->next = NULL;

  return ATCD_OK;
}
//------------------------------------------------------------------------------
void atcd_atc_check(atcd_at_cmd_t *at_cmd)    //check AT command
{
  if(at_cmd->state != ATCD_ATC_STATE_DONE)
  {



  }
}
//------------------------------------------------------------------------------
uint8_t atcd_atc_check_success(atcd_at_cmd_t *at_cmd)  //check AT command state and result
{
  if(at_cmd->state == ATCD_ATC_STATE_DONE && at_cmd->result == ATCD_ATC_RESULT_OK) return ATCD_OK;
  return ATCD_ERR;
}
//------------------------------------------------------------------------------
uint8_t atcd_atc_exec(atcd_at_cmd_t *at_cmd)         //execute AT command
{
  //nejaky if, ne?
  atcd_atc_check_queue(at_cmd);

  //tohle prepsat, radeji vzdy overovat, ze neni ve fronte?
  if(at_cmd->state != ATCD_ATC_STATE_DONE)
  {
    return ATCD_ERR_LOCK;
  }

  at_cmd->next = NULL;
  at_cmd->result = ATCD_ATC_RESULT_UNKNOWN;

  if(at_cmd->resp == NULL) 
  {
    at_cmd->resp = atcd.parser.buff;
    at_cmd->resp_buff_size = ATCD_P_BUFF_SIZE - 1;  //proc -1?
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
      ATCD_DBG_ATC_EXE
      atcd_atc_send_cmd();
    }
    else
    {
      ATCD_DBG_ATC_WAIT_P
      at_cmd->state = ATCD_ATC_STATE_WAIT;  
    }
  }
  else
  {
    ATCD_DBG_ATC_WAIT_Q
    at_cmd->state  = ATCD_ATC_STATE_WAIT;                  
    
    atcd.parser.at_cmd_end->next = at_cmd;
    atcd.parser.at_cmd_end = at_cmd;
  }

  return ATCD_OK;
}
//------------------------------------------------------------------------------
uint8_t atcd_atc_exec_cmd(atcd_at_cmd_t *at_cmd, char *cmd)    //execute and set AT command
{
  at_cmd->cmd = cmd;
  at_cmd->res_str = NULL;
  return atcd_atc_exec(at_cmd);
}
//------------------------------------------------------------------------------
uint8_t atcd_atc_exec_cmd_res(atcd_at_cmd_t *at_cmd, char *cmd, char *res)   //execute and set AT command with result string
{
  at_cmd->cmd = cmd;
  at_cmd->res_str = res;
  return atcd_atc_exec(at_cmd);
}
//------------------------------------------------------------------------------
void atcd_atc_proc()                     //AT commands processing 
{
  atcd_at_cmd_t *at_cmd;
 
  at_cmd = atcd.parser.at_cmd_top;

  if (atcd.parser.mode != ATCD_P_MODE_ATC) //pro ATCD_P_MODE_IPD duplo v atcd_proc
    if ((atcd_get_ms()-atcd.parser.mode_time>20000) && (atcd_get_ms()-atcd.parser.mode_time>at_cmd->timeout+1))
    { //vlastne by se nemelo stavat, timeout je nize, mozna jen tam nastavit ATCD_P_MODE_ATC
      atcd.parser.mode = ATCD_P_MODE_ATC;
      at_cmd->data=NULL;
      at_cmd->data_len=0;
    }

  // Pokud je nejaky prikaz na vrcholu fronty
  if(at_cmd != NULL)
  {             
    if(atcd.parser.at_cmd_top->state == ATCD_ATC_STATE_TX && atcd.parser.tx_state == ATCD_P_TX_COMPLETE)
    {
      if(atcd.parser.echo_en == ATCD_P_ECHO_ON)
      {
        ATCD_DBG_ATC_W_ECHO
        atcd.parser.at_cmd_top->state = ATCD_ATC_STATE_W_ECHO;
      }
      else
      {
        ATCD_DBG_ATC_W_END
        atcd.parser.at_cmd_top->state = ATCD_ATC_STATE_W_END;

        if(atcd.parser.at_cmd_top->data != NULL)
        {
          atcd.parser.mode = ATCD_P_MODE_TX_PEND;
          atcd.parser.mode_time = atcd_get_ms();
        };
      }
    }

    // Kontrola, zda neni cekajici k odeslani
    //atcd_atc_queue_proc();

    if(at_cmd->state == ATCD_ATC_STATE_WAIT && atcd.parser.mode == ATCD_P_MODE_ATC)
    {
      //neni to duplicita s kodem nize?
      ATCD_DBG_ATC_QUEUE_EXE
      atcd_atc_send_cmd();
    }
    // Kontrola, zda nevyprsel timeout
    else if((at_cmd->state == ATCD_ATC_STATE_W_ECHO || at_cmd->state == ATCD_ATC_STATE_W_END || at_cmd->state == ATCD_ATC_STATE_TX) && (atcd_get_ms() - atcd.parser.timer > at_cmd->timeout))
    {
      ATCD_DBG_ATC_TIM

      if(at_cmd->state == ATCD_ATC_STATE_W_ECHO) ATCD_DBG_ATC_ECHO_T_FAIL;

      at_cmd->result = ATCD_ATC_RESULT_TIMEOUT;
      at_cmd->resp_len = 0;
      at_cmd->state = ATCD_ATC_STATE_DONE;

      if(atcd.parser.mode == ATCD_P_MODE_TX_PEND)
      {
        atcd_hw_tx(NULL, at_cmd->data_len);
        atcd.parser.mode = ATCD_P_MODE_ATC;
      }
      if (atcd.parser.mode != ATCD_P_MODE_ATC)
        atcd.parser.mode = ATCD_P_MODE_ATC;
      //nemuzu ale jen tak sahat na cizi at_cmd at_cmd->data_=NULL; at_cmd->data_len_=0;

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
      ATCD_DBG_ATC_QUEUE_EXE
      atcd_atc_send_cmd();
    }
  }
  else 
  {
    if(atcd.parser.at_cmd_end != NULL)
    {
      ATCD_DBG_ATC_QUEUE_END
      atcd.parser.at_cmd_end = NULL;
    }
  }
}
//------------------------------------------------------------------------------
uint8_t atcd_atc_send_cmd()                     //send AT command
{
  uint16_t len;

  ATCD_DBG_ATC_SEND_CMD
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

  return 1;
}
//------------------------------------------------------------------------------
uint8_t atcd_atc_send_data()                     //send AT command data
{
  ATCD_DBG_ATC_SEND_DATA
  //atcd.parser.tx_state = ATCD_P_TX_ONGOING;
  atcd.parser.tx_state = ATCD_P_TX_COMPLETE;

  atcd.parser.tx_rbuff = *atcd.parser.at_cmd_top->data;
  atcd.parser.tx_data_len = atcd.parser.at_cmd_top->data_len;

  atcd_hw_tx(&atcd.parser.tx_rbuff, atcd.parser.tx_data_len);

  return 1;
}
//------------------------------------------------------------------------------
void atcd_atc_complete(atcd_at_cmd_t *at_cmd)         //AT command complete after result change
{
  if(at_cmd->resp_len == 0)
    at_cmd->resp_len = 0;
  else
    at_cmd->resp_len -= 2;

  at_cmd->resp[at_cmd->resp_len] = 0;
  at_cmd->state  = ATCD_ATC_STATE_DONE;

  if(atcd.parser.mode == ATCD_P_MODE_TX_PEND) atcd.parser.mode = ATCD_P_MODE_ATC;

  atcd.parser.buff_pos = 0;
  atcd.parser.line_pos = 0;

  atcd.parser.at_cmd_top = at_cmd->next;
  atcd_atc_queue_proc();

  if(at_cmd->callback != NULL && (at_cmd->cb_events & ATCD_ATC_EV_DONE) != 0) at_cmd->callback(ATCD_ATC_EV_DONE);
}
//------------------------------------------------------------------------------
uint8_t atcd_atc_cancell(atcd_at_cmd_t *at_cmd)       //cancell execute AT command
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
          return 2;
        }

        at_cmd_pp = NULL;
        atcd.parser.at_cmd_top = at_cmd_p->next;
      }

      if(at_cmd_p == atcd.parser.at_cmd_end) atcd.parser.at_cmd_end = at_cmd_pp;
      if(at_cmd_pp != NULL) at_cmd_pp->next = at_cmd_p->next;

      ATCD_DBG_ATC_CANCELL

      at_cmd->result = ATCD_ATC_RESULT_CANCELL;
      at_cmd->state = ATCD_ATC_STATE_DONE;

      if(at_cmd->callback != NULL && (at_cmd->cb_events & ATCD_ATC_EV_DONE) != 0) at_cmd->callback(ATCD_ATC_EV_DONE);
      return 0;
    }

    at_cmd_pp = at_cmd_p;
    at_cmd_p = at_cmd_p->next;
  }

  ATCD_DBG_ATC_CANCELL_EQ

  return 1;
}
//------------------------------------------------------------------------------
void atcd_atc_cancel_all()               //cancel all AT commands in queue
{
  atcd_at_cmd_t *at_cmd;

  ATCD_DBG_ATC_CANCELL_ALL

  while(atcd.parser.at_cmd_top != NULL)
  {
    //nemela by se volat fce vyse
    //to by musel nekdo rozhodnout co delat v state != ATCD_ATC_STATE_WAIT
    ATCD_DBG_ATC_CANCELL

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
      //if(strncmp(atcd.parser.buff + atcd.parser.line_pos, at_cmd->cmd, strlen(at_cmd->cmd)) == 0)
      if(strncmp(atcd.parser.buff + atcd.parser.line_pos, at_cmd->cmd, strlen(at_cmd->cmd) - 1) == 0)
      {
        ATCD_DBG_ATC_ECHO_DET
        at_cmd->state = ATCD_ATC_STATE_W_END;
        if(atcd.parser.at_cmd_top->data != NULL)
        {
          atcd.parser.mode = ATCD_P_MODE_TX_PEND;
          atcd.parser.mode_time = atcd_get_ms();
        }

        atcd.parser.buff_pos = 0;
        atcd.parser.line_pos = 0;

        if(at_cmd->callback != NULL && (at_cmd->cb_events & ATCD_ATC_EV_ECHO) != 0) at_cmd->callback(ATCD_ATC_EV_ECHO);
        return 1;
      }
    } 
    else if(at_cmd->state == ATCD_ATC_STATE_W_END) 
    {
      // Vymazani pocatecnich prazdnych radku
      if(at_cmd->resp_len == 0 && atcd.parser.buff_pos == 2)
      {
        atcd.parser.buff_pos = 0;
        atcd.parser.line_pos = 0;
        return 1;
      }
      // Test odpovedi a zpracovani dat
      if(strncmp(atcd.parser.buff + atcd.parser.line_pos, "OK\r\n", strlen("OK\r\n")) == 0)
      {
        ATCD_DBG_ATC_OK_DET
        at_cmd->result = ATCD_ATC_RESULT_OK;
      }
      else if(strncmp(atcd.parser.buff + atcd.parser.line_pos, "SHUT OK\r\n", strlen("SHUT OK\r\n")) == 0)
      {
        ATCD_DBG_ATC_OK_DET
        at_cmd->result = ATCD_ATC_RESULT_OK;
      }
      else if(strncmp(atcd.parser.buff + atcd.parser.line_pos, "ERROR\r\n", strlen("ERROR\r\n")) == 0)
      {
        ATCD_DBG_ATC_ERR_DET
        at_cmd->result = ATCD_ATC_RESULT_ERROR;
        at_cmd->resultcode = 0;
      }
      else if(at_cmd->res_str != NULL && strncmp(atcd.parser.buff + atcd.parser.line_pos, at_cmd->res_str, strlen(at_cmd->res_str)) == 0)
      {
        ATCD_DBG_ATC_OK_DET
        at_cmd->result = ATCD_ATC_RESULT_OK;
      }
      // Neni tohle nahodou i asynchronni zprava?
      else if(strncmp(atcd.parser.buff + atcd.parser.line_pos, "+CME ERROR:", strlen("+CME ERROR:")) == 0)
      {
        ATCD_DBG_ATC_CME_ERR_DET
        at_cmd->result = ATCD_ATC_RESULT_ERROR;
        at_cmd->resultcode = atoi(atcd.parser.buff + atcd.parser.line_pos + strlen("+CME ERROR:"));
      }
      else if(strncmp(atcd.parser.buff + atcd.parser.line_pos, "+CMS ERROR:", strlen("+CMS ERROR:")) == 0)
      {
        ATCD_DBG_ATC_CMS_ERR_DET
        at_cmd->result = ATCD_ATC_RESULT_ERROR;
        at_cmd->resultcode = atoi(atcd.parser.buff + atcd.parser.line_pos + strlen("+CMS ERROR:"));
      }
      else if(strncmp(atcd.parser.buff + atcd.parser.line_pos, "FAIL\r\n", strlen("FAIL\r\n")) == 0)
      {
        ATCD_DBG_ATC_FAIL_DET
        at_cmd->result = ATCD_ATC_RESULT_FAIL;
      }

      if(at_cmd->result != ATCD_ATC_RESULT_UNKNOWN)
      {
        // AT prikaz byl v casti vyse dokoncen
        atcd_atc_complete(at_cmd);
        return 1;
      }

      //AT prikaz nebyl dokoncen a v radce je nejaky text - zkopirujeme ji do odpovedi...
      if(at_cmd->resp != atcd.parser.buff)              //Pokud ma ATC vlastni buffer
      {
        if(at_cmd->resp_len + atcd.parser.buff_pos - atcd.parser.line_pos < at_cmd->resp_buff_size)
        {
          memcpy(at_cmd->resp + at_cmd->resp_len, atcd.parser.buff, atcd.parser.buff_pos - atcd.parser.line_pos);
          at_cmd->resp_len += atcd.parser.buff_pos - atcd.parser.line_pos;
        }
        else
        {
          ATCD_DBG_ATC_BUFF_OV
          if(at_cmd->state == ATCD_ATC_STATE_W_END)
          {
            ATCD_DBG_ATC_LN_BUFF_OV
            //ten priznak se ztrati
            //at_cmd->resp           = NULL;  //tohle je ultra nebezpecne!
            at_cmd->resp_len       = 0;
            at_cmd->resp_buff_size = 0;

            if(at_cmd->callback != NULL && (at_cmd->cb_events & ATCD_ATC_EV_OVERRUN) != 0) at_cmd->callback(ATCD_ATC_EV_OVERRUN);
          }
        }

        atcd.parser.buff_pos = 0;
        atcd.parser.line_pos = 0;
        return 1;
      }

      //Pokud ATC nema vlastni buffer
      at_cmd->resp_len += atcd.parser.buff_pos - atcd.parser.line_pos;
      atcd.parser.line_pos = atcd.parser.buff_pos;

      return 1;
    }    
  }

  return 0;
}                
//------------------------------------------------------------------------------
uint8_t atcd_atc_prompt_tst()
{
  // "> " test
  if(atcd.parser.mode == ATCD_P_MODE_TX_PEND && strncmp(atcd.parser.buff + atcd.parser.line_pos, "> ", strlen("> ")) == 0)
  {
    ATCD_DBG_ATC_PROMT_DET
    atcd.parser.mode = ATCD_P_MODE_PROMPT;

    atcd.parser.buff_pos = 0;
    atcd.parser.line_pos = 0;

    // Nasypat data
    // eventualne budou nasypana z proc funkce
    
    // SEM pripsat dalsi promenou ve ktere bude pocet byte co se maji posilat
    // a mozna a ikazazatel na data

    //tohle patri asi pro jiny modem
    //atcd_dbg_inf("CONN: Ocekavam vyzvu k zadani odesilanych dat.\r\n");

    // po zaniku spojeni je nutno minimalne vedet kolik se toho melo posilat, pokud
    // uz se prikaz zacal provadet...

    // tohle by se melo prepsat - bez dat k odeslani by nemel jit nastavit prompt, nejlepe jej zahodit a rozhodovat dle dat...

    if(atcd.parser.at_cmd_top->data != NULL)
    {
      atcd_atc_send_data(atcd.parser.at_cmd_top->data, atcd.parser.at_cmd_top->data_len);
    }
    else
    {
      ATCD_DBG_ATC_SEND_DATA_ERR
    }

    atcd.parser.mode = ATCD_P_MODE_ATC;
    return 1;
  }

  return 0;
}
//------------------------------------------------------------------------------
