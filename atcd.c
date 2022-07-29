/*
 * usart.c
 *
 *  Created on: Apr 9, 2011
 *      Author: Martin Polasek
 */
//------------------------------------------------------------------------------
#include "atcd.h"

#define DEBUG_TX_COMPLETE 0
atcd_t atcd;

extern rbuff_t atcd_rx_ring_buff;         //kruhovy buffer pro prijimana data

//------------------------------------------------------------------------------
void atcd_rx_proc();                      //Zpracovani prijatych dat
void atcd_proc_ch(char ch);               //Zpracovani prijateho znaku

void atcd_state_reset();                  //Reset stavoveho automatu ATCD

uint16_t atcd_proc_step();                //pruchod jednim krokem kolecka modemu
//--------------------------------------------------------------

//------------------------------------------------------------------------------
void atcd_init()                          //init AT command device
{
  atcd_hw_init();                         //HW init

  atcd.state      = ATCD_STATE_OFF;
  atcd.sleep_mode = ATCD_SM_OFF;

  atcd.cb_events  = ATCD_EV_ALL;
  atcd.callback   = NULL;

  atcd.stat.rx_ov = 0;
  atcd.stat.reset = 0;
  atcd.stat.warn = 0;
  atcd.stat.err = 0;

  atcd.stat.wake_time = 0;
  atcd.stat.awake_time_acc = 0;

  atcd.stat.echo_bad = 0;
  atcd.stat.echo_uns = 0;

  memset(&atcd.profiler, 0x00, sizeof(atcd.profiler));

  atcd_sim_init();
  atcd_gsm_init();
  atcd_phone_init();
  atcd_gprs_init();
  atcd_gps_init();
  atcd_wifi_init();

  atcd_state_reset();
}
//------------------------------------------------------------------------------
void atcd_start()               //Spusteni zarizeni
{
  ATCD_DBG_STARTING
  atcd.stat.reset++;
  atcd_state_reset();

  atcd_hw_pwr(ATCD_PWR_ON);

	#if(ATCD_USE_DEVICE == ATCD_SIM868 || ATCD_USE_DEVICE == ATCD_SIM7000)
		atcd_hw_reset();
	#elif
		atcd_hw_igt();
	#endif

  atcd.timer = atcd_get_ms();
  atcd.state = ATCD_STATE_STARTING;
}
//------------------------------------------------------------------------------
void atcd_reset()               //Reset zarizeni
{
  ATCD_DBG_RESET
  atcd_state_reset();

  //atcd_sw_reset();
  atcd_hw_reset();
  //atcd_hw_igt();

  atcd.timer = atcd_get_ms();
  atcd.state = ATCD_STATE_STARTING;
}
//------------------------------------------------------------------------------
void atcd_begin()
{
  if (atcd.state != ATCD_STATE_STARTING) //nebo ==NO_INIT?
  {
    //zakomentuju nez vymyslim co s RDY po startu
    //normalne se startuje po H na M_STAT_Pin
    //atcd_dbg_err("atcd_begin: ", "not in STARTING");
  }
  else
  {
    atcd.state = ATCD_STATE_NO_INIT;
    atcd.parser.buff_pos = atcd.parser.line_pos;
    atcd_state_reset();
    atcd.proc_step = 0; //Asi je dublovano, nastavi se uz v resetu
    atcd.proc_step_initfailed = 0;
    atcd.err_max   = 5;
    if(atcd.callback != NULL && (atcd.cb_events & ATCD_EV_STATE) != 0) atcd.callback(ATCD_EV_STATE);
  }
}
//------------------------------------------------------------------------------
void atcd_state_reset()                  //state machine reset
{
  atcd_conns_reset();
  atcd_atc_cancel_all();

  //atcd.powersave_act = -1;
  atcd.timer = atcd_get_ms();

  atcd.tx_state    = ATCD_P_TX_COMPLETE;
  //atcd.tx_pending  = 0;
  //atcd.tx_conn_num = 0;
  atcd.tx_data_len = 0;
  rbuff_init(&atcd.tx_rbuff, NULL, 0);

  atcd_parser_init();

  atcd_atc_init(&atcd.at_cmd);
  atcd.at_cmd_buff[0]        = 0;
  atcd.at_cmd_result_buff[0] = 0;
  rbuff_init(&atcd.at_cmd_data, NULL, 0);

  atcd.proc_step = 0;
  atcd.proc_step_initfailed = 0;
  atcd.err_cnt   = 0;
  atcd.err_max   = 10;

  atcd.sleep_state = ATCD_SS_SLEEP;
  atcd.sleep_disable = 0;
  atcd.sleep_timer = 0;

  atcd_sim_reset();
  atcd_gsm_reset();
  atcd_phone_reset();
  atcd_gprs_reset();
  atcd_gps_reset();
  atcd_wifi_reset();

  if(atcd.callback != NULL && (atcd.cb_events & ATCD_EV_STATE_RESET) != 0) atcd.callback(ATCD_EV_STATE_RESET);
}
//------------------------------------------------------------------------------
/*void atcd_set_powersave(atcd_powersave_req_t mode)          //set sleep mode
{
  atcd.powersave_req = mode;
}
//------------------------------------------------------------------------------
void atcd_set_powersave_hwsetter(void (*powersave_hwsetter)(uint8_t awake))
{
  atcd.powersave_hwsetter=powersave_hwsetter;
}*/
//------------------------------------------------------------------------------
void atcd_set_sleep_mode(atcd_sleep_mode_t mode)
{
  atcd.sleep_mode = mode;
}
//------------------------------------------------------------------------------
atcd_sleep_mode_t atcd_sleep_mode()
{
  return atcd.sleep_mode;
}
//------------------------------------------------------------------------------
void atcd_set_system_callback(uint8_t eventmask, void (*system_callback)(uint8_t event))
{
  atcd.cb_events = eventmask;
  atcd.callback = system_callback;
}
//------------------------------------------------------------------------------
#if DEBUG_TX_COMPLETE
uint32_t rxed1, rxed2;
atcd_tx_state_t txst1, txst2;
uint8_t echo1, echo2;
int atst1, atst2;
#endif

