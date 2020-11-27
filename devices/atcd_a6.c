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
void atcd_seq_step()
{
  switch(atcd.proc_step)
  {
    case 0:
      //-------------------------------
      // Init cast
      //-------------------------------
      //Upravit, je treba nastavit buffery
      &atcd.at_cmd.resp           = NULL;
      &atcd.at_cmd.resp_len       = 0;
      &atcd.at_cmd.resp_buff_size = 0;

      &atcd.at_cmd.data           = NULL;
      &atcd.at_cmd.data_len       = 0;

      &atcd.at_cmd.timeout        = 5000;
    case 1:
      if(atcd_check_atc_proc(&atcd.atc) != ATCD_OK) return 1;
      atcd_atc_exec_cmd(&atcd.atc, "ATE1\r\n");         // Enable AT cmd echo
    case 2:
      if(atcd_check_atc_proc(&atcd.atc) != ATCD_OK) return 2;
      atcd_atc_exec_cmd(&atcd.atc, "ATV1\r\n");        // ???
    case 3:
      if(atcd_check_atc_proc(&atcd.atc) != ATCD_OK) return 3;
      atcd_atc_exec_cmd(&atcd.atc, "AT+CMEE=1\r\n");    // Rezim vypisu chybovych hlaseni +CME
    case 4:
      if(atcd_check_atc_proc(&atcd.atc) != ATCD_OK) return 4;
      atcd_atc_exec_cmd(&atcd.atc, "AT+CFUN=1\r\n");    // Plna fce zarizeni
    case 5:
      if(atcd_check_atc_proc(&atcd.atc) != ATCD_OK) return 5;
      atcd_atc_exec_cmd(&atcd.atc, "AT+CPIN=1\r\n");    // Je vyzadovan PIN?
    case 6:
      if(atcd_check_atc_proc(&atcd.atc) != ATCD_OK) return 6;
      if(atcd.at_cmd.resp_len != 0 && strncmp(atcd.at_cmd.resp, ATCD_STR_SIM_READY, strlen(ATCD_STR_SIM_READY)) == 0)
      {
        ATCD_DBG_PIN_NONE
      }
      else if(atcd.at_cmd.resp_len != 0 && strncmp(atcd.at_cmd.resp, ATCD_STR_SIM_PIN, strlen(ATCD_STR_SIM_PIN)) == 0)
      {
        ATCD_DBG_PIN_REQ
        atcd_atc_exec_cmd(&atcd.atc, "AT+CPIN=\"1234\\r\n");            // Zadame PIN
      }
      else
      {
        ATCD_DBG_PIN_ERR
        return 0;
      }
    case 7:
      if(atcd_check_atc_proc(&atcd.atc) != ATCD_OK) return 7;
      atcd_atc_exec_cmd(&atcd.atc, "AT+CLIP=1\r\n");           // Zobrazovat cislo volajiciho
    case 8:
      if(atcd_check_atc_proc(&atcd.atc) != ATCD_OK) return 8;
      atcd_atc_exec_cmd(&atcd.atc, "AT+CMGF=1\r\n");           // Textovy rezim SMS
    case 9:
      if(atcd_check_atc_proc(&atcd.atc) != ATCD_OK) return 9;
      atcd_atc_exec_cmd(&atcd.atc, "AT+CPMS?\r\n");           // --- Test vyuziti pametovych prostoru na SMS
    case 10:
      if(atcd_check_atc_proc(&atcd.atc) != ATCD_OK) return 10;
      atcd_atc_exec_cmd(&atcd.atc, "AT+CMGD=1,4\r\n");           // Smaze vsechny SMS na karte
    case 11:
      if(atcd_check_atc_proc(&atcd.atc) != ATCD_OK) return 11;
      atcd_atc_exec_cmd(&atcd.atc, "AT+CSDH=1\r\n");           //
    case 12:
      if(atcd_check_atc_proc(&atcd.atc) != ATCD_OK) return 12;
      atcd_atc_exec_cmd(&atcd.atc, "AT+CNMI=0,2,0,0,0\r\n");          // Rezim nakladani s novymi SMS
    /*case 13:
      if(atcd_check_atc_proc(&atcd.atc) != ATCD_OK) return 13;
      atcd_atc_exec_cmd(&atcd.atc, "AT+CNMI=1,1,0,0,0\r\n");          // Rezim nakladani s novymi SMS*/
    case 14:
      if(atcd_check_atc_proc(&atcd.atc) != ATCD_OK) return 14;
      // Inicializace byla dokoncena
      ATCD_DBG_INIT_DONE
      atc_seq->state = ATCD_ATC_SEQ_STATE_DONE;
      //atc_seq->step = 2000;
    case 100:
      //---------------------------------------
      // ---- Pocatek pravidelneho kolecka ----
      //---------------------------------------
      if(! ma se delat kolecko)
      {
        return 200;
      } 
      // Zalogovat
    case 101:
      if(atcd_check_atc_proc(&atcd.atc) != ATCD_OK) return 101;
      atcd_atc_exec_cmd(&atcd.atc, "AT+CREG=?\r\n");    // Network registration status
    case 102:
      if(atcd_check_atc_proc(&atcd.atc) != ATCD_OK) return 102;
      atcd_atc_exec_cmd(&atcd.atc, "AT+CREG=?\r\n");    // Network registration status
    case 103:
      if(atcd_check_atc_proc(&atcd.atc) != ATCD_OK) return 103;
      // Pravidelne kolecko bylo dokonceno
      // Zalogovat
    case 200: 
      //-------------------------------
      // GPRS INIT
      //-------------------------------
      if(! ma se delat GPRS init)
      {
        return xxx;
      } 
      ATCD_DBG_GPRS_INIT_START
    case 201:
      if(atcd_check_atc_proc(&atcd.atc) != ATCD_OK) return 201;
      atcd_atc_exec_cmd(&atcd.atc, "AT+CGATT=1\r\n");          
    case 202:
      if(atcd_check_atc_proc(&atcd.atc) != ATCD_OK) return 202;
      atcd_atc_exec_cmd(&atcd.atc, "AT+CIPMUX=1\r\n");         
    case 203:
      if(atcd_check_atc_proc(&atcd.atc) != ATCD_OK) return 203;
      atcd_atc_exec_cmd(&atcd.atc, "AT+CSTT=\"internet\",\"\",\"\"\r\n");     
    case 204:
      if(atcd_check_atc_proc(&atcd.atc) != ATCD_OK) return 204;
      atcd_atc_exec_cmd(&atcd.atc, "AT+CGDCONT=1,\"IP\",\"internet\"\r\n");     
    /*case 205:
      if(atcd_check_atc_proc(&atcd.atc) != ATCD_OK) return 205;
      atcd_atc_exec_cmd(&atcd.atc, "AT+CGACT=1,1\r\n"); */    
    case 206:
      if(atcd_check_atc_proc(&atcd.atc) != ATCD_OK) return 206;
      atcd_atc_exec_cmd(&atcd.atc, "AT+CIICR\r\n");   
    /*case 207:
      if(atcd_check_atc_proc(&atcd.atc) != ATCD_OK) return 207;
      atcd_atc_exec_cmd(&atcd.atc, "AT+CIFSR\r\n"); */ 
    case 208:
      if(atcd_check_atc_proc(&atcd.atc) != ATCD_OK) return 208;
      ATCD_DBG_GPRS_INIT_OK
      //atcd.gprs.state = ATCD_GPRS_STATE_CONNECTING;
      atcd.gprs.state = ATCD_GPRS_STATE_CONN;

    default:
      return 0;
  }
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
