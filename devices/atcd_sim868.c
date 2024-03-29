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
uint32_t step_time;

uint16_t expecting_binary=0;

struct  //je potreba vsechny chyby pocitat zvlast, aby jedna neresetovala druhou
{
  int cpas2;

} fails_after_init = {cpas2: 0};

//------------------------------------------------------------------------------
uint16_t atcd_proc_step()
{
  uint32_t tx_data_len;
  static atcd_conn_t *conn;    //pozor, tohle nebude delat dobrotu na vice instancich...

  switch(atcd.proc_step)
  {
      //-------------------------------
      // Init cast
      //-------------------------------
    case ATCD_SB_INIT:
      // Zahajime inicializacni sekvenci
      ATCD_DBG_INIT_START

      //Upravit, je treba nastavit buffery
      /*atcd.at_cmd.resp           = NULL;
      atcd.at_cmd.resp_len       = 0;
      atcd.at_cmd.resp_buff_size = 0;

      atcd.at_cmd.data           = NULL;
      atcd.at_cmd.data_len       = 0;

      atcd.at_cmd.timeout        = 5000;*/
      atcd_atc_set_defaults(&atcd.at_cmd);
      init_time_inner=atcd_get_ms();

    case ATCD_SB_INIT + 1:
      if (atcd_get_ms()-init_time_inner<500) return ATCD_SB_INIT + 1; //kdyz prijde RDY a mam rozdelany prikaz, tak me nejak neslysi nebo to neposlu nebo nevim
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

      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CFUN?\r");
    case ATCD_SB_INIT + 6:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_INIT + 6;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_INIT + ATCD_SO_ERR;
      if (strncmp(atcd.at_cmd.resp, "+CFUN: ", 7)==0)
      {
        int cfun=atoi(atcd.at_cmd.resp+7);
        if (cfun==1)
          return ATCD_SB_INIT + 8;
      };
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CFUN=1\r\n");    // Plna fce zarizeni
    case ATCD_SB_INIT + 7:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_INIT + 7;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_INIT + ATCD_SO_ERR;

    case ATCD_SB_INIT + 8:
      init_time_inner=atcd_get_ms();
      init_time_outer=init_time_inner;
      atcd_atc_exec_cmd_res_(&atcd.at_cmd, "AT+CPIN?\r\n", "+CME ERROR: 14"); //system nerozlisi skutecny OK od _res // Je vyzadovan PIN?
    case ATCD_SB_INIT + 9:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_INIT + 9;
      if((atcd.at_cmd.result == ATCD_ATC_RESULT_ERROR) && (atcd.at_cmd.result_code == 10))
      { //err 10= NO SIM
        atcd.sim.state = ATCD_SIM_STATE_NONE;
        if (atcd.selfcheck_state==atcd_selfcheck_stateBUSY)
          return ATCD_SB_SELFCHECK;
        init_time_inner=atcd_get_ms();
        return ATCD_SB_INIT + 90; //pockej 5s
      };
      if (atcd.at_cmd.result == ATCD_ATC_RESULT_MATCH)
      { //a tady muzu cekat na unso +CPIN: READY
        if (atcd_get_ms()-init_time_outer>=12000)
          return ATCD_SB_INIT + ATCD_SO_ERR;
        if (atcd_get_ms()-init_time_inner>=500)
        {
          init_time_inner=atcd_get_ms();
          atcd_atc_exec_cmd_res_(&atcd.at_cmd, "AT+CPIN?\r\n", "+CME ERROR: 14");
        }
        return ATCD_SB_INIT + 9;
      }
      /* ted uz je cekani v INIT+1 tak snad to bude ok
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK)
      {
        init_time_inner=atcd_get_ms();
        return ATCD_SB_INIT + 91; //pockej 0.5s
      }*/
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
        //atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CPIN=\"1234\"\r\n");            // Zadame PUK
        //TODO: Doimplementovat
      }
      else
      {
        ATCD_DBG_PIN_ERR
        atcd.sim.state = ATCD_SIM_STATE_ERROR;
        return ATCD_SB_INIT + ATCD_SO_ERR;
      }      
    case ATCD_SB_INIT + 10:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_INIT + 10;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_INIT + ATCD_SO_ERR;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CLIP=1\r\n");           // Zobrazovat cislo volajiciho
    case ATCD_SB_INIT + 11:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_INIT + 11;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_INIT + ATCD_SO_ERR;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CMGF=1\r\n");           // Textovy rezim SMS
    case ATCD_SB_INIT + 12:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_INIT + 12;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_INIT + ATCD_SO_ERR;
      init_time_inner=atcd_get_ms();
      init_time_outer=init_time_inner;
      atcd_atc_exec_cmd_res_(&atcd.at_cmd, "AT+CPMS?\r\n", "+CMS ERROR: 302");//system nerozlisi skutecny OK od _res , "SMS Ready\r\n"); // --- Test vyuziti pametovych prostoru na SMS
    case ATCD_SB_INIT + 13:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_INIT + 13;
      //if ((atcd.at_cmd.result == ATCD_ATC_RESULT_ERROR) && (atcd.at_cmd.result_code == 302))
      if (atcd.at_cmd.result == ATCD_ATC_RESULT_MATCH)
      {
        //mozna cekat na SMS Ready\r\n ? V tomhle systemu se unso nedelaji tak snadno
        //a predtim taky prijde Call Ready\r\n, mozna proste mit priznak ze prisly?
        if (atcd_get_ms()-init_time_outer>=12000)
          return ATCD_SB_INIT + ATCD_SO_ERR;
        if (atcd_get_ms()-init_time_inner>=500)
        {
          init_time_inner=atcd_get_ms();
          atcd_atc_exec_cmd_res_(&atcd.at_cmd, "AT+CPMS?\r\n", "+CMS ERROR: 302");           // --- Test vyuziti pametovych prostoru na SMS
        }
        return ATCD_SB_INIT + 13;
      }
      else if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_INIT + ATCD_SO_ERR;

      //+CPMS: "SM_P",0,20,"SM_P",0,20,"SM_P",0,20
      if (strncmp(atcd.at_cmd.resp, "+CPMS: ", 7)==0)
      {
        const char *has=strstr(atcd.at_cmd.resp+7, "\"SM\",0,");
        if (has==0)
          has=strstr(atcd.at_cmd.resp+7, "\"SM_P\",0,");
        if (has!=0)
          return ATCD_SB_INIT + 15;
      };
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CMGD=1,4\r\n");           // Smaze vsechny SMS na karte
    case ATCD_SB_INIT + 14:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_INIT + 14;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_INIT + ATCD_SO_ERR;
    case ATCD_SB_INIT + 15:
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CSDH=1\r\n");           //
    case ATCD_SB_INIT + 16:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_INIT + 16;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_INIT + ATCD_SO_ERR;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CNMI=1,2,0,0,0\r\n");          // Rezim nakladani s novymi SMS
    case ATCD_SB_INIT + 17:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_INIT + 17;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_INIT + ATCD_SO_ERR;

      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CREG=2\r\n");
    case ATCD_SB_INIT + 18:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_INIT + 18;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_INIT + ATCD_SO_ERR;

      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CLCC?\r");
    case ATCD_SB_INIT + 19:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_INIT + 19;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_INIT + ATCD_SO_ERR;
      if (strncmp(atcd.at_cmd.resp, "+CLCC: ", 7)==0)
      {
        int clcc=atoi(atcd.at_cmd.resp+7);
        if (clcc==1)
          return ATCD_SB_INIT + 24;
      };

      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CLCC=1\r\n");
    case ATCD_SB_INIT + 20:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_INIT + 20;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_INIT + ATCD_SO_ERR;


    case ATCD_SB_INIT + 24:
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+IPR?\r\n");
    case ATCD_SB_INIT + 25:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_INIT + 25;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_INIT + ATCD_SO_ERR;
      if ((atcd.at_cmd.resp_len>=6+1+2) && (strncmp(atcd.at_cmd.resp, "+IPR: ", 6)==0))
      {
        int ipr=atoi(atcd.at_cmd.resp+strlen("+IPR: "));
        if (ipr==115200)
          return ATCD_SB_INIT+28;
      }
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+IPR=115200\r\n");
    case ATCD_SB_INIT + 26:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_INIT + 26;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_INIT + ATCD_SO_ERR;

      atcd_atc_exec_cmd(&atcd.at_cmd, "AT&W\r\n");
    case ATCD_SB_INIT + 27:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_INIT + 27;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_INIT + ATCD_SO_ERR;

    case ATCD_SB_INIT + 28:
      if(atcd.ble.init == 0b01111111 ) {
          return ATCD_SB_INIT + 80;
      }

    if((atcd.ble.init & 0b1) != 0) return ATCD_SB_INIT + 32;
      atcd.at_cmd.timeout = 5000 * 3;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+BTPOWER=1\r\n");
    case ATCD_SB_INIT + 31:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_INIT + 31;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) {atcd.at_cmd.timeout = 5000; return ATCD_SB_INIT + ATCD_SO_ERR;}
      atcd.ble.init |= 0b1;

    case ATCD_SB_INIT + 32:
      if((atcd.ble.init & 0b10) != 0) return ATCD_SB_INIT + 34;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+BLESREG\r\n");
    case ATCD_SB_INIT + 33:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_INIT + 33;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_INIT + ATCD_SO_ERR;
      sscanf (atcd.at_cmd.resp,"+BLESREG: %d,%s\r\n", &atcd.ble.server_index, atcd.ble.user_id);
      atcd.ble.inst = 1;
      atcd.ble.init |= 0b10;

    case ATCD_SB_INIT + 34:
      if((atcd.ble.init & 0b100) != 0) return ATCD_SB_INIT + 36;
      sprintf(atcd.at_cmd_buff, "AT+BLESSAD=%d,\"%s\",%d,%d,%d\r\n",
              atcd.ble.server_index,
              "9ECADC240EE5A9E093F3A3B50100406E",
              15,
              1,
              1);
      atcd_atc_exec_cmd(&atcd.at_cmd, atcd.at_cmd_buff);
    case ATCD_SB_INIT + 35:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_INIT + 35;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_INIT + ATCD_SO_ERR;
      atcd.ble.init |= 0b100;

    case ATCD_SB_INIT + 36:
      if((atcd.ble.init & 0b1000) != 0) return ATCD_SB_INIT + 38;
      sprintf(atcd.at_cmd_buff, "AT+BLESSC=%d,\"%s\",%d,%d,%d\r\n",
              1,
              "9ECADC240EE5A9E093F3A3B50200406E",
              1,
              12,
              255);
      atcd_atc_exec_cmd(&atcd.at_cmd, atcd.at_cmd_buff);
    case ATCD_SB_INIT + 37:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_INIT + 37;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_INIT + ATCD_SO_ERR;
      atcd.ble.init |= 0b1000;

    case ATCD_SB_INIT + 38:
      if((atcd.ble.init & 0b10000) != 0) return ATCD_SB_INIT + 40;
      sprintf(atcd.at_cmd_buff, "AT+BLESSC=%d,\"%s\",%d,%d,%d\r\n",
              1,
              "9ECADC240EE5A9E093F3A3B50300406E",
              1,
              16,
              255);
      atcd_atc_exec_cmd(&atcd.at_cmd, atcd.at_cmd_buff);
    case ATCD_SB_INIT + 39:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_INIT + 39;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_INIT + ATCD_SO_ERR;
      atcd.ble.init |= 0b10000;

    case ATCD_SB_INIT + 40:
      if((atcd.ble.init & 0b100000) != 0) return ATCD_SB_INIT + 42;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+BLESSD=1,\"0229\",1,0\r\n");
    case ATCD_SB_INIT + 41:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_INIT + 41;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_INIT + ATCD_SO_ERR;
      atcd.ble.init |= 0b100000;

    case ATCD_SB_INIT + 42:
      if((atcd.ble.init & 0b1000000) != 0) return ATCD_SB_INIT + 44;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+BLESSSTART=1,0\r\n");
    case ATCD_SB_INIT + 43:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_INIT + 43;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_INIT + ATCD_SO_ERR;
      atcd.ble.init |= 0b1000000;

    case ATCD_SB_INIT + 44:



    case ATCD_SB_INIT + 80:
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
    case ATCD_SB_INIT + 91:
      if (atcd_get_ms()-init_time_inner<500) return ATCD_SB_INIT + 91;
    case ATCD_SB_INIT + ATCD_SO_ERR:
      // V prubehu inicializace doslo k chybe
    {
      char hlaska[37+3*5 +10];
      snprintf(hlaska, sizeof(hlaska), "Inicializace selhala s=%u c.s=%d c.r=%d !\r\n",
          atcd.proc_step_initfailed, atcd.at_cmd.state, atcd.at_cmd.result);
      atcd_dbg_err("ATCD: INIT: ", hlaska);
      //ATCD_DBG_INIT_ERR

      atcd.err_cnt++;
      //nekde resit reset pri prekroceni poctu pokusu
      return ATCD_SB_INIT;
    }

    case ATCD_SB_INIT + ATCD_SO_END:

    case ATCD_SB_SETUP:
      if (atcd.setup.clean)
        return ATCD_SB_SETUP + ATCD_SO_END;

      atcd_atc_set_defaults(&atcd.at_cmd);

      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CLVL?\r");
    case ATCD_SB_SETUP + 1:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_SETUP + 1;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_SETUP + ATCD_SO_ERR;
      if (strncmp(atcd.at_cmd.resp, "+CLVL:", 6)==0)
      {
        int clvl=atoi(atcd.at_cmd.resp+6);
        if (clvl==atcd.setup.clvl)
          return ATCD_SB_SETUP + 3;
      };
      snprintf(atcd.at_cmd_buff, sizeof(atcd.at_cmd_buff), "AT+CLVL=%d\r", atcd.setup.clvl);
      atcd_atc_exec_cmd(&atcd.at_cmd, atcd.at_cmd_buff);
    case ATCD_SB_SETUP + 2:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_SETUP + 2;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_SETUP + ATCD_SO_ERR;

    case ATCD_SB_SETUP + 3:
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+ECHO?\r");
    case ATCD_SB_SETUP + 4:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_SETUP + 4;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_SETUP + ATCD_SO_ERR;
      // +ECHO: (0,96,253,16388,20488),(1,0,0,0,0)
      //default po factory reset: +ECHO: (0,96,253,16388,20488),(1,96,224,5256,20488)
      //4 parametry si pamatuje, 5. nejde zjistit a po restartu je 1
      if (strncmp(atcd.at_cmd.resp, "+ECHO: (0,", 10)==0)
      {
        int want_size=snprintf(atcd.at_cmd_buff, sizeof(atcd.at_cmd_buff), "%u,%u,%u,%u)",
            atcd.setup.echo.np, atcd.setup.echo.ae, atcd.setup.echo.nr, atcd.setup.echo.ns);//, atcd.setup.echo_can_en);
        if (strncmp(atcd.at_cmd.resp+10, atcd.at_cmd_buff, want_size)==0)
        {
          if (atcd.setup.echo.mirror_can==atcd.setup.echo.can)
            return ATCD_SB_SETUP+6;
        };
      };
      snprintf(atcd.at_cmd_buff, sizeof(atcd.at_cmd_buff), "AT+ECHO=0,%u,%u,%u,%u,%u\r",
          atcd.setup.echo.np, atcd.setup.echo.ae, atcd.setup.echo.nr, atcd.setup.echo.ns, atcd.setup.echo.can);
      atcd_atc_exec_cmd(&atcd.at_cmd, atcd.at_cmd_buff);
      atcd.setup.echo.mirror_can=atcd.setup.echo.can;
    case ATCD_SB_SETUP + 5:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_SETUP + 5;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) { atcd.setup.echo.mirror_can=2; return ATCD_SB_SETUP + ATCD_SO_ERR; }

    case ATCD_SB_SETUP + 6:
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CMIC?\r");
    case ATCD_SB_SETUP + 7:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_SETUP + 7;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_SETUP + ATCD_SO_ERR;
      // +CMIC: (0,12),(1,0),(2,12),(3,0)
      if (strncmp(atcd.at_cmd.resp, "+CMIC: (0,", 10)==0)
      {
        int cmic=atoi(atcd.at_cmd.resp+10);
        if (cmic==atcd.setup.cmic)
          return ATCD_SB_SETUP + 9;
      };
      snprintf(atcd.at_cmd_buff, sizeof(atcd.at_cmd_buff), "AT+CMIC=0,%u\r", atcd.setup.cmic);
      atcd_atc_exec_cmd(&atcd.at_cmd, atcd.at_cmd_buff);
    case ATCD_SB_SETUP + 8:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_SETUP + 8;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_SETUP + ATCD_SO_ERR;

    case ATCD_SB_SETUP + 9:
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CRSL?\r");
    case ATCD_SB_SETUP + 10:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_SETUP + 10;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_SETUP + ATCD_SO_ERR;
      // +CRSL: 0
      if (strncmp(atcd.at_cmd.resp, "+CRSL: ", 7)==0)
      {
        int crsl=atoi(atcd.at_cmd.resp+7);
        if (crsl==atcd.setup.crsl)
          return ATCD_SB_SETUP + 12;
      };
      snprintf(atcd.at_cmd_buff, sizeof(atcd.at_cmd_buff), "AT+CRSL=%u\r", atcd.setup.crsl);
      atcd_atc_exec_cmd(&atcd.at_cmd, atcd.at_cmd_buff);
    case ATCD_SB_SETUP + 11:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_SETUP + 11;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_SETUP + ATCD_SO_ERR;

    case ATCD_SB_SETUP + 12:
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+SIDET?\r");
    case ATCD_SB_SETUP + 13:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_SETUP + 13;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_SETUP + ATCD_SO_ERR;
      // +SIDET: (0,0),(1,0),(2,0),(3,0)
      if (strncmp(atcd.at_cmd.resp, "+SIDET: (0,", 11)==0)
      {
        int sidet0=atoi(atcd.at_cmd.resp+11);
        if (sidet0==atcd.setup.sidet0)
          return ATCD_SB_SETUP + 15;
      };
      snprintf(atcd.at_cmd_buff, sizeof(atcd.at_cmd_buff), "AT+SIDET=0,%u\r", atcd.setup.sidet0);
      atcd_atc_exec_cmd(&atcd.at_cmd, atcd.at_cmd_buff);
    case ATCD_SB_SETUP + 14:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_SETUP + 14;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_SETUP + ATCD_SO_ERR;

    case ATCD_SB_SETUP + 15:
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CAGCSET?\r");
    case ATCD_SB_SETUP + 16:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_SETUP + 16;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_SETUP + ATCD_SO_ERR;
      // +CAGCSET: 1
      if (strncmp(atcd.at_cmd.resp, "+CAGCSET: ", 10)==0)
      {
        int cagcset=atoi(atcd.at_cmd.resp+10);
        if (cagcset==atcd.setup.cagcset)
          return ATCD_SB_SETUP + 18;
      };
      snprintf(atcd.at_cmd_buff, sizeof(atcd.at_cmd_buff), "AT+CAGCSET=%u\r", atcd.setup.cagcset);
      atcd_atc_exec_cmd(&atcd.at_cmd, atcd.at_cmd_buff);
    case ATCD_SB_SETUP + 17:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_SETUP + 17;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_SETUP + ATCD_SO_ERR;

    case ATCD_SB_SETUP + 18:
      atcd.setup.clean=1;
      return ATCD_SB_SETUP + ATCD_SO_END;
    case ATCD_SB_SETUP + ATCD_SO_ERR:
      // V prubehu inicializace doslo k chybe
    {
      char hlaska[37+3*5 +10];
      snprintf(hlaska, sizeof(hlaska), "Setup selhal c.s=%d c.r=%d !\r\n",
          atcd.at_cmd.state, atcd.at_cmd.result);
      atcd_dbg_err("ATCD: SETUP: ", hlaska);
      //ATCD_DBG_INIT_ERR

      //setup vicemene funguje, pravdepodobnejsi je jednorazova chyba->nechat clean==0 a zkusi se znovu
      //atcd.setup.clean=2; //nemam jinou moznost nez dat true, jinak pojede furt

      return ATCD_SB_SETUP + ATCD_SO_END;
    }


    case ATCD_SB_SETUP + ATCD_SO_END:
    //------------------------------------------------------------------------
    // ---- Pocatek pravidelneho kolecka ----
    //------------------------------------------------------------------------
    case ATCD_SB_STAT:
      // Zarizeni je pripraveno k praci, pripadne spi...
      // Pripadne testy stavu a dalsi cinnosti na pozadi...
      if(atcd_get_ms() - atcd.timer < 30000)
      {
        return ATCD_SB_STAT + ATCD_SO_END;
      }
      // Je cast spustit kontrolu stavu modemu
      ATCD_DBG_STAT_START

      atcd.timer = atcd_get_ms();
      atcd_atc_set_defaults(&atcd.at_cmd);
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
      //odpoved se chyta jako unso

      // Doplnit sleepmode...

      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CGATT?\r\n");    // stav PDP kontextu
    case ATCD_SB_STAT + 4:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_STAT + 4;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_STAT + ATCD_SO_ERR;

      if(atcd.at_cmd.resp_len >= 10) //+CGATT:1\r\n i kdyz spis +CGATT: 1\r\n
      {
        if(strncmp(atcd.at_cmd.resp, "+CGATT:", 7) == 0)
        {
          //kdyz nesouhlasi CGATT a atcd.gprs.state tak disco
          int cgatt=atoi(atcd.at_cmd.resp+7);
          if(cgatt == 0)
          {
            if(atcd.gprs.state==ATCD_GPRS_STATE_CONN) atcd_gprs_disconnect(0);
          }
          else if(cgatt == 1)
          {
            if (atcd.gprs.state==ATCD_GPRS_STATE_DISCONN) atcd_gprs_disconnect(1);
          }
        }
      }

      /*atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CIFSR\r\n");     // Check IP address
    case ATCD_SB_STAT + 5:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_STAT + 5;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_STAT + ATCD_SO_ERR;
*/
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CIPSTATUS\r\n");  // TCP/UDP connections status
      atcd.conns.awaitingC5__=1;
      step_time=atcd_get_ms();
    case ATCD_SB_STAT + 6:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_STAT + 6;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_STAT + ATCD_SO_ERR;
      if ((atcd.conns.awaitingC5__) && (atcd_get_ms()-step_time<100)) //normalne asi 25ms
      { //prijem z modemu ho tak zdrzuje ze se sem vetsinou prijde az po prijeti "C5: ," ale ne uplne vzdy
        return ATCD_SB_STAT+6;
      };

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


      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CLCC\r\n");      // Probihajici hovory
    case ATCD_SB_STAT + 8:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_STAT + 8;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_STAT + ATCD_SO_ERR;
      //+CLCC: 1,0,0,0,0,"777262425",129,""
      //nesezerou mi odpovedi v unso? asi jo, tady nejsou
      /*takze nic uint8_t prevcallout=atcd.phone.state_call_out;
      atcd.phone.state_call_out=0;
      if (atcd.at_cmd.resp_len>=16) //+CLCC: 1,0,0,0,0\r\n
      { //idealne projit vsechny radky odpovedi ale co uz
        if (strncmp(atcd.at_cmd.resp, "+CLCC:", 6)==0)
        {
          int id=atcd.at_cmd.resp[7]-'0';
          int inco=atcd.at_cmd.resp[9]-'0';//atoi(atcd.at_cmd.resp+9);
          int stat=atcd.at_cmd.resp[11]-'0';
          if ((id>=1) && (id<=7) && (inco==0) && (stat==0))
            atcd.phone.state_call_out|=(1<<(id-1));
        };
      };
      if (prevcallout!=atcd.phone.state_call_out)
      {
        char tmps[30];
        snprintf(tmps, sizeof(tmps), "%02x->%02x", prevcallout, atcd.phone.state_call_out);
        atcd_dbg_inf2("call_out", tmps);
      };*/

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
            atcd_dbg_warn("@atcd", "FIX phone.state\n");
          switch (doit)
          {
          case 0: atcd.phone.state=ATCD_PHONE_STATE_IDLE; break;
          case 3: atcd.phone.state=ATCD_PHONE_STATE_RING; break;
          case 4: atcd.phone.state=ATCD_PHONE_STATE_CALL; break; //CALL_IN nebo taky CALL_OUT
          }
          if ((cpas==0) && (atcd.phone.state_call_out!=0))
          {
            char tmps[30];

            uint8_t prevcallout=atcd.phone.state_call_out;
            atcd.phone.state_call_out=0;
            snprintf(tmps, sizeof(tmps), "%02x->%02x", prevcallout, atcd.phone.state_call_out);
            atcd_dbg_inf2("call_out", tmps);
          };
          if ((cpas==0) && (atcd.phone.state_call_in!=0))
          {
            char tmps[30];

            uint8_t prevcallin=atcd.phone.state_call_in;
            atcd.phone.state_call_in=0;
            snprintf(tmps, sizeof(tmps), "%02x->%02x", prevcallin, atcd.phone.state_call_in);
            atcd_dbg_inf2("call_in", tmps);
          };

          //cpas2 neresit, nema signal...
          //taky se stane, kdyz ma vybito, pomuze reset AT+CFUN=1,1 pokud se mezitim nabiji
          /*if (cpas==2)
          {
            atcd_dbg_err("fail_ai", "+CPAS: 2");
            fails_after_init.cpas2++;
            if (fails_after_init.cpas2>=5)
            {
              fails_after_init.cpas2=0;
              atcd_reset(); //reset hw ale i state a vubec
            };
          };*/
        }
      }

      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CSCLK?\r\n");
    case ATCD_SB_STAT + 11:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_STAT + 11;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_STAT + ATCD_SO_ERR;

      {
        if ((atcd.at_cmd.resp_len>=11) && (strncmp(atcd.at_cmd.resp, "+CSCLK: ", 8)==0))
        {
          int csclk=atoi(atcd.at_cmd.resp+strlen("+CSCLK: "));

          //univerzalni fix
          atcd_sleep_mode_t next_sleep_mode=ATCD_SM__UNUSED;
          switch (atcd.sleep_mode)
          {
          case ATCD_SM_OFF: if (csclk!=0) next_sleep_mode=ATCD_SM_W_OFF; break;
          case ATCD_SM_AUTO: if (csclk!=2) next_sleep_mode=ATCD_SM_W_AUTO; break;
          case ATCD_SM_MANUAL: if (csclk!=1) next_sleep_mode=ATCD_SM_W_MANUAL; break;
          default: ;
          }

          //tipcove-spaci fix
          if (csclk!=1 && atcd.sleep_mode!=ATCD_SM_W_MANUAL)
            next_sleep_mode=ATCD_SM_W_MANUAL;

          if (next_sleep_mode!=ATCD_SM__UNUSED)
          { //nebyli jsme v ATCD_SM_W_... a pritom to nesouhlasi -> asi byl reset, udelam reinit
            char tmps[50];
            snprintf(tmps, sizeof(tmps), " =%d, mode=%d->%d", csclk, atcd.sleep_mode, next_sleep_mode);
            atcd_dbg_warn("CSCLK: ", tmps);
            atcd_begin();
            atcd.sleep_mode=next_sleep_mode;
          };
        };
      }

      /*atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CMIC?\r\n");
    case ATCD_SB_STAT + 11:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_STAT + 11;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_STAT + ATCD_SO_ERR;*/

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
      if(atcd.phone.state != ATCD_PHONE_STATE_CALL) // || CALL_OUT
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

      atcd_atc_set_defaults(&atcd.at_cmd);
      atcd.at_cmd.timeout = 5000;

    case ATCD_SB_PHONE + 1:

      if(atcd.phone.state != ATCD_PHONE_STATE_RING_WA || atcd.selfcheck_state==atcd_selfcheck_stateBUSY) return ATCD_SB_PHONE + 3;
      atcd_atc_exec_cmd(&atcd.at_cmd, "ATA\r\n");
    case ATCD_SB_PHONE + 2:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_PHONE + 2;
      //TODO: pokud hovor zmizi (+CME ERROR: 3), posilam ATA kazdych 20ms dokud se nezkusi AT+CPAS
      //ale pro opravu bych potreboval timer na zpozdeni, priznak protoze nechci vynechat ATH a dalsi ale pockat az na konci,
      //a jeste pocitadlo chyb abych to nebrzdil hned ale az treba po 3.
      //snad to vyresi zpracovani +CLCC: x,1,6,0,0,"+xxx",145,""
      //jak to ze neprijde NO CARRIER? potrebuji aby CLCC: ..6 davalo jen notifikaci CALL, CALL_END az potom
      //NO CARRIER neprijde kdyz prichozi hovor vzda nez se prijme, pak dostanu jen +CLCC
      //ted by to melo byt OK
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

      atcd.at_cmd.timeout = 60000;

      rbuff_lin_space(&atcd.at_cmd_data, (uint8_t *)atcd.phone.sms_tx.message, atcd.phone.sms_tx.len); //const message nevyresis
      atcd.at_cmd.data = &atcd.at_cmd_data;           //to nepujde atcd.phone.sms_tx.message; //TODO: ten buffer presunout do SMS ktera jej pouziva
      atcd.at_cmd.data_len = atcd.phone.sms_tx.len;

      atcd_atc_exec_cmd(&atcd.at_cmd, atcd.at_cmd_buff);
    case ATCD_SB_PHONE + 8:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_PHONE + 8;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK)
      {
        atcd.phone.sms_tx.state = ATCD_PHONE_SMS_STATE_ERROR;
        return ATCD_SB_PHONE + 9;
      }

      atcd.phone.sms_tx.state = ATCD_PHONE_SMS_STATE_SEND;
      if(atcd.phone.sms_tx.callback) atcd.phone.sms_tx.callback(ATCD_SMS_EV_SEND, &atcd.phone.sms_tx);

    case ATCD_SB_PHONE + 9:
      //Pozor, poradjsou nastaven buffery!!!

    case ATCD_SB_PHONE + ATCD_SO_ERR:
    case ATCD_SB_PHONE + ATCD_SO_END:

    //------------------------------------------------------------------------
    // POWERSAVE
    //------------------------------------------------------------------------
    case ATCD_SB_POWERSAVE:

      atcd_atc_set_defaults(&atcd.at_cmd);
      atcd.at_cmd.timeout = 5000;

    case ATCD_SB_POWERSAVE + 1:

      //TODO: Overit, ze v modu AUTO funguje GPS i kdyz nikdo neovlada DTR. Pripadne rozsirit podminky nize i o stav GPS...
      //po startu je AT+CSCLK=0 ale lze zmenit pomoci AT&W

      if(atcd.sleep_mode == ATCD_SM_W_OFF) atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CSCLK=0\r");
      else if(atcd.sleep_mode == ATCD_SM_W_MANUAL) atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CSCLK=1\r");
      //po startu na tenhle prijde +CSCLK: 0 ale ne OK else if(atcd.sleep_mode == ATCD_SM_W_MANUAL) atcd_atc_exec_cmd(&atcd.at_cmd, "AT+csclk?;+CSCLK=1\r");
      //dela totez                                     else if(atcd.sleep_mode == ATCD_SM_W_MANUAL) atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CSCLK?;+CSCLK=1\r");
      else if(atcd.sleep_mode == ATCD_SM_W_AUTO) atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CSCLK=2\r");
      else return ATCD_SB_POWERSAVE + ATCD_SO_END;

    case ATCD_SB_POWERSAVE + 2:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_POWERSAVE + 2;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_POWERSAVE + ATCD_SO_ERR;

      if(atcd.sleep_mode == ATCD_SM_W_OFF) atcd.sleep_mode = ATCD_SM_OFF;
      else if(atcd.sleep_mode == ATCD_SM_W_MANUAL) atcd.sleep_mode = ATCD_SM_MANUAL;
      else if(atcd.sleep_mode == ATCD_SM_W_AUTO) atcd.sleep_mode = ATCD_SM_AUTO;
      else return ATCD_SB_POWERSAVE + ATCD_SO_END;

    case ATCD_SB_POWERSAVE + 7:

    case ATCD_SB_POWERSAVE + ATCD_SO_ERR:
    case ATCD_SB_POWERSAVE + ATCD_SO_END:
    //------------------------------------------------------------------------
    // GPRS INIT
    //------------------------------------------------------------------------
    case ATCD_SB_GPRS_INIT:
    {
      atcd_reg_state_e reg=atcd_gsm_state();
      atcd_sim_state_e sim=atcd_sim_state();
      if(reg==ATCD_REG_STATE_OFF || reg==ATCD_REG_STATE_SEARCHING ||
         reg==ATCD_REG_STATE_DENIED || reg==ATCD_REG_STATE_EMERGENCY ||
         sim!=ATCD_SIM_STATE_OK ||
         atcd.gprs.state != ATCD_GPRS_STATE_CONNECTING)
      {
        return ATCD_SB_GPRS_INIT + ATCD_SO_END;
      }

      ATCD_DBG_GPRS_INIT_START

      atcd_atc_set_defaults(&atcd.at_cmd);
      //TODO: co je toto, nechybi tu nejaky AT prikaz? nebo zrusit cekani, az se dokonci
    }
    case ATCD_SB_GPRS_INIT + 1:
      //if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_GPRS_INIT + 1;
      if (atcd.phone.state != ATCD_PHONE_STATE_IDLE)
        return ATCD_SB_GPRS_INIT + ATCD_SO_END;

      atcd.at_cmd.timeout = 40000;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CIPSHUT\r\n");
    case ATCD_SB_GPRS_INIT + 4:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_GPRS_INIT + 4;
      if((atcd.at_cmd.result == ATCD_ATC_RESULT_ERROR)) //opravdu nepotrebuji posilat 50x/s kdyz nekdo vytahne SIMku
      {
        init_time_inner=atcd_get_ms();
        return ATCD_SB_GPRS_INIT + 90;
      }

      atcd_conn_reset_all();

      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_GPRS_INIT + ATCD_SO_ERR;
      if(atcd.phone.state != ATCD_PHONE_STATE_IDLE)
        return ATCD_SB_GPRS_INIT + ATCD_SO_END;

      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CGATT=0\r\n");
    case ATCD_SB_GPRS_INIT + 5:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_GPRS_INIT + 5;
      if((atcd.at_cmd.result == ATCD_ATC_RESULT_ERROR) && (atcd.at_cmd.result_code == 4))
      {
        init_time_inner = atcd_get_ms();
        return ATCD_SB_GPRS_INIT + 90;
      }
      if(atcd.phone.state != ATCD_PHONE_STATE_IDLE)
        return ATCD_SB_GPRS_INIT + ATCD_SO_END;

      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_GPRS_INIT + ATCD_SO_ERR;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CIPMUX=1\r\n");
    case ATCD_SB_GPRS_INIT + 6:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_GPRS_INIT + 6;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_GPRS_INIT + ATCD_SO_ERR;
      if (atcd.phone.state != ATCD_PHONE_STATE_IDLE)
        return ATCD_SB_GPRS_INIT + ATCD_SO_END;

      init_time_inner=atcd_get_ms();
      init_time_outer=init_time_inner;
      atcd_atc_exec_cmd_res_(&atcd.at_cmd, "AT+CGATT=1\r\n", "+CME ERROR: 100");
    case ATCD_SB_GPRS_INIT + 7:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_GPRS_INIT + 7;
      if (atcd.at_cmd.result == ATCD_ATC_RESULT_MATCH)
      { //
        if (atcd_get_ms()-init_time_outer>=5000)
        {
          char errbuf[20];
          snprintf(errbuf, sizeof(errbuf), "reg=%d sim=%d\r\n", atcd_gsm_state(), atcd_sim_state());
          atcd_dbg_inf2("GPRS: ", errbuf);
          init_time_inner=atcd_get_ms();
          return ATCD_SB_GPRS_INIT + 85;
        };
        if (atcd_get_ms()-init_time_inner>=500)
        {
          init_time_inner=atcd_get_ms();
          atcd_atc_exec_cmd_res_(&atcd.at_cmd, "AT+CGATT=1\r\n", "+CME ERROR: 100");
        }
        return ATCD_SB_GPRS_INIT + 7;
      };

      if((atcd.at_cmd.result == ATCD_ATC_RESULT_ERROR) && ((atcd.at_cmd.result_code == 4) || (atcd.at_cmd.result_code == 100)))
      { //+CME ERROR: 100 not here any more, only : 4
        char errbuf[20];
        snprintf(errbuf, sizeof(errbuf), "reg=%d sim=%d\r\n", atcd_gsm_state(), atcd_sim_state());
        atcd_dbg_inf2("GPRS: ", errbuf);
        init_time_inner=atcd_get_ms();
        return ATCD_SB_GPRS_INIT + 85;
      }
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK)
      { //pri vypnuti modemu kvuli slabe baterii mi to nejspis ufikne a vrati se do ATCD_SB_INIT, ale zkusim
        char errbuf[20];
        snprintf(errbuf, sizeof(errbuf), "cgatt=1 -> %d", atcd.at_cmd.result);
        atcd_dbg_inf2("GPRS: ", errbuf);

        return ATCD_SB_GPRS_INIT + ATCD_SO_ERR;
      }
      if (atcd.phone.state != ATCD_PHONE_STATE_IDLE)
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

      if (atcd.phone.state != ATCD_PHONE_STATE_IDLE) return ATCD_SB_GPRS_INIT + ATCD_SO_END;
      atcd.at_cmd.timeout = 95000;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CIICR\r\n");
    case ATCD_SB_GPRS_INIT + 9:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_GPRS_INIT + 9;
      if(atcd.at_cmd.result == ATCD_ATC_RESULT_ERROR)
      { //muze prijit
        //  +PDP: DEACT
        //  ERROR
        //trvalo 90.5s, mezitim nesly zadne AT prikazy
        //podle doc max. 85s
        if(strcmp(atcd.at_cmd.resp, "+PDP: DEACT\r\n")==0)
        {
          init_time_inner = atcd_get_ms();
          return ATCD_SB_GPRS_INIT + 89;
        };
      };
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_GPRS_INIT + ATCD_SO_ERR;
      if(atcd.phone.state != ATCD_PHONE_STATE_IDLE) return ATCD_SB_GPRS_INIT + ATCD_SO_END;

      atcd.at_cmd.timeout = 1500;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CIFSR\r\n");
    case ATCD_SB_GPRS_INIT + 10:
      if((atcd.at_cmd.state == ATCD_ATC_STATE_W_END) && (atcd.at_cmd.resp_len > 0))
      {
/*
[16.023] AT+CIFSR
[16.175] ATCD: ATC: Odesilani bylo dokoceno - prechazime na W_ECHO...
[16.178] AT+CIFSR
[16.180] ATCD: ATC: ECHO detected.
[16.180] 10.173.126.18
[16.182] ATCD: ATC: OK detected.

 */
        size_t iplen;
        atcd.at_cmd.resp[atcd.at_cmd.resp_len] = '\0';
        iplen = strspn(atcd.at_cmd.resp, "1234567890.");
        if(iplen >= 7)
          if(strcmp(atcd.at_cmd.resp + iplen, "\r\n") == 0)
          {
            // AT prikaz byl prave dokoncen
            ATCD_DBG_ATC_OK_DET

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

    case ATCD_SB_GPRS_INIT + 85: //vysetreni velmi bezne chyby +CME ERROR: 100 v INIT+7 (at+cgatt=1)
      if (atcd_get_ms()-init_time_inner<500) return ATCD_SB_GPRS_INIT + 85;
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+COPS?;+CREG?;+CPAS;+CGATT?;+CPIN?\r");
    case ATCD_SB_GPRS_INIT + 86:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_GPRS_INIT + 86;
      //AT+COPS?;+CREG?;+CPAS;+CGATT?
      //+COPS: 0,0,"OSKAR"
      //+CREG: 2,1,"9664","3873"
      //+CPAS: 0
      //+CGATT: 1
      //OK
      //vsechno happy ale proste to nejde
      atcd_dbg_inf3("INV7ION", atcd.at_cmd.resp);
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CIPSTATUS\r");
      atcd.conns.awaitingC5__=1;
    case ATCD_SB_GPRS_INIT + 87:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_GPRS_INIT + 87;
      //OK + unso
      //atcd_dbg_inf3("INV7ION", atcd.at_cmd.resp);
      init_time_inner=atcd_get_ms();
    case ATCD_SB_GPRS_INIT + 88:
      if (atcd_get_ms()-init_time_inner<3000) return ATCD_SB_GPRS_INIT + 88;
      atcd_dbg_inf3("GPRS ST", atcd.conns.parsedCipStatus.state);
      atcd_dbg_inf3("GPRS 0", atcd.conns.parsedCipStatus.chans[0]);
      atcd_dbg_inf3("GPRS 1", atcd.conns.parsedCipStatus.chans[1]);
      atcd_dbg_inf3("GPRS 2", atcd.conns.parsedCipStatus.chans[2]);
      return ATCD_SB_GPRS_INIT + ATCD_SO_ERR;

    case ATCD_SB_GPRS_INIT + 89: //nebyl by lepsi priznak "skip_all_gprs +_time" ? takhle zastavim vsechno
      if (atcd_get_ms()-init_time_inner<5000) return ATCD_SB_GPRS_INIT + 89;

    case ATCD_SB_GPRS_INIT + 90: //nebyl by lepsi priznak "skip_all_gprs +_time" ? takhle zastavim vsechno
      if (atcd_get_ms()-init_time_inner<500) return ATCD_SB_GPRS_INIT + 90;

    case ATCD_SB_GPRS_INIT + ATCD_SO_ERR:
      //GPRS init selhalo
      ATCD_DBG_GPRS_INIT_ERR
      //atcd.gprs.state = ATCD_GPRS_STATE_DISCONN;
      atcd_gprs_disconnect(0);

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

      atcd_atc_set_defaults(&atcd.at_cmd);

    case ATCD_SB_GPRS_DEINIT + 1:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_GPRS_DEINIT + 1;
      if (atcd.phone.state != ATCD_PHONE_STATE_IDLE)
        return ATCD_SB_GPRS_DEINIT + ATCD_SO_END;

      atcd.at_cmd.timeout = 40000; //tesne po skonceni hovoru trvalo CIPSHUT 9s
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CIPSHUT\r\n");
    case ATCD_SB_GPRS_DEINIT + 2:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_GPRS_DEINIT + 2;
      if((atcd.at_cmd.result == ATCD_ATC_RESULT_ERROR)) //opravdu nepotrebuji posilat 50x/s kdyz nekdo vytahne SIMku
      {
        init_time_inner=atcd_get_ms();
        return ATCD_SB_GPRS_DEINIT + 90;
      }
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_GPRS_DEINIT + ATCD_SO_ERR;
      if(atcd.phone.state != ATCD_PHONE_STATE_IDLE) return ATCD_SB_GPRS_DEINIT + ATCD_SO_END;

      atcd.at_cmd.timeout = 20000; //typicke je, ze selze AT+CIPSTART, jde se sem, AT+CIPSHUT se zrejme udela OK
        //a pak vytimeoutuje tohle, tim pristimu prikazu selze echo a vznika nekonecne peklo
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CGATT=0\r\n");
    case ATCD_SB_GPRS_DEINIT + 3:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_GPRS_DEINIT + 3;

      {
        uint32_t take_time=atcd_get_ms() - atcd.parser.at_cmd_timer;// > at_cmd->timeout
        if (take_time>500)
        { //naprosto vyjimecne 704ms -> OK (result_code=3 mozna od minula)
          char bufajzl[40]; //~25
          snprintf(bufajzl, sizeof(bufajzl), "DEINIT/2: %u ms -> %d/%d", (unsigned int)take_time, atcd.at_cmd.result, atcd.at_cmd.result_code);
          atcd_dbg_warn("ATCD: CONN: ", bufajzl);
        };
      }

      if((atcd.at_cmd.result == ATCD_ATC_RESULT_ERROR) && (atcd.at_cmd.result_code == 4))
      {
        init_time_inner=atcd_get_ms();
        return ATCD_SB_GPRS_DEINIT + 90;
      }
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_GPRS_DEINIT + ATCD_SO_ERR;

      ATCD_DBG_GPRS_DEINIT_OK
      atcd.gprs.state = ATCD_GPRS_STATE_DISCONN;
      atcd_gprs_autoconn(); //kdyz jsme chteli conn behem deinitu tak se pozadavek zahodil
      atcd_conn_reset_all();
      return ATCD_SB_GPRS_DEINIT + ATCD_SO_END;

    case ATCD_SB_GPRS_DEINIT + 90: //nebyl by lepsi priznak "skip_all_gprs +_time" ? takhle zastavim vsechno
      if (atcd_get_ms()-init_time_inner<500) return ATCD_SB_GPRS_DEINIT + 90;

    case ATCD_SB_GPRS_DEINIT + ATCD_SO_ERR:
      //GPRS deinit selhalo
      if (atcd_get_ms()-atcd.gprs.timer<65000)
      { //zkusim jednou zacyklit, jestli se to nespravi... jinak zbyva uz jen reset modemu
        atcd_dbg_warn("ATCD GPRS: ", "deinit fail");
        return ATCD_SB_GPRS_DEINIT;
      }
      ATCD_DBG_GPRS_DEINIT_ERR
      atcd.gprs.state = ATCD_GPRS_STATE_DISCONN;
      atcd.gprs.ip[0] = 0;
      atcd_gprs_autoconn();

      atcd_conn_reset_all();

    case ATCD_SB_GPRS_DEINIT + ATCD_SO_END:
    //------------------------------------------------------------------------
    // CONN OPEN
    //------------------------------------------------------------------------
    case ATCD_SB_CONN_OPEN:
      if(atcd.phone.state != ATCD_PHONE_STATE_IDLE) return ATCD_SB_CONN_OPEN + ATCD_SO_END;
      if(atcd.gprs.state != ATCD_GPRS_STATE_CONN) return ATCD_SB_CONN_OPEN + ATCD_SO_END;

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

      if(conn == NULL) return ATCD_SB_CONN_OPEN;
      if(conn->state != ATCD_CONN_STATE_W_OPEN) return ATCD_SB_CONN_OPEN;

      ATCD_DBG_CONN_OPENING

      atcd_atc_set_defaults(&atcd.at_cmd);
      atcd.at_cmd.timeout = 15000;
      atcd.at_cmd.cmd = atcd.at_cmd_buff;

      /*
       * pro SSL staci zapnout AT+CIPSSL=1 tesne pred otevrenim spojeni
       * nebo AT+CIPSSL=0 pro TCP
       * certifikaty nejak pomoci AT+FSCREATE, AT+FSWRITE, AT+SSLSETROOT aspol.
       * nezkousel jsem to
       */

      /*if(conn->ssl_en == 0) atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CIPSSL=0\r\n");
                       else atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CIPSSL=1\r\n");

    case ATCD_SB_CONN_OPEN + 1:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_CONN_OPEN + 1;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_CONN_OPEN + ATCD_SO_ERR;*/

      if(conn->protocol == ATCD_CONN_T_TCP)
      {
        snprintf(atcd.at_cmd_buff, sizeof(atcd.at_cmd_buff), "AT+CIPSTART=%u,\"TCP\",\"%s\",%u\r\n", conn->num, conn->host, conn->port);
        atcd_atc_exec(&atcd.at_cmd);
      }
      else if(conn->protocol == ATCD_CONN_T_UDP)
      {
        snprintf(atcd.at_cmd_buff, sizeof(atcd.at_cmd_buff), "AT+CIPSTART=%u,\"UDP\",\"%s\",%u\r\n", conn->num, conn->host, conn->port);
        atcd_atc_exec(&atcd.at_cmd);
      }
      else
      {
        ATCD_DBG_CONN_PROT_ERR
        return ATCD_SB_CONN_OPEN + ATCD_SO_ERR;
      }

    case ATCD_SB_CONN_OPEN + 2:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_CONN_OPEN + 2;
      if(atcd.at_cmd.result == ATCD_ATC_RESULT_OK)
      {
        ATCD_DBG_CONN_W_CONN
        //Pozor - stav spojeni se pomoci async msg muze zmenit jeste drive,
        //nez se dozvime, ze je prikaz dokoncen!!! Proto nasleduje testovani stavu!
        if(conn->state == ATCD_CONN_STATE_W_OPEN) conn->state = ATCD_CONN_STATE_OPENING;
      }
      else
      {
        ATCD_DBG_CONN_OPENING_ERR
        //Nechame spojeni ve stavu jak je, pokud je ATCD_CONN_STATE_W_OPEN, bude to
        //zkouset znovu do timeoutu...

        if((atcd.at_cmd.result == ATCD_ATC_RESULT_ERROR) && (atcd.at_cmd.result_code == 3))
        {
          // TODO mohlo se podelat GPRS, udelej reinit, doresit autoreconect
          atcd_gprs_disconnect(1); //po dokonceni disco by se mel udelat autoconnect
        }
      }
      return ATCD_SB_CONN_OPEN;

    case ATCD_SB_CONN_OPEN + ATCD_SO_ERR:
      //Otevreni spojeni selhalo
      ATCD_DBG_CONN_OPEN_FAIL
      atcd_conn_free(conn);
      // TODO FAIL nebo nechat CLOSE? Fail by mel byt asi nekde jinde nez ve stavu, ne?
      conn->state = ATCD_CONN_STATE_FAIL;
      return ATCD_SB_CONN_OPEN;

    case ATCD_SB_CONN_OPEN + ATCD_SO_END:
    //------------------------------------------------------------------------
    // CONN WRITE
    //------------------------------------------------------------------------
    case ATCD_SB_CONN_WRITE:
      if(atcd.phone.state != ATCD_PHONE_STATE_IDLE) return ATCD_SB_CONN_WRITE + ATCD_SO_END;
      if(atcd.gprs.state != ATCD_GPRS_STATE_CONN) return ATCD_SB_CONN_WRITE + ATCD_SO_END;

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

      ATCD_DBG_CONN_SEND

      if (conn->protocol!=ATCD_CONN_T_UDP)
        if(tx_data_len > 512) tx_data_len = 512;

      atcd_atc_set_defaults(&atcd.at_cmd);
      atcd.at_cmd.cmd = atcd.at_cmd_buff;
      atcd.at_cmd.timeout = 30000;
      atcd.at_cmd.data = &conn->tx_rbuff;
      atcd.at_cmd.data_len = tx_data_len;

      snprintf(atcd.at_cmd_buff, sizeof(atcd.at_cmd_buff), "AT+CIPSEND=%u,%u\r\n", conn->num, (unsigned int)tx_data_len);
      atcd_atc_exec(&atcd.at_cmd);

      // provedeni testovat
      //0, SEND OK
      // TODO jaktoze to prochazi i bez udane odpovedi k testu?

    case ATCD_SB_CONN_WRITE + 1:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_CONN_WRITE + 1;
      if(atcd.at_cmd.result == ATCD_ATC_RESULT_OK)
      {
        //posunout ukazovatko dat
        rbuff_seek(&conn->tx_rbuff, atcd.at_cmd.data_len);
        atcd.gprs.stat.bytes_sent+=atcd.at_cmd.data_len;
        // asi volat call back pokud je nastaven?
      }
      else
      {
        ATCD_DBG_CONN_SEND_ERR
        return ATCD_SB_CONN_WRITE + ATCD_SO_ERR;
      }
      return ATCD_SB_CONN_WRITE;

    case ATCD_SB_CONN_WRITE + ATCD_SO_ERR:
      //Zapis do spojeni selhal
      ATCD_DBG_CONN_SEND_FAIL
      //Uzavrit spojeni...
      atcd_conn_close(conn, 1);

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
      if(atcd.phone.state != ATCD_PHONE_STATE_IDLE) return ATCD_SB_CONN_CLOSE + ATCD_SO_END;
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

      atcd_atc_set_defaults(&atcd.at_cmd);
      atcd.at_cmd.timeout = 15000;

      //TODO: Tohle by nemelo fungovat - %u, CLOSE OK\r\n se odchytava jako async...
      //TODO: nebude prehlednejsi makro?
      //TODO: JE tu nastaveny response string, mel by byt v castech vyse, kterych se to tyka
      snprintf(atcd.at_cmd_buff, sizeof(atcd.at_cmd_buff), "AT+CIPCLOSE=%u\r\n", conn->num);
      snprintf(atcd.at_cmd_result_buff, sizeof(atcd.at_cmd_result_buff), "%u, CLOSE OK\r\n", conn->num);

      atcd_atc_exec_cmd_res_(&atcd.at_cmd, atcd.at_cmd_buff, atcd.at_cmd_result_buff);

    case ATCD_SB_CONN_CLOSE + 1:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_CONN_CLOSE + 1;
      if((atcd.at_cmd.result == ATCD_ATC_RESULT_OK) || //ve skutecnosti se prijme jako unso v atcd_conn_asc_msg a nastavi se na OK
         (atcd.at_cmd.result == ATCD_ATC_RESULT_MATCH)) //k tomuhle se vubec nedostane
      {
        ATCD_DBG_CONN_W_CLOSE
        //Pozor - stav spojeni se pomoci async msg muze zmenit jeste drive,
        //nez se dozvime, ze je prikaz dokoncen!!! Proto nasleduje testovani stavu!
        if(conn->state == ATCD_CONN_STATE_W_CLOSE) conn->state = ATCD_CONN_STATE_CLOSING;
      }
      else
      {
        ATCD_DBG_CONN_CLOSING_ERR
        return ATCD_SB_CONN_CLOSE + ATCD_SO_ERR;
      }
      return ATCD_SB_CONN_CLOSE;

    case ATCD_SB_CONN_CLOSE + ATCD_SO_ERR:
      //Uzavreni spojeni selhalo
      ATCD_DBG_CONN_CLOSE_FAIL
      // TODO FAIL nebo nechat CLOSE? Fail by mel byt asi nekde jinde nez ve stavu, ne?
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

      atcd_atc_set_defaults(&atcd.at_cmd);
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
      atcd.gps.last_nmea_time = atcd_get_ms();
      atcd.gps.stat.start_time = atcd_get_ms();
      atcd.gps.stat.first_search = 1;           //TODO tohle se melo nastavovat jen pri prvni hledani, pokud uz jednou signal mela, tak 0!
      atcd.sleep_disable = 1;     //s uspanym modemem nechodi data od GPS, zapnout trvale probuzeni
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

      atcd_atc_set_defaults(&atcd.at_cmd);
      atcd.at_cmd.timeout = 5000;

    case ATCD_SB_GPS_STOP + 1:
      atcd_atc_exec_cmd(&atcd.at_cmd, "AT+CGNSPWR=0\r\n");
    case ATCD_SB_GPS_STOP + 2:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_GPS_STOP + 2;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_GPS_STOP + ATCD_SO_ERR;
      ATCD_DBG_GPS_DIABLED
      atcd.gps.state = ATCD_GPS_STATE_OFF;
      atcd.gps.stat.run_time_acc += atcd_get_ms() - atcd.gps.stat.start_time;  //melo by se aktualizovat prubezne...
      atcd.sleep_disable = 0;     //s uspanym modemem nechodi data od GPS, vypnuti trvaleho probuzeni
      return ATCD_SB_GPS_STOP + ATCD_SO_END;

    case ATCD_SB_GPS_STOP + ATCD_SO_ERR:
      //GPS disabling fail
      //Zalogovat!

    case ATCD_SB_GPS_STOP + ATCD_SO_END:
    //------------------------------------------------------------------------
    // BLE START
    //------------------------------------------------------------------------
	case ATCD_SB_BLE:

	if(atcd.ble.init != 0b01111111){
		return ATCD_SB_BLE + ATCD_SO_END;
	}

	if(atcd.ble.state == ATCD_BLE_STATE_TO_ON){
		return ATCD_SB_BLE + 1;
	}
	else if(atcd.ble.state == ATCD_BLE_STATE_TO_OFF){
		return ATCD_SB_BLE + 3;
	}
	else return ATCD_SB_BLE + 10;

	case ATCD_SB_BLE + 1:
	atcd_atc_exec_cmd(&atcd.at_cmd, "AT+BLESLSTART=1\r\n");
	case ATCD_SB_BLE + 2:
		if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_BLE + 2;
		if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_BLE + ATCD_SO_ERR;
		atcd.ble.state = ATCD_BLE_STATE_ON;
		return ATCD_SB_BLE + 10;

	case ATCD_SB_BLE + 3:
	atcd_atc_exec_cmd(&atcd.at_cmd, "AT+BLESLSTOP=1\r\n");
	case ATCD_SB_BLE + 4:
		if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_BLE + 4;
		if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_BLE + ATCD_SO_ERR;
		atcd.ble.state = ATCD_BLE_STATE_OFF;

	case ATCD_SB_BLE + 10:
	if(atcd.ble.state != ATCD_BLE_STATE_ON){
		return ATCD_SB_BLE + ATCD_SO_END;
	}

	if(atcd.ble.devname_state == ATCD_BLE_DEVNAME_STATE_UPDATED){
		return ATCD_SB_BLE + 20;
	}

	atcd.ble.devname_state = ATCD_BLE_DEVNAME_STATE_UPDATED;
	atcd_atc_exec_cmd(&atcd.at_cmd, "AT+BLESLSTOP=1\r\n");
	case ATCD_SB_BLE + 11:
	  if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_BLE + 11;
	  if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_BLE + ATCD_SO_ERR;
	  atcd.ble.state = ATCD_BLE_STATE_OFF;

    sprintf(atcd.at_cmd_buff, "AT+BTHOST=\"%s\"\r\n", atcd.ble.device_name);
	atcd_atc_exec_cmd(&atcd.at_cmd, atcd.at_cmd_buff);
	case ATCD_SB_BLE + 12:
	  if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_BLE + 12;
	  if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_BLE + ATCD_SO_ERR;

	atcd_atc_exec_cmd(&atcd.at_cmd, "AT+BLESLSTART=1\r\n");
	case ATCD_SB_BLE + 13:
	  if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_BLE + 13;
	  if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_BLE + ATCD_SO_ERR;
	  atcd.ble.state = ATCD_BLE_STATE_ON;


	case ATCD_SB_BLE + 20:
		if(atcd.ble.response_state == ATCD_BLE_RESP_STATE_COMM){
			return ATCD_SB_BLE + ATCD_SO_END;
		}
		
//	atcd_atc_exec_cmd(&atcd.at_cmd, "AT+BLESRSP=1\r\n");
//		case ATCD_SB_BLE + 7:
//		  if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_BLE + 7;
//		  if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_BLE + ATCD_SO_ERR;

	sprintf(atcd.at_cmd_buff, "AT+BLESIND=2,\"%s\"\r\n",
				  atcd.ble.resp_buff);
	  atcd_atc_exec_cmd(&atcd.at_cmd, atcd.at_cmd_buff);
//	case ATCD_SB_BLE + 8:
//	  if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_BLE + 8;
//	  if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_BLE + ATCD_SO_ERR;
	  atcd.ble.response_state = ATCD_BLE_RESP_STATE_COMM;

	case ATCD_SB_BLE + ATCD_SO_ERR:

	case ATCD_SB_BLE + ATCD_SO_END:
	
	
	
	
	    case ATCD_SB_SELFCHECK:
    {
      if (atcd.selfcheck_state!=atcd_selfcheck_stateBUSY)
        return ATCD_SB_SELFCHECK + ATCD_SO_END;
      atcd_reg_state_e reg=atcd_gsm_state();
      atcd_sim_state_e sim=atcd_sim_state();
      /*  sim  reg  gprs
          none off  ...    go
          none sear ...    stop                       x
          none emer ...    go
          ok   off  ...    go
          ok   sear ...    stop                       x
          ok   home cing   stop                        x
          ok   home coed   go
          ok   home ding   stop
          ok   home died   go
          unk  ...  ...    stop                       x
          wait ...  ...    stop                       x
          pin,puk,err = none
               roam = home
               other = home?
       */
      /*
      if(sim==ATCD_SIM_STATE_UNKNOWN || sim==ATCD_SIM_STATE_WAIT ||
         reg==ATCD_REG_STATE_SEARCHING ||
         (sim==ATCD_SIM_STATE_OK &&  //unsafe
          (reg!=ATCD_REG_STATE_OFF && reg!=ATCD_REG_STATE_EMERGENCY) && //not safe
          (atcd.gprs.state == ATCD_GPRS_STATE_CONNECTING || atcd.gprs.state == ATCD_GPRS_STATE_DISCONNING))) //unsafe
      AT+CGATT=1 is inside ATCD_SB_GPRS_INIT, not now
      */
      if (sim==ATCD_SIM_STATE_UNKNOWN || sim==ATCD_SIM_STATE_WAIT ||
          reg==ATCD_REG_STATE_SEARCHING)
        return ATCD_SB_SELFCHECK + ATCD_SO_END;
      atcd_atc_set_defaults(&atcd.at_cmd);

      atcd.stat.selftest_run++;
      atcd_atc_exec_cmd(&atcd.at_cmd, "at+fsflsize=Z:\\NVRAM\\NVD_DATA\\MT6B_010\r");
    }
    case ATCD_SB_SELFCHECK + 1:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_SELFCHECK + 1;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_SELFCHECK + ATCD_SO_ERR;
      // +FSFLSIZE: 1854
      if(atcd.at_cmd.resp_len != 0 && strncmp(atcd.at_cmd.resp, "+FSFLSIZE: 1854", 15) != 0)
      {
        atcd_dbg_err("atcd", "selfcheck 1: bad size");
        return ATCD_SB_SELFCHECK + 3;
      };

      //fsread jde s uvozovkami i bez
      atcd_atc_exec_cmd(&atcd.at_cmd, "at+fsread=Z:\\NVRAM\\NVD_DATA\\MT6B_010,1,64,900\r");
      expecting_binary=64;
    case ATCD_SB_SELFCHECK + 2:
    {
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_SELFCHECK + 2;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK) return ATCD_SB_SELFCHECK + ATCD_SO_ERR;
      char tmp[65];
      int zeros=0, resp_len=atcd.at_cmd.resp_len;
      if (resp_len==64)
      {
        int i;
        for (i=0; i<resp_len; i++)
          if (atcd.at_cmd.resp[atcd.parser.line_pos+i]==0)
            zeros++;
      };

      /*static uint32_t error_simulator=0;
      if (error_simulator++==4)
        zeros=100;*/

      /*snprintf(tmp, sizeof(tmp), "selfcheck 2 got %d bytes, %d zeros, at_res=%d sta=%d\r\n",
          resp_len, zeros, atcd.at_cmd.result, atcd.at_cmd.state); //res 1=OK, sta 0=DONE
      atcd_dbg_inf3("atcd", tmp);*/

      if (resp_len!=64)
      {
        atcd_dbg_err("atcd", "selfcheck 2: not 64");
        return ATCD_SB_SELFCHECK + ATCD_SO_ERR;
      };


      if (zeros<50) //64 po poruse, 5 po oprave
        return ATCD_SB_SELFCHECK + 5;
      snprintf(tmp, sizeof(tmp), "selfcheck 2 got %d bytes, %d zeros, at_res=%d sta=%d\r\n",
          resp_len, zeros, atcd.at_cmd.result, atcd.at_cmd.state); //res 1=OK, sta 0=DONE
      atcd_dbg_warn("atcd", tmp);
    }
    case ATCD_SB_SELFCHECK + 3:
      atcd.stat.selftest_wrong++;
      //fsdel jde s uzovkami i bez
      atcd_atc_exec_cmd(&atcd.at_cmd, "at+fsdel=Z:\\NVRAM\\NVD_DATA\\MT6B_010\r");
      //atcd_atc_exec_cmd(&atcd.at_cmd, "at+fsdel=\"Z:\\NVRAM\\NVD_DATA\\MT6B_010\"\r");
      //atcd_atc_exec_cmd(&atcd.at_cmd, "at+fsdel=\"Z:\\NVRAM\\NVD_DATA\\xxx\"\r");
    case ATCD_SB_SELFCHECK + 4:
      if(atcd.at_cmd.state != ATCD_ATC_STATE_DONE) return ATCD_SB_SELFCHECK + 4;
      if(atcd.at_cmd.result != ATCD_ATC_RESULT_OK && atcd.at_cmd.result != ATCD_ATC_RESULT_ERROR) return ATCD_SB_SELFCHECK + ATCD_SO_ERR;
      atcd.reset_needed=1;
      atcd.selfcheck_state=atcd_selfcheck_stateWRONG;
      return ATCD_SB_SELFCHECK + ATCD_SO_END;

    case ATCD_SB_SELFCHECK + 5:
      atcd.selfcheck_state=atcd_selfcheck_stateOK;
      return ATCD_SB_SELFCHECK + ATCD_SO_END;

    case ATCD_SB_SELFCHECK + ATCD_SO_ERR:
      atcd_dbg_warn("atcd", "failed selfcheck");
      atcd.stat.selftest_fail++;
      atcd.selfcheck_state=atcd_selfcheck_stateFAILED;

    case ATCD_SB_SELFCHECK + ATCD_SO_END:
      if (atcd.sim.state == ATCD_SIM_STATE_NONE) //kdyz jsem si sem jenom odskocil z INIT protoze neni SIM
        return ATCD_SB_INIT;	
      //------------------------------------------------------------------------
      // END
      //------------------------------------------------------------------------
    case ATCD_SB_END:
      //Konec, navrat na pocatek...
      atcd.stat.full_cycles++;
      return ATCD_SB_SETUP;
      //------------------------------------------------------------------------
    default:
      //Chyba, logovat
      ATCD_DBG_SW_ERR
      return ATCD_SB_INIT;
  }
}

