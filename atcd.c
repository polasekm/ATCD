/*
 * usart.c
 *
 *  Created on: Apr 9, 2011
 *      Author: Martin
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

  atcd.state     = ATCD_STATE_OFF;
  atcd.cb_events = ATCD_EV_ALL;
  atcd.callback  = NULL;

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
  atcd.buff[0]                  = 0;
  atcd.buff[ATCD_BUFF_SIZE - 1] = 0;
  
  atcd.buff_pos   = 0;
  atcd.line_pos   = 0;
  
  atcd.timer      = 0;
  atcd.at_cmd_seq = 0;

  atcd_atc_cancell_all();
  atcd_conn_reset_all(); 

  atcd_atc_init(&atcd.at_cmd);

  atcd.at_cmd_buff[0] = 0;
  //----------------------------------------
  atcd.parser.mode       = ATCD_P_MODE_ATC;
  atcd.parser.echo_en    = ATCD_P_ECHO_ON;

  atcd.parser.timer      = atcd_get_ms();        //optravdu nastavit na aktualni cas?
  
  atcd.parser.at_cmd_top = NULL;
  atcd.parser.at_cmd_end = NULL;
  
  atcd.parser.tx_state    = ATCD_P_TX_COMPLETE;
  atcd.parser.tx_conn_num = 0;
  atcd.parser.tx_data_len = 0;
  rbuff_init(&atcd.parser.tx_rbuff, NULL, 0);

  atcd.parser.rx_conn_num = 0;
  atcd.parser.rx_data_len = 0;
  atcd.parser.rx_data_pos = 0;
  //----------------------------------------
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
    // Prechod parseru do rezimu AT prikazu
    if(atcd_get_ms() - atcd.parser.timer > 4000)
    {
      atcd_dbg_err("ATCD: Vyprsel timeout na IPD!\r\n");
      atcd.parser.mode = ATCD_P_MODE_ATC;

      //osetrit spojeni kde dochazelo k prijmu dat...
    }
  }

  // Test timeoutu v rezimu startu modemu
  if(atcd.state == ATCD_STATE_STARTING)
  {
    if(atcd_get_ms() - atcd.timer > 10000)
    {
      // Restart modemu
      atcd_dbg_err("ATCD: Vyprsel timeout na start!\r\n");
      atcd_reset();
    }
  }

  // Test dokonceni startu modemu
  if(atcd.state == ATCD_STATE_ON)
  {
    // Zahajime inicializacni sekcenci
    atcd_dbg_inf("ATCD: INIT: Zacina inicializace zarizeni.\r\n");
    atcd.at_cmd_seq = 0;
    atcd.state      = ATCD_STATE_INIT;
  }
  
  if(atcd.state == ATCD_STATE_INIT)
  {
    // Provedeme dalsi krok inicializace
    atcd_init_seq(); 
  }

  atcd_check_state_seq();

  atcd_gsm_proc();
  atcd_phone_proc();                     //phone processing
  atcd_gprs_proc();                      //gprs processing
  atcd_conn_proc();                      //connections processing  
  atcd_gps_proc();
  atcd_wifi_proc();
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
  
  atcd_dbg_in(&ch, 1);                    // Logovani prijatych dat
  if(atcd_conn_data_proc(ch) != 0) return;  // Zpracovani prichozich dat TCP/UDP spojeni

  if(atcd.buff_pos >= ATCD_BUFF_SIZE - 1) // Test mista v bufferu
  {
    ATCD_DBG_BUFF_OVERRUN
    at_cmd = atcd.parser.at_cmd_top;
    if(at_cmd != NULL && at_cmd->state == ATCD_ATC_STATE_W_END)
    {
      ATCD_DBG_ATC_LN_BUFF_OV
      at_cmd->resp           = NULL;
      at_cmd->resp_len       = 0;
      at_cmd->resp_buff_size = 0;

      if(at_cmd->callback != NULL && (at_cmd->cb_events & ATCD_ATC_EV_OVERRUN) != 0) at_cmd->callback(ATCD_ATC_EV_OVERRUN);
    }

    atcd.buff_pos = 0;
    atcd.line_pos = 0;
  }
  //-------------------
  // Ulozi prijaty znak do bufferu
  // Do ATC se pak pripadne memcopy na konci radku po prislusnych testech
  // Radek muze byt vymaskovan jako async zprava...
  atcd.buff[atcd.buff_pos++] = ch;
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
  atcd_atc_ln_proc();   // Zpracovani AT prikazu

  if(atcd.callback != NULL && (atcd.cb_events & ATCD_EV_ASYNC_MSG) != 0) atcd.callback(ATCD_EV_ASYNC_MSG);
  atcd_conn_asc_msg();  // Zpracovani TCP/UDP spojeni
  atcd_wifi_asc_msg();  // Zpracovani udalosti WLAN
  atcd_gsm_asc_msg();   // Zpracovani udalosti GSM site
  atcd_phone_asc_msg();
  //------------------------------
  // Zpracovani startovaci sekvence
  //------------------------------
  if(strncmp(atcd.buff + atcd.line_pos, ATCD_STR_START_SEQ, strlen(ATCD_STR_START_SEQ)) == 0)
  {
    ATCD_DBG_BOOT_SEQ
    atcd.state = ATCD_STATE_ON;
    atcd.buff_pos = atcd.line_pos;
    atcd_state_reset();
    if(atcd.callback != NULL && (atcd.cb_events & ATCD_EV_STATE) != 0) atcd.callback(ATCD_EV_STATE);
  }
  //------------------------------
  // Aktualizace zacatku posledniho radku
  // Zalezi, jesli se zrovna zpracovava nejaky AT prikaz
  // Pokud ano, drzi se buffer a posouvaji radky, jinak muzeme resetovat na zacatek
  //------------------------------
  at_cmd = atcd.parser.at_cmd_top;
  if(at_cmd != NULL && at_cmd->state == ATCD_ATC_STATE_W_END)
  {
    if(atcd.line_pos != atcd.buff_pos)    //Pokud radek nebyl vymazan nekde vyse
    {
      
      if(at_cmd->resp != atcd.buff)       //Pokud ma ATC vlastni buffer
      {
        if(at_cmd->resp_len + atcd.buff_pos - atcd.line_pos < at_cmd->resp_buff_size)
        {
          memcpy(at_cmd->resp + at_cmd->resp_len, atcd.buff, atcd.buff_pos - atcd.line_pos);
          at_cmd->resp_len += atcd.buff_pos - atcd.line_pos;
        }
        else
        {
          ATCD_DBG_ATC_BUFF_OV
          if(at_cmd->state == ATCD_ATC_STATE_W_END)
          {
            ATCD_DBG_ATC_LN_BUFF_OV
            at_cmd->resp           = NULL;
            at_cmd->resp_len       = 0;
            at_cmd->resp_buff_size = 0;

            if(at_cmd->callback != NULL && (at_cmd->cb_events & ATCD_ATC_EV_OVERRUN) != 0) at_cmd->callback(ATCD_ATC_EV_OVERRUN);
          }
        }

        atcd.buff_pos = 0;
        atcd.line_pos = 0;
        return;
      }

      //Pokud ATC nema vlastni buffer
      at_cmd->resp_len += atcd.buff_pos - atcd.line_pos;
      atcd.line_pos = atcd.buff_pos;
    }
    return;
  }

  atcd.buff_pos = 0;
  atcd.line_pos = 0;
}
//------------------------------------------------------------------------------
void atcd_tx_complete()                  //call on tx data complete
{
  atcd.parser.tx_state = ATCD_P_TX_COMPLETE;
}
//------------------------------------------------------------------------------
