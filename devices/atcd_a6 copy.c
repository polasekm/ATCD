/*
 * usart.c
 *
 *  Created on: Apr 9, 2011
 *      Author: Martin
 */
//------------------------------------------------------------------------------
#include "../atcd.h"

//------------------------------------------------------------------------------
#if(ATCD_USE_DEVICE == ATCD_A6)
//------------------------------------------------------------------------------

extern atcd_t atcd;

//------------------------------------------------------------------------------
void atcd_init_seq_step(atcd_at_cmd_seq_t *atc_seq)
{
  char *cmd = NULL;

  if(atcd_atc_seq_proc(atc_seq) != 0) return;

  switch(atc_seq->step)
  {
    // Init cast
    case 0: cmd = "ATE1\r\n";            break;   // Enable AT cmd echo
    case 1: cmd = "ATV1\r\n";            break;   // ???
    case 2: cmd = "AT+CMEE=1\r\n";       break;   // Rezim vypisu chybovych hlaseni +CME
    case 3: cmd = "AT+CFUN=1\r\n";       break;   // Plna fce zarizeni
    case 4: cmd = "AT+CPIN?\r\n";        break;   // Je vyzadovan PIN?
    case 5:
      if(atcd.at_cmd.resp_len != 0 && strncmp(atcd.at_cmd.resp, ATCD_STR_SIM_READY, strlen(ATCD_STR_SIM_READY)) == 0)
      {
        ATCD_DBG_PIN_NONE
        atc_seq->step++;
      }
      else if(atcd.at_cmd.resp_len != 0 && strncmp(atcd.at_cmd.resp, ATCD_STR_SIM_PIN, strlen(ATCD_STR_SIM_PIN)) == 0)
      {
        ATCD_DBG_PIN_REQ
        cmd = "AT+CPIN=\"1234\"\r\n";             // Zadame PIN
        break;
      }
      else
      {
        ATCD_DBG_PIN_ERR
        atc_seq->step = 0;
        cmd = "ATE1\r\n";
        break;
      }

    case 6: cmd = "AT+CLIP=1\r\n";     break;  // Zobrazovat cislo volajiciho
    case 7: cmd = "AT+CMGF=1\r\n";     break;  // Textovy rezim SMS
    case 8: cmd = "AT+CPMS?\r\n";      break;  // --- Test vyuziti pametovych prostoru na SMS
    case 9: cmd = "AT+CMGD=1,4\r\n";  break;  // Smaze vsechny SMS na karte
    case 10: cmd = "AT+CSDH=1\r\n";    break;  //
    case 11: cmd = "AT+CNMI=0,2,0,0,0\r\n";    break;   // Rezim nakladani s novymi SMS
    //case 12: cmd = "AT+CNMI=1,1,0,0,0\r\n";    break;   // Rezim nakladani s novymi SMS
    case 12:
      // Inicializace byla dokoncena
      ATCD_DBG_INIT_DONE
      atc_seq->state = ATCD_ATC_SEQ_STATE_DONE;
      break;

    default:
      ATCD_DBG_INIT_ERR_R
      atc_seq->step = 0;
      cmd = "ATE1\r\n";
      break;

     case 0:         
      ATCD_DBG_GPRS_INIT_START
      atc_seq->step = 1;

    //GPRS cast
    case 1: cmd = "AT+CGATT=1\r\n";                           break;
    case 2: cmd = "AT+CIPMUX=1\r\n";                          break;
    case 3: cmd = "AT+CSTT=\"internet\",\"\",\"\"\r\n";       break;
    case 4: cmd = "AT+CGDCONT=1,\"IP\",\"internet\"\r\n";     break;
    //case 4: cmd = "AT+CGACT=1,1\r\n";                         break;
    case 5: cmd = "AT+CIICR\r\n";                             break;
    //case 4: cmd = "AT+CIFSR \r\n";                             break;

    // Konec inicializacni sekvence
    case 6:
      ATCD_DBG_GPRS_INIT_OK
      //atcd.gprs.state = ATCD_GPRS_STATE_CONNECTING;
      atcd.gprs.state = ATCD_GPRS_STATE_CONN;
      atc_seq->state = ATCD_ATC_SEQ_STATE_DONE;
      return;

    // Neocekavana hodnota - reset sekvence (nebo radeji ATCD?)
    default:
      ATCD_DBG_GPRS_INIT_ERR_R
      atc_seq->step = 1;
      cmd = "AT+CGATT=1\r\n";   
      break;
  }

  atc_seq->at_cmd->cmd = cmd;
}
//------------------------------------------------------------------------------
void atcd_restart_seq_step(atcd_at_cmd_seq_t *atc_seq)
{
  
}
//------------------------------------------------------------------------------
void atcd_check_state_seq_step(atcd_at_cmd_seq_t *atc_seq)
{
  char *cmd = NULL;

  switch(atc_seq->step)
  {
    case 0: cmd = "AT+CREG?\r\n";        break;   // Stav registrace do site
    case 1:
      // Inicializace byla dokoncena
      ATCD_DBG_STAT_DONE
      atc_seq->state = ATCD_ATC_SEQ_STATE_DONE;
      break;

    default:
      atc_seq->step = 0;
      cmd = "AT+CREG?\r\n";
      break;
  }

  atc_seq->at_cmd->cmd = cmd;
}
//------------------------------------------------------------------------------
void atcd_gprs_conn_seq_step(atcd_at_cmd_seq_t *atc_seq)
{
  char *cmd = NULL;

  switch(atc_seq->step)
  {
    case 0:         
      ATCD_DBG_GPRS_INIT_START
      atc_seq->step = 1;

    case 1: cmd = "AT+CGATT=1\r\n";                           break;
    case 2: cmd = "AT+CIPMUX=1\r\n";                          break;
    case 3: cmd = "AT+CSTT=\"internet\",\"\",\"\"\r\n";       break;
    case 4: cmd = "AT+CGDCONT=1,\"IP\",\"internet\"\r\n";     break;
    //case 4: cmd = "AT+CGACT=1,1\r\n";                         break;
    case 5: cmd = "AT+CIICR\r\n";                             break;
    //case 4: cmd = "AT+CIFSR \r\n";                             break;

    // Konec inicializacni sekvence
    case 6:
      ATCD_DBG_GPRS_INIT_OK
      //atcd.gprs.state = ATCD_GPRS_STATE_CONNECTING;
      atcd.gprs.state = ATCD_GPRS_STATE_CONN;
      atc_seq->state = ATCD_ATC_SEQ_STATE_DONE;
      return;

    // Neocekavana hodnota - reset sekvence (nebo radeji ATCD?)
    default:
      ATCD_DBG_GPRS_INIT_ERR_R
      atc_seq->step = 1;
      cmd = "AT+CGATT=1\r\n";   
      break;
  }

  //atcd_dbg_inf("GPRS: INIT: Odesilam AT prikaz ze sekvence.\r\n");
  atc_seq->at_cmd->timeout = 40000;
  atc_seq->at_cmd->cmd     = cmd;
}
//------------------------------------------------------------------------------
void atcd_gprs_disconn_seq_step(atcd_at_cmd_seq_t *atc_seq)
{
  char *cmd;

  // Volba prikazu ze sekvence
  switch(atc_seq->step)
  {
    case 0:
      ATCD_DBG_GPRS_DEINIT_START
      atc_seq->step = 1;

    case 1: cmd = "AT+CIPSHUT\r\n";                          break;

    // Konec deinicializacni sekvence
    case 2:
      ATCD_DBG_GPRS_DEINIT_OK
      atcd.gprs.state = ATCD_GPRS_STATE_DISCONN;
      atc_seq->step = 0;
      break;

    // Neocekavana hodnota - reset sekvence (nebo radeji ATCD?)
    default:
      ATCD_DBG_GPRS_DEINIT_ERR_R
      atc_seq->step = 1;
      cmd = "AT+CIPSHUT\r\n";
      break;
  }

  // Odesleme AT prikaz
  //atcd_dbg_inf("GPRS: DEINIT: Odesilam AT prikaz ze sekvence.\r\n");
  atc_seq->at_cmd->timeout = 40000;
  atc_seq->at_cmd->cmd     = cmd;
}
//------------------------------------------------------------------------------
void atcd_gprs_check_state_seq(atcd_at_cmd_seq_t *atc_seq)
{
  char *cmd;

  // Volba prikazu ze sekvence
  switch(atc_seq->step)
  {
    case 0:
      ATCD_DBG_GPRS_STAT_CGATT
      cmd = "AT+CGATT?\r\n";
      break;

    case 1:
      atcd_dbg_inf("GPRS: STAT: Dotazuji se na stav GPRS.\r\n");
      cmd = "AT+CGATT?\r\n";
      break;

    default:
      atc_seq->step = 1;
      cmd = "AT+CGATT?\r\n";
      break;
  }

  // Odesleme AT prikaz
  //atcd_dbg_inf("GPRS: STAT: Odesilam AT prikaz ze sekvence.\r\n");
  atc_seq->at_cmd->timeout = 5000;
  atc_seq->at_cmd->cmd     = cmd;
}
//------------------------------------------------------------------------------
void atcd_conns_check_state_seq(atcd_at_cmd_seq_t *atc_seq)
{

}
//------------------------------------------------------------------------------
void atcd_conn_open_seq(atcd_conn_t *conn)
{
  if(conn->at_cmd.state == ATCD_ATC_STATE_FREE)
  {
    atcd_dbg_inf("CONN: Otevitam spojeni.\r\n");
    //conn->state = ATCD_CONN_STATE_OPENING;

    atcd_atc_init(&conn->at_cmd);
    conn->at_cmd.cmd = conn->at_cmd_buff;
    conn->at_cmd.timeout = 15000;

    if(conn->protocol == ATCD_CONN_T_TCP)
    {
      sprintf(conn->at_cmd_buff, "AT+CIPSTART=%u,\"TCP\",\"%s\",%u \r\n", conn->num, conn->host, conn->port);
      atcd_atc_exec(&conn->at_cmd);
    }
    else if(conn->protocol == ATCD_CONN_T_UDP)
    {
      sprintf(conn->at_cmd_buff, "AT+CIPSTART=%u,\"UDP\",\"%s\",%u \r\n", conn->num, conn->host, conn->port);
      atcd_atc_exec(&conn->at_cmd);
    }
    else
    {
      atcd_dbg_err("CONN: Pokus otevrit spojeni nepodporovanym protokolem!\r\n");
      // Osetrit ze to nastalo - call back, stav spojeni...
    }
  }
  else if(conn->at_cmd.state == ATCD_ATC_STATE_DONE)
  {
    if(conn->at_cmd.result == ATCD_ATC_RESULT_OK)
    {
      atcd_dbg_inf("CONN: Prikaz pro otevreni spojeni dokoncen - cekam na spojeni\r\n");
      conn->state        = ATCD_CONN_STATE_OPENING;
      conn->at_cmd.state = ATCD_ATC_STATE_FREE;
      conn->timer        = atcd_get_ms();
      // asi volat call back pokud je nastaven?
    }
    else
    {
      atcd_dbg_warn("CONN: Prikaz pro otevreni spojeni skoncil chybou!\r\n");
      atcd.conns.conn[conn->num]  = NULL;

      conn->state        = ATCD_CONN_STATE_FAIL;
      conn->num          = ATCD_CONN_NO_NUM;
      conn->at_cmd.state = ATCD_ATC_STATE_FREE;
      // asi volat call back pokud je nastaven?
    }
  }
}
//------------------------------------------------------------------------------
void atcd_conn_close_seq(atcd_conn_t *conn)
{
  
}
//------------------------------------------------------------------------------
void atcd_conn_check_state_seq(atcd_conn_t *conn)
{
  
}
//------------------------------------------------------------------------------
void atcd_conn_read_seq(atcd_conn_t *conn)
{


}
//------------------------------------------------------------------------------
void atcd_conn_write_seq(atcd_conn_t *conn)
{
  uint32_t tx_data_len;

  if(conn->at_cmd.state == ATCD_ATC_STATE_FREE)
  {
    atcd_dbg_inf("CONN: Odesilam prikaz pro odeslani dat.\r\n");
    //conn->state = ATCD_CONN_STATE_OPENING;

    tx_data_len = rbuff_size(&conn->tx_rbuff);
    if(tx_data_len > 512) tx_data_len = 512;

    atcd_atc_init(&conn->at_cmd);
    sprintf(conn->at_cmd_buff, "AT+CIPSEND=%u,%u\r\n", conn->num, tx_data_len);
    conn->at_cmd.cmd = conn->at_cmd_buff;
    conn->at_cmd.timeout = 30000;

    conn->at_cmd.data = &conn->tx_rbuff;
    conn->at_cmd.data_len = tx_data_len;

    //je to jeste potreba?
    atcd.parser.tx_conn_num = conn->num;


    atcd_atc_exec(&conn->at_cmd);

    // ze by se mel nastavit stav parseru na conn tx pending
    // to ovsem nejde - musi se provest az ke konkretnimu atc
    // muze byt totiz nekde hloubeji ve fronte
    // vymyslet jak s tim

    // provedeni testovat
    //0, SEND OK

  }
  else if(conn->at_cmd.state == ATCD_ATC_STATE_DONE)
  {
    if(conn->at_cmd.result == ATCD_ATC_RESULT_OK)
    {
      atcd_dbg_inf("CONN: Ocekavam vyzvu k zadani odesilanych dat.\r\n");
      //conn->state = ATCD_CONN_STATE_OPENING;

      //ne ne tady uz jsou data zadana

      //posunout ukazovatko dat
      rbuff_seek(&conn->tx_rbuff, conn->at_cmd.data_len);

      conn->at_cmd.state = ATCD_ATC_STATE_FREE;

      // asi volat call back pokud je nastaven?
    }
    else
    {
      atcd_dbg_warn("CONN: Prikaz pro odeslani dat skoncil chybou.\r\n");
      //conn->state = ATCD_CONN_STATE_FAIL;
      conn->at_cmd.state = ATCD_ATC_STATE_FREE;

      // asi volat call back pokud je nastaven?
    }
  }
}
//------------------------------------------------------------------------------
#endif /* ATCD_A6 */
//------------------------------------------------------------------------------
