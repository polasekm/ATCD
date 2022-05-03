/*
 * usart.c
 *
 *  Created on: Apr 9, 2011
 *      Author: Martin
 */
//------------------------------------------------------------------------------
#include "atcd_conn.h"
#include "atcd.h"

extern atcd_t atcd;

//------------------------------------------------------------------------------
void atcd_conn_proc()                    //connections processing 
{
  uint8_t i;
  atcd_conn_t *conn;

  for(i = 0; i < ATCD_CONN_MAX_NUMBER; i++)
  {
    conn = atcd.conns.conn[i];

    if(conn != NULL)
    {
      // TODO tohle je spatne, dodelat...
      if((conn->state == ATCD_CONN_STATE_W_OPEN || /*conn->state == ATCD_CONN_STATE_W_OPENFAILED ||*/
          conn->state == ATCD_CONN_STATE_OPENING || conn->state == ATCD_CONN_STATE_CLOSING))
      {
        if(atcd_get_ms() - conn->timer > conn->timeout)
        {
          ATCD_DBG_CONN_TIM
          atcd_conn_close(conn, 1);
        }
      }
    }
  }
}
//------------------------------------------------------------------------------
void atcd_conn_reset_all()
{
  uint8_t i;
  atcd_conn_t *conn;

  for(i = 0; i < ATCD_CONN_MAX_NUMBER; i++)
  {
    conn = atcd.conns.conn[i];

    if(conn != NULL)
    {
      conn->num    = ATCD_CONN_NO_NUM;
      conn->state  = ATCD_CONN_STATE_CLOSE;

      atcd.conns.conn[i] = NULL;

      //Opravdu event na close? Spojeni je ruseno nasilne - nemuselo byt ani navazano, mozna spise novy priznak...
      if(conn->callback != NULL && (conn->cb_events & ATCD_CONN_EV_CLOSE) != 0) conn->callback(conn, ATCD_CONN_EV_CLOSE);
    }
  }

  atcd_gprs_autoconn();
}
//------------------------------------------------------------------------------
void atcd_conn_init(atcd_conn_t *conn, uint8_t *rx_buff, uint16_t rx_buff_size, uint8_t *tx_buff, uint16_t tx_buff_size)     //init connection
{
  conn->num        = ATCD_CONN_NO_NUM;
  conn->state      = ATCD_CONN_STATE_CLOSE;
  
  conn->protocol   = ATCD_CONN_T_TCP;
  conn->host       = NULL;
  conn->port       = 0;

  conn->timeout    = 15000;
  conn->timer      = 0;
  
  rbuff_init(&conn->rx_rbuff, rx_buff, rx_buff_size);
  rbuff_init(&conn->tx_rbuff, tx_buff, tx_buff_size);
  
  conn->cb_events  = ATCD_CONN_EV_ALL;
  conn->callback   = NULL;

  conn->dontPrint  = 0;
  conn->in_overflow = 0;
}
//------------------------------------------------------------------------------
void atcd_conn_open(atcd_conn_t *conn, const char *dest, uint16_t port, atcd_conn_type_t type) //open conenction
{
  uint8_t i;

  conn->protocol = type;
  conn->host     = dest;
  conn->port     = port;

  conn->timeout  = 10000;
  conn->timer    = atcd_get_ms();
  
  rbuff_reset(&conn->rx_rbuff);
  rbuff_reset(&conn->tx_rbuff);
  
  conn->cb_events = ATCD_CONN_EV_NONE;
  conn->callback = NULL;
  
  conn->dontPrint  = 0;

  for(i = 0; i < ATCD_CONN_MAX_NUMBER; i++)
  {
    if(atcd.conns.conn[i] == NULL)
    {
      ATCD_DBG_CONN_ALLOC
      
      atcd.conns.conn[i] = conn;
      conn->num = i;
      
      conn->state = ATCD_CONN_STATE_W_OPEN;

      atcd_gprs_autoconn();
      return;
    }
  }
  
  ATCD_DBG_CONN_ALLOC_ERR
  conn->state = ATCD_CONN_STATE_MAX_CONN;
  conn->num   = ATCD_CONN_NO_NUM;
  return;
}
//------------------------------------------------------------------------------
void atcd_conn_write(atcd_conn_t *conn, const uint8_t* data, uint16_t len)   //write data to connection
{
  ATCD_DBG_CONN_WRITE
  rbuff_write(&conn->tx_rbuff, data, len);
}
//------------------------------------------------------------------------------
uint32_t atcd_conn_write_rb(atcd_conn_t *conn, rbuff_t *data)   //write data to connection
{
  uint32_t len;

  ATCD_DBG_CONN_WRITE
  len = rbuff_size(data);

  if(rbuff_write_rb(&conn->tx_rbuff, data, len))
    return len;
  else
    return 0;
}
//------------------------------------------------------------------------------
void atcd_conn_close(atcd_conn_t *conn, uint8_t force_free)                       //close connection
{
  //if(conn->state != ATCD_CONN_STATE_W_CLOSE && conn->state != ATCD_CONN_STATE_CLOSING)
  if(conn->state == ATCD_CONN_STATE_OPEN || conn->state == ATCD_CONN_STATE_OPENING ||
     (force_free && conn->state == ATCD_CONN_STATE_W_OPEN)) //kdyz prislo x, ALREADY CONNECT musim se zbavit toho co tam je
  {
    ATCD_DBG_CONN_CLOSE_W

    conn->state = ATCD_CONN_STATE_W_CLOSE;
    conn->timer = atcd_get_ms();
  }
  else if(conn->state == ATCD_CONN_STATE_W_OPEN)
  {
    atcd_conn_free(conn);
  }
}
//------------------------------------------------------------------------------
void atcd_conn_free(atcd_conn_t *conn)                         //free connection
{
	if(conn->num < ATCD_CONN_MAX_NUMBER)
  {
    ATCD_DBG_CONN_FREE
    atcd.conns.conn[conn->num] = NULL;
  }
  else
  {
    ATCD_DBG_CONN_FREE_ERR
  }

  conn->state = ATCD_CONN_STATE_CLOSE;
  conn->num   = ATCD_CONN_NO_NUM;
  if(conn->callback != NULL && (conn->cb_events & ATCD_CONN_EV_CLOSE) != 0) conn->callback(conn, ATCD_CONN_EV_CLOSE);

  atcd_gprs_autoconn();
}
//------------------------------------------------------------------------------
atcd_conn_state_t atcd_conn_state(const atcd_conn_t *conn)
{
  return conn->state;
}
//------------------------------------------------------------------------------
uint8_t atcd_conn_ipd_tst()
{
  #ifndef ATCD_DATA_RX_NL
  // "+IPD," test
  if(ch == ':' && strncmp(atcd.parser.buff + atcd.parser.line_pos, ATCD_STR_DATA_RX, strlen(ATCD_STR_DATA_RX)) == 0)
  {
    conn_id = (uint8_t)atoi(atcd.parser.buff + atcd.parser.line_pos + strlen(ATCD_STR_DATA_RX));
    
    if(conn_id < ATCD_CONN_MAX_NUMBER)
    {
      atcd.parser.rx_conn_num = conn_id;
      atcd.parser.rx_data_len = (uint16_t)atoi(atcd.parser.buff + atcd.parser.line_pos + strlen(ATCD_STR_DATA_RX) + 2);

      if(atcd.parser.rx_data_len != 0)
      {
        atcd_dbg_inf("CONN: ATCD_STR_DATA_RX detected.\r\n");
        atcd.parser.mode  = ATCD_P_MODE_IPD;
        atcd.parser.timer = atcd_get_ms();

        atcd.parser.buff_pos = 0;
        atcd.parser.line_pos = 0;

        //atcd.conn[conn_id]->data_len = 0;
      }
      else atcd_dbg_err("CONN: ATCD_STR_DATA_RX ma velikost 0!\r\n");
    }
    else atcd_dbg_err("CONN: ATCD_STR_DATA_RX ma conn_id mimo rozsah!\r\n");
    return 1;
  }
  #endif /* ATCD_DATA_RX_NL */

  return 0;
}
//------------------------------------------------------------------------------
uint8_t atcd_conn_asc_msg()
{
  atcd_conn_t *conn;
  uint8_t conn_id;
  uint16_t op;

  #ifdef ATCD_DATA_RX_NL
  // "+IPD," test
  if(atcd.parser.buff[atcd.parser.buff_pos - 3] == ':' && strncmp(atcd.parser.buff + atcd.parser.line_pos, ATCD_STR_DATA_RX, strlen(ATCD_STR_DATA_RX)) == 0)
  {
    conn_id = (uint8_t)atoi(atcd.parser.buff + atcd.parser.line_pos + strlen(ATCD_STR_DATA_RX));

    if(conn_id < ATCD_CONN_MAX_NUMBER)
    {
      atcd.parser.rx_conn_num = conn_id;
      atcd.parser.rx_data_len = (uint16_t)atoi(atcd.parser.buff + atcd.parser.line_pos + strlen(ATCD_STR_DATA_RX) + 2);

      if(atcd.parser.rx_data_len != 0)
      {
        ATCD_DBG_CONN_DATA_RX_DET
        /*if(atcd.parser.mode == ATCD_P_MODE_WAITOK) //+RECEIVE muze prijit pred echem, nebo mezi send a 0, SEND OK
          atcd.parser.mode = ATCD_P_MODE_IPD_WAITOK;
        else if ((atcd.parser.mode==ATCD_P_MODE_SLEEP) || (atcd.parser.mode==ATCD_P_MODE_WAKING))
        {*/
          /* WAKING taky vratit do SLEEP:
          [323.557] sleep->wake
          [323.646]+RECEIVE,0,4: P��
          [323.648] @atcd pmode ipdidl->idle
          [323.648] AT
          [328.649] ATCD: ATC: Probihajicimu AT prikazu vyprsel timeout. */
          /*atcd.parser.mode = ATCD_P_MODE_IPD_SLEEP;
        }
        else
          atcd.parser.mode = ATCD_P_MODE_IPD;*/

        atcd.parser.mode = ATCD_P_MODE_IPD;
        atcd.parser.timer = atcd_get_ms();

        atcd.parser.buff_pos = 0;
        atcd.parser.line_pos = 0;

        //atcd.conn[conn_id]->data_len = 0;
      }
      else ATCD_DBG_CONN_DATA_RX_LEN_E
    }
    else ATCD_DBG_CONN_DATA_RX_RNG_E

    atcd.parser.buff_pos = atcd.parser.line_pos;  //vymaze prijaty radek
    return 1;
  }
  #endif /* ATCD_DATA_RX_NL */

  if (atcd.conns.awaitingC5__)
  {
    if (strncmp(atcd.parser.buff + atcd.parser.line_pos, "C: 5,", strlen("C: 5,")) == 0)
      atcd.conns.awaitingC5__=0;;
  };

  if(atcd.gprs.state == ATCD_GPRS_STATE_CONN)     //pokud je pripojeno WiFi a ma IP addr
  {
    if(strncmp(atcd.parser.buff + atcd.parser.line_pos + 1, ", CONNECT OK\r\n", strlen(", CONNECT OK\r\n")) == 0)
    {
      conn_id = (uint8_t)atoi(atcd.parser.buff + atcd.parser.line_pos);

      if(conn_id < ATCD_CONN_MAX_NUMBER)
      {
        ATCD_DBG_CONN_DET
        conn = atcd.conns.conn[conn_id];
        if(conn != NULL)
        {
          conn->state = ATCD_CONN_STATE_OPEN;
          atcd.parser.buff_pos = atcd.parser.line_pos;
          if(conn->callback != NULL && (conn->cb_events & ATCD_CONN_EV_OPEN) != 0) conn->callback(conn, ATCD_CONN_EV_OPEN);
        }
        else
        {
          ATCD_DBG_CONN_UNREG
        }
      }
      else ATCD_DBG_CONN_RNG_E

      atcd.parser.buff_pos = atcd.parser.line_pos;  //vymaze prijaty radek
      return 1;
    }
    else if(strncmp(atcd.parser.buff + atcd.parser.line_pos, "^SISW: ", strlen("^SISW: ")) == 0)
    {
      //TODO: Tohle cele proverit a vyzkouset
      conn_id = (uint8_t)atoi(atcd.parser.buff + atcd.parser.line_pos + strlen("^SISW: "));

      if(conn_id < ATCD_CONN_MAX_NUMBER)
      {
        ATCD_DBG_CONN_SISW_DET
        conn = atcd.conns.conn[conn_id];
        if(conn != NULL)
        {
          op = (uint8_t)atoi(atcd.parser.buff + atcd.parser.line_pos + strlen("^SISW: x,"));

          if(atcd.parser.at_cmd_top->state == ATCD_ATC_STATE_W_PROMPT)
          {
            ATCD_DBG_CONN_SISW_P

            atcd.parser.buff_pos = 0;
            atcd.parser.line_pos = 0;

            if(atcd.parser.at_cmd_top->data != NULL)
            {
              atcd_atc_send_data_top(atcd.parser.at_cmd_top->data, atcd.parser.at_cmd_top->data_len);
            }
            else
            {
              ATCD_DBG_CONN_SISW_P_DE
            }

            return 1;
          }
          else if(op == 1)
          {
            ATCD_DBG_CONN
            conn->state = ATCD_CONN_STATE_OPEN;
            atcd.parser.buff_pos = atcd.parser.line_pos;
            if(conn->callback != NULL && (conn->cb_events & ATCD_CONN_EV_OPEN) != 0) conn->callback(conn, ATCD_CONN_EV_OPEN);
          }
          else
          {
            ATCD_DBG_CONN_OP_ERR
          }
        }
        else
        {
          ATCD_DBG_CONN_UNREG
        }
      }
      else ATCD_DBG_CONN_SISW_RNG_E

      atcd.parser.buff_pos = atcd.parser.line_pos;  //vymaze prijaty radek
      return 1;
    }
    else if(strncmp(atcd.parser.buff + atcd.parser.line_pos + 1, ", CLOSED\r\n", strlen(", CLOSED\r\n")) == 0)
    {
      conn_id = (uint8_t)atoi(atcd.parser.buff + atcd.parser.line_pos);

      if(conn_id < ATCD_CONN_MAX_NUMBER)
      {
        ATCD_DBG_CONN_CLOSE_DET
        conn = atcd.conns.conn[conn_id];
        if(conn != NULL)
        {
          atcd.conns.conn[conn_id] = NULL;
          conn->state = ATCD_CONN_STATE_CLOSE;
          conn->num   = ATCD_CONN_NO_NUM;

          atcd.parser.buff_pos = atcd.parser.line_pos;
          if(conn->callback != NULL && (conn->cb_events & ATCD_CONN_EV_CLOSE) != 0) conn->callback(conn, ATCD_CONN_EV_CLOSE);
          atcd_gprs_autoconn();
        }
        else
        {
          ATCD_DBG_CONN_UNREG
        }
      }
      else ATCD_DBG_CONN_CLOSE_RNG_E

      atcd.parser.buff_pos = atcd.parser.line_pos;  //vymaze prijaty radek
      return 1;
    }
    else if(strncmp(atcd.parser.buff + atcd.parser.line_pos + 1, ", CLOSE OK\r\n", strlen(", CLOSE OK\r\n")) == 0)
    {
      conn_id = (uint8_t)atoi(atcd.parser.buff + atcd.parser.line_pos);

      if(conn_id < ATCD_CONN_MAX_NUMBER)
      {
        ATCD_DBG_CONN_CLOSE_OK_DET
        conn = atcd.conns.conn[conn_id];
        if(conn != NULL)
        {
          if(conn->state == ATCD_CONN_STATE_W_CLOSE)
          {
            //TODO: Muze probohat ATC kteremu se prepisi data - opravit!!! //Nechapu...
            //TODO: Tohle bych si rad nechal vysvetlit...

            if(strncmp(atcd.at_cmd_result_buff, atcd.parser.buff + atcd.parser.line_pos, strlen("0, CLOSE OK\r\n")) == 0)
            {
              // AT prikaz byl prave dokoncen
              ATCD_DBG_ATC_OK_DET

              atcd.at_cmd.result = ATCD_ATC_RESULT_OK;
              atcd_atc_complete(&atcd.at_cmd);
            }
          }

          atcd.conns.conn[conn_id] = NULL;
          conn->state = ATCD_CONN_STATE_CLOSE;
          conn->num   = ATCD_CONN_NO_NUM;

          atcd.parser.buff_pos = atcd.parser.line_pos;
          if(conn->callback != NULL && (conn->cb_events & ATCD_CONN_EV_CLOSE) != 0) conn->callback(conn, ATCD_CONN_EV_CLOSE);
          atcd_gprs_autoconn();
        }
        else
        {
          ATCD_DBG_CONN_UNREG
        }
      }
      else ATCD_DBG_CONN_CLOSE_OK_RE

      atcd.parser.buff_pos = atcd.parser.line_pos;  //vymaze prijaty radek
      return 1;
    }
    else if(strncmp(atcd.parser.buff + atcd.parser.line_pos + 1, ", CONNECT FAIL\r\n", strlen(", CONNECT FAIL\r\n")) == 0)
    {
      conn_id = (uint8_t)atoi(atcd.parser.buff + atcd.parser.line_pos);

      if(conn_id < ATCD_CONN_MAX_NUMBER)
      {
        ATCD_DBG_CONN_FAIL_DET
        conn = atcd.conns.conn[conn_id];
        if(conn != NULL)
        {
          atcd.conns.conn[conn_id] = NULL;
          conn->state = ATCD_CONN_STATE_FAIL;
          conn->num   = ATCD_CONN_NO_NUM;

          atcd.parser.buff_pos = atcd.parser.line_pos;
          if(conn->callback != NULL && (conn->cb_events & ATCD_CONN_EV_FAIL) != 0) conn->callback(conn, ATCD_CONN_EV_FAIL);
          atcd_gprs_autoconn();
        }
        else
        {
          ATCD_DBG_CONN_UNREG
        }
      }
      else ATCD_DBG_CONN_FAIL_RNG_E

      atcd.parser.buff_pos = atcd.parser.line_pos;  //vymaze prijaty radek
      return 1;
    }
    else if(strncmp(atcd.parser.buff + atcd.parser.line_pos, "^SIS: ", strlen("^SIS: ")) == 0)
    {
      conn_id = (uint8_t)atoi(atcd.parser.buff + atcd.parser.line_pos + strlen("^SIS: "));

      if(conn_id < ATCD_CONN_MAX_NUMBER)
      {
        ATCD_DBG_CONN_SIS_DET
        conn = atcd.conns.conn[conn_id];
        if(conn != NULL)
        {
          op = (uint8_t)atoi(atcd.parser.buff + atcd.parser.line_pos + strlen("^SIS: x,"));

          if(op == 0)
          {
            ATCD_DBG_CONN_SIS_CLOSE

            atcd.conns.conn[conn_id] = NULL;
            conn->state = ATCD_CONN_STATE_CLOSE;
            conn->num   = ATCD_CONN_NO_NUM;

            atcd.parser.buff_pos = atcd.parser.line_pos;
            if(conn->callback != NULL && (conn->cb_events & ATCD_CONN_EV_CLOSE) != 0) conn->callback(conn, ATCD_CONN_EV_CLOSE);
            atcd_gprs_autoconn();
          }
          else
          {
            ATCD_DBG_CONN_SIS_PARAM_YE
          }
        }
        else
        {
          ATCD_DBG_CONN_UNREG
        }
      }
      else ATCD_DBG_CONN_SIS_PARAM_XE

      atcd.parser.buff_pos = atcd.parser.line_pos;  //vymaze prijaty radek
      return 1;
    }
    else if(strncmp(atcd.parser.buff + atcd.parser.line_pos + 1, ", SEND OK\r\n", strlen(", SEND OK\r\n")) == 0)
    {
      ATCD_DBG_CONN_SEND_OK_DET

      atcd.parser.buff_pos = atcd.parser.line_pos;   //vymaze prijaty radek

      #if(ATCD_USE_DEVICE == ATCD_SIM868 || ATCD_USE_DEVICE == ATCD_SIM7000)
      //TODO: SIM868 v defaultu ceka v ramci provadeneho ATC na odeslani dat a pak to ohlasi.
      //      Lze tedy cekat na ", SEND OK" jako na vysledek ATC. Lepe by bylo jej prepnout
      //      do async modu, kdy je odesle hned a ohlasi "DATA ACCEPT:<n>,<length>".
      //      Prepina se pomoci AT+CIPQSEND.
      //      M.P.: Jinak to sem naptri, ukonceni ATC by melo byy reseno v modulu ATC...

      /* MV: a co reknes na tohle?
[17.037] AT+CIPSEND=0,28
[17.042] ATCD: ATC: Odesilani bylo dokoceno - prechazime na W_ECHO...
[17.043] AT+CIPSEND=0,28
[17.045] ATCD: ATC: ECHO detected.
[17.045] > [17.046] ATCD: ATC: Prompt ">" detected.
[17.046] ATCD: ATC: Odesilani dat bylo zahajeno.
[17.046]  #06 00 00 65 00 00 0F A5 15 06 07 12 24 1E 34 30 30 35 31 36 36 35 30 30 30 30 30 90
[17.053] ATCD: ATC: Odesilani bylo dokoceno - prechazime na W_END...
[17.054]  #06 00 00 65 00 00 0F A5 15 06 07 12 24 1E 34 30 30 35 31 36 36 35 30 30 30 30 30 90
$PMTK011,MTKGPS*08
      -> unsolicited, preskoceno
$PMTK010,001*2E
      -> unsolicited, preskoceno

1, CONNECT OK
[17.281] ATCD: CONN: x, CONNECT OK detect.
[17.282]
>>> coap_make_request

[17.283] send request
[17.283] wait for response ...
[17.284] ATCD: CONN: Ukladam data k odeslani do bufferu.
[17.791]
0, SEND OK
[17.793] ATCD: CONN: x, SEND OK.
[17.794] ATCD: ATC: OK detected.
       */
      if(atcd.parser.at_cmd_top != NULL && atcd.parser.at_cmd_top->state == ATCD_ATC_STATE_W_END)
      {
        // AT prikaz byl prave dokoncen
        ATCD_DBG_ATC_OK_DET
        atcd.parser.at_cmd_top->result = ATCD_ATC_RESULT_OK;
        atcd_atc_complete(atcd.parser.at_cmd_top);
      }
      #endif

      // nad jakym spojenim to bude?
      //if(conn->callback != NULL && (conn->events & ATCD_CONN_EV_SEND_OK) != 0) conn->callback(conn, ATCD_CONN_EV_SEND_OK);
      return 1;
    }
    else if(strncmp(atcd.parser.buff + atcd.parser.line_pos + 1, ", ALREADY CONNECT\r\n", strlen(", ALREADY CONNECT\r\n")) == 0)
    {
      conn_id = (uint8_t)atoi(atcd.parser.buff + atcd.parser.line_pos);

      if(conn_id < ATCD_CONN_MAX_NUMBER)
      {
        ATCD_DBG_CONN_ALRD_CON_DET
        conn = atcd.conns.conn[conn_id];
        if(conn != NULL)
        {
          //TODO pokud spojeni existuje, tak jej chceme opravdu zavrit?
          atcd_conn_close(conn, 1);   //Timto dojde i k uzavreni neznameho spojeni na strane modemu
        }
        else
        {
          ATCD_DBG_CONN_UNREG
        }
      }
      else ATCD_DBG_CONN_RNG_E

      atcd.parser.buff_pos = atcd.parser.line_pos;   //vymaze prijaty radek
      return 1;
    }
    //CONNECT FAIL
    else if(strncmp(atcd.parser.buff + atcd.parser.line_pos, "DNS Fail\r\n", strlen("DNS Fail\r\n")) == 0)
    {
      ATCD_DBG_CONN_DNS_FAIL_DET

      //TODO stavy dle doku...

      atcd.parser.buff_pos = atcd.parser.line_pos;   //vymaze prijaty radek

      // nad jakym spojenim to bude?
      //if(conn->callback != NULL && (conn->events & ATCD_CONN_EV_FAIL) != 0) conn->callback(conn, ATCD_CONN_EV_FAIL);
      return 1;
    }
    else 
    {

    }
  }

  return 0;
}
//------------------------------------------------------------------------------
uint8_t atcd_conn_data_proc(char ch)
{
  atcd_conn_t *conn;

  // If parser in IPD receiving mode
  if(atcd.parser.mode == ATCD_P_MODE_IPD)
  {
    atcd.gprs.stat.bytes_recv++;
    conn = atcd.conns.conn[atcd.parser.rx_conn_num];
    // If any connection
    if(conn != NULL)
    {
      // Pokud je v bufferu misto
      // Zapise prijaty byte do bufferu
      if (rbuff_write_b(&conn->rx_rbuff, ch))
        conn->in_overflow=0;
      else
      {
        if (!conn->in_overflow)
        {
          ATCD_DBG_CONN_BUFF_E //staci 1x, nemusis pro kazdy znak
          conn->in_overflow=1;
        };
        conn->cb_events |=  ATCD_CONN_EV_OVERRUN;
      }

      atcd.parser.buff_pos++;

      // Pokud jsme dosahli konce IPD bloku
      //if(atcd.buff_pos >= atcd.parser.ipd_len)
      if(atcd.parser.buff_pos >= atcd.parser.rx_data_len)
      {
        ATCD_DBG_CONN_IPD_END

        atcd.parser.mode = ATCD_P_MODE_IDLE;
        atcd.parser.buff_pos = 0;
        atcd.parser.line_pos = 0;

        conn->cb_events |= ATCD_CONN_EV_NEW_DATA;
        if(conn->callback != NULL) conn->callback(conn, ATCD_CONN_EV_NEW_DATA);
      }
    }
    else
    {
      ATCD_DBG_CONN_IPD_ERR

      atcd.parser.mode = ATCD_P_MODE_IDLE;
      atcd.parser.buff_pos = 0;
      atcd.parser.line_pos = 0;
    }
    return 1;
  }

  return 0;
}
//------------------------------------------------------------------------------
