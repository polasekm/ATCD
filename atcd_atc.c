/*
 * usart.c
 *
 *  Created on: Apr 9, 2011
 *      Author: Martin
 */
//------------------------------------------------------------------------------
#include "atcd_atc.h"
#include "atcd.h"

#define DEBUG_ATCD_SLEEP 0

extern atcd_t atcd;
//------------------------------------------------------------------------------
// AT commands
void atcd_atc_queue_proc();                   //AT commands queue processing 
void atcd_atc_cancell_all();                  //cancel all AT commands in queue

//void atcd_atc_check(atcd_at_cmd_t *at_cmd);    //check AT command

//------------------------------------------------------------------------------
void atcd_atc_init(atcd_at_cmd_t *at_cmd)     //init AT command
{
  // napred test zda neni ve fronte...
  
  at_cmd->state       = ATCD_ATC_STATE_DONE;
  at_cmd->result      = ATCD_ATC_RESULT_UNKNOWN;
  at_cmd->result_code = 0;
  at_cmd->next        = NULL;

  atcd_atc_set_defaults(at_cmd);
}
//------------------------------------------------------------------------------
/*void atcd_atc_check_queue(atcd_at_cmd_t *at_cmd)    //check AT command in queue
{

}*/
//------------------------------------------------------------------------------
atcd_r_t atcd_atc_set_defaults(atcd_at_cmd_t *at_cmd)  //set default AT commands values
{
  //atcd_atc_check(at_cmd);
  if(at_cmd->state != ATCD_ATC_STATE_DONE)
  {
    return ATCD_ERR_LOCK;
  }

  at_cmd->cmd            = NULL;
  at_cmd->result_str     = NULL;
  
  at_cmd->resp           = NULL;
  at_cmd->resp_len       = 0;
  at_cmd->resp_buff_size = 0;

  at_cmd->data            = NULL;
  at_cmd->data_len        = 0;

  at_cmd->timeout         = 5000;

  at_cmd->cb_events       = ATCD_ATC_EV_ALL;
  at_cmd->callback        = NULL;
  
  return ATCD_OK;
}
//------------------------------------------------------------------------------
/*void atcd_atc_check(atcd_at_cmd_t *at_cmd)    //check AT command
{
  if(at_cmd->state != ATCD_ATC_STATE_DONE)
  {

  }
}*/
//------------------------------------------------------------------------------
uint8_t atcd_atc_check_success(atcd_at_cmd_t *at_cmd)  //check AT command state and result
{
  if(at_cmd->state == ATCD_ATC_STATE_DONE && at_cmd->result == ATCD_ATC_RESULT_OK) return ATCD_OK;
  return ATCD_ERR;
}
//------------------------------------------------------------------------------
uint8_t atcd_atc_exec(atcd_at_cmd_t *at_cmd)         //execute AT command
{
  //TODO: Zvazit overovani pritomnosti ve fronte
  //atcd_atc_check_queue(at_cmd);

  if(at_cmd->state != ATCD_ATC_STATE_DONE)
  {
    return ATCD_ERR_LOCK;
  }

  at_cmd->next = NULL;
  at_cmd->result = ATCD_ATC_RESULT_UNKNOWN;

  if(at_cmd->resp == NULL) 
  {
    at_cmd->resp = atcd.parser.buff;
    at_cmd->resp_buff_size = ATCD_P_BUFF_SIZE - 1;  //proc -1 //protoze ATCD_P_BUFF ma posledni znak (0) vyhrany proti prepsani
  }

  at_cmd->resp_len = 0;
  
  // If not any other at cmd in queue
  if(atcd.parser.at_cmd_end == NULL)
  {              
    atcd.parser.at_cmd_end = at_cmd;
    atcd.parser.at_cmd_top = at_cmd;
    
    // If is possible tx data now
    if(atcd.parser.mode == ATCD_P_MODE_IDLE)
    {
      ATCD_DBG_ATC_EXE
      atcd_atc_send_cmd_top();
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
atcd_r_t atcd_atc_exec_cmd(atcd_at_cmd_t *at_cmd, char *cmd)    //execute and set AT command
{
  at_cmd->cmd = cmd;
  at_cmd->result_str = NULL;
  return atcd_atc_exec(at_cmd);
}
//------------------------------------------------------------------------------
atcd_r_t atcd_atc_exec_cmd_res_(atcd_at_cmd_t *at_cmd, char *cmd, char *res)   //execute and set AT command with result string
{
  at_cmd->cmd = cmd;
  at_cmd->result_str = res;
  return atcd_atc_exec(at_cmd);
}
//------------------------------------------------------------------------------
void atcd_atc_proc(uint8_t timeouts_also)                     //AT commands processing
{
  atcd_at_cmd_t *at_cmd;
 
  at_cmd = atcd.parser.at_cmd_top;
  if(at_cmd != NULL)
  {             
    //Na vrcholu fronty je AT prikaz
    if(at_cmd->state == ATCD_ATC_STATE_TX && atcd.tx_state == ATCD_P_TX_COMPLETE)
    {
      //Bylo dokonceno odesilani prikazu
      if(atcd.parser.echo_en == ATCD_P_ECHO_ON)
      {                                                       //Je zapla detekce echa
        ATCD_DBG_ATC_W_ECHO
        at_cmd->state = ATCD_ATC_STATE_W_ECHO;                //Prejdeme do stavu jejiho overeni
      }
      else
      {
        if(at_cmd->data != NULL)
        {
          //AT prikaz ma dodatecna data pro konzoli (SMS, datove spojeni)
          ATCD_DBG_ATC_TX_PEND
          at_cmd->state = ATCD_ATC_STATE_W_PROMPT;            //Prejdeme to stavu cekani na vyzvu pro zadani dat
        }
        else
        {
          //AT prikaz nema dodatrecna data pro konzoli
          ATCD_DBG_ATC_W_END
          at_cmd->state = ATCD_ATC_STATE_W_END;               //Prejdeme do stavu cekani na konec odpovedi AT prikazu
        }
      }
    }
    else if(at_cmd->state == ATCD_ATC_STATE_TX_DATA && atcd.tx_state == ATCD_P_TX_COMPLETE)
    {
      //Bylo dokonceno odesilani dodatecnych dat k AT prikazu
      ATCD_DBG_ATC_W_END
      at_cmd->state = ATCD_ATC_STATE_W_END;                   //Prejdeme do stavu cekani na konec odpovedi AT prikazu
    }
    else if(timeouts_also && (at_cmd->state != ATCD_ATC_STATE_WAIT && at_cmd->state != ATCD_ATC_STATE_DONE) && (atcd_get_ms() - atcd.parser.at_cmd_timer > at_cmd->timeout))
    {
      //AT prikazu vyprsel timeout
      {
        //ATCD_DBG_ATC_TIM
        char bufajzl[44+20 +10];
        snprintf(bufajzl, sizeof(bufajzl), "Probihajicimu AT prikazu vyprsel timeout: %s\r\n", at_cmd->cmd);
        atcd_dbg_warn("ATCD: ATC: ", bufajzl);
      }
      atcd.parser.stat.atc_tim++;

      if(at_cmd->state == ATCD_ATC_STATE_W_ECHO)
      {
        ATCD_DBG_ATC_ECHO_T_FAIL;
        atcd.parser.stat.atc_echo++;
      }

      at_cmd->result = ATCD_ATC_RESULT_TIMEOUT;
      at_cmd->resp_len = 0;
      at_cmd->state = ATCD_ATC_STATE_DONE;

      if(at_cmd->data != NULL)
      {
        //AT prikaz mel odesilat data do kozole
        ATCD_DBG_ATC_ESC_DATA
        {
          char tmps[50];
          snprintf(tmps, sizeof(tmps), "timeout->send %d whites\n", at_cmd->data_len + 1);
          atcd_dbg_err("@atcd error: ", tmps);
        }
        //TODO: poresit abu se neprali o ukazatel na TX data, pokud se zrovna vysila...
        atcd_hw_tx_esc("/r", at_cmd->data_len + 1);  //Odesle plonkova data o dane delce pokud by je modem nahodou ocekaval
      }

      atcd.parser.at_cmd_top = at_cmd->next;
      if(atcd.parser.at_cmd_top == NULL)
      {
        //Fronta je prazdna
        ATCD_DBG_ATC_QUEUE_END
        atcd.parser.at_cmd_end = NULL;
      }

      if(at_cmd->callback != NULL && (at_cmd->cb_events & ATCD_ATC_EV_TIMEOUT) != 0) at_cmd->callback(ATCD_ATC_EV_TIMEOUT);

      //at_cmd = atcd.parser.at_cmd_top;
      //atcd_atc_queue_proc();
    }
    /*else if(at_cmd->state == ATCD_ATC_STATE_WAIT && atcd.parser.mode == ATCD_P_MODE_IDLE)
    {
      //AT prikaz ceka na spusteni
      ATCD_DBG_ATC_QUEUE_EXE
      atcd_atc_send_cmd();                      //Odesleme jej...
    }*/

    atcd_atc_queue_proc();
  }
}
//------------------------------------------------------------------------------
void atcd_atc_queue_proc()               //AT commands queue processing 
{
  atcd_at_cmd_t *at_cmd;

  at_cmd = atcd.parser.at_cmd_top;
  if(at_cmd != NULL)
  {
    if(atcd.parser.mode == ATCD_P_MODE_IDLE && at_cmd->state == ATCD_ATC_STATE_WAIT)
    {              
      ATCD_DBG_ATC_QUEUE_EXE
      atcd_atc_send_cmd_top();
    }
  }
  /*else
  {
    if(atcd.parser.at_cmd_end != NULL)
    {
      //TODO: Tohle je asii error!
      ATCD_DBG_ATC_QUEUE_END
      atcd.parser.at_cmd_end = NULL;
    }
  }*/
}
//------------------------------------------------------------------------------
void atcd_atc_send_cmd_top()                     //send AT command
{
  uint16_t len;

  ATCD_DBG_ATC_SEND_CMD
  atcd.parser.stat.atc_cnt++;

  len = strlen(atcd.parser.at_cmd_top->cmd);
  rbuff_lin_space(&atcd.tx_rbuff, (uint8_t*)atcd.parser.at_cmd_top->cmd, len);

  atcd.parser.at_cmd_top->state = ATCD_ATC_STATE_TX;
  atcd.parser.at_cmd_timer = atcd_get_ms();

  atcd_hw_tx(&atcd.tx_rbuff, len);
}
//------------------------------------------------------------------------------
void atcd_atc_send_data_top()                     //send AT command data
{
  ATCD_DBG_ATC_SEND_DATA

  atcd.tx_rbuff = *atcd.parser.at_cmd_top->data;
  atcd.tx_data_len = atcd.parser.at_cmd_top->data_len;

  atcd.parser.at_cmd_top->state = ATCD_ATC_STATE_TX_DATA;

  atcd_hw_tx(&atcd.tx_rbuff, atcd.tx_data_len);
}
//------------------------------------------------------------------------------
void atcd_atc_complete(atcd_at_cmd_t *at_cmd)         //AT command complete after result change
{
  //Vymazeme zalomeni radku na konci
  if(at_cmd->resp_len >= 2) at_cmd->resp_len -= 2;

  at_cmd->resp[at_cmd->resp_len] = 0;
  at_cmd->state  = ATCD_ATC_STATE_DONE;

  //atcd.sleep_timer = atcd_get_ms();

  //if(atcd.parser.mode == ATCD_P_MODE_IDLE)
  //{
    atcd.parser.buff_pos = 0;
    atcd.parser.line_pos = 0;
  //}

  atcd.parser.at_cmd_top = at_cmd->next;
  if(atcd.parser.at_cmd_top == NULL)
  {
    //Fronta je prazdna
    ATCD_DBG_ATC_QUEUE_END
    atcd.parser.at_cmd_end = NULL;
  }

  if(at_cmd->callback != NULL && (at_cmd->cb_events & ATCD_ATC_EV_DONE) != 0) at_cmd->callback(ATCD_ATC_EV_DONE);

  atcd_atc_queue_proc();
}
//------------------------------------------------------------------------------
// TODO: Udajne ji nikdo krom cancell all nevola
uint8_t atcd_atc_cancell(atcd_at_cmd_t *at_cmd)       //cancell execute AT command
{
  atcd_at_cmd_t *at_cmd_pre;

  ATCD_DBG_ATC_CANCELL

  if(at_cmd == atcd.parser.at_cmd_top)
  {
    //Prikaz je na vrcholu fronty
    atcd.parser.at_cmd_top = at_cmd->next;
    at_cmd_pre = NULL;

    if(at_cmd->data != NULL)
    {
      //AT prikaz mel odesilat data do kozole
      ATCD_DBG_ATC_ESC_DATA
      {
        char tmps[50];
        snprintf(tmps, sizeof(tmps), "cancel->send %d whites\n", at_cmd->data_len + 1);
        atcd_dbg_err("@atcd error: ", tmps);
      }
      //TODO: poresit abu se neprali o ukazatel na TX data, pokud se zrovna vysila...
      atcd_hw_tx_esc("/r", at_cmd->data_len + 1);  //Odesle plonkova data o dane delce pokud by je modem nahodou ocekaval
    }
  }
  else
  {
    //Prikaz je unitr fronty
    at_cmd_pre = atcd.parser.at_cmd_top;

    //Hledame, ktery mu predchazi
    while(at_cmd != at_cmd_pre->next)
    {
      at_cmd_pre = at_cmd_pre->next;
      if(at_cmd_pre == NULL)
      {
        //AT prikaz neni uvnitr fronty
        ATCD_DBG_ATC_CANCELL_EQ
        return 1;
      }
    }

    at_cmd_pre->next = at_cmd->next;
  }

  if(at_cmd == atcd.parser.at_cmd_end)
  {
    //Prikaz je na klonci fronty
    atcd.parser.at_cmd_end = at_cmd_pre;
  }

  at_cmd->result = ATCD_ATC_RESULT_CANCELL;
  at_cmd->state = ATCD_ATC_STATE_DONE;

  if(at_cmd->callback != NULL && (at_cmd->cb_events & ATCD_ATC_EV_DONE) != 0) at_cmd->callback(ATCD_ATC_EV_DONE);
  return 0;
}
//------------------------------------------------------------------------------
void atcd_atc_cancel_all()               //cancel all AT commands in queue
{
  ATCD_DBG_ATC_CANCELL_ALL

  while(atcd.parser.at_cmd_top != NULL)
  {
    atcd_atc_cancell(atcd.parser.at_cmd_top);
  }
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
          //Pokud ma AT prikaz dalsi data, cekame na vyzvu kjejich zadani
          at_cmd->state = ATCD_ATC_STATE_W_PROMPT;
        }

        atcd.parser.buff_pos = 0;
        atcd.parser.line_pos = 0;

        if(at_cmd->callback != NULL && (at_cmd->cb_events & ATCD_ATC_EV_ECHO) != 0) at_cmd->callback(ATCD_ATC_EV_ECHO);
        return 1;
      }
      else if ((atcd.parser.buff_pos>=atcd.parser.line_pos+2) && (atcd.parser.buff[atcd.parser.line_pos]=='\r') && (atcd.parser.buff[atcd.parser.line_pos+1]=='\n')) //ignoruj prazdny radek
      {
        atcd.parser.buff_pos = 0;
        atcd.parser.line_pos = 0;
        return 1;
      }
      else //to je nejdulezitejsi chyba ze vsech - prislo poskozene echo (nebo mozna nejake unso)
      {
        if ((atcd.parser.buff_pos > atcd.parser.line_pos+5) &&
          (strncmp(atcd.parser.buff+atcd.parser.line_pos, "$PMTK", 5)==0))
        {
        }
        else if ((atcd.parser.buff_pos-atcd.parser.line_pos>=3) &&
            (atcd.parser.buff[atcd.parser.line_pos]=='A') &&
            (atcd.parser.buff[atcd.parser.line_pos+1]=='T')) //echo ale ne to nase
        { //ztraceji se znaky v echu, ladim jak moc. Prikazy slysi spravne
          char tmps[60];
          snprintf(tmps, sizeof(tmps), "echo-bad %d,%d: %.*s\n",
              atcd.parser.buff_pos, atcd.parser.line_pos, atcd.parser.buff_pos-atcd.parser.line_pos, atcd.parser.buff+atcd.parser.line_pos);
          atcd_dbg_err("@sys unso: ", tmps); //asi kdyz cekam echo a neprijde echo ani prazdny radek, obcas se to zrejme deje
          atcd.stat.echo_bad++;
        }
        else
        {
          char tmps[60];
          snprintf(tmps, sizeof(tmps), "echo-uns %d,%d: %.*s\n",
              atcd.parser.buff_pos, atcd.parser.line_pos, atcd.parser.buff_pos-atcd.parser.line_pos, atcd.parser.buff+atcd.parser.line_pos);
          atcd_dbg_warn("@sys unso: ", tmps);
          atcd.stat.echo_uns++;
        }
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
        atcd.parser.stat.atc_err++;
        at_cmd->result = ATCD_ATC_RESULT_ERROR;
        at_cmd->result_code = 0;
      }
      else if(at_cmd->result_str != NULL && strncmp(atcd.parser.buff + atcd.parser.line_pos, at_cmd->result_str, strlen(at_cmd->result_str)) == 0)
      {
        ATCD_DBG_ATC_OK_DET
        at_cmd->result = ATCD_ATC_RESULT_MATCH;
      }
      // Neni tohle nahodou i asynchronni zprava?
      else if(strncmp(atcd.parser.buff + atcd.parser.line_pos, "+CME ERROR:", strlen("+CME ERROR:")) == 0)
      {
        atcd.parser.stat.atc_err++;
        at_cmd->result = ATCD_ATC_RESULT_ERROR;
        at_cmd->result_code = atoi(atcd.parser.buff + atcd.parser.line_pos + strlen("+CME ERROR:"));

        char hlaska[27+2*5 +10];
        snprintf(hlaska, sizeof(hlaska), "CME ERROR detected: %u @ s=%u\r\n",
            at_cmd->result_code, atcd.proc_step);
        atcd_dbg_warn("ATCD: ATC: ", hlaska);
        //ATCD_DBG_ATC_CME_ERR_DET
      }
      else if(strncmp(atcd.parser.buff + atcd.parser.line_pos, "+CMS ERROR:", strlen("+CMS ERROR:")) == 0)
      {
        ATCD_DBG_ATC_CMS_ERR_DET
        atcd.parser.stat.atc_err++;
        at_cmd->result = ATCD_ATC_RESULT_ERROR;
        at_cmd->result_code = atoi(atcd.parser.buff + atcd.parser.line_pos + strlen("+CMS ERROR:"));
      }
      else if(strncmp(atcd.parser.buff + atcd.parser.line_pos, "FAIL\r\n", strlen("FAIL\r\n")) == 0)
      {
        ATCD_DBG_ATC_FAIL_DET
        atcd.parser.stat.atc_fail++;
        at_cmd->result = ATCD_ATC_RESULT_FAIL;
      }

      if(at_cmd->result != ATCD_ATC_RESULT_UNKNOWN)
      {
        // AT prikaz byl v casti vyse dokoncen
        atcd_atc_complete(at_cmd);
        return 1;
      }

      //TODO: predelat cely ATCD_ATC_STATE_W_END, nebo zde odchytit $PMTK...
      //if (strncmp(at_cmd->cmd, "AT+CIPSEND=", 11)==0)
        //atcd.parser.line_pos=atcd.parser.line_pos;
      if (atcd.parser.buff_pos > atcd.parser.line_pos+5)
      {
        if (strncmp(atcd.parser.buff+atcd.parser.line_pos, "$PMTK", 5)==0)
        { //idealne bych dal return 0 ale tam mi smazou dosud prijatou cast odpovedi
          if(atcd.callback != NULL && (atcd.cb_events & ATCD_EV_ASYNC_MSG) != 0) atcd.callback(ATCD_EV_ASYNC_MSG);
          atcd.parser.buff_pos=atcd.parser.line_pos; //zahodit posledni radek ($PMTK...)
          return 1;
        }
      };

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
  if (atcd.parser.at_cmd_top==NULL)
    return 0;
  if(atcd.parser.at_cmd_top->state == ATCD_ATC_STATE_W_PROMPT && strncmp(atcd.parser.buff + atcd.parser.line_pos, "> ", strlen("> ")) == 0)
  {
    //AT prikaz ceka na vyzvu k zadani dat a ta prisla
    ATCD_DBG_ATC_PROMT_DET

    atcd.parser.buff_pos = 0;
    atcd.parser.line_pos = 0;

    if(atcd.parser.at_cmd_top->data != NULL)
    {
      //Odesleme data
      atcd_atc_send_data_top();
    }
    else
    {
      ATCD_DBG_ATC_SEND_DATA_ERR
      //TODO: Co se stavem kdy modem ocekava nejaka data ale my je nemame a nevime kolik?
    }

    return 1;
  }

  return 0;
}
//------------------------------------------------------------------------------
