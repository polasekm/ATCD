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
void atcd_rx_proc();                      //proc rx data 
void atcd_proc_ch(char ch);               //proc rx char 

void atcd_state_reset();                  //state machine reset
//--------------------------------------------------------------

//------------------------------------------------------------------------------
void atcd_init()                          //init AT command device
{
  atc_dev_hw_init();                      //HW init

  atcd.state    = ATCD_STATE_OFF;
  atcd.callback = NULL;

  atcd.phone.callback = NULL;
  atcd.gprs.callback  = NULL;
  atcd.wifi.callback  = NULL;

  atcd_phone_init(&atcd.phone);
  atcd_gprs_init(&atcd.gprs);
  //atcd_gps_init();
  //atcd_wifi_init();

  //atcd_reset();
  atcd_start();   //opravdu chceme zarizeni spustit?
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

  atcd.parser.mode       = ATCD_P_MODE_ATC;
  atcd.parser.echo_en    = ATCD_P_ECHO_ON;

  atcd.parser.timer      = atcd_get_ms();        //optravdu nastavit na aktualni cas?
  
  atcd.parser.at_cmd_top = NULL;
  atcd.parser.at_cmd_end = NULL;
  
  atcd.parser.tx_state    = ATCD_P_TX_COMPLETE;
  rbuff_init(&atcd.parser.tx_rbuff, NULL, 0);
  atcd.parser.tx_data_len = 0;

  atcd.parser.tx_pend_conn_num = 0;

  atcd.parser.ipd_conn_num = 0;
  atcd.parser.ipd_len      = 0;
  atcd.parser.ipd_pos      = 0;

  //--------------------------------
  atcd_phone_reset(&atcd.phone);
  atcd_gprs_reset(&atcd.gprs);
  //atcd_gps_reset();
  //atcd_wifi_reset();
  //--------------------------------
}
//------------------------------------------------------------------------------
void atcd_start()               //start AT command device
{
  atcd_dbg_inf("ATCD: Spusteni zarizeni.\r\n");
  atcd_state_reset();

  atcd_hw_pwr(ATCD_PWR_ON);
  atcd_hw_igt();
  
  atcd.timer = atcd_get_ms();
  atcd.state = ATCD_STATE_STARTING;
}
//------------------------------------------------------------------------------
void atcd_reset()               //reset AT command device
{
  // SW reset
  atcd_dbg_warn("ATCD: Reset zarizeni.\r\n");
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

  atcd_phone_proc();                     //phone processing
  atcd_gprs_proc();                      //gprs processing
  atcd_conn_proc();                      //connections processing  
}
//------------------------------------------------------------------------------
void atcd_tx_complete()                  //call on tx data complete
{
  atcd.parser.tx_state = ATCD_P_TX_COMPLETE;
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

  n = rbuff_read(&atcd_rx_ring_buff, &ch, 1);
  while(n == 1)
  {
    atcd_rx_ch((char) ch);
    n = rbuff_read(&atcd_rx_ring_buff, &ch, 1);
  }
}
//------------------------------------------------------------------------------
void atcd_rx_ch(char ch)
{
  atcd_conn_t *conn;
  atcd_at_cmd_t *at_cmd;
  uint8_t conn_id;
  uint16_t op;
  uint8_t num;
  uint16_t data_len;
  
  // Logovani prijatych dat
  atcd_dbg_in(&ch, 1);
  //--------------------------------------------
  //   - IPD -
  //--------------------------------------------
  // If parser in IPD receiving mode
  if(atcd.parser.mode == ATCD_P_MODE_IPD)
  {
    conn = atcd.conns.conn[atcd.parser.ipd_conn_num];
    // If any connection
    if(conn != NULL)
    {
      // Pokud je v bufferu misto
      if(rbuff_available(&conn->rx_rbuff) != 0)
      {
        // Zapise prijaty byte do bufferu
        rbuff_write(&conn->rx_rbuff, (uint8_t*)&ch, 1);
      } 
      else 
      {
        atcd_dbg_warn("ATCD: CONN: V bufferu spojeni neni dostatek mista pro dalsi prijem dat!\r\n");
        conn->events |=  ATCD_CONN_EV_OVERRUN;
      }   

      atcd.buff_pos++;

      // Pokud jsme dosahli komce IPD bloku
      //if(atcd.buff_pos >= atcd.parser.ipd_len)
      if(atcd.buff_pos >= atcd.parser.ipd_len)
      {
        atcd_dbg_inf("ATCD: CONN: Dosahli jsme konce IPD bloku.\r\n");
        atcd.parser.mode = ATCD_P_MODE_ATC;

        atcd.buff_pos = 0;
        atcd.line_pos = 0;

        conn->events |= ATCD_CONN_EV_NEW_DATA;
        if(conn->callback != NULL) conn->callback(conn, ATCD_CONN_EV_NEW_DATA);
      }
    }
    else
    {
      atcd_dbg_err("ATCD: CONN: Rezim IPD, ale zadne prijimajici spojeni! - Prechazim do rezimu ATC.\r\n");
      atcd.parser.mode = ATCD_P_MODE_ATC;

      atcd.buff_pos = 0;
      atcd.line_pos = 0;
    }
	  return;
  }

  //--------------------------------------------
  //   - != '\n' -
  // If it do not end line
  //--------------------------------------------

  //-------------------
  // Tady ulozit do spolecneho bufferu
  // -- do ATC se pak bude pripadne delat memcopy az na konci radku a prislusnych testech
  // -- Je totiz mozne, ze se bdue radek vymaskovavat...
  //-------------------
  if(atcd.buff_pos >= ATCD_BUFF_SIZE - 1)
  {
    atcd_dbg_warn("ATCD: V bufferu pro ATCD doslo misto!\r\n");

    at_cmd = atcd.parser.at_cmd_top;
    if(at_cmd != NULL && at_cmd->state == ATCD_ATC_STATE_W_END)
    {
      atcd_dbg_warn("ATCD: Nastavuji overrun u ATC??????\r\n");
      //at_cmd->result = ATCD_ATC_RESULT_OVERRUN;

      // neni poreseno stav se s koncem ATC prepise....

      //if(at_cmd->callback != NULL && (at_cmd->events & ATCD_ATC_EV_OVERRUN) != 0) at_cmd->callback(ATCD_ATC_EV_OVERRUN);
    }

    atcd.buff_pos = 0;
    atcd.line_pos = 0;
    return;
  }

  // Save received char to buffer
  atcd.buff[atcd.buff_pos++] = ch;

  //if((ch != '\n') || (ch != '\r'))
  if(ch != '\n')
  {
    #ifndef ATCD_DATA_RX_NL
    // "+IPD," test
    if(ch == ':' && strncmp(atcd.buff + atcd.line_pos, ATCD_STR_DATA_RX, strlen(ATCD_STR_DATA_RX)) == 0)
    {
      conn_id = (uint8_t)atoi(atcd.buff + atcd.line_pos + strlen(ATCD_STR_DATA_RX));
      
      if(conn_id < ATCD_CONN_MAX_NUMBER)
      {
        atcd.parser.ipd_conn_num = conn_id;
        atcd.parser.ipd_len     = (uint16_t)atoi(atcd.buff + atcd.line_pos + strlen(ATCD_STR_DATA_RX) + 2);

        if(atcd.parser.ipd_len != 0)
        {
          atcd_dbg_inf("CONN: ATCD_STR_DATA_RX detected.\r\n");
          atcd.parser.mode  = ATCD_P_MODE_IPD;
          atcd.parser.timer = atcd_get_ms();

          atcd.buff_pos = 0;
          atcd.line_pos = 0;

          //atcd.conn[conn_id]->data_len = 0;
        }
        else atcd_dbg_err("CONN: ATCD_STR_DATA_RX ma velikost 0!\r\n");
      }
      else atcd_dbg_err("CONN: ATCD_STR_DATA_RX ma conn_id mimo rozsah!\r\n");
      return;
    }
    #endif /* ATCD_DATA_RX_NL */
    
    // "> " test
    if(atcd.parser.mode == ATCD_P_MODE_TX_PEND && strncmp(atcd.buff + atcd.line_pos, "> ", strlen("> ")) == 0)
    {
      atcd_dbg_inf("ATCD: Prompt \">\" detected.\r\n");
      atcd.parser.mode = ATCD_P_MODE_PROMPT;

      atcd.buff_pos = 0;
      atcd.line_pos = 0;

      // Nasypat data
      // eventualne budou nasypana z proc funkce
      
      // SEM pripsat dalsi promenou ve ktere bude pocet byte co se maji posilat
      // a moyna a ikazayatel na data
      // po yaniku spojeni je nutno minimalne vedet kolik e toho melo posilat, pokud
      // uz se prikaz zacal provadet...

      // tohle by se melo prepsat - bez dat k odeslani by nemel jit nastavit prompt, nejlepe jej zahodit a rozhodovat dle dat...

      if(atcd.parser.at_cmd_top->data != NULL)
      {
        atcd_atc_send_data(atcd.parser.at_cmd_top->data, atcd.parser.at_cmd_top->data_len);
      }
      else
      {
        atcd_dbg_err("ATCD: ATC nema zadna data k odeslani!\r\n");
      }

      atcd.parser.mode = ATCD_P_MODE_ATC;
      return;
    }
    return;
  }
  
  //--------------------------------------------
  //   - != '\n' -
  // If is it end line
  //--------------------------------------------
  #ifdef ATCD_DATA_RX_NL
  // "+IPD," test
  if(atcd.buff[atcd.buff_pos - 3] == ':' && strncmp(atcd.buff + atcd.line_pos, ATCD_STR_DATA_RX, strlen(ATCD_STR_DATA_RX)) == 0)
  {
    conn_id = (uint8_t)atoi(atcd.buff + atcd.line_pos + strlen(ATCD_STR_DATA_RX));

    if(conn_id < ATCD_CONN_MAX_NUMBER)
    {
      atcd.parser.ipd_conn_num = conn_id;
      atcd.parser.ipd_len      = (uint16_t)atoi(atcd.buff + atcd.line_pos + strlen(ATCD_STR_DATA_RX) + 2);

      if(atcd.parser.ipd_len != 0)
      {
        atcd_dbg_inf("CONN: ATCD_STR_DATA_RX detected.\r\n");
        atcd.parser.mode  = ATCD_P_MODE_IPD;
        atcd.parser.timer = atcd_get_ms();

        atcd.buff_pos = 0;
        atcd.line_pos = 0;

        //atcd.conn[conn_id]->data_len = 0;
      }
      else atcd_dbg_err("CONN: ATCD_STR_DATA_RX ma velikost 0!\r\n");
    }
    else atcd_dbg_err("CONN: ATCD_STR_DATA_RX ma conn_id mimo rozsah!\r\n");
    return;
  }
  #endif /* ATCD_DATA_RX_NL */

  //------------------------------
  // Zpracovani AT prikazu
  //------------------------------
  at_cmd = atcd.parser.at_cmd_top;
  // Pokud se zpracovava nejaky ATC
  if(at_cmd != NULL)                            
  {
    if(at_cmd->state == ATCD_ATC_STATE_W_ECHO) 
    {
      // Test echa
      //if(strncmp(atcd.buff + atcd.line_pos, at_cmd->cmd, strlen(at_cmd->cmd)) == 0)
      if(strncmp(atcd.buff + atcd.line_pos, at_cmd->cmd, strlen(at_cmd->cmd) - 1) == 0)
      {
        atcd_dbg_inf("ATC: ECHO detected.\r\n");
        at_cmd->state = ATCD_ATC_STATE_W_END;
        if(atcd.parser.at_cmd_top->data != NULL) atcd.parser.mode = ATCD_P_MODE_TX_PEND;

        atcd.buff_pos = 0;
        atcd.line_pos = 0;

        if(at_cmd->callback != NULL && (at_cmd->events & ATCD_ATC_EV_ECHO) != 0) at_cmd->callback(ATCD_ATC_EV_ECHO);
        return;
      }
      else atcd_dbg_warn("ATC: ECHO test FAIL.\r\n");
    } 
    else if(at_cmd->state == ATCD_ATC_STATE_W_END) 
    {
      // Vymazani pocatecnich prazdnych radku
      if(at_cmd->resp_len == 0 && atcd.buff_pos == 2) 
      {
        atcd.buff_pos = 0;
        atcd.line_pos = 0;
        return;
      }
      // Test odpovedi a zpracovani dat
      if(strncmp(atcd.buff + atcd.line_pos, "OK\r\n", strlen("OK\r\n")) == 0)
      {
        atcd_dbg_inf("ATC: OK detected.\r\n");
        at_cmd->result = ATCD_ATC_RESULT_OK;
      }
      else if(strncmp(atcd.buff + atcd.line_pos, "ERROR\r\n", strlen("ERROR\r\n")) == 0)
      {
        atcd_dbg_inf("ATC: ERROR detected.\r\n");
        at_cmd->result = ATCD_ATC_RESULT_ERROR;
      }
      // Neni tohle nahodou asynchorinni zprava?
      else if(strncmp(atcd.buff + atcd.line_pos, "+CME ERROR:", strlen("+CME ERROR:")) == 0)
      {
        atcd_dbg_inf("ATC: ERROR detected.\r\n");
        at_cmd->result = ATCD_ATC_RESULT_ERROR;
      }
      else if(strncmp(atcd.buff + atcd.line_pos, "+CMS ERROR:", strlen("+CMS ERROR:")) == 0)
      {
        atcd_dbg_inf("ATC: ERROR detected.\r\n");
        at_cmd->result = ATCD_ATC_RESULT_ERROR;
      }
      else if(strncmp(atcd.buff + atcd.line_pos, "FAIL\r\n", strlen("FAIL\r\n")) == 0)
      {
        atcd_dbg_inf("ATC: FAIL detected.\r\n");
        at_cmd->result = ATCD_ATC_RESULT_FAIL;
      }

      if(at_cmd->result != ATCD_ATC_RESULT_UNKNOWN)
      {
        // AT prikaz byl v casti vyse dokoncen
        if(at_cmd->resp_len == 0)
          at_cmd->resp_len = 0;
        else
          at_cmd->resp_len -= 2;

        at_cmd->resp[at_cmd->resp_len] = 0;
        at_cmd->state  = ATCD_ATC_STATE_DONE;

        if(atcd.parser.mode == ATCD_P_MODE_TX_PEND) atcd.parser.mode = ATCD_P_MODE_ATC;

        atcd.buff_pos = 0;
        atcd.line_pos = 0;

        atcd.parser.at_cmd_top = at_cmd->next;
        atcd_atc_queue_proc(); 

        if(at_cmd->callback != NULL && (at_cmd->events & ATCD_ATC_EV_DONE) != 0) at_cmd->callback(ATCD_ATC_EV_DONE);
        return;
      }
    }    
  }

  //------------------------------
  // Zpracovani TCP/UDP spojeni
  //------------------------------
  if(atcd.gprs.state == ATCD_GPRS_STATE_CONN)     //pokud je pripojeno WiFi a ma IP addr
  {
    //if(strncmp(atcd.buff + atcd.line_pos + 1, ",CONNECT\r\n", strlen(",CONNECT\r\n")) == 0)
    if(strncmp(atcd.buff + atcd.line_pos + 1, ", CONNECT OK\r\n", strlen(", CONNECT OK\r\n")) == 0)
    {
      conn_id = (uint8_t)atoi(atcd.buff + atcd.line_pos);

      if(conn_id < ATCD_CONN_MAX_NUMBER)
      {
        atcd_dbg_inf("CONN: x, CONNECT OK detect.\r\n");
        conn = atcd.conns.conn[conn_id];
        if(conn != NULL)
        {
          conn->state = ATCD_CONN_STATE_OPEN;
          atcd.buff_pos = atcd.line_pos;
          if(conn->callback != NULL && (conn->events & ATCD_CONN_EV_OPEN) != 0) conn->callback(conn, ATCD_CONN_EV_OPEN);
        }
        else
        {
          atcd_dbg_err("CONN: Nelze zmenit stav neregistrovaneho spojeni!\r\n");
        }
      }
      else atcd_dbg_err("CONN: x, CONNECT detect - x mimo rozah.\r\n");
    }
    else if(strncmp(atcd.buff + atcd.line_pos, "^SISW: ", strlen("^SISW: ")) == 0)
    {
      conn_id = (uint8_t)atoi(atcd.buff + atcd.line_pos + strlen("^SISW: "));

      if(conn_id < ATCD_CONN_MAX_NUMBER)
      {
        atcd_dbg_inf("CONN: ^SISW: x detected.\r\n");
        conn = atcd.conns.conn[conn_id];
        if(conn != NULL)
        {
          op = (uint8_t)atoi(atcd.buff + atcd.line_pos + strlen("^SISW: x,"));

          if(atcd.parser.mode == ATCD_P_MODE_TX_PEND)
          {
            // Testovat kolik muzeme zapsat dat...
            //
            //

            atcd_dbg_inf("ATCD: Prompt \"AT^SISW=x,x,x\" detected.\r\n");
            atcd.parser.mode = ATCD_P_MODE_PROMPT;

            atcd.buff_pos = 0;
            atcd.line_pos = 0;

            // Nasypat data
            // eventualne budou nasypana z proc funkce

            // SEM pripsat dalsi promenou ve ktere bude pocet byte co se maji posilat
            // a moyna a ikazayatel na data
            // po yaniku spojeni je nutno minimalne vedet kolik e toho melo posilat, pokud
            // uz se prikaz zacal provadet...


            // tohle by se melo prepsat - bez dat k odeslani by nemel jit nastavit prompt, nejlepe jej zahodit a rozhodovat dle dat...

            if(atcd.parser.at_cmd_top->data != NULL)
            {
              atcd_atc_send_data(atcd.parser.at_cmd_top->data, atcd.parser.at_cmd_top->data_len);
            }
            else
            {
              atcd_dbg_err("ATCD: ATC nema zadna data k odeslani!\r\n");
            }

            atcd.parser.mode = ATCD_P_MODE_ATC;
            return;
          }
          else if(op == 1)
          {
            atcd_dbg_inf("CONN: Spojeni je navazano.\r\n");
            conn->state = ATCD_CONN_STATE_OPEN;
            atcd.buff_pos = atcd.line_pos;
            if(conn->callback != NULL && (conn->events & ATCD_CONN_EV_OPEN) != 0) conn->callback(conn, ATCD_CONN_EV_OPEN);
          }
          else
          {
            atcd_dbg_warn("CONN: Neznama operace.\r\n");
          }
        }
        else
        {
          atcd_dbg_err("CONN: Nelze zmenit stav neregistrovaneho spojeni!\r\n");
        }
      }
      else atcd_dbg_err("CONN: ^SISW: x detect - x mimo rozah.\r\n");
    }
    else if(strncmp(atcd.buff + atcd.line_pos + 1, ", CLOSED\r\n", strlen(", CLOSED\r\n")) == 0)
    {
      conn_id = (uint8_t)atoi(atcd.buff + atcd.line_pos);

      if(conn_id < ATCD_CONN_MAX_NUMBER)
      {
        atcd_dbg_inf("CONN: x, CLOSED detect.\r\n");
        conn = atcd.conns.conn[conn_id];
        if(conn != NULL)
        {
          atcd.conns.conn[conn_id] = NULL;
          conn->state = ATCD_CONN_STATE_CLOSE;
          conn->num   = ATCD_CONN_NO_NUM;
          if(conn->at_cmd.state == ATCD_ATC_STATE_WAIT) atcd_atc_cancell(&conn->at_cmd);

          atcd.buff_pos = atcd.line_pos;
          if(conn->callback != NULL && (conn->events & ATCD_CONN_EV_CLOSE) != 0) conn->callback(conn, ATCD_CONN_EV_CLOSE);
        }
        else
        {
          atcd_dbg_err("CONN: Nelze zmenit stav neregistrovaneho spojeni!\r\n");
        }
      }
      else atcd_dbg_err("CONN: x, CLOSED detect - x mimo rozah.\r\n");
    }
    else if(strncmp(atcd.buff + atcd.line_pos + 1, ", CLOSE OK\r\n", strlen(", CLOSE OK\r\n")) == 0)
    {
      conn_id = (uint8_t)atoi(atcd.buff + atcd.line_pos);

      if(conn_id < ATCD_CONN_MAX_NUMBER)
      {
        atcd_dbg_inf("CONN: x, CLOSE OKdetect.\r\n");
        conn = atcd.conns.conn[conn_id];
        if(conn != NULL)
        {
          atcd.conns.conn[conn_id] = NULL;
          conn->state = ATCD_CONN_STATE_CLOSE;
          conn->num   = ATCD_CONN_NO_NUM;
          if(conn->at_cmd.state == ATCD_ATC_STATE_WAIT) atcd_atc_cancell(&conn->at_cmd);

          atcd.buff_pos = atcd.line_pos;
          if(conn->callback != NULL && (conn->events & ATCD_CONN_EV_CLOSE) != 0) conn->callback(conn, ATCD_CONN_EV_CLOSE);
        }
        else
        {
          atcd_dbg_err("CONN: Nelze zmenit stav neregistrovaneho spojeni!\r\n");
        }
      }
      else atcd_dbg_err("CONN: x, CLOSE OK detect - x mimo rozah.\r\n");
    }
    else if(strncmp(atcd.buff + atcd.line_pos, "^SIS: ", strlen("^SIS: ")) == 0)
    {
      conn_id = (uint8_t)atoi(atcd.buff + atcd.line_pos + strlen("^SIS: "));

      if(conn_id < ATCD_CONN_MAX_NUMBER)
      {
        atcd_dbg_inf("SIS: x,y... detect.\r\n");
        conn = atcd.conns.conn[conn_id];
        if(conn != NULL)
        {
          op = (uint8_t)atoi(atcd.buff + atcd.line_pos + strlen("^SIS: x,"));

          if(op == 0)
          {
            atcd_dbg_inf("CONN: Spojeni je zruseno.\r\n");

            atcd.conns.conn[conn_id] = NULL;
            conn->state = ATCD_CONN_STATE_CLOSE;
            conn->num   = ATCD_CONN_NO_NUM;
            if(conn->at_cmd.state == ATCD_ATC_STATE_WAIT) atcd_atc_cancell(&conn->at_cmd);

            atcd.buff_pos = atcd.line_pos;
            if(conn->callback != NULL && (conn->events & ATCD_CONN_EV_CLOSE) != 0) conn->callback(conn, ATCD_CONN_EV_CLOSE);
          }
          else
          {
            atcd_dbg_warn("CONN: SIS: x,y - spatna hodnota parametru y.\r\n");
          }
        }
        else
        {
          atcd_dbg_err("CONN: Nelze zmenit stav neregistrovaneho spojeni!\r\n");
        }
      }
      else atcd_dbg_err("CONN: SIS: x,y... detect - x mimo rozah.\r\n");
    }
    else if(strncmp(atcd.buff + atcd.line_pos + 1, ", SEND OK\r\n", strlen(", SEND OK\r\n")) == 0)
    {
      atcd_dbg_inf("CONN: x, SEND OK.\r\n");

      atcd.buff_pos = atcd.line_pos;

      // nad jakym spojenim to bude?
      //if(conn->callback != NULL && (conn->events & ATCD_CONN_EV_SEND_OK) != 0) conn->callback(conn, ATCD_CONN_EV_SEND_OK);
    }
    //CONNECT FAIL
    else if(strncmp(atcd.buff + atcd.line_pos, "DNS Fail\r\n", strlen("DNS Fail\r\n")) == 0)
    {
      atcd_dbg_inf("CONN: DNS Fail.\r\n");

      atcd.buff_pos = atcd.line_pos;

      // nad jakym spojenim to bude?
      //if(conn->callback != NULL && (conn->events & ATCD_CONN_EV_FAIL) != 0) conn->callback(conn, ATCD_CONN_EV_FAIL);
    }
    else 
    {

    }
  }

  //------------------------------
  // Zpracovani udalosti WLAN
  //------------------------------
  if(strncmp(atcd.buff + atcd.line_pos, "WIFI CONNECTED\r\n", strlen("WIFI CONNECTED\r\n")) == 0)
  {
    atcd_dbg_inf("WIFI CONNECTED detect...\r\n");
    atcd.wifi.state = ATCD_WIFI_STATE_W_DHCP;
    atcd.buff_pos = atcd.line_pos;
    if(atcd.wifi.callback != NULL && (atcd.wifi.events & ATCD_WIFI_EV_CONN) != 0) atcd.wifi.callback(ATCD_WIFI_EV_CONN);
  }
  else if(strncmp(atcd.buff + atcd.line_pos, "WIFI GOT IP\r\n", strlen("WIFI GOT IP\r\n")) == 0)
  {
    atcd_dbg_inf("WIFI GOT IP detect...\r\n");
    atcd.wifi.state = ATCD_WIFI_STATE_CONN;
    atcd.buff_pos = atcd.line_pos;
    if(atcd.wifi.callback != NULL && (atcd.wifi.events & ATCD_WIFI_EV_GOT_IP) != 0) atcd.wifi.callback(ATCD_WIFI_EV_GOT_IP);
  }
  else if(strncmp(atcd.buff + atcd.line_pos, "WIFI DISCONNECT\r\n", strlen("WIFI DISCONNECT\r\n")) == 0)
  {
    atcd_dbg_inf("WIFI DISCONNECT detect...\r\n");
    atcd.wifi.state = ATCD_WIFI_STATE_DISCONN;
    atcd.buff_pos = atcd.line_pos;
    atcd_conn_reset_all();
    if(atcd.wifi.callback != NULL && (atcd.wifi.events & ATCD_WIFI_EV_DISCONN) != 0) atcd.wifi.callback(ATCD_WIFI_EV_DISCONN);
  }

  //------------------------------
  // Zpracovani udalosti GSM site
  //------------------------------
  else if(strncmp(atcd.buff + atcd.line_pos, "+CREG: ", strlen("+CREG: ")) == 0)
  {
    //num = (uint8_t)atoi(atcd.buff + atcd.line_pos + strlen("+CREG: "));
    num = (uint8_t)atoi(atcd.buff + atcd.buff_pos - ATCD_RX_NL_LEN - 1);

    switch(num)
    {
      case 0:
        atcd_dbg_inf("ATCD: GSM +CREG: 0 detect.\r\n");
        atcd.phone.state = ATCD_PHONE_STATE_OFF;
        atcd.buff_pos = atcd.line_pos;
        atcd_conn_reset_all();
        if(atcd.phone.callback != NULL && (atcd.phone.events & ATCD_PHONE_EV_UNREG) != 0) atcd.phone.callback(ATCD_PHONE_EV_UNREG);
        break;

      case 1:
        atcd_dbg_inf("ATCD: GSM +CREG: 1 detect.\r\n");
        atcd.phone.state = ATCD_PHONE_STATE_REG_HOME;
        atcd.buff_pos = atcd.line_pos;
        if(atcd.phone.callback != NULL && (atcd.phone.events & ATCD_PHONE_EV_REG) != 0) atcd.phone.callback(ATCD_PHONE_EV_REG);
        break;

      case 2:
        atcd_dbg_inf("ATCD: GSM +CREG: 2 detect.\r\n");
        atcd.phone.state = ATCD_PHONE_STATE_SEARCHING;
        atcd.buff_pos = atcd.line_pos;
        atcd_conn_reset_all();
        if(atcd.phone.callback != NULL && (atcd.phone.events & ATCD_PHONE_EV_UNREG) != 0) atcd.phone.callback(ATCD_PHONE_EV_UNREG);
        break;

      case 3:
        atcd_dbg_inf("ATCD: GSM +CREG: 3 detect.\r\n");
        atcd.phone.state = ATCD_PHONE_STATE_DENIED;
        atcd.buff_pos = atcd.line_pos;
        atcd_conn_reset_all();
        if(atcd.phone.callback != NULL && (atcd.phone.events & ATCD_PHONE_EV_UNREG) != 0) atcd.phone.callback(ATCD_PHONE_EV_UNREG);
        break;

      case 4:
        atcd_dbg_inf("ATCD: GSM +CREG: 4 detect.\r\n");
        atcd.phone.state = ATCD_PHONE_STATE_UNKNOWN;
        atcd.buff_pos = atcd.line_pos;
        atcd_conn_reset_all();
        if(atcd.phone.callback != NULL && (atcd.phone.events & ATCD_PHONE_EV_UNREG) != 0) atcd.phone.callback(ATCD_PHONE_EV_UNREG);
        break;

      case 5:
        atcd_dbg_inf("ATCD: GSM +CREG: 5 detect.\r\n");
        atcd.phone.state = ATCD_PHONE_STATE_REG_ROAM;
        atcd.buff_pos = atcd.line_pos;
        if(atcd.phone.callback != NULL && (atcd.phone.events & ATCD_PHONE_EV_REG) != 0) atcd.phone.callback(ATCD_PHONE_EV_REG);
        break;

      default:
        atcd_dbg_err("ATCD: +CREG je mimo rozsah!\r\n");
        atcd.buff_pos = atcd.line_pos;
        break;
    }
  }
  else if(strncmp(atcd.buff + atcd.line_pos, "RING\r\n", strlen("RING\r\n")) == 0)
  {
    atcd_dbg_inf("PHONE: RING detect.\r\n");
    //atcd.phone.state = ATCD_PHONE_STATE_REG_ROAM;
    atcd.buff_pos = atcd.line_pos;
    if(atcd.phone.callback != NULL && (atcd.phone.events & ATCD_PHONE_EV_RING) != 0) atcd.phone.callback(ATCD_PHONE_EV_RING);
  }
  else if(strncmp(atcd.buff + atcd.line_pos, "+CMT: ", strlen("+CMT: ")) == 0)
  {
    atcd_dbg_inf("PHONE: New SMS detected.\r\n");
    // Bude nasledovat SMS - pocet znaku je uveden na konci...

    //atcd.phone.state = ATCD_PHONE_STATE_REG_ROAM;
    atcd.buff_pos = atcd.line_pos;
    if(atcd.phone.callback != NULL && (atcd.phone.events & ATCD_PHONE_EV_SMS_IN) != 0) atcd.phone.callback(ATCD_PHONE_EV_SMS_IN);
  }
  //------------------------------
  // Zpracovani startovaci sekvence
  //------------------------------
  else if(strncmp(atcd.buff + atcd.line_pos, ATCD_STR_START_SEQ, strlen(ATCD_STR_START_SEQ)) == 0)
  {
    atcd_dbg_inf("ATCD: Detekovana bootovaci sekvence.\r\n");
    atcd.state = ATCD_STATE_ON;
    atcd.buff_pos = atcd.line_pos;
    atcd_state_reset();
    if(atcd.callback != NULL && (atcd.events & ATCD_EV_READY) != 0) atcd.callback(ATCD_EV_READY);
  }
  //------------------------------
  // Aktualizace zacatku posledniho radku
  // Zalezi, jesli se zrovna zpracovava nejaky AT prikaz
  // Pokud ano, drzi se buffer a posouvaji radky, jinak muzeme resetovat na zacatek
  //------------------------------
  if(atcd.parser.at_cmd_top != NULL && atcd.parser.at_cmd_top->state == ATCD_ATC_STATE_W_END)
  {
    if(atcd.line_pos != atcd.buff_pos)
    {
      // kopirovat radek odpovedi AT prikazu
      if(at_cmd->resp != atcd.buff) 
      {
        if(at_cmd->resp_len + atcd.buff_pos - atcd.line_pos < at_cmd->resp_buff_size)
        {
          memcpy(at_cmd->resp + at_cmd->resp_len, atcd.buff, atcd.buff_pos - atcd.line_pos);
          at_cmd->resp_len += atcd.buff_pos - atcd.line_pos;
        }
        else
        {
          atcd_dbg_warn("ATCD: V cilovem bufferu ATC neni dost mista!\r\n");
          atcd_dbg_warn("ATCD: Nastavuji overrun u zpracovavaneho AT prikazu.\r\n");
          //at_cmd->result = ATCD_ATC_RESULT_OVERRUN;

          // neni poreseno stav se s koncem ATC prepise....
        }

        atcd.buff_pos = 0;
        atcd.line_pos = 0;
      }
      else 
      {
        at_cmd->resp_len += atcd.buff_pos - atcd.line_pos;
        atcd.line_pos = atcd.buff_pos;
      }
    }
  }
  else
  {
    atcd.buff_pos = 0;
    atcd.line_pos = 0;
  }
}
//------------------------------------------------------------------------------
