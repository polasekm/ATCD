/*
 * usart.c
 *
 *  Created on: Apr 9, 2011
 *      Author: Martin
 */
//------------------------------------------------------------------------------
#include "../atcd.h"

//------------------------------------------------------------------------------
#if(ATCD_USE_DEVICE == ATCD_A7)
//------------------------------------------------------------------------------
extern atcd_t atcd;

//------------------------------------------------------------------------------
uint16_t atcd_proc_step()
{
  uint32_t tx_data_len;
  static atcd_conn_t *conn;

  switch(atcd.proc_step)
  {
      //------------------------------------------------------------------------
      // Init cast
      //------------------------------------------------------------------------
  case ATCD_SB_INIT:
      // Zahajime inicializacni sekvenci
      ATCD_DBG_INIT_START

      //Upravit, je treba nastavit buffery
      atcd.at_cmd.resp           = NULL;
      atcd.at_cmd.resp_len       = 0;
      atcd.at_cmd.resp_buff_size = 0;

      atcd.at_cmd.data           = NULL;
      atcd.at_cmd.data_len       = 0;

      atcd.at_cmd.timeout        = 5000;
    case ATCD_SB_INIT + 1:
      atcd_atc_exec_cmd(&atcd.at_cmd, "ATE1\r\n");         // Enable AT cmd echo
    case ATCD_SB_INIT + 2:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_INIT + 2;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_INIT + ATCD_SO_ERR;
      atcd_atc_exec_cmd(&atcd.at_cmd, "ATV1\r\n");        // ???
    case ATCD_SB_INIT + 3:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_INIT + 3;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_INIT + ATCD_SO_ERR;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CMEE=1\r\n");    // Rezim vypisu chybovych hlaseni +CME
    /*case ATCD_SB_INIT + 4:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_INIT + 4;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_INIT + ATCD_SO_ERR;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CFUN=1\r\n");    // Plna fce zarizeni*/
    case ATCD_SB_INIT + 5:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_INIT + 5;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_INIT + ATCD_SO_ERR;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CPIN?\r\n");    // Je vyzadovan PIN?
    case ATCD_SB_INIT + 6:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_INIT + 6;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_INIT + ATCD_SO_ERR;
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
        return ATCD_SB_INIT + ATCD_SO_ERR;
      }      
    case ATCD_SB_INIT + 7:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_INIT + 7;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_INIT + ATCD_SO_ERR;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CLIP=1\r\n");           // Zobrazovat cislo volajiciho
    case ATCD_SB_INIT + 8:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_INIT + 8;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_INIT + ATCD_SO_ERR;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CMGF=1\r\n");           // Textovy rezim SMS
    case ATCD_SB_INIT + 9:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_INIT + 9;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_INIT + ATCD_SO_ERR;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CPMS?\r\n");           // --- Test vyuziti pametovych prostoru na SMS
    case ATCD_SB_INIT + 10:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_INIT + 10;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_INIT + ATCD_SO_ERR;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CMGD=1,4\r\n");           // Smaze vsechny SMS na karte
    case ATCD_SB_INIT + 11:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_INIT + 11;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_INIT + ATCD_SO_ERR;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CSDH=1\r\n");           //
    case ATCD_SB_INIT + 12:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_INIT + 12;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_INIT + ATCD_SO_ERR;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CNMI=0,2,0,0,0\r\n");          // Rezim nakladani s novymi SMS
    /*case ATCD_SB_INIT + 13:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_INIT + 13;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_INIT + ATCD_SO_ERR;
      atcd_atc_exec_cmd(&atcd.atc, "AT+CNMI=1,1,0,0,0\r\n");          // Rezim nakladani s novymi SMS*/
    case ATCD_SB_INIT + 14:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_INIT + 14;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_INIT + ATCD_SO_ERR;
      // Inicializace byla dokoncena
      ATCD_DBG_INIT_DONE
      // Inicializace je dokoncena
      ATCD_DBG_INIT_OK
      // Zmenime stav zarizeni
      atcd.state = ATCD_STATE_ON;
      atcd.err_cnt = 0;
      return ATCD_SB_INIT + ATCD_SO_END;

    case ATCD_SB_INIT + ATCD_SO_ERR:
      // V prubehu inicializace doslo k chybe
      ATCD_DBG_INIT_ERR
      atcd.err_cnt++;
      //nekde resit reset pri prekroceni poctu pokusu
      return ATCD_SB_INIT;

    case ATCD_SB_INIT + ATCD_SO_END:
      //------------------------------------------------------------------------
      // ---- Pocatek pravidelneho kolecka ----
      //------------------------------------------------------------------------
    case ATCD_SB_STAT:
      // Zarizeni je pripraveno k praci, pripadne spi...
      // Pripadne testy stavu a dalsi cinnosti na pozadi...
      if(atcd_get_ms() - atcd.timer < 7500)
      {
        return ATCD_SB_STAT + ATCD_SO_END;
      } 
      // Je cast spustit kontrolu stavu modemu
      ATCD_DBG_STAT_START
      atcd.timer = atcd_get_ms();
      atcd.at_cmd.timeout = 5000;
    case ATCD_SB_STAT + 1:
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CREG?\r\n");    // Network registration status
    case ATCD_SB_STAT + 2:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_STAT + 2;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_STAT + ATCD_SO_ERR;

      //Zpracovat stav registrace...

      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CGATT?\r\n");    // GPRS registration status
    case ATCD_SB_STAT + 3:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_STAT + 3;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_STAT + ATCD_SO_ERR;

      //Zpracovat stav PDP kontextu

      // Pravidelne kolecko bylo dokonceno
      // Dotazovani na stav je hotovo
      ATCD_DBG_STAT_OK
      atcd.err_cnt = 0;
      return ATCD_SB_STAT + ATCD_SO_END;

      //Nemel by se testovat stav spojeni?

    case ATCD_SB_STAT + ATCD_SO_ERR:
      // Dotazovani na stav selhalo
      if(atcd.phone.state != ATCD_PHONE_STATE_CALL)
      {
        ATCD_DBG_STAT_ERR
        atcd.err_cnt++;
        //nekde resit reset pri prekroceni poctu pokusu
      }

    case ATCD_SB_STAT + ATCD_SO_END:
      //------------------------------------------------------------------------
      // PHONE
      //------------------------------------------------------------------------
    case ATCD_SB_PHONE:
      //if(atcd.phone.state == ATCD_PHONE_STATE_IDLE) return ATCD_SB_PHONE + ATCD_SO_END;

      atcd.at_cmd.timeout = 5000;
    case ATCD_SB_PHONE + 1:

      if(atcd.phone.state != ATCD_PHONE_STATE_RING_WA) return ATCD_SB_PHONE + 3;
      atcd_atc_exec_cmd(&atcd.at_cmd, "ATA\r\n");
    case ATCD_SB_PHONE + 2:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_PHONE + 2;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_PHONE + 3;
      atcd.phone.state = ATCD_PHONE_STATE_CALL;

    case ATCD_SB_PHONE + 3:
      if(atcd.phone.state != ATCD_PHONE_STATE_HANG_W) return ATCD_SB_PHONE + 5;
      atcd_atc_exec_cmd(&atcd.at_cmd, "ATH\r\n");
    case ATCD_SB_PHONE + 4:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_PHONE + 4;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_PHONE + 5;
      atcd.phone.state = ATCD_PHONE_STATE_IDLE;
      atcd.phone.number[0] = 0;

    case ATCD_SB_PHONE + 5:
      if(atcd.phone.state != ATCD_PHONE_STATE_DIAL_W) return ATCD_SB_PHONE + 7;
      strcpy(atcd.at_cmd_buff, "ATD");
      strcat(atcd.at_cmd_buff, atcd.phone.number);
      strcat(atcd.at_cmd_buff, ";\r\n");

      atcd_atc_exec_cmd(&atcd.at_cmd, atcd.at_cmd_buff);
    case ATCD_SB_PHONE + 6:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_PHONE + 6;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_PHONE + 7;
      atcd.phone.state = ATCD_PHONE_STATE_DIAL;

    case ATCD_SB_PHONE + 7:
      //SMS...
    case ATCD_SB_PHONE + ATCD_SO_ERR:
    case ATCD_SB_PHONE + ATCD_SO_END:
      //------------------------------------------------------------------------
      // GPRS INIT
      //------------------------------------------------------------------------
    case ATCD_SB_GPRS_INIT:
      if(atcd.gprs.state != ATCD_GPRS_STATE_CONNECTING)
      {
        return ATCD_SB_GPRS_INIT + ATCD_SO_END;
      } 

      ATCD_DBG_GPRS_INIT_START
      atcd.at_cmd.timeout = 40000;
    case ATCD_SB_GPRS_INIT + 1:
      /*atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CGATT=0\r\n");
    case ATCD_SB_GPRS_INIT + 2:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_GPRS_INIT + 2;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_GPRS_INIT + ATCD_SO_ERR;*/
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CGATT=1\r\n");
    case ATCD_SB_GPRS_INIT + 3:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_GPRS_INIT + 3;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_GPRS_INIT + ATCD_SO_ERR;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CIPMUX=1\r\n");
    case ATCD_SB_GPRS_INIT + 4:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_GPRS_INIT + 4;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_GPRS_INIT + ATCD_SO_ERR;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CSTT=\"internet\",\"\",\"\"\r\n");
    case ATCD_SB_GPRS_INIT + 5:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_GPRS_INIT + 5;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_GPRS_INIT + ATCD_SO_ERR;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CGDCONT=1,\"IP\",\"internet\"\r\n");
    case ATCD_SB_GPRS_INIT + 6:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_GPRS_INIT + 6;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_GPRS_INIT + ATCD_SO_ERR;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CGACT=1,1\r\n");
    case ATCD_SB_GPRS_INIT + 7:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_GPRS_INIT + 7;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_GPRS_INIT + ATCD_SO_ERR;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CIICR\r\n");
    case ATCD_SB_GPRS_INIT + 8:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_GPRS_INIT + 8;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_GPRS_INIT + ATCD_SO_ERR;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CIFSR\r\n");
    case ATCD_SB_GPRS_INIT + 9:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_GPRS_INIT + 9;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_GPRS_INIT + ATCD_SO_ERR;
      ATCD_DBG_GPRS_INIT_OK
      //atcd.gprs.state = ATCD_GPRS_STATE_CONNECTING;
      // Opravdu? Neceka se asznc?
      atcd.gprs.state = ATCD_GPRS_STATE_CONN;
      return ATCD_SB_GPRS_INIT + ATCD_SO_END;

    case ATCD_SB_GPRS_INIT + ATCD_SO_ERR:
      //GPRS init selhalo
      //Zalogovat!
      atcd.gprs.state = ATCD_GPRS_STATE_DISCONN;
    case ATCD_SB_GPRS_INIT + ATCD_SO_END:
      //------------------------------------------------------------------------
      // GPRS DEINIT
      //------------------------------------------------------------------------
    case ATCD_SB_GPRS_DEINIT:
      if(atcd.gprs.state != ATCD_GPRS_STATE_DISCONNING)
      {
        return ATCD_SB_GPRS_DEINIT + ATCD_SO_END;
      } 

      ATCD_DBG_GPRS_DEINIT_START
      atcd.at_cmd.timeout = 40000;
    case ATCD_SB_GPRS_DEINIT + 1:
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CIPSHUT\r\n");
    case ATCD_SB_GPRS_DEINIT + 2:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_GPRS_DEINIT + 2;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_GPRS_DEINIT + ATCD_SO_ERR;
      ATCD_DBG_GPRS_DEINIT_OK
      atcd.gprs.state = ATCD_GPRS_STATE_DISCONN;
      return ATCD_SB_GPRS_DEINIT + ATCD_SO_END;

    case ATCD_SB_GPRS_DEINIT + ATCD_SO_ERR:
      //GPRS deinit selhalo
      //Zalogovat!
      atcd.gprs.state = ATCD_GPRS_STATE_DISCONN;

    case ATCD_SB_GPRS_DEINIT + ATCD_SO_END:
      //------------------------------------------------------------------------
      // CONN OPEN
      //------------------------------------------------------------------------
    case ATCD_SB_CONN_OPEN:
      if(atcd.conns.conn_num_proc >= ATCD_CONN_MAX_NUMBER)
      {
        atcd.conns.conn_num_proc = 0;
        return ATCD_SB_CONN_OPEN + ATCD_SO_END;
      }
      else
      {
        conn = atcd.conns.conn[atcd.conns.conn_num_proc];
        atcd.conns.conn_num_proc++;
      }

      if(conn == NULL || conn->state != ATCD_CONN_STATE_W_OPEN) return ATCD_SB_CONN_OPEN;

      ATCD_DBG_CONN_OPENING
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
        ATCD_DBG_CONN_PROT_ERR
        // Osetrit ze to nastalo - call back, stav spojeni...
        return ATCD_SB_CONN_OPEN + ATCD_SO_END;
      }
    case ATCD_SB_CONN_OPEN + 1:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_CONN_OPEN + 1;
      if(atcd.at_cmd.result == ATCD_ATC_RESULT_OK)
      {
        ATCD_DBG_CONN_W_CONN
        conn->state = ATCD_CONN_STATE_OPENING;
        conn->timer = atcd_get_ms();
        // asi volat call back pokud je nastaven?
      }
      else
      {
        ATCD_DBG_CONN_OPENING_ERR
        //atcd.conns.conn[conn->num]  = NULL;

        //atcd_conn_free(conn);
        //conn->state = ATCD_CONN_STATE_FAIL;
        // asi volat call back pokud je nastaven?
        return ATCD_SB_CONN_OPEN + ATCD_SO_END;
      }

      return ATCD_SB_CONN_OPEN;

    case ATCD_SB_CONN_OPEN + ATCD_SO_ERR:
      //Otevreni spojeni selhalo
      //Zalogovat!
      //bude volat call back od close, pozor...
      //opravdu to neni reduncance - kde vsude se vola, projit..
      atcd_conn_free(conn);
      conn->state = ATCD_CONN_STATE_FAIL;
      return ATCD_SB_CONN_OPEN;

    case ATCD_SB_CONN_OPEN + ATCD_SO_END:
      //------------------------------------------------------------------------
      // CONN WRITE
      //------------------------------------------------------------------------
    case ATCD_SB_CONN_WRITE:
      if(atcd.conns.conn_num_proc >= ATCD_CONN_MAX_NUMBER)
      {
        atcd.conns.conn_num_proc = 0;
        return ATCD_SB_CONN_WRITE + ATCD_SO_END;
      }
      else
      {
        conn = atcd.conns.conn[atcd.conns.conn_num_proc];
        atcd.conns.conn_num_proc++;
      }

      if(conn == NULL || conn->state != ATCD_CONN_STATE_OPEN) return ATCD_SB_CONN_WRITE;

      tx_data_len = rbuff_size(&conn->tx_rbuff);
      if(tx_data_len == 0) return ATCD_SB_CONN_WRITE;

      atcd.at_cmd.timeout = 30000;

      ATCD_DBG_CONN_SEND
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

    case ATCD_SB_CONN_WRITE + 1:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_CONN_WRITE + 1;
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
        ATCD_DBG_CONN_SEND_ERR
        //conn->state = ATCD_CONN_STATE_FAIL;

        // asi volat call back pokud je nastaven?
        return ATCD_SB_CONN_WRITE + ATCD_SO_ERR;
      }

    case ATCD_SB_CONN_WRITE + 2:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_CONN_WRITE + 2;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_CONN_WRITE + ATCD_SO_ERR;

      return ATCD_SB_CONN_WRITE;

    case ATCD_SB_CONN_WRITE + ATCD_SO_ERR:
      //Zapis do spojeni selhal
      //Zalogovat!
      return ATCD_SB_CONN_WRITE;

    case ATCD_SB_CONN_WRITE + ATCD_SO_END:
      //------------------------------------------------------------------------
      // CONN READ - Doplnit
      //------------------------------------------------------------------------
    case ATCD_SB_CONN_READ:
    case ATCD_SB_CONN_READ + ATCD_SO_ERR:
    case ATCD_SB_CONN_READ + ATCD_SO_END:
      //------------------------------------------------------------------------
      // CONN CLOSE
      //------------------------------------------------------------------------
    case ATCD_SB_CONN_CLOSE:
      if(atcd.conns.conn_num_proc >= ATCD_CONN_MAX_NUMBER)
      {
        atcd.conns.conn_num_proc = 0;
        return ATCD_SB_CONN_CLOSE + ATCD_SO_END;
      }
      else
      {
        conn = atcd.conns.conn[atcd.conns.conn_num_proc];
        atcd.conns.conn_num_proc++;
      }

      if(conn == NULL || conn->state != ATCD_CONN_STATE_W_CLOSE) return ATCD_SB_CONN_CLOSE;

      ATCD_DBG_CONN_CLOSING
      //conn->state = ATCD_CONN_STATE_OPENING;
      atcd.at_cmd.timeout = 15000;
      atcd.at_cmd.cmd = atcd.at_cmd_buff;

      sprintf(atcd.at_cmd.cmd, "AT+CIPCLOSE=%u\r\n", conn->num);
      atcd_atc_exec(&atcd.at_cmd);

    case ATCD_SB_CONN_CLOSE + 1:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_CONN_CLOSE + 1;
      if(atcd.at_cmd.result == ATCD_ATC_RESULT_OK)
      {
        ATCD_DBG_CONN_W_CLOSE
        conn->state = ATCD_CONN_STATE_CLOSING;
        conn->timer = atcd_get_ms();
        // asi volat call back pokud je nastaven?
      }
      else
      {
        ATCD_DBG_CONN_CLOSING_ERR
        //atcd.conns.conn[conn->num]  = NULL;

        //atcd_conn_free(conn);
        //conn->state = ATCD_CONN_STATE_FAIL;
        // asi volat call back pokud je nastaven?
        return ATCD_SB_CONN_CLOSE + ATCD_SO_ERR;
      }
      return ATCD_SB_CONN_CLOSE;

    case ATCD_SB_CONN_CLOSE + ATCD_SO_ERR:
      //Uzavreni spojeni selhalo
      //Zalogovat!
      //bude volat call back od close, pozor...
      //opravdu to neni reduncance - kde vsude se vola, projit..
      atcd_conn_free(conn);
      conn->state = ATCD_CONN_STATE_FAIL;
      return ATCD_SB_CONN_CLOSE;

    case ATCD_SB_CONN_CLOSE + ATCD_SO_END:
    //------------------------------------------------------------------------
    // GPS START
    //------------------------------------------------------------------------
    case ATCD_SB_GPS_START:
      if(atcd.gps.state != ATCD_GPS_STATE_W_SEARCH)
      {
        return ATCD_SB_GPS_START + ATCD_SO_END;
      }

      ATCD_DBG_GPS_ENABLING
      atcd.at_cmd.timeout = 5000;
    case ATCD_SB_GPS_START + 1:
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+GPSRD=1\r\n");
    case ATCD_SB_GPS_START + 2:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_GPS_START + 2;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_GPS_START + ATCD_SO_ERR;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+GPS=1\r\n");
    case ATCD_SB_GPS_START + 3:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_GPS_START + 3;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_GPS_START + ATCD_SO_ERR;
      ATCD_DBG_GPS_ENABLED
      atcd.gps.state = ATCD_GPS_STATE_SEARCHING;
      return ATCD_SB_GPS_START + ATCD_SO_END;

    case ATCD_SB_GPS_START + ATCD_SO_ERR:
      //GPS start selhalo
      //Zalogovat!

    case ATCD_SB_GPS_START + ATCD_SO_END:
    //------------------------------------------------------------------------
    // GPS STOP
    //------------------------------------------------------------------------
    case ATCD_SB_GPS_STOP:
      if(atcd.gps.state != ATCD_GPS_STATE_W_OFF)
      {
        return ATCD_SB_GPS_STOP + ATCD_SO_END;
      }

      ATCD_DBG_GPS_DIABLING
      atcd.at_cmd.timeout = 5000;
    case ATCD_SB_GPS_STOP + 1:
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+GPS=0\r\n");
    case ATCD_SB_GPS_STOP + 2:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_GPS_STOP + 2;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_GPS_STOP + ATCD_SO_ERR;
      ATCD_DBG_GPS_DIABLED
      atcd.gps.state = ATCD_GPS_STATE_OFF;
      return ATCD_SB_GPS_STOP + ATCD_SO_END;

    case ATCD_SB_GPS_STOP + ATCD_SO_ERR:
      //GPS disabling fail
      //Zalogovat!

    case ATCD_SB_GPS_STOP + ATCD_SO_END:
      //------------------------------------------------------------------------
      // END
      //------------------------------------------------------------------------
    case ATCD_SB_END:
      //Konec, navrat na pocatek...
      return ATCD_SB_STAT;
      //------------------------------------------------------------------------
    default:
      //Chyba, logovat
      ATCD_DBG_SW_ERR
      return ATCD_SB_INIT;
  }
}
//------------------------------------------------------------------------------
void atcd_sw_reset()            //SW reset
{
    uint16_t len;

    //ATCD_DBG_ATC_SEND_CMD
    //atcd.parser.at_cmd_top->state = ATCD_ATC_STATE_TX;
    //atcd.parser.tx_state = ATCD_P_TX_ONGOING;
    //atcd.parser.tx_state = ATCD_P_TX_COMPLETE;

    len = strlen("AT+RST=1\r\n");
    rbuff_lin_space(&atcd.parser.tx_rbuff, (uint8_t*)"AT+RST=1\r\n", len);

    //atcd.parser.timer = atcd_get_ms();
    atcd_hw_tx(&atcd.parser.tx_rbuff, len);
}
//------------------------------------------------------------------------------
#endif /* ATCD_A7 */
//------------------------------------------------------------------------------
