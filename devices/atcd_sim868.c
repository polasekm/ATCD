/*
 * usart.c
 *
 *  Created on: Apr 9, 2011
 *      Author: Martin
 */
//------------------------------------------------------------------------------
#include "../atcd.h"

//------------------------------------------------------------------------------
#if(ATCD_USE_DEVICE == ATCD_SIM868)
//------------------------------------------------------------------------------
extern atcd_t atcd;
uint32_t init_time_inner;
uint32_t init_time_outer;
atcd_at_cmd_t at_cmd2; //kdyz mam data k odesilani, pouziju jiny at_cmd aby tam ty data nezustaly pro nesouvisejici prikaz
rbuff_t at_rbuff2; //tak to je slepici ulet

struct  //je potreba vsechny chyby pocitat zvlast, aby jedna neresetovala druhou
 {
  int cpas2;
 } fails_after_init={cpas2: 0};
//------------------------------------------------------------------------------
uint16_t atcd_proc_step()
{
  uint32_t tx_data_len;
  static atcd_conn_t *conn;

  switch(atcd.proc_step)
  {
      //-------------------------------
      // Init cast
      //-------------------------------
    case ATCD_SB_INIT:
      // Zahajime inicializacni sekvenci
      ATCD_DBG_INIT_START

      //Upravit, je treba nastavit buffery
      atcd.at_cmd.resp           = NULL;
      atcd.at_cmd.resp_len       = 0;
      atcd.at_cmd.resp_buff_size = 0;

      atcd.at_cmd.data           = NULL;
      atcd.at_cmd.data_len       = 0;

      atcd_atc_init(&at_cmd2);

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
      atcd_atc_exec_cmd(&atcd.at_cmd, "ATI\r\n");         //
    case ATCD_SB_INIT + 4:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_INIT + 4;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_INIT + ATCD_SO_ERR;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CMEE=1\r\n");    // Rezim vypisu chybovych hlaseni +CME
    case ATCD_SB_INIT + 5:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_INIT + 5;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_INIT + ATCD_SO_ERR;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CFUN=1\r\n");    // Plna fce zarizeni
    case ATCD_SB_INIT + 6:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_INIT + 6;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_INIT + ATCD_SO_ERR;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CPIN?\r\n");    // Je vyzadovan PIN?
    case ATCD_SB_INIT + 7:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_INIT + 7;
      if ((atcd.at_cmd.result==ATCD_ATC_RESULT_ERROR) && (atcd.at_cmd.resultcode==10))
      {
        init_time_inner=atcd_get_ms();
        return ATCD_SB_INIT + 90;
      };
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_INIT + ATCD_SO_ERR;
      if(atcd.at_cmd.resp_len != 0 && strncmp(atcd.at_cmd.resp, ATCD_STR_SIM_READY, strlen(ATCD_STR_SIM_READY)) == 0)
      {
        ATCD_DBG_PIN_NONE
        atcd.sim.state = ATCD_SIM_STATE_OK;
      }
      else if(atcd.at_cmd.resp_len != 0 && strncmp(atcd.at_cmd.resp, ATCD_STR_SIM_PIN, strlen(ATCD_STR_SIM_PIN)) == 0)
      {
        ATCD_DBG_PIN_REQ
        atcd.sim.state = ATCD_SIM_STATE_PIN;

        strcpy(atcd.at_cmd_buff, "AT+CPIN=\"");
        strcat(atcd.at_cmd_buff, atcd.sim.pin);
        strcat(atcd.at_cmd_buff, "\"\r\n");

        atcd_atc_exec_cmd(&atcd.at_cmd, atcd.at_cmd_buff);  // Zadame PIN          // Zadame PIN
      }
      else if(atcd.at_cmd.resp_len != 0 && strncmp(atcd.at_cmd.resp, ATCD_STR_SIM_PUK, strlen(ATCD_STR_SIM_PUK)) == 0)
      {
        ATCD_DBG_PUK_REQ
        atcd.sim.state = ATCD_SIM_STATE_PUK;
        //Bude to chtit PUK
        //Co delat?
        //atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CPIN=\"1234\\r\n");            // Zadame PUK
      }
      else
      {
        ATCD_DBG_PIN_ERR
        atcd.sim.state = ATCD_SIM_STATE_ERROR;
        return ATCD_SB_INIT + ATCD_SO_ERR;
      }      
    case ATCD_SB_INIT + 8:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_INIT + 8;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_INIT + ATCD_SO_ERR;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CLIP=1\r\n");           // Zobrazovat cislo volajiciho
    case ATCD_SB_INIT + 9:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_INIT + 9;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_INIT + ATCD_SO_ERR;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CMGF=1\r\n");           // Textovy rezim SMS
    case ATCD_SB_INIT + 10:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_INIT + 10;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_INIT + ATCD_SO_ERR;
      init_time_inner=atcd_get_ms();
      init_time_outer=init_time_inner;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CPMS?\r\n");//system nerozlisi skutecny OK od _res , "SMS Ready\r\n"); // --- Test vyuziti pametovych prostoru na SMS
    case ATCD_SB_INIT + 11:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_INIT + 11;
      if ((atcd.at_cmd.result==ATCD_ATC_RESULT_ERROR) && (atcd.at_cmd.resultcode==302))
      {
        //mozna cekat na SMS Ready\r\n ? V tomhle systemu se unso nedelaji tak snadno
        //a predtim taky prijde Call Ready\r\n, mozna proste mit priznak ze prisly?
        if (atcd_get_ms()-init_time_outer>=12000)
          return ATCD_SB_INIT + ATCD_SO_ERR;
        if (atcd_get_ms()-init_time_inner>=500)
        {
          init_time_inner=atcd_get_ms();
          atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CPMS?\r\n");           // --- Test vyuziti pametovych prostoru na SMS
        }
        return ATCD_SB_INIT + 11;
      }
      else if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_INIT + ATCD_SO_ERR;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CMGD=1,4\r\n");           // Smaze vsechny SMS na karte
    case ATCD_SB_INIT + 12:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_INIT + 12;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_INIT + ATCD_SO_ERR;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CSDH=1\r\n");           //
    case ATCD_SB_INIT + 13:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_INIT + 13;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_INIT + ATCD_SO_ERR;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CNMI=1,2,0,0,0\r\n");          // Rezim nakladani s novymi SMS
    case ATCD_SB_INIT + 15:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_INIT + 15;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_INIT + ATCD_SO_ERR;

      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CRSL=0\r\n");
    case ATCD_SB_INIT + 16:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_INIT + 16;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_INIT + ATCD_SO_ERR;

      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CLVL=60\r\n");
    case ATCD_SB_INIT + 17:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_INIT + 17;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_INIT + ATCD_SO_ERR;

      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+ECHO=0,40000,40000,6200,6200,1\r\n");
    case ATCD_SB_INIT + 20:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_INIT + 20;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_INIT + ATCD_SO_ERR;

      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CMIC=0,12\r\n");
    case ATCD_SB_INIT + 21:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_INIT + 21;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_INIT + ATCD_SO_ERR;

      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+SIDET=0,0\r\n");
    case ATCD_SB_INIT + 22:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_INIT + 22;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_INIT + ATCD_SO_ERR;

      // Inicializace byla dokoncena
      ATCD_DBG_INIT_DONE
      // Inicializace je dokoncena
      ATCD_DBG_INIT_OK
      // Zmenime stav zarizeni
      atcd.state = ATCD_STATE_ON;
      atcd.err_cnt = 0;
      return ATCD_SB_INIT + ATCD_SO_END;

    case ATCD_SB_INIT + 90:
      if (atcd_get_ms()-init_time_inner<5000) return ATCD_SB_INIT + 90;
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
      if(atcd_get_ms() - atcd.timer < 20000)
      {
        return ATCD_SB_STAT + ATCD_SO_END;
      }
      // Je cast spustit kontrolu stavu modemu
      ATCD_DBG_STAT_START
      atcd.timer = atcd_get_ms();
      atcd.at_cmd.timeout = 5000;
    case ATCD_SB_STAT + 1:

      atcd_atc_exec_cmd(&atcd.at_cmd, "AT\r\n");           // Test response
    case ATCD_SB_STAT + 2:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_STAT + 2;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_STAT + ATCD_SO_ERR;


      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CREG?\r\n");    // Network registration status
    case ATCD_SB_STAT + 3:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_STAT + 3;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_STAT + ATCD_SO_ERR;

      //Zpracovat stav registrace...

      // Doplnit sleepmode...

      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CGATT?\r\n");    // stav PDP kontextu
    case ATCD_SB_STAT + 4:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_STAT + 4;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_STAT + ATCD_SO_ERR;

      /*atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CIFSR\r\n");     // Check IP address
    case ATCD_SB_STAT + 5:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_STAT + 5;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_STAT + ATCD_SO_ERR;
*/
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CIPSTATUS\r\n");  // TCP/UDP connections status
    case ATCD_SB_STAT + 6:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_STAT + 6;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_STAT + ATCD_SO_ERR;

      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CSQ\r\n");      // Signal quality
    case ATCD_SB_STAT + 7:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_STAT + 7;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_STAT + ATCD_SO_ERR;
      if (atcd.at_cmd.resp_len>=8) //+CSQ:9\r\n i kdyz spis +CSQ: 9,0\r\n
      { //diky cekani na echo sem nechodi STATE: IP INITIAL aspol.
        if (strncmp(atcd.at_cmd.resp, "+CSQ:", 5)==0)
        {
          int csq=atoi(atcd.at_cmd.resp+5);
          if ((csq<0) || (csq>=32)) //hlavne 99
            atcd.gsm.gsm_sig=-1;
          else
            atcd.gsm.gsm_sig=csq*100/31;
        };
      };

      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CPAS\r\n");
    case ATCD_SB_STAT + 10:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_STAT + 10;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_STAT + ATCD_SO_ERR;

      {
        if ((atcd.at_cmd.resp_len>=10) && (strncmp(atcd.at_cmd.resp, "+CPAS: ", 7)==0))
        {
          int cpas=atoi(atcd.at_cmd.resp+strlen("+CPAS:"));
          int doit=-1;
          switch (atcd.phone.state)
          {
          case ATCD_PHONE_STATE_IDLE:
            if ((cpas==3) || (cpas==4))
              doit=cpas;
            break;
          case ATCD_PHONE_STATE_RING:
          case ATCD_PHONE_STATE_RING_WA:
            if ((cpas==0) || (cpas==4))
              doit=cpas;
            break;
          case ATCD_PHONE_STATE_CALL:
          case ATCD_PHONE_STATE_HANG_W:
            if ((cpas==0) || (cpas==3))
              doit=cpas; //call->ring je hodne zvlastni, mozna druhy hovor
            break;
          case ATCD_PHONE_STATE_DIAL:
          case ATCD_PHONE_STATE_DIAL_W: //TODO: je otazka jestli chceme prijit o DIAL_W
            if ((cpas==3) || (cpas==4))
              doit=cpas;
            break;
          }

          if (doit>=0)
            atcd_dbg_warn("@atcd", "FIX phone.state");
          switch (doit)
          {
          case 0: atcd.phone.state=ATCD_PHONE_STATE_IDLE; break;
          case 3: atcd.phone.state=ATCD_PHONE_STATE_RING; break;
          case 4: atcd.phone.state=ATCD_PHONE_STATE_CALL; break;
          }

          if (cpas==2)
          {
            atcd_dbg_err("fail_ai", "+CPAS: 2");
            fails_after_init.cpas2++;
            if (fails_after_init.cpas2>=5)
            {
              fails_after_init.cpas2=0;
              atcd_reset(); //reset hw ale i state a vubec
            };
          };
        }
      }

      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CMIC?\r\n");
    case ATCD_SB_STAT + 11:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_STAT + 11;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_STAT + ATCD_SO_ERR;

      //CGREG, IP ADRESA....

      //Zpracovat stav PDP kontextu

      // Pravidelne kolecko bylo dokonceno
      // Dotazovani na stav je hotovo
      if (atcd.state_update_callback)
        atcd.state_update_callback();
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
      atcd.phone.numbertype=-1;

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
      if(atcd.phone.sms_tx.state != ATCD_PHONE_SMS_STATE_SEND_W) return ATCD_SB_PHONE + 9;

      strcpy(atcd.at_cmd_buff, "AT+CMGS=\"");
      strcat(atcd.at_cmd_buff, atcd.phone.sms_tx.sender);
      strcat(atcd.at_cmd_buff, "\",145\r");

      at_cmd2.timeout=60000;
      rbuff_lin_space(&at_rbuff2, (uint8_t *)atcd.phone.sms_tx.message, atcd.phone.sms_tx.len); //const message nevyresis
      at_cmd2.data = &at_rbuff2;//to nepujde atcd.phone.sms_tx.message;
      at_cmd2.data_len = atcd.phone.sms_tx.len;

      atcd_atc_exec_cmd(&at_cmd2, atcd.at_cmd_buff);
    case ATCD_SB_PHONE + 8:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_PHONE + 8;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK)
      {
        atcd.phone.sms_tx.state = ATCD_PHONE_SMS_STATE_ERROR;
        return ATCD_SB_PHONE + 9;
      }

      atcd.phone.sms_tx.state = ATCD_PHONE_SMS_STATE_SEND;

    case ATCD_SB_PHONE + 9:

      //SMS...
    case ATCD_SB_PHONE + ATCD_SO_ERR:
    case ATCD_SB_PHONE + ATCD_SO_END:

    case ATCD_SB_POWERSAVE:
    {
      char *send=NULL; //const nejde protoze atcd_atc_exec_cmd(..., ne const)

      atcd_powersave_req_t req_tmp=atcd.powersave_req;
      switch (atcd_gps_state())
      {
      case ATCD_GPS_STATE_SEARCHING:
      case ATCD_GPS_STATE_W_SEARCH:
      case ATCD_GPS_STATE_FIX:
        req_tmp=atcd_pwrsFull;
        break;
      }

      switch (req_tmp)
      {
      case atcd_pwrsFull:
        if (atcd.powersave_act!=0)
        {
          send="AT+CSCLK=0\r";
          atcd.powersave_otw=0;
        }
        break;
      case atcd_pwrsSave:
        if (atcd.powersave_act!=1)
        {
          send="AT+CSCLK=1\r";
          atcd.powersave_otw=1;
        }
        break;
      }

      if (!send)
        return ATCD_SB_POWERSAVE + ATCD_SO_END;
      atcd_atc_exec_cmd(&atcd.at_cmd, send);   //sleep mode modemu
    }
    case ATCD_SB_POWERSAVE + 1:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_POWERSAVE + 1;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_POWERSAVE + ATCD_SO_ERR;

      atcd.powersave_act = atcd.powersave_otw;

    //case ATCD_SB_POWERSAVE + 90:
    //  if (atcd_get_ms()-init_time_inner<500) return ATCD_SB_POWERSAVE + 90;

    case ATCD_SB_POWERSAVE + ATCD_SO_ERR:
    case ATCD_SB_POWERSAVE + ATCD_SO_END:
      //------------------------------------------------------------------------
      // GPRS INIT
      //------------------------------------------------------------------------
    case ATCD_SB_GPRS_INIT:
      if(atcd.gprs.state != ATCD_GPRS_STATE_CONNECTING)
      {
        return ATCD_SB_GPRS_INIT + ATCD_SO_END;
      }

      ATCD_DBG_GPRS_INIT_START

    case ATCD_SB_GPRS_INIT + 1:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_GPRS_INIT + 1;
      if (atcd.phone.state!=ATCD_PHONE_STATE_IDLE)
        return ATCD_SB_GPRS_INIT + ATCD_SO_END;

      atcd.at_cmd.timeout = 40000;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CIPSHUT\r\n");
    case ATCD_SB_GPRS_INIT + 4:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_GPRS_INIT + 4;
      if ((atcd.at_cmd.result == ATCD_ATC_RESULT_ERROR)) //opravdu nepotrebuji posilat 50x/s kdyz nekdo vytahne SIMku
      {
        init_time_inner=atcd_get_ms();
        return ATCD_SB_GPRS_INIT + 90;
      };
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_GPRS_INIT + ATCD_SO_ERR;
      if (atcd.phone.state!=ATCD_PHONE_STATE_IDLE)
        return ATCD_SB_GPRS_INIT + ATCD_SO_END;

      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CGATT=0\r\n");
    case ATCD_SB_GPRS_INIT + 5:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_GPRS_INIT + 5;
      if ((atcd.at_cmd.result == ATCD_ATC_RESULT_ERROR) &&
          (atcd.at_cmd.resultcode==4))
      {
        init_time_inner=atcd_get_ms();
        return ATCD_SB_GPRS_INIT + 90;
      };
      if (atcd.phone.state!=ATCD_PHONE_STATE_IDLE)
        return ATCD_SB_GPRS_INIT + ATCD_SO_END;

      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_GPRS_INIT + ATCD_SO_ERR;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CIPMUX=1\r\n");
    case ATCD_SB_GPRS_INIT + 6:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_GPRS_INIT + 6;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_GPRS_INIT + ATCD_SO_ERR;
      if (atcd.phone.state!=ATCD_PHONE_STATE_IDLE)
        return ATCD_SB_GPRS_INIT + ATCD_SO_END;

      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CGATT=1\r\n");
    case ATCD_SB_GPRS_INIT + 7:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_GPRS_INIT + 7;
      if ((atcd.at_cmd.result == ATCD_ATC_RESULT_ERROR) &&
          ((atcd.at_cmd.resultcode==4) || (atcd.at_cmd.resultcode==100)))
      {
        init_time_inner=atcd_get_ms();
        return ATCD_SB_GPRS_INIT + 90;
      };
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_GPRS_INIT + ATCD_SO_ERR;
      if (atcd.phone.state!=ATCD_PHONE_STATE_IDLE)
        return ATCD_SB_GPRS_INIT + ATCD_SO_END;

      strcpy(atcd.at_cmd_buff, "AT+CSTT=\"");
      strcat(atcd.at_cmd_buff, atcd.gprs.apn);
      strcat(atcd.at_cmd_buff, "\",\"");
      strcat(atcd.at_cmd_buff, atcd.gprs.name);
      strcat(atcd.at_cmd_buff, "\",\"");
      strcat(atcd.at_cmd_buff, atcd.gprs.psswd);
      strcat(atcd.at_cmd_buff, "\"\r\n");

      atcd_atc_exec_cmd(&atcd.at_cmd, atcd.at_cmd_buff);

    case ATCD_SB_GPRS_INIT + 8:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_GPRS_INIT + 8;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_GPRS_INIT + ATCD_SO_ERR;
      //podle doc az 85s ale prakticky pod 1s; jednou odpoved neprisla vubec ani OK ani ERR ani nic
      if (atcd.phone.state!=ATCD_PHONE_STATE_IDLE)
        return ATCD_SB_GPRS_INIT + ATCD_SO_END;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CIICR\r\n");
    case ATCD_SB_GPRS_INIT + 9:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_GPRS_INIT + 9;
      if (atcd.at_cmd.result==ATCD_ATC_RESULT_ERROR)
      {
        if(strcmp(atcd.at_cmd.resp, "+PDP: DEACT\r\n")==0)
        {
          init_time_inner=atcd_get_ms();
          return ATCD_SB_GPRS_INIT + 89;
        };
      };
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_GPRS_INIT + ATCD_SO_ERR;
      if (atcd.phone.state!=ATCD_PHONE_STATE_IDLE)
        return ATCD_SB_GPRS_INIT + ATCD_SO_END;

      atcd.at_cmd.timeout = 1500;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CIFSR\r\n");
    case ATCD_SB_GPRS_INIT + 10:
      if(atcd.at_cmd.resp_len > 0)
      {
        size_t iplen;
        atcd.at_cmd.resp[atcd.at_cmd.resp_len] = '\0';
        iplen = strspn(atcd.at_cmd.resp, "1234567890.");
        if(iplen >= 7)
          if(strcmp(atcd.at_cmd.resp + iplen, "\r\n") == 0)
          {
            atcd.at_cmd.result = ATCD_ATC_RESULT_OK;
            atcd_atc_complete(&atcd.at_cmd);

            strncpy(atcd.gprs.ip, atcd.at_cmd.resp, 15);
            atcd.gprs.ip[15] = 0;
          }
      }

      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_GPRS_INIT + 10;
      //if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_GPRS_INIT + ATCD_SO_ERR;

      ATCD_DBG_GPRS_INIT_OK
      //atcd.gprs.state = ATCD_GPRS_STATE_CONNECTING;
      // Opravdu? Neceka se async?
      atcd.gprs.state = ATCD_GPRS_STATE_CONN;
      atcd_gprs_autoconn(); //kdyz jsme chteli disco behem initu tak se pozadavek zahodil
      return ATCD_SB_GPRS_INIT + ATCD_SO_END;

    case ATCD_SB_GPRS_INIT + 89: //nebyl by lepsi priznak "skip_all_gprs +_time" ? takhle zastavim vsechno
      if (atcd_get_ms()-init_time_inner<5000) return ATCD_SB_GPRS_INIT + 89;
    case ATCD_SB_GPRS_INIT + 90: //nebyl by lepsi priznak "skip_all_gprs +_time" ? takhle zastavim vsechno
      if (atcd_get_ms()-init_time_inner<500) return ATCD_SB_GPRS_INIT + 90;

    case ATCD_SB_GPRS_INIT + ATCD_SO_ERR:
      //GPRS init selhalo
      ATCD_DBG_GPRS_INIT_ERR
      //atcd.gprs.state = ATCD_GPRS_STATE_DISCONN;
      atcd_gprs_disconnect();

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

    case ATCD_SB_GPRS_DEINIT + 1:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_GPRS_DEINIT + 1;
      if (atcd.phone.state!=ATCD_PHONE_STATE_IDLE)
        return ATCD_SB_GPRS_DEINIT + ATCD_SO_END;

      atcd.at_cmd.timeout = 40000; //tesne po skonceni hovoru trvalo CIPSHUT 9s
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CIPSHUT\r\n");
    case ATCD_SB_GPRS_DEINIT + 2:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_GPRS_DEINIT + 2;
      if ((atcd.at_cmd.result == ATCD_ATC_RESULT_ERROR)) //opravdu nepotrebuji posilat 50x/s kdyz nekdo vytahne SIMku
      {
        init_time_inner=atcd_get_ms();
        return ATCD_SB_GPRS_DEINIT + 90;
      };
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_GPRS_DEINIT + ATCD_SO_ERR;
      if (atcd.phone.state!=ATCD_PHONE_STATE_IDLE)
        return ATCD_SB_GPRS_DEINIT + ATCD_SO_END;

      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CGATT=0\r\n");
    case ATCD_SB_GPRS_DEINIT + 3:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_GPRS_DEINIT + 3;
      if ((atcd.at_cmd.result == ATCD_ATC_RESULT_ERROR) &&
          (atcd.at_cmd.resultcode==4))
      {
        init_time_inner=atcd_get_ms();
        return ATCD_SB_GPRS_DEINIT + 90;
      };
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_GPRS_DEINIT + ATCD_SO_ERR;

      ATCD_DBG_GPRS_DEINIT_OK
      atcd.gprs.state = ATCD_GPRS_STATE_DISCONN;
      atcd_gprs_autoconn(); //kdyz jsme chteli conn behem deinitu tak se pozadavek zahodil
      return ATCD_SB_GPRS_DEINIT + ATCD_SO_END;

    case ATCD_SB_GPRS_DEINIT + 90: //nebyl by lepsi priznak "skip_all_gprs +_time" ? takhle zastavim vsechno
      if (atcd_get_ms()-init_time_inner<500) return ATCD_SB_GPRS_DEINIT + 90;

    case ATCD_SB_GPRS_DEINIT + ATCD_SO_ERR:
      //GPRS deinit selhalo
      ATCD_DBG_GPRS_DEINIT_ERR
      atcd.gprs.state = ATCD_GPRS_STATE_DISCONN;
      atcd.gprs.ip[0] = 0;
      atcd_gprs_autoconn();

    case ATCD_SB_GPRS_DEINIT + ATCD_SO_END:
      //------------------------------------------------------------------------
      // CONN OPEN
      //------------------------------------------------------------------------
    case ATCD_SB_CONN_OPEN:
      if (atcd.phone.state!=ATCD_PHONE_STATE_IDLE)
        return ATCD_SB_CONN_OPEN + ATCD_SO_END;

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

      if (conn == NULL) return ATCD_SB_CONN_OPEN;
      if ((conn->state != ATCD_CONN_STATE_W_OPEN1) &&
    	  ((conn->state != ATCD_CONN_STATE_W_OPENFAILED) ||
    	   (atcd_get_ms()-conn->timer<=500))) return ATCD_SB_CONN_OPEN;

      ATCD_DBG_CONN_OPENING
      //conn->state = ATCD_CONN_STATE_OPENING;
      atcd.at_cmd.timeout = 15000;
      atcd.at_cmd.cmd = atcd.at_cmd_buff;

      /*
       * pro SSL staci zapnout AT+CIPSSL=1 tesne pred otevrenim spojeni
       * nebo AT+CIPSSL=0 pro TCP
       * certifikaty nejak pomoci AT+FSCREATE, AT+FSWRITE, AT+SSLSETROOT aspol.
       * nezkousel jsem to
       */
      if(conn->protocol == ATCD_CONN_T_TCP)
      {
        snprintf(atcd.at_cmd.cmd, 64, "AT+CIPSTART=%u,\"TCP\",\"%s\",%u\r\n", conn->num, conn->host, conn->port);
        atcd_atc_exec(&atcd.at_cmd);
      }
      else if(conn->protocol == ATCD_CONN_T_UDP)
      {
        snprintf(atcd.at_cmd.cmd, 64, "AT+CIPSTART=%u,\"UDP\",\"%s\",%u\r\n", conn->num, conn->host, conn->port);
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
        if ((atcd.at_cmd.result==ATCD_ATC_RESULT_ERROR) &&
            (atcd.at_cmd.resultcode==3))
        { //mohlo se podelat GPRS, udelej reinit
          atcd_gprs_disconnect(); //po dokonceni disco by se mel udelat autoconnect
        };
        //atcd_conn_free(conn);
        conn->state = ATCD_CONN_STATE_W_OPENFAILED;
        conn->timer = atcd_get_ms();
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

      at_cmd2.timeout = 30000;

      ATCD_DBG_CONN_SEND
      //conn->state = ATCD_CONN_STATE_OPENING;

      if(tx_data_len > 512) tx_data_len = 512;

      at_cmd2.cmd = atcd.at_cmd_buff;
      at_cmd2.timeout = 30000;
      at_cmd2.data = &conn->tx_rbuff;
      at_cmd2.data_len = tx_data_len;

      sprintf(at_cmd2.cmd, "AT+CIPSEND=%u,%u\r\n", conn->num, (unsigned int)tx_data_len);

      //je to jeste potreba?
      atcd.parser.tx_conn_num = conn->num;

      atcd_atc_exec(&at_cmd2);

      // ze by se mel nastavit stav parseru na conn tx pending
      // to ovsem nejde - musi se provest az ke konkretnimu atc
      // muze byt totiz nekde hloubeji ve fronte
      // vymyslet jak s tim

      // provedeni testovat
      //0, SEND OK

    case ATCD_SB_CONN_WRITE + 1:
      if(at_cmd2.state != ATCD_ATC_STATE_DONE) return ATCD_SB_CONN_WRITE + 1;
      if(at_cmd2.result == ATCD_ATC_RESULT_OK)
      {
        //necekat na vyzvu az tady
        //nemusi byt, u A6 je vyzva jeste v tele AT prikazu
        //vetsinou v tele prijde i +IPD

        //
        //ne ne tady uz jsou data zadana

        //posunout ukazovatko dat
        rbuff_seek(&conn->tx_rbuff, at_cmd2.data_len);

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
      //at_cmd nebo at_cmd2? Nedela nic tak dam return
      return ATCD_SB_CONN_WRITE;
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
      if (atcd.phone.state!=ATCD_PHONE_STATE_IDLE)
        return ATCD_SB_CONN_CLOSE + ATCD_SO_END;
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

      snprintf(atcd.at_cmd_buff, sizeof(atcd.at_cmd_buff), "AT+CIPCLOSE=%u\r\n", conn->num);
      snprintf(atcd.at_cmd_resbuff, sizeof(atcd.at_cmd_resbuff), "%u, CLOSE OK\r\n", conn->num);
      //ja se zabiju, , CLOSE OK\r\n se odchytava
      atcd_atc_exec_cmd_res(&atcd.at_cmd, atcd.at_cmd_buff, atcd.at_cmd_resbuff);

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
      //opravdu to neni redundance - kde vsude se vola, projit..
      atcd_conn_free(conn);
      conn->state = ATCD_CONN_STATE_FAIL;
      return ATCD_SB_CONN_CLOSE;

    case ATCD_SB_CONN_CLOSE + ATCD_SO_END:
    //-------------------------------
    // GPS START
    //-------------------------------
    case ATCD_SB_GPS_START:
      if(atcd.gps.state != ATCD_GPS_STATE_W_SEARCH)
      {
        return ATCD_SB_GPS_START + ATCD_SO_END;
      }

      ATCD_DBG_GPS_ENABLING
      atcd.at_cmd.timeout = 5000;
    case ATCD_SB_GPS_START + 1:
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CGNSTST=1\r\n");
    case ATCD_SB_GPS_START + 2:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_GPS_START + 2;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_GPS_START + ATCD_SO_ERR;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CGNSPWR=1\r\n");
    case ATCD_SB_GPS_START + 3:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_GPS_START + 3;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_GPS_START + ATCD_SO_ERR;
      ATCD_DBG_GPS_ENABLED
      atcd.gps.state = ATCD_GPS_STATE_SEARCHING;
      atcd.gps.start_time = atcd_get_ms();
      return ATCD_SB_GPS_START + ATCD_SO_END;

     case ATCD_SB_GPS_START + ATCD_SO_ERR:
      //GPS start selhalo
      //Zalogovat!

    case ATCD_SB_GPS_START + ATCD_SO_END:
      //-------------------------------
      // GPS STOP
      //-------------------------------
    case ATCD_SB_GPS_STOP:
      if(atcd.gps.state != ATCD_GPS_STATE_W_OFF)
      {
        return ATCD_SB_GPS_STOP + ATCD_SO_END;
      }

      ATCD_DBG_GPS_DIABLING
      atcd.at_cmd.timeout = 5000;
    case ATCD_SB_GPS_STOP + 1:
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CGNSPWR=0\r\n");
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

    len = strlen("AT+CFUN=1,1\r\n");
    rbuff_lin_space(&atcd.parser.tx_rbuff, (uint8_t*)"AT+CFUN=1,1\r\n", len);

    //atcd.parser.timer = atcd_get_ms();
    atcd_hw_tx(&atcd.parser.tx_rbuff, len);
}
//------------------------------------------------------------------------------
#endif /* ATCD_SIM868 */
//------------------------------------------------------------------------------