void atcd_proc_linepreview()
{
  if (atcd.conns.awaitingC5__)
  {
    if (strncmp(atcd.parser.buff + atcd.parser.line_pos, "STATE: ", 7) == 0)
    {
      atcd.parser.buff[atcd.parser.buff_pos]=0;
      strncpy(atcd.conns.parsedCipStatus.state, atcd.parser.buff + atcd.parser.line_pos+6, sizeof(atcd.conns.parsedCipStatus.state));
    }
    else if (strncmp(atcd.parser.buff + atcd.parser.line_pos, "C: ", 3) == 0)
    {
      const char *ptr=atcd.parser.buff + atcd.parser.line_pos+3;
      int ch=*ptr-'0';
      if (ch>=0 && ch<sizeof(atcd.conns.parsedCipStatus.chans)/sizeof(atcd.conns.parsedCipStatus.chans[0]))
      {
        atcd.parser.buff[atcd.parser.buff_pos]=0;
        strncpy(atcd.conns.parsedCipStatus.chans[ch], ptr+2, sizeof(atcd.conns.parsedCipStatus.chans[ch]));
      };
    }
    //necham v atcd_conn.c
    //if (strncmp(atcd.parser.buff + atcd.parser.line_pos, "C: 5,", strlen("C: 5,")) == 0)
    //  atcd.conns.awaitingC5__=0;;
  };

  if (expecting_binary>0)
  {
    if (strncmp(atcd.parser.buff + atcd.parser.line_pos, "at+fsread=", 10) == 0)
      atcd_parser_expect_binary(expecting_binary);
    else
    {
      atcd_dbg_err2("sim868-bin", atcd.parser.buff);
    }
    expecting_binary=0;
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
    rbuff_lin_space(&atcd.tx_rbuff, (uint8_t*)"AT+CFUN=1,1\r\n", len);

    //atcd.parser.timer = atcd_get_ms();
    atcd_hw_tx(&atcd.tx_rbuff, len);
}
//------------------------------------------------------------------------------
#endif /* ATCD_SIM868 */
//------------------------------------------------------------------------------
