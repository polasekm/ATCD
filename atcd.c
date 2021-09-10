/*
 * usart.c
 *
 *  Created on: Apr 9, 2011
 *      Author: Martin Polasek
 */
//------------------------------------------------------------------------------
#include "atcd.h"

atcd_t atcd;

extern rbuff_t atcd_rx_ring_buff;         //kruhovy buffer pro prijimana data

//--------------------------------
// Ven!!!!!
extern uint32_t ccl_atcd_rx_ov;
extern uint32_t ccl_atcd_restart;
//--------------------------------
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
  memset(&atcd.errors, 0x00, sizeof(atcd.errors));



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

  atcd_sw_reset();
  atcd_hw_reset();
  //atcd_hw_igt();

  atcd.timer = atcd_get_ms();
  atcd.state = ATCD_STATE_STARTING;
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
  atcd.tx_conn_num = 0;
  atcd.tx_data_len = 0;
  rbuff_init(&atcd.tx_rbuff, NULL, 0);

  atcd_parser_init();

  atcd_atc_init(&atcd.at_cmd);
  atcd.at_cmd_buff[0]        = 0;
  atcd.at_cmd_result_buff[0] = 0;
  rbuff_init(&atcd.at_cmd_data, NULL, 0);

  atcd.proc_step = 0;
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
void atcd_proc()                         //data processing
{
  atcd_hw_proc();
  atcd_atc_proc();                       //AT commands processing
  // mozna radku vyse prohodit - atc se pred echem musi prenastavit
  // docasne opraveno takto, dole byt take musi - po prijmu se musi reagovat na pripadne udalosti
  // potiz je s dokoncenym vysilanim, kdy tak separovat...

  atcd_rx_proc();                        //Zpracovani prijatych dat
  atcd_parser_proc();                    //Parser processing
  atcd_atc_proc();                       //AT commands processing 

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
      // Modemy, ktere nehlasi svuj start
      atcd.state = ATCD_STATE_NO_INIT;
      atcd.timer = atcd_get_ms();
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

    atcd.proc_step = atcd_proc_step();     // Provede krok sekvencniho automatu
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

    atcd.proc_step = atcd_proc_step();     // Provede krok sekvencniho automatu

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
    atcd_rx_ch((char) ch);
    n = rbuff_read_b(&atcd_rx_ring_buff, &ch);
  }
}
//------------------------------------------------------------------------------
void atcd_rx_ch(char ch)
{
  atcd_at_cmd_t *at_cmd;
  
  //TODO: Co to je?
  /*if (((atcd.parser.mode==ATCD_P_MODE_IPD) || (atcd.parser.mode == ATCD_P_MODE_IPD_WAITOK) || (atcd.parser.mode == ATCD_P_MODE_IPD_SLEEP)) &&
      (atcd.parser.rx_conn_num<ATCD_CONN_MAX_NUMBER) &&
      (atcd.conns.conn[atcd.parser.rx_conn_num]!=NULL) &&
      (atcd.conns.conn[atcd.parser.rx_conn_num]->dontPrint))
  { }
  else*/
  atcd_dbg_in(&ch, 1);                           // Logovani prijatych dat

  //sem mozna navratit if na stav parseru a pak mozne zpracovani dat...
  if(atcd_conn_data_proc(ch) != 0) return;       // Zpracovani prichozich dat TCP/UDP spojeni
  //sem mozna navratit if na stav parseru a pak mozne zpracovani dat...
  if(atcd_phone_sms_proc(ch) != 0) return;      // Zpracovani prichozich dat prijimane SMS zpravy
  //---------------------------------------
  // Test volneho mista v bufferu
  if(atcd.parser.buff_pos >= ATCD_P_BUFF_SIZE - 1)
  {
    ATCD_DBG_BUFF_OVERRUN
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
  if(ch != '\n')
  {
    if(atcd_conn_ipd_tst() != 0) return;
    if(atcd_atc_prompt_tst() != 0) return;
    return;
  }
  //------------------------------
  // Jedna se o konec radku
  //------------------------------
  // Tohle nebude fungovat --- !
  // Je potreba volat jen pokud doslo skutecne k detekci, neboi je to ve stavu mimo ATC...
  //if(atcd.callback != NULL && (atcd.cb_events & ATCD_EV_ASYNC_MSG) != 0) atcd.callback(ATCD_EV_ASYNC_MSG);

  if(atcd_conn_asc_msg() != 0) return;  // Zpracovani TCP/UDP spojeni
  if(atcd_wifi_asc_msg() != 0) return;  // Zpracovani udalosti WLAN
  if(atcd_gsm_asc_msg() != 0) return;   // Zpracovani udalosti GSM site
  if(atcd_phone_asc_msg() != 0) return; // Zpracovani udalosti telefonu

  if(atcd_gps_asc_msg() != 0) return;   // Zpracovani udalosti GPS
  //------------------------------
  // Zpracovani startovaci sekvence
  //------------------------------
  if(strncmp(atcd.parser.buff + atcd.parser.line_pos, ATCD_STR_START_SEQ, strlen(ATCD_STR_START_SEQ)) == 0)
  {
    ATCD_DBG_BOOT_SEQ
    atcd.state = ATCD_STATE_NO_INIT;
    atcd.parser.buff_pos = atcd.parser.line_pos;
    atcd_state_reset();
    atcd.proc_step = 0; //Asi je dublovano, nastavi se uz v resetu
    atcd.err_max   = 5;
    if(atcd.callback != NULL && (atcd.cb_events & ATCD_EV_STATE) != 0) atcd.callback(ATCD_EV_STATE);
    return;
  }
  //------------------------------
  // Aktualizace zacatku posledniho radku
  // Zalezi, jesli se zrovna zpracovava nejaky AT prikaz
  // Pokud ano, drzi se buffer a posouvaji radky, jinak muzeme resetovat na zacatek
  //------------------------------
  if(atcd_atc_ln_proc() != 0) return;   // Zpracovani AT prikazu
  //------------------------------
  // Callback pro zpracovani nezpracovanych nevyzadanych zprav
  if(atcd.callback != NULL && (atcd.cb_events & ATCD_EV_ASYNC_MSG) != 0) atcd.callback(ATCD_EV_ASYNC_MSG);
  //------------------------------
  atcd.parser.buff_pos = 0;
  atcd.parser.line_pos = 0;
}
//------------------------------------------------------------------------------
uint8_t atcd_state()
{
  return atcd.state;
}
//------------------------------------------------------------------------------
