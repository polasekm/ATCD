/*
 * usart.c
 *
 *  Created on: Apr 9, 2011
 *      Author: Martin
 */
//------------------------------------------------------------------------------
#include "../atcd.h"

//------------------------------------------------------------------------------
#if(ATCD_USE_DEVICE == ATCD_SIM7000)
//------------------------------------------------------------------------------
extern atcd_t atcd;

//------------------------------------------------------------------------------
uint16_t atcd_proc_step()
{
  uint32_t tx_data_len;
  static atcd_conn_t *conn;

  switch(atcd.proc_step)
  {
    case 0:
      //-------------------------------
      // Init cast
      //-------------------------------
      // Zahajime inicializacni sekvenci
      ATCD_DBG_INIT_START

      //Upravit, je treba nastavit buffery
      atcd.at_cmd.resp           = NULL;
      atcd.at_cmd.resp_len       = 0;
      atcd.at_cmd.resp_buff_size = 0;

      atcd.at_cmd.data           = NULL;
      atcd.at_cmd.data_len       = 0;


      atcd.at_cmd.timeout        = 5000;
    case 1:
      atcd_atc_exec_cmd(&atcd.at_cmd, "ATE1\r\n");         // Enable AT cmd echo
    case 2:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return 2;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return 99;
      atcd_atc_exec_cmd(&atcd.at_cmd, "ATV1\r\n");        // ???
    case 3:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return 3;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return 99;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CMEE=1\r\n");    // Rezim vypisu chybovych hlaseni +CME
    case 4:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return 4;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return 99;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CFUN=1\r\n");    // Plna fce zarizeni
    case 5:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return 5;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return 99;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CPIN?\r\n");    // Je vyzadovan PIN?
    case 6:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return 6;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return 99;
      if(atcd.at_cmd.resp_len != 0 && strncmp(atcd.at_cmd.resp, ATCD_STR_SIM_READY, strlen(ATCD_STR_SIM_READY)) == 0)
      {
        ATCD_DBG_PIN_NONE
      }
      else if(atcd.at_cmd.resp_len != 0 && strncmp(atcd.at_cmd.resp, ATCD_STR_SIM_PIN, strlen(ATCD_STR_SIM_PIN)) == 0)
      {
        ATCD_DBG_PIN_REQ
        atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CPIN=\"1234\"\r\n");            // Zadame PIN
      }
      else if(atcd.at_cmd.resp_len != 0 && strncmp(atcd.at_cmd.resp, ATCD_STR_SIM_PUK, strlen(ATCD_STR_SIM_PUK)) == 0)
      {
        ATCD_DBG_PUK_REQ
        //Bude to chtit PUK
        //Co delat?
        //atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CPIN=\"1234\\r\n");            // Zadame PUK
      }
      else
      {
        ATCD_DBG_PIN_ERR
        return 99;
      }
    case 7:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return 7;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return 99;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CLIP=1\r\n");           // Zobrazovat cislo volajiciho
    case 8:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return 8;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return 99;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CMGF=1\r\n");           // Textovy rezim SMS
    case 9:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return 9;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return 99;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CPMS?\r\n");           // --- Test vyuziti pametovych prostoru na SMS
    case 10:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return 10;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return 99;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CMGD=1,4\r\n");           // Smaze vsechny SMS na karte
    case 11:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return 11;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return 99;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CSDH=1\r\n");           //
    case 12:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return 12;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return 99;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CNMI=0,2,0,0,0\r\n");          // Rezim nakladani s novymi SMS
    /*case 13:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return 13;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return 99;
      atcd_atc_exec_cmd(&atcd.atc, "AT+CNMI=1,1,0,0,0\r\n");          // Rezim nakladani s novymi SMS*/
    case 14:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return 14;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return 99;
      // Inicializace byla dokoncena
      ATCD_DBG_INIT_DONE
      // Inicializace je dokoncena
      ATCD_DBG_INIT_OK
      // Zmenime stav zarizeni
      atcd.state = ATCD_STATE_ON;
      atcd.err_cnt = 0;
      return 100;
    case 99:
      // V prubehu inicializace doslo k chybe
      ATCD_DBG_INIT_ERR
      atcd.err_cnt++;
      //nekde resit reset pri prekroceni poctu pokusu
      return 0;

    case 100:
      //---------------------------------------
      // ---- Pocatek pravidelneho kolecka ----
      //---------------------------------------
      // Zarizeni je pripraveno k praci, pripadne spi...
      // Pripadne testy stavu a dalsi cinnosti na pozadi...
      if(atcd_get_ms() - atcd.timer < 7500)
      {
        return 200;
      } 
      // Je cast spustit kontrolu stavu modemu
      ATCD_DBG_STAT_START
      atcd.timer = atcd_get_ms();
      atcd.at_cmd.timeout = 5000;
    case 101:
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CREG?\r\n");    // Network registration status
    case 102:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return 102;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return 199;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CGATT?\r\n");    // GPRS registration status
    case 103:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return 103;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return 199;
      // Pravidelne kolecko bylo dokonceno
      // Dotazovani na stav je hotovo
      ATCD_DBG_STAT_OK
      atcd.err_cnt = 0;
      return 200;

      //Nemel by se testovat stav spojeni?

    case 199:
      // Dotazovani na stav selhalo
      ATCD_DBG_STAT_ERR
      atcd.err_cnt++;
      //nekde resit reset pri prekroceni poctu pokusu

    case 200: 
      //-------------------------------
      // GPRS INIT
      //-------------------------------
      if(atcd.gprs.state != ATCD_GPRS_STATE_CONNECTING)
      {
        return 300;
      } 
      ATCD_DBG_GPRS_INIT_START
      atcd.at_cmd.timeout = 40000;
    case 201:
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CGATT=1\r\n");
    case 202:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return 202;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return 299;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CIPMUX=1\r\n");
    case 203:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return 203;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return 299;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CSTT=\"internet\",\"\",\"\"\r\n");
    case 204:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return 204;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return 299;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CGDCONT=1,\"IP\",\"internet\"\r\n");
    /*case 205:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return 205;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return 299;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CGACT=1,1\r\n"); */
    case 206:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return 206;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return 299;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CIICR\r\n");
    /*case 207:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return 207;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return 299;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CIFSR\r\n"); */
    case 208:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return 208;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return 299;
      ATCD_DBG_GPRS_INIT_OK
      //atcd.gprs.state = ATCD_GPRS_STATE_CONNECTING;
      // Opravdu? Neceka se asznc?
      atcd.gprs.state = ATCD_GPRS_STATE_CONN;
      return 300;
    case 299:
      //GPRS init selhalo
      //Zalogovat!
      atcd.gprs.state = ATCD_GPRS_STATE_DISCONN;

    case 300: 
      //-------------------------------
      // GPRS DEINIT
      //-------------------------------
      if(atcd.gprs.state != ATCD_GPRS_STATE_DISCONNING)
      {
        return 400;
      } 
      ATCD_DBG_GPRS_DEINIT_START
      atcd.at_cmd.timeout = 40000;
    case 301:
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CIPSHUT\r\n");
    case 302:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return 302;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return 399;
      ATCD_DBG_GPRS_DEINIT_OK
      atcd.gprs.state = ATCD_GPRS_STATE_DISCONN;
      return 400;
    case 399:
      //GPRS deinit selhalo
      //Zalogovat!
      atcd.gprs.state = ATCD_GPRS_STATE_DISCONN;

    case 400: 
      //-------------------------------
      // CONN OPEN
      //-------------------------------
      if(atcd.conns.conn_num_proc >= ATCD_CONN_MAX_NUMBER)
      {
        atcd.conns.conn_num_proc = 0;
        return 500;
      }
      else
      {
        conn = atcd.conns.conn[atcd.conns.conn_num_proc];
        atcd.conns.conn_num_proc++;
      }

      if(conn == NULL || conn->state != ATCD_CONN_STATE_W_OPEN) return 400;

      atcd_dbg_inf("CONN: Otevitam spojeni.\r\n");
      //conn->state = ATCD_CONN_STATE_OPENING;
      atcd.at_cmd.timeout = 15000;
      atcd.at_cmd.cmd = atcd.at_cmd_buff;

      if(conn->protocol == ATCD_CONN_T_TCP)
      {
        sprintf(atcd.at_cmd.cmd, "AT+CIPSTART=%u,\"TCP\",\"%s\",%u\r\n", conn->num, conn->host, conn->port);
        atcd_atc_exec(&atcd.at_cmd);
      }
      else if(conn->protocol == ATCD_CONN_T_UDP)
      {
        sprintf(atcd.at_cmd.cmd, "AT+CIPSTART=%u,\"UDP\",\"%s\",%u\r\n", conn->num, conn->host, conn->port);
        atcd_atc_exec(&atcd.at_cmd);
      }
      else
      {
        atcd_dbg_err("CONN: Pokus otevrit spojeni nepodporovanym protokolem!\r\n");
        // Osetrit ze to nastalo - call back, stav spojeni...
        return 499;
      }
    case 401:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return 401;
      if(atcd.at_cmd.result == ATCD_ATC_RESULT_OK)
      {
        atcd_dbg_inf("CONN: Prikaz pro otevreni spojeni dokoncen - cekam na spojeni\r\n");
        conn->state = ATCD_CONN_STATE_OPENING;
        conn->timer = atcd_get_ms();
        // asi volat call back pokud je nastaven?
      }
      else
      {
        atcd_dbg_warn("CONN: Prikaz pro otevreni spojeni skoncil chybou!\r\n");
        //atcd.conns.conn[conn->num]  = NULL;

        //atcd_conn_free(conn);
        //conn->state = ATCD_CONN_STATE_FAIL;
        // asi volat call back pokud je nastaven?
        return 499;
      }

      return 400;
    case 499:
      //Otevreni spojeni selhalo
      //Zalogovat!
      //bude volat call back od close, pozor...
      //opravdu to neni reduncance - kde vsude se vola, projit..
      atcd_conn_free(conn);
      conn->state = ATCD_CONN_STATE_FAIL;
      return 400;

    case 500: 
      //-------------------------------
      // CONN WRITE
      //-------------------------------

      if(atcd.conns.conn_num_proc >= ATCD_CONN_MAX_NUMBER)
      {
        atcd.conns.conn_num_proc = 0;
        return 600;
      }
      else
      {
        conn = atcd.conns.conn[atcd.conns.conn_num_proc];
        atcd.conns.conn_num_proc++;
      }

      if(conn == NULL || conn->state != ATCD_CONN_STATE_OPEN) return 500;

      tx_data_len = rbuff_size(&conn->tx_rbuff);
      if(tx_data_len == 0) return 500;

      atcd.at_cmd.timeout = 30000;

      atcd_dbg_inf("CONN: Odesilam prikaz pro odeslani dat.\r\n");
      //conn->state = ATCD_CONN_STATE_OPENING;

      if(tx_data_len > 512) tx_data_len = 512;

      atcd.at_cmd.cmd = atcd.at_cmd_buff;
      atcd.at_cmd.timeout = 30000;
      atcd.at_cmd.data = &conn->tx_rbuff;
      atcd.at_cmd.data_len = tx_data_len;

      sprintf(atcd.at_cmd.cmd, "AT+CIPSEND=%u,%u\r\n", conn->num, tx_data_len);

      //je to jeste potreba?
      atcd.parser.tx_conn_num = conn->num;

      atcd_atc_exec(&atcd.at_cmd);

      // ze by se mel nastavit stav parseru na conn tx pending
      // to ovsem nejde - musi se provest az ke konkretnimu atc
      // muze byt totiz nekde hloubeji ve fronte
      // vymyslet jak s tim

      // provedeni testovat
      //0, SEND OK

    case 501:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return 501;
      if(atcd.at_cmd.result == ATCD_ATC_RESULT_OK)
      {
        //necekat na vyzvu az tady
        //nemusi byt, u A6 je vyzva jeste v tele AT prikazu
        //vetsinou v tele prijde i +IPD

        //
        //ne ne tady uz jsou data zadana

        //posunout ukazovatko dat
        rbuff_seek(&conn->tx_rbuff, atcd.at_cmd.data_len);

        // asi volat call back pokud je nastaven?
      }
      else
      {
        atcd_dbg_warn("CONN: Prikaz pro odeslani dat skoncil chybou.\r\n");
        //conn->state = ATCD_CONN_STATE_FAIL;

        // asi volat call back pokud je nastaven?
        return 599;
      }

    case 502:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return 502;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return 599;

      return 600;
    case 599:
      //Zapis do spojeni selhal
      //Zalogovat!

      //-------------------------------
      // CONN READ - Doplnit
      //-------------------------------

    case 600:
      //-------------------------------
      // CONN CLOSE
      //-------------------------------
      if(atcd.conns.conn_num_proc >= ATCD_CONN_MAX_NUMBER)
      {
        atcd.conns.conn_num_proc = 0;
        return 700;
      }
      else
      {
        conn = atcd.conns.conn[atcd.conns.conn_num_proc];
        atcd.conns.conn_num_proc++;
      }

      if(conn == NULL || conn->state != ATCD_CONN_STATE_W_CLOSE) return 600;

      atcd_dbg_inf("CONN: Zaviram spojeni.\r\n");
      //conn->state = ATCD_CONN_STATE_OPENING;
      atcd.at_cmd.timeout = 15000;
      atcd.at_cmd.cmd = atcd.at_cmd_buff;

      sprintf(atcd.at_cmd.cmd, "AT+CIPCLOSE=%u\r\n", conn->num);
      atcd_atc_exec(&atcd.at_cmd);

    case 601:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return 601;
      if(atcd.at_cmd.result == ATCD_ATC_RESULT_OK)
      {
        atcd_dbg_inf("CONN: Prikaz pro uzavreni spojeni dokoncen - cekam na ukonceni\r\n");
        conn->state = ATCD_CONN_STATE_CLOSING;
        conn->timer = atcd_get_ms();
        // asi volat call back pokud je nastaven?
      }
      else
      {
        atcd_dbg_warn("CONN: Prikaz pro ukoceni spojeni skoncil chybou!\r\n");
        //atcd.conns.conn[conn->num]  = NULL;

        //atcd_conn_free(conn);
        //conn->state = ATCD_CONN_STATE_FAIL;
        // asi volat call back pokud je nastaven?
        return 699;
      }

      return 700;
    case 699:
      //Uzavreni spojeni selhalo
      //Zalogovat!
      //bude volat call back od close, pozor...
      //opravdu to neni reduncance - kde vsude se vola, projit..
      atcd_conn_free(conn);
      conn->state = ATCD_CONN_STATE_FAIL;
      return 400;

    case 700:
      //Konec, navrat na pocatek...
      return 100;

    default:
      return 0;
  }
}
//------------------------------------------------------------------------------
void atcd_init_seq()
{
  char *cmd = NULL;

  if(atcd.at_cmd.state == ATCD_ATC_STATE_DONE || atcd.at_cmd.state == ATCD_ATC_STATE_FREE)
  {
    if(atcd.at_cmd.result == ATCD_ATC_RESULT_OK)
    {
      atcd.at_cmd_seq++;
    }
    else if(atcd.at_cmd.state != ATCD_ATC_STATE_FREE)
    {
      atcd_dbg_warn("ATCD: INIT: Sekvence skoncila chybou - zacinam znovu!\r\n");
      atcd.at_cmd_seq = 0;
    }

    switch(atcd.at_cmd_seq)
    {
      case 0: cmd = "ATE1\r\n";            break;   // Enable AT cmd echo
      case 1: cmd = "ATV1\r\n";            break;   // ???
      case 2: cmd = "AT+CMEE=1\r\n";       break;   // Rezim vypisu chybovych hlaseni +CME
      case 3: cmd = "AT+CFUN=1\r\n";       break;   // Plna fce zarizeni
      case 4: cmd = "AT+CPIN?\r\n";        break;   // Je vyzadovan PIN?
      case 5:
        if(atcd.at_cmd.resp_len != 0 && strncmp(atcd.at_cmd.resp, ATCD_STR_SIM_READY, strlen(ATCD_STR_SIM_READY)) == 0)
        {
          atcd_dbg_inf("ATCD: INIT: PIN neni treba.\r\n");
          atcd.at_cmd_seq++;
        }
        else if(atcd.at_cmd.resp_len != 0 && strncmp(atcd.at_cmd.resp, ATCD_STR_SIM_PIN, strlen(ATCD_STR_SIM_PIN)) == 0)
        {
          atcd_dbg_inf("ATCD: INIT: Je treba zadat PIN.\r\n");
          cmd = "AT+CPIN=\"1234\"\r\n";             // Zadame PIN
          break;
        }
        else
        {
          atcd_dbg_err("ATCD: INIT: Na dotaz na PIN prisla neocekavana odpoved - zacinam znovu!\r\n");
          atcd.at_cmd_seq = 0;
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
        atcd_dbg_inf("ATCD: INIT: Sekvence byla dokoncena.\r\n");
        atcd.at_cmd.state = ATCD_ATC_STATE_FREE;
        atcd.state = ATCD_STATE_READY;
        atcd.at_cmd_seq = 0;
        break;

      default: 
        atcd_dbg_err("ATCD: INIT: atcd.init_seq je  mimo rozsah - zacinam znovu!\r\n");
        atcd.at_cmd_seq = 0;
        cmd = "ATE1\r\n";
        break;
    }

    if(atcd.state == ATCD_STATE_INIT)
    {
      atcd_dbg_inf("ATCD: INIT: Odesilam dalsi krok sekvence.\r\n");
      atcd_atc_init(&atcd.at_cmd);
      atcd.at_cmd.cmd = cmd;
      atcd_atc_exec(&atcd.at_cmd);
    }
  }
}
//------------------------------------------------------------------------------
void atcd_restart_seq()
{
  
}
//------------------------------------------------------------------------------
void atcd_check_state_seq()
{
  char *cmd;

  if(atcd_get_ms() - atcd.timer > 7500)
  {
    if(atcd.state == ATCD_STATE_READY && (atcd.at_cmd.state == ATCD_ATC_STATE_FREE || atcd.at_cmd.state == ATCD_ATC_STATE_DONE))
    {
      atcd.timer = atcd_get_ms();

      atcd_dbg_inf("ATCD: STAT: Dotazuji se na stav registrace do site.\r\n");
      atcd_atc_init(&atcd.at_cmd);
      cmd = "AT+CREG?\r\n";
      atcd.at_cmd.cmd = cmd;
      atcd_atc_exec(&atcd.at_cmd);
    }
  }
}
//------------------------------------------------------------------------------
void atcd_gprs_init_seq()
{
  char *cmd;

  //--------------------------------------------------
  // Zpracovani vysledku posledne volaneho AT prikazu
  //--------------------------------------------------
  if(atcd.gprs.at_cmd.state == ATCD_ATC_STATE_DONE)
  {
    // AT prikaz byl dokoncen
    if(atcd.gprs.at_cmd_seq == 0)
    {
      // Pokud se jedna o prechod z jineho stavu,
      // tak nas vysledek predchoziho AT prikazu nezajima...
      atcd.gprs.at_cmd.state = ATCD_ATC_STATE_FREE;
    }
    else 
    {
      // Zpracujeme vysledek AT prikazu
      if(atcd.gprs.at_cmd.result == ATCD_ATC_RESULT_OK)
      {
        // Zpracovani probehlo v poradku
        atcd_dbg_inf("GPRS: INIT: AT prikaz byl dokoncen.\r\n");

        // ... Zpracovat odpoved...

        // Posuneme se na dalsi krok sekvence
        atcd.gprs.at_cmd_seq++;
        // Vynulujeme citac chyb
        atcd.gprs.err_cnt = 0;
      }
      else 
      {
        // Pri zpracovani doslo k chybe
        atcd_dbg_warn("GPRS: INIT: Pri zpracovani AT prikazu doslo k chybe.\r\n");
        // Posuneme sekvenci zpet na zacatek
        atcd.gprs.at_cmd_seq = 1;
        // Inkrementujeme citac chyb
        atcd.gprs.err_cnt++;
        if(atcd.gprs.err_cnt >= 5)
        {
          // Pokus o opravu odpojenim a znovu pripojenim
          atcd_dbg_warn("GPRS: INIT: Prekrocen pocet neuspesnych pokusu, zkousim o opravu odpojenim a znovu pripojenim.\r\n");
          atcd_atc_init(&atcd.gprs.at_cmd);
          atcd.gprs.at_cmd.cmd = "AT+CGATT=0\r\n";
          atcd.gprs.at_cmd.timeout = 10000;
          atcd_atc_exec(&atcd.gprs.at_cmd);
          return;
        }
      }

      // Nastavime AT prikaz jako uvolneny
      atcd.gprs.at_cmd.state = ATCD_ATC_STATE_FREE;
    }
  }
  //--------------------------------------------------
  // Pripadne odeslani dotazu
  //--------------------------------------------------
  // Pokud se jeste provadi AT prikaz, pockame na jeho dokonceni
  if(atcd.gprs.at_cmd.state == ATCD_ATC_STATE_FREE)
  {
    // Inicializace struktuty AT prikazu
    atcd_atc_init(&atcd.gprs.at_cmd);
    atcd.gprs.at_cmd.timeout = 40000;

    // Volba prikazu ze sekvence
    switch(atcd.gprs.at_cmd_seq)
    {
      case 0:         
        atcd_dbg_inf("GPRS: INIT: Zacinam s inicializaci GPRS.\r\n");
        atcd.gprs.at_cmd_seq = 1;

      case 1: cmd = "AT+CGATT=1\r\n";                           break;
      case 2: cmd = "AT+CIPMUX=1\r\n";                          break;
      case 3: cmd = "AT+CSTT=\"internet\",\"\",\"\"\r\n";       break;
      case 4: cmd = "AT+CGDCONT=1,\"IP\",\"internet\"\r\n";     break;
      //case 4: cmd = "AT+CGACT=1,1\r\n";                         break;
      case 5: cmd = "AT+CIICR\r\n";                             break;
      //case 4: cmd = "AT+CIFSR \r\n";                             break;

      // Konec inicializacni sekvence
      case 6:
        atcd_dbg_inf("GPRS: INIT: Init sekvence dokoncena - cekam na pripojeni.\r\n");
        //atcd.gprs.state = ATCD_GPRS_STATE_CONNECTING;
        atcd.gprs.state = ATCD_GPRS_STATE_CONN;
        atcd.gprs.at_cmd_seq = 0;
        atcd.gprs.err_cnt = 0;
        return;

      // Neocekavana hodnota - reset sekvence (nebo radeji ATCD?)
      default:
        atcd_dbg_err("GPRS: INIT: Sekvence ma neplatne cislo kroku - zacinam znovu!\r\n");
        atcd.gprs.at_cmd_seq = 1;
        cmd = "AT+CGATT=1\r\n";   
        break;
    }

    // Odesleme AT prikaz
    atcd_dbg_inf("GPRS: INIT: Odesilam AT prikaz ze sekvence.\r\n");
    atcd.gprs.at_cmd.cmd = cmd;
    atcd_atc_exec(&atcd.gprs.at_cmd);
  }
}
//------------------------------------------------------------------------------
void atcd_gprs_deinit_seq()
{
  char *cmd;

  //--------------------------------------------------
  // Zpracovani vysledku posledne volaneho AT prikazu
  //--------------------------------------------------
  if(atcd.gprs.at_cmd.state == ATCD_ATC_STATE_DONE)
  {
    // AT prikaz byl dokoncen
    if(atcd.gprs.at_cmd_seq == 0)
    {
      // Pokud se jedna o prechod z jineho stavu,
      // tak nas vysledek predchoziho AT prikazu nezajima...
      atcd.gprs.at_cmd.state = ATCD_ATC_STATE_FREE;
    }
    else 
    {
      // Zpracujeme vysledek AT prikazu
      if(atcd.gprs.at_cmd.result == ATCD_ATC_RESULT_OK)
      {
        // Zpracovani probehlo v poradku
        atcd_dbg_inf("GPRS: DEINIT: AT prikaz byl dokoncen.\r\n");

        // ... Zpracovat odpoved...

        // Posuneme se na dalsi krok sekvence
        atcd.gprs.at_cmd_seq++;
        // Vynulujeme citac chyb
        atcd.gprs.err_cnt = 0;
      }
      else 
      {
        // Pri zpracovani doslo k chybe
        atcd_dbg_warn("GPRS: DEINIT: Pri zpracovani AT prikazu doslo k chybe.\r\n");
        // Posuneme sekvenci zpet na zacatek
        atcd.gprs.at_cmd_seq = 1;
      }

      // Nastavime AT prikaz jako uvolneny
      atcd.gprs.at_cmd.state = ATCD_ATC_STATE_FREE;
    }
  }
  //--------------------------------------------------
  // Pripadne odeslani dotazu
  //--------------------------------------------------
  // Pokud se jeste provadi AT prikaz, pockame na jeho dokonceni
  if(atcd.gprs.at_cmd.state == ATCD_ATC_STATE_FREE)
  {
    // Inicializace struktuty AT prikazu
    atcd_atc_init(&atcd.gprs.at_cmd);
    atcd.gprs.at_cmd.timeout = 40000;

    // Volba prikazu ze sekvence
    switch(atcd.gprs.at_cmd_seq)
    {
      case 0:         
        atcd_dbg_inf("GPRS: DEINIT: Zacinam s deinicializaci GPRS.\r\n");
        atcd.gprs.at_cmd_seq = 1;

      case 1: cmd = "AT+CIPSHUT\r\n";                          break;

      // Konec deinicializacni sekvence
      case 2:
        atcd_dbg_inf("GPRS: DEINIT: Deinit sekvence dokoncena.\r\n");
        atcd.gprs.state = ATCD_GPRS_STATE_DISCONN;
        atcd.gprs.at_cmd_seq = 0;
        atcd.gprs.err_cnt = 0;
        break;

      // Neocekavana hodnota - reset sekvence (nebo radeji ATCD?)
      default:
        atcd_dbg_err("GPRS: DEINIT: Sekvence ma neplatne cislo kroku - zacinam znovu!\r\n");
        atcd.gprs.at_cmd_seq = 1;
        cmd = "AT+CIPSHUT\r\n";  
        break;
    }

    // Odesleme AT prikaz
    atcd_dbg_inf("GPRS: DEINIT: Odesilam AT prikaz ze sekvence.\r\n");
    atcd.gprs.at_cmd.cmd = cmd;
    atcd_atc_exec(&atcd.gprs.at_cmd);
  }
}
//------------------------------------------------------------------------------
void atcd_gprs_check_state_seq()
{
  //--------------------------------------------------
  // Zpracovani vysledku posledne volaneho AT prikazu
  //--------------------------------------------------
  if(atcd.gprs.at_cmd.state == ATCD_ATC_STATE_DONE)
  {
    // AT prikaz byl dokoncen
    if(atcd.gprs.at_cmd_seq == 0)
    {
      // Pokud se jedna o prechod z jineho stavu,
      // tak nas vysledek predchoziho AT prikazu nezajima...
      atcd.gprs.at_cmd.state = ATCD_ATC_STATE_FREE;
    }
    else 
    {
      // Zpracujeme vysledek AT prikazu
      if(atcd.gprs.at_cmd.result == ATCD_ATC_RESULT_OK)
      {
        // Zpracovani probehlo v poradku
        atcd_dbg_inf("GPRS: STAT: AT prikaz byl dokoncen.\r\n");
        // ... Zpracovbat odpoved...
      }
      else 
      {
        // Pri zpracovani doslo k chybe
        atcd_dbg_warn("GPRS: STAT: Pri zpracovani AT prikazu doslo k chybe.\r\n");
      }

      // Posuneme se na dalsi krok sekvence
      atcd.gprs.at_cmd_seq++;
      // Nastavime AT prikaz jako uvolneny
      atcd.gprs.at_cmd.state = ATCD_ATC_STATE_FREE;
    }
  }
  //--------------------------------------------------
  // Test timeoutu a pripadne odeslani dotazu
  //--------------------------------------------------
  if(atcd_get_ms() - atcd.gprs.timer > 2000)
  {
    atcd.gprs.timer = atcd_get_ms();
    atcd_dbg_inf("GPRS: STAT: Kontrola stavu pripojeni.\r\n");
    
    // Pokud se jeste provadi AT prikaz, pockame na jeho dokonceni
    if(atcd.gprs.at_cmd.state == ATCD_ATC_STATE_FREE)
    {
      // Inicializace struktuty AT prikazu
      atcd_atc_init(&atcd.gprs.at_cmd);
      atcd.gprs.at_cmd.timeout = 5000;

      // Volba prikazu ze sekvence
      switch(atcd.gprs.at_cmd_seq)
      {
        case 0:         
          atcd_dbg_inf("GPRS: STAT: Zacinam s testy stavu GPRS.\r\n");
          atcd.gprs.at_cmd_seq = 1;
        case 1:         
          atcd_dbg_inf("GPRS: STAT: Dotazuji se na stav GPRS.\r\n");
          atcd.gprs.at_cmd.cmd = "AT+CGATT?\r\n";  
          break;

        case 2:
          atcd_dbg_inf("GPRS: STAT: Dotazuji se na stav GPRS.\r\n");
          atcd.gprs.at_cmd.cmd = "AT+CGATT?\r\n";  
          break;

        case 3:
          atcd_dbg_inf("GPRS: STAT: Dotazuji se na stav GPRS.\r\n");
          atcd.gprs.at_cmd.cmd = "AT+CGATT?\r\n";  
          break;

        default:
         atcd_dbg_inf("GPRS: STAT: Dotazuji se na stav GPRS.\r\n");
          atcd.gprs.at_cmd_seq = 1;
          atcd.gprs.at_cmd.cmd = "AT+CGATT?\r\n";  
          break;
      }

      // Odesleme AT prikaz
      atcd_dbg_inf("GPRS: STAT: Odesilam AT prikaz ze sekvence.\r\n");
      atcd_atc_exec(&atcd.gprs.at_cmd);
    }
  }
}
//------------------------------------------------------------------------------
void atcd_conns_check_state_seq()
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
#endif /* ATCD_SIM7000 */
//------------------------------------------------------------------------------