void atcd_proc()                         //data processing
{
  atcd_hw_proc();
#if DEBUG_TX_COMPLETE
  rxed1=rbuff_size(&atcd_rx_ring_buff); txst1=atcd.tx_state; echo1=atcd.parser.echo_en;
  if (atcd.parser.at_cmd_top)
    atst1=atcd.parser.at_cmd_top->state;
  else
    atst1=12345;
#endif
  //sel do atcd_rx_proc   atcd_atc_proc(0);                       //AT commands processing
  // mozna radku vyse prohodit - atc se pred echem musi prenastavit
  // docasne opraveno takto, dole byt take musi - po prijmu se musi reagovat na pripadne udalosti
  // potiz je s dokoncenym vysilanim, kdy tak separovat...
#if 0
  rxed2=rbuff_size(&atcd_rx_ring_buff);  txst2=atcd.tx_state; echo2=atcd.parser.echo_en;
  if (atcd.parser.at_cmd_top)
    atst2=atcd.parser.at_cmd_top->state;
  else
    atst2=12345;
#endif
  atcd_rx_proc();                        //Zpracovani prijatych dat
  atcd_parser_proc();                    //Parser processing
  atcd_atc_proc(1);                       //AT commands processing

  //----------------------------------------------
  if(atcd.state == ATCD_STATE_STARTING)
  {
     // Test timeoutu v rezimu startu modemu
    if(atcd_get_ms() - atcd.timer > 10000)
    {
      // Vyprsel cas na start
      ATCD_DBG_START_TIM
      atcd_reset();
      return;
    }

    #if(ATCD_USE_DEVICE == ATCD_SIM868 || ATCD_USE_DEVICE == ATCD_SIM7000)
      // Modemy, ktere nehlasi svuj start...
      // ...cekaji na atcd_begin() (dokonceni IGNITION) atcd.state = ATCD_STATE_NO_INIT_;
      // atcd.timer = atcd_get_ms();
    #endif
  }
  else if(atcd.state == ATCD_STATE_NO_INIT)
  {
    // Provadime inicializacni sekvenci
    if(atcd.err_cnt >= atcd.err_max)
    {
      // Prekrocen maximalni pocet chyb v inicializace
      //ZALOGOVAT!
      //atcd_reset();
      //return;
    }

    if(atcd_get_ms() - atcd.timer > 20000)
    {
      // Vyprsel cas na inicializaci
      ATCD_DBG_INIT_TIM;
      atcd_reset();
      return;
    }

    uint16_t prev_step=atcd.proc_step;
    atcd.proc_step = atcd_proc_step();     // Provede krok sekvencniho automatu
    if (atcd.proc_step==ATCD_SB_INIT + ATCD_SO_ERR)
      atcd.proc_step_initfailed=prev_step;
  }
  //else if(atcd.state == ATCD_STATE_ON || atcd.state == ATCD_STATE_SLEEP)
  else if(atcd.state == ATCD_STATE_ON)
  {
    // Zarizeni je pripraveno k praci, pripadne spi...
    // Pripadne testy stavu a dalsi cinnosti async vuci sekvencnimu atomatu

    if(atcd.err_cnt >= atcd.err_max)
    {
      // Prekrocen maximalni pocet chyb v sekvencnim automatu
      //ZALOGOVAT!
      atcd_reset();
      return;
    }

    uint16_t prev_step=atcd.proc_step;
    atcd.proc_step = atcd_proc_step();     // Provede krok sekvencniho automatu
    if (atcd.proc_step==ATCD_SB_INIT + ATCD_SO_ERR)
      atcd.proc_step_initfailed=prev_step;

    atcd_gsm_proc();
    atcd_phone_proc();                     //phone processing
    atcd_gprs_proc();                      //gprs processing
    atcd_conn_proc();                      //connections processing  
    atcd_gps_proc();                       //gps processing
    atcd_wifi_proc();
  }
}
//------------------------------------------------------------------------------
void atcd_rx_data(uint8_t *data, uint16_t len)
{
  uint16_t i;

  for(i = 0; i < len; i++)
  {
    atcd_rx_ch(*(char*)(data++));
  }
}
//------------------------------------------------------------------------------
void atcd_rx_str(char *ch)
{
  while(*ch != 0)
  {
    atcd_rx_ch(*(ch++));
  }
}
//------------------------------------------------------------------------------
void atcd_rx_proc()                       //proc rx data 
{
  uint16_t n;
  uint8_t ch;

  // Pozor, hrozi ztrata data pokud atc / spojnei nemely vlastni buffer rbuff se nastradalo mnoho dat, zpracuje nekoklik dotazu v tomto jednom volani

  n = rbuff_read_b(&atcd_rx_ring_buff, &ch);
  while(n == 1)
  {
    atcd_atc_proc(0); //protoze behem atcd_rx_ch se muze 1) zacit at_cmd 2) dokoncit odesilani
    atcd_rx_ch((char) ch);
    n = rbuff_read_b(&atcd_rx_ring_buff, &ch);
  }
}
//------------------------------------------------------------------------------
void atcd_rx_ch(char ch)
{
  uint32_t tick_start=atcd_get_ms();
  uint32_t time_dbgin, time_dataproc;
  atcd_at_cmd_t *at_cmd;

  static uint8_t dbg_fejla=0;
  if (atcd.parser.at_cmd_top)
    if(atcd.parser.at_cmd_top->state == ATCD_ATC_STATE_TX && atcd.tx_state == ATCD_P_TX_COMPLETE)
    {
      if (dbg_fejla==0)
      {
        #if DEBUG_TX_COMPLETE
        char tmps[100];
        snprintf(tmps, sizeof(tmps), "complete but not: {%u %d %d %d} -> {%u %d %d %d} -> {%u %d %d %d}\r\n",
            (unsigned int)rxed1, txst1, echo1, atst1,
            (unsigned int)rxed2, txst2, echo2, atst2,
            (unsigned int)rbuff_size(&atcd_rx_ring_buff), atcd.tx_state, atcd.parser.echo_en, atcd.parser.at_cmd_top->state);
        atcd_dbg_warn("ATCD rx: ", tmps);
        #else
        atcd_dbg_warn("ATCD rx: ", "complete but not");
        // [1081.189] AT+GMR
        // [1081.190] $G[1081.190] ATCD rx: complete but not[1081.191] P[1081.191] ATCD: ATC: Odesilani bylo dokoceno - prechazime na W_ECHO...
        // [1081.192] GSA,A,1,,,,,,,,,,,,,,,*1E
        #endif
      }
      dbg_fejla=2;
      __asm volatile ("nop"); //nevim v jakem #include je definovany __NOP(); //prejit do state == ATCD_ATC_STATE_W_ECHO ?
    }
  if (dbg_fejla>0)
    dbg_fejla--;

  //TODO: Co to je? Odpoved: nevypisuj na terminal stahovani firmware
  if ((atcd.parser.mode==ATCD_P_MODE_IPD) &&
      (atcd.parser.rx_conn_num<ATCD_CONN_MAX_NUMBER) &&
      (atcd.conns.conn[atcd.parser.rx_conn_num]!=NULL) &&
      (atcd.conns.conn[atcd.parser.rx_conn_num]->dontPrint))
  { }
  else
    atcd_dbg_in(&ch, 1);                           // Logovani prijatych dat
  time_dbgin=atcd_get_ms();
  //sem mozna navratit if na stav parseru a pak mozne zpracovani dat...
  if ((atcd_conn_data_proc(ch) != 0) ||       // Zpracovani prichozich dat TCP/UDP spojeni
  //sem mozna navratit if na stav parseru a pak mozne zpracovani dat...
      (atcd_phone_sms_proc(ch) != 0))         // Zpracovani prichozich dat prijimane SMS zpravy
  {
    atcd.profiler.return1+=(atcd_get_ms()-tick_start);
    return;
  }
  //---------------------------------------
  // Test volneho mista v bufferu
  if(atcd.parser.buff_pos >= ATCD_P_BUFF_SIZE - 1)
  {
    ATCD_DBG_BUFF_OVERRUN
    atcd.stat.rx_ov++;
    at_cmd = atcd.parser.at_cmd_top;
    //nemely by se resit i jine stavy?
    if(at_cmd != NULL && at_cmd->state == ATCD_ATC_STATE_W_END)
    {
      ATCD_DBG_ATC_LN_BUFF_OV
      //ten priznak se ztrati
      //at_cmd->resp           = NULL;  //tohle je ultra nebezpecne!
      at_cmd->resp_len       = 0;
      at_cmd->resp_buff_size = 0;

      // pozor, atc bude dale pokracovat, jak se pak se pak zjisti ze doslo k chybe?

      if(at_cmd->callback != NULL && (at_cmd->cb_events & ATCD_ATC_EV_OVERRUN) != 0) at_cmd->callback(ATCD_ATC_EV_OVERRUN);
    }

    atcd.parser.buff_pos = 0;
    atcd.parser.line_pos = 0;
  }
  //------------------------------
  // Ulozi prijaty znak do bufferu
  // Do ATC se pak pripadne memcopy na konci radku po prislusnych testech
  // Radek muze byt vymaskovan jako async zprava...
  atcd.parser.buff[atcd.parser.buff_pos++] = ch;
  //------------------------------
  // Pokud se nejedna o konec radku
  //------------------------------
  time_dataproc=atcd_get_ms();
  if(ch != '\n')
  {
    if ((atcd_conn_ipd_tst() != 0) ||
        (atcd_atc_prompt_tst() != 0))
    { }
    atcd.profiler.dbgin+=time_dbgin-tick_start;         //48
    atcd.profiler.dataproc+=time_dataproc-tick_start;   //57
    atcd.profiler.return2+=(atcd_get_ms()-tick_start);  //61
    return;
  }
  //------------------------------
  // Jedna se o konec radku
  //------------------------------
  // Tohle nebude fungovat --- !
  // Je potreba volat jen pokud doslo skutecne k detekci, neboi je to ve stavu mimo ATC...
  //if(atcd.callback != NULL && (atcd.cb_events & ATCD_EV_ASYNC_MSG) != 0) atcd.callback(ATCD_EV_ASYNC_MSG);

  if ((atcd_conn_asc_msg() != 0) ||  // Zpracovani TCP/UDP spojeni
      (atcd_wifi_asc_msg() != 0) ||  // Zpracovani udalosti WLAN
      (atcd_gsm_asc_msg() != 0) ||   // Zpracovani udalosti GSM site
      (atcd_phone_asc_msg() != 0) || // Zpracovani udalosti telefonu
      (atcd_gps_asc_msg() != 0))   // Zpracovani udalosti GPS
  {
    atcd.profiler.return3+=(atcd_get_ms()-tick_start);
    return;
  }
  //------------------------------
  // Zpracovani startovaci sekvence
  //------------------------------
  if (atcd.parser.buff_pos>=atcd.parser.line_pos+strlen(ATCD_STR_START_SEQ))
    if(strncmp(atcd.parser.buff + atcd.parser.line_pos, ATCD_STR_START_SEQ, strlen(ATCD_STR_START_SEQ)) == 0)
    {
      ATCD_DBG_BOOT_SEQ

      if (atcd.state != ATCD_STATE_STARTING && atcd.state != ATCD_STATE_NO_INIT)
      {
        atcd.timer = atcd_get_ms();
        atcd.state = ATCD_STATE_STARTING;
        atcd_dbg_err("RDY", "Prislo necekane RDY");
      };
      atcd_begin();
      //RDY se musi zahodit z bufferu return;
    }
  //------------------------------
  // Aktualizace zacatku posledniho radku
  // Zalezi, jesli se zrovna zpracovava nejaky AT prikaz
  // Pokud ano, drzi se buffer a posouvaji radky, jinak muzeme resetovat na zacatek
  //------------------------------
  if(atcd_atc_ln_proc() != 0)   // Zpracovani AT prikazu
  {
    atcd.profiler.return4+=(atcd_get_ms()-tick_start);
    return;
  }
  //------------------------------
  // Callback pro zpracovani nezpracovanych nevyzadanych zprav
  if(atcd.callback != NULL && (atcd.cb_events & ATCD_EV_ASYNC_MSG) != 0 && (atcd.parser.buff_pos>2+atcd.parser.line_pos)) atcd.callback(ATCD_EV_ASYNC_MSG);
  //------------------------------
  atcd.parser.buff_pos = 0;
  atcd.parser.line_pos = 0;
  atcd.profiler.return5+=(atcd_get_ms()-tick_start);
}
//------------------------------------------------------------------------------
uint8_t atcd_state()
{
  return atcd.state;
}
//------------------------------------------------------------------------------
uint32_t atcd_awaketime()
{
  uint32_t awake_time;

  awake_time = atcd.stat.awake_time_acc;
  if(atcd.sleep_state != ATCD_SS_SLEEP) awake_time += atcd_get_ms() - atcd.stat.wake_time;

  return awake_time;
}
//------------------------------------------------------------------------------
