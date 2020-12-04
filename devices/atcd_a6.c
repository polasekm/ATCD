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
#endif /* ATCD_A6 */
//------------------------------------------------------------------------------
