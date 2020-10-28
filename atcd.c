/*
 * usart.c
 *
 *  Created on: Apr 9, 2011
 *      Author: Martin Polasek
 */
//------------------------------------------------------------------------------
#include "atcd.h"

atcd_t atcd;

//char *strs[NUMBER_OF_STRINGS] = {"foo", "bar", "bletch", ...};
//char *strs[4] = {"foo", "bar", "bletch", ""};

extern rbuff_t atcd_rx_ring_buff;         //kruhovy buffer pro prijimana data

//------------------------------------------------------------------------------
void atcd_rx_proc();                      //Zpracovani prijatych dat
void atcd_proc_ch(char ch);               //Zpracovani prijateho znaku

void atcd_state_reset();                  //Reset stavoveho automatu ATCD
//--------------------------------------------------------------

//------------------------------------------------------------------------------
void atcd_init()                          //init AT command device
{
  atc_dev_hw_init();                      //HW init

  atcd.state      = ATCD_STATE_OFF;
  //atcd.init_state = ATCD_INIT_NONE;            // Prekryva se se stavem inicializacni sekvence...
  atcd.cb_events  = ATCD_EV_ALL;
  atcd.callback   = NULL;

  atcd_sim_init();
  atcd_gsm_init();
  atcd_phone_init();
  atcd_gprs_init();
  atcd_gps_init();
  atcd_wifi_init();

  atcd_start();                          //Zvazit, zda nespoustte pozdeji manualne
}
//------------------------------------------------------------------------------
void atcd_state_reset()                  //state machine reset
{
  atcd_parser_init();
  atcd.timer = atcd_get_ms(); 

  atcd_atc_cancell_all();
  atcd_conn_reset_all(); 

  atcd_atc_init(&atcd.at_cmd);

  atcd_atc_seq_init(&atcd.init_seq);
  atcd.init_seq.at_cmd    = &atcd.at_cmd;
  atcd.init_seq.err_max   = 10;            //0 znamena neomezene - pozor, uint8, casem pretece - realne tedy 256, osetrit!!!!
  atcd.init_seq.make_step = &atcd_init_seq_step();              //mela by se nastavovat v init fce...

  atcd_atc_seq_init(&atcd.state_seq);
  atcd.init_seq.at_cmd    = &atcd.at_cmd;
  atcd.init_seq.err_max   = 3;            //0 znamena neomezene - pozor, uint8, casem pretece - realne tedy 256, osetrit!!!!
  atcd.init_seq.make_step = &atcd_check_state_seq_step();    //mela by se nastavovat v init fce... 

  atcd.at_cmd_buff[0] = 0;

  atcd_gsm_reset();
  atcd_phone_reset();
  atcd_gprs_reset();
  atcd_gps_reset();
  atcd_wifi_reset();
}
//------------------------------------------------------------------------------
void atcd_start()               //Spusteni zarizeni
{
  ATCD_DBG_STARTING
  atcd_state_reset();

  atcd_hw_pwr(ATCD_PWR_ON);
  atcd_hw_igt();
  
  atcd.timer = atcd_get_ms();
  atcd.state = ATCD_STATE_STARTING;
}
//------------------------------------------------------------------------------
void atcd_reset()               //Reset zarizeni
{
  // SW reset
  ATCD_DBG_RESET
  atcd_state_reset();
  
  atcd_hw_reset();
  atcd_hw_igt();
  
  atcd.timer = atcd_get_ms();
  atcd.state = ATCD_STATE_STARTING;
}
//------------------------------------------------------------------------------
void atcd_proc()               //data processing 
{
  uint32_t time_ms;
  char *cmd;

  atcd_rx_proc();                        //Zpracovani prijatych dat
  atcd_atc_proc();                       //AT commands processing 

  // -- Nepatri to do jine _proc funkce?
  // Test timeoutu v rezimu prijmu dat
  if(atcd.parser.mode == ATCD_P_MODE_IPD)
  {
    if(atcd_get_ms() - atcd.parser.timer > 4000)
    {
      // Pokud vyprsel timeout
      // Prechod parseru do rezimu AT prikazu
      ATCD_DBG_IPD_TIM
      atcd.parser.mode = ATCD_P_MODE_ATC;

      //osetrit spojeni kde dochazelo k prijmu dat...
    }
  }

  if(atcd.state == ATCD_STATE_STARTING)
  {
     // Test timeoutu v rezimu startu modemu
    if(atcd_get_ms() - atcd.timer > 10000)
    {
      // Restart modemu
      ATCD_DBG_START_TIM
      atcd_reset();
    }
  }
  else if(atcd.init_seq.state == ATCD_ATC_SEQ_STATE_WAIT)
  {
    // Inicializovat jen pokud je toho HW schopen...
    if(atcd.state == ATCD_STATE_ON || atcd.state == ATCD_STATE_SLEEP)
    {
      // Zahajime inicializacni sekvenci
      ATCD_DBG_INIT_START
      atcd_atc_seq_run(&atcd.init_seq);
    }
  }
  else if(atcd.init_seq.state == ATCD_ATC_SEQ_STATE_RUN)
  {
    // Provedeme dalsi krok inicializace
    ATCD_DBG_INIT_STEP
    atcd_atc_seq_proc(&atcd.init_seq);
  }
  else if(atcd.init_seq.state == ATCD_ATC_SEQ_STATE_ERROR)
  {
    // Znovu spustime inicializaci
    // === Doplnit restart po opakovanem selhani...
    ATCD_DBG_INIT_ERR
    atcd_atc_seq_run(&atcd.init_seq);
  }
  else if(atcd.init_seq.state == ATCD_ATC_SEQ_STATE_DONE)
  {
    // Inicializace je dokoncena
    // Pripadne testy stavu a dalsi cinnosti na pozadi...
    ATCD_DBG_INIT_OK

    if(atcd_get_ms() - atcd.timer > 7500)
    {
      // Je cast spustit kontrolu stavu modemu
      if(atcd.state_seq.state == ATCD_ATC_SEQ_STATE_WAIT)
      {
        ATCD_DBG_STAT_START

        atcd.timer = atcd_get_ms();
        atcd_atc_seq_run(&atcd.state_seq);
      }
    }

    if(atcd.state_seq.state == ATCD_ATC_SEQ_STATE_RUN)
    {
      // Provedeme dalsi krok dotazovani na stav
      ATCD_DBG_STAT_STEP
      atcd_atc_seq_proc(&atcd.state_seq);
    }
    else if(atcd.state_seq.state == ATCD_ATC_SEQ_STATE_ERROR)
    {
      // Znovu spustime dotazovani na stav
      // === Doplnit restart po opakovanem selhani...
      ATCD_DBG_STAT_ERR
      atcd.state_seq.state == ATCD_ATC_SEQ_STATE_WAIT;
    }
    else if(atcd.state_seq.state == ATCD_ATC_SEQ_STATE_DONE)
    {
      // Dotazovani na stav je hotovo
      ATCD_DBG_STAT_OK
      atcd.state_seq.state == ATCD_ATC_SEQ_STATE_WAIT;
    }

    atcd_gsm_proc();
    atcd_phone_proc();                     //phone processing
    atcd_gprs_proc();                      //gprs processing
    atcd_conn_proc();                      //connections processing  
    atcd_gps_proc();
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
  
  atcd_dbg_in(&ch, 1);                           // Logovani prijatych dat

  //sem mozna navratit if na stav parseru a pak mozne zpracovani dat...
  if(atcd_conn_data_proc(ch) != 0) return;       // Zpracovani prichozich dat TCP/UDP spojeni
  //---------------------------------------
  // Test volneho mista v bufferu
  if(atcd.parser.buff_pos >= ATCD_BUFF_SIZE - 1) 
  {
    ATCD_DBG_BUFF_OVERRUN
    at_cmd = atcd.parser.at_cmd_top;
    //nemely by se resit i jine stavy?
    if(at_cmd != NULL && at_cmd->state == ATCD_ATC_STATE_W_END)
    {
      ATCD_DBG_ATC_LN_BUFF_OV
      at_cmd->resp           = NULL;
      at_cmd->resp_len       = 0;
      at_cmd->resp_buff_size = 0;

      if(agt_cmd->callback != NULL && (at_cmd->cb_events & ATCD_ATC_EV_OVERRUN) != 0) at_cmd->callback(ATCD_ATC_EV_OVERRUN);
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
  //------------------------------
  // Zpracovani startovaci sekvence
  //------------------------------
  if(strncmp(atcd.parser.buff + atcd.parser.line_pos, ATCD_STR_START_SEQ, strlen(ATCD_STR_START_SEQ)) == 0)
  {
    ATCD_DBG_BOOT_SEQ
    atcd.state = ATCD_STATE_ON;
    atcd.parser.buff_pos = atcd.parser.line_pos;
    atcd_state_reset();
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
  atcd.parser.buff_pos = 0;
  atcd.parser.line_pos = 0;
}
//------------------------------------------------------------------------------
void atcd_tx_complete()                  //call on tx data complete
{
  atcd.parser.tx_state = ATCD_P_TX_COMPLETE;
}
//------------------------------------------------------------------------------
