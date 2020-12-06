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
      if((conn->state == ATCD_CONN_STATE_W_OPEN || conn->state == ATCD_CONN_STATE_OPENING || conn->state == ATCD_CONN_STATE_CLOSING) && atcd_get_ms() - conn->timer > 15000)
      {
        ATCD_DBG_CONN_TIM

        conn->state = ATCD_CONN_STATE_W_CLOSE;
        conn->timer = atcd_get_ms();

        if(conn->callback != NULL && (conn->cb_events & ATCD_CONN_EV_CLOSE) != 0) conn->callback(conn, ATCD_CONN_EV_CLOSE);
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
}
//------------------------------------------------------------------------------
void atcd_conn_init(atcd_conn_t *conn, uint8_t *rx_buff, uint16_t rx_buff_size, uint8_t *tx_buff, uint16_t tx_buff_size)     //init connection
{
  conn->num      = ATCD_CONN_NO_NUM;
  conn->state    = ATCD_CONN_STATE_CLOSE;
  
  conn->protocol = ATCD_CONN_T_TCP;
  conn->host     = NULL;
  conn->port     = 0;

  conn->timeout    = 10000;
  conn->timer      = 0;
  
  rbuff_init(&conn->rx_rbuff, rx_buff, rx_buff_size);
  rbuff_init(&conn->tx_rbuff, tx_buff, tx_buff_size);
  
  conn->cb_events  = ATCD_CONN_EV_ALL;
  conn->callback   = NULL;
}
//------------------------------------------------------------------------------
void atcd_conn_open(atcd_conn_t *conn, char* dest, uint16_t port, uint8_t type) //open conenction
{
  uint8_t i;

  conn->protocol = type;
  conn->host     = dest;
  conn->port     = port;

  conn->timeout    = 10000;
  conn->timer      = atcd_get_ms();
  
  rbuff_reset(&conn->rx_rbuff);
  rbuff_reset(&conn->tx_rbuff);
  
  conn->cb_events = ATCD_CONN_EV_NONE;
  conn->callback = NULL;
  
  for(i = 0; i < ATCD_CONN_MAX_NUMBER; i++)
  {
    if(atcd.conns.conn[i] == NULL)
    {
      ATCD_DBG_CONN_ALLOC
      
      atcd.conns.conn[i] = conn;
      conn->num = i;
      
      conn->state = ATCD_CONN_STATE_W_OPEN;
      return;
    }
  }
  
  ATCD_DBG_CONN_ALLOC_ERR
  conn->state = ATCD_CONN_STATE_MAX_CONN;
  conn->num   = ATCD_CONN_NO_NUM;
  return;
}
//------------------------------------------------------------------------------
void atcd_conn_write(atcd_conn_t *conn, uint8_t* data, uint16_t len)   //write data to connection
{
  ATCD_DBG_CONN_WRITE
  rbuff_write(&conn->tx_rbuff, data, len);
}
//------------------------------------------------------------------------------
void atcd_conn_close(atcd_conn_t *conn)                       //close connection
{
  ATCD_DBG_CONN_CLOSE_W
  conn->state = ATCD_CONN_STATE_W_CLOSE;
  conn->timer = atcd_get_ms();
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
        atcd.parser.mode  = ATCD_P_MODE_IPD;
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

  if(atcd.gprs.state == ATCD_GPRS_STATE_CONN)     //pokud je pripojeno WiFi a ma IP addr
  {
    //if(strncmp(atcd.buff + atcd.line_pos + 1, ",CONNECT\r\n", strlen(",CONNECT\r\n")) == 0)
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
      conn_id = (uint8_t)atoi(atcd.parser.buff + atcd.parser.line_pos + strlen("^SISW: "));

      if(conn_id < ATCD_CONN_MAX_NUMBER)
      {
        ATCD_DBG_CONN_SISW_DET
        conn = atcd.conns.conn[conn_id];
        if(conn != NULL)
        {
          op = (uint8_t)atoi(atcd.parser.buff + atcd.parser.line_pos + strlen("^SISW: x,"));

          if(atcd.parser.mode == ATCD_P_MODE_TX_PEND)
          {
            // Testovat kolik muzeme zapsat dat...
            //
            //

            ATCD_DBG_CONN_SISW_P
            atcd.parser.mode = ATCD_P_MODE_PROMPT;

            atcd.parser.buff_pos = 0;
            atcd.parser.line_pos = 0;

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
              ATCD_DBG_CONN_SISW_P_DE
            }

            atcd.parser.mode = ATCD_P_MODE_ATC;

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
          atcd.conns.conn[conn_id] = NULL;
          conn->state = ATCD_CONN_STATE_CLOSE;
          conn->num   = ATCD_CONN_NO_NUM;

          atcd.parser.buff_pos = atcd.parser.line_pos;
          if(conn->callback != NULL && (conn->cb_events & ATCD_CONN_EV_CLOSE) != 0) conn->callback(conn, ATCD_CONN_EV_CLOSE);
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

      // nad jakym spojenim to bude?
      //if(conn->callback != NULL && (conn->events & ATCD_CONN_EV_SEND_OK) != 0) conn->callback(conn, ATCD_CONN_EV_SEND_OK);
      return 1;
    }
    //CONNECT FAIL
    else if(strncmp(atcd.parser.buff + atcd.parser.line_pos, "DNS Fail\r\n", strlen("DNS Fail\r\n")) == 0)
    {
      ATCD_DBG_CONN_DNS_FAIL_DET

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
    conn = atcd.conns.conn[atcd.parser.rx_conn_num];
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
        ATCD_DBG_CONN_BUFF_E
        conn->cb_events |=  ATCD_CONN_EV_OVERRUN;
      }   

      atcd.parser.buff_pos++;

      // Pokud jsme dosahli komce IPD bloku
      //if(atcd.buff_pos >= atcd.parser.ipd_len)
      if(atcd.parser.buff_pos >= atcd.parser.rx_data_len)
      {
        ATCD_DBG_CONN_IPD_END
        atcd.parser.mode = ATCD_P_MODE_ATC;

        atcd.parser.buff_pos = 0;
        atcd.parser.line_pos = 0;

        conn->cb_events |= ATCD_CONN_EV_NEW_DATA;
        if(conn->callback != NULL) conn->callback(conn, ATCD_CONN_EV_NEW_DATA);
      }
    }
    else
    {
      ATCD_DBG_CONN_IPD_ERR
      atcd.parser.mode = ATCD_P_MODE_ATC;

      atcd.parser.buff_pos = 0;
      atcd.parser.line_pos = 0;
    }
	  return 1;
  }

  return 0;
}
//------------------------------------------------------------------------------
