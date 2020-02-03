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
      if((conn->state == ATCD_CONN_STATE_W_OPEN || conn->state == ATCD_CONN_STATE_OPENING || conn->state == ATCD_CONN_STATE_W_CLOSE || conn->state == ATCD_CONN_STATE_CLOSING) && atcd_get_ms() - conn->timer > 15000)
      {
        atcd_dbg_warn("CONN: Spojeni vyprsel timeout - rusim jej.\r\n");

        atcd.conns.conn[i] = NULL;
        conn->state = ATCD_CONN_STATE_TIMEOUT;
        conn->num   = ATCD_CONN_NO_NUM;
        conn->at_cmd_seq = 0;
        if(conn->at_cmd.state == ATCD_ATC_STATE_WAIT) atcd_atc_cancell(&conn->at_cmd);

        if(conn->callback != NULL && (conn->events & ATCD_CONN_EV_CLOSE) != 0) conn->callback(conn, ATCD_CONN_EV_CLOSE);
      }

      if(conn->state == ATCD_CONN_STATE_W_OPEN)
      {
        atcd_conn_open_seq(conn);
      }
      else if(conn->state == ATCD_CONN_STATE_OPEN && rbuff_size(&conn->tx_rbuff) != 0)
      {
        atcd_conn_write_seq(conn);
      }

      atcd_conn_check_state_seq(conn);
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
      atcd_atc_init(&conn->at_cmd);
      atcd.conns.conn[i] = NULL;

      //Opravbdu event na close? Spojeni je ruseno nasilne - nemuselo byt ani navazano, mozna spise novy priznak...
      if(conn->callback != NULL && (conn->events & ATCD_CONN_EV_CLOSE) != 0) conn->callback(conn, ATCD_CONN_EV_CLOSE);
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

  atcd_atc_init(&conn->at_cmd);
  conn->at_cmd_buff[0] = 0;

  conn->at_cmd_seq = 0;
  conn->timeout    = 10000;
  conn->timer      = 0;
  
  rbuff_init(&conn->rx_rbuff, rx_buff, rx_buff_size);
  rbuff_init(&conn->tx_rbuff, tx_buff, tx_buff_size);
  //conn->tx_data_len = 0;
  
  conn->events   = ATCD_CONN_EV_NONE;
  conn->callback = NULL;
}
//------------------------------------------------------------------------------
void atcd_conn_open(atcd_conn_t *conn, char* dest, uint16_t port, uint8_t type) //open conenction
{
  uint8_t i;

  conn->protocol = type;
  conn->host     = dest;
  conn->port     = port;

  if(conn->at_cmd.state == ATCD_ATC_STATE_WAIT) atcd_atc_cancell(&conn->at_cmd);

  // blbost - atc muze byt dale pouzivana...
  //atcd_atc_init(&conn->at_cmd);
  //conn->at_cmd_buff[0] = 0;
  // opraveno jinde... ve spusteni init sekvence...

  conn->at_cmd_seq = 0;
  conn->timeout    = 10000;
  conn->timer      = atcd_get_ms();
  
  rbuff_reset(&conn->rx_rbuff);
  rbuff_reset(&conn->tx_rbuff);
  
  conn->events = ATCD_CONN_EV_NONE;
  conn->callback = NULL;
  
  for(i = 0; i < ATCD_CONN_MAX_NUMBER; i++)
  {
    if(atcd.conns.conn[i] == NULL)
    {
      atcd_dbg_inf("CONN: Spojeni prirazeno.\r\n");
      
      atcd.conns.conn[i] = conn;
      conn->num = i;
      
      conn->state = ATCD_CONN_STATE_W_OPEN;
      return;
    }
  }
  
  atcd_dbg_warn("CONN: Spojeni neprirazeno - vycerpan max pocet.\r\n");
  conn->state = ATCD_CONN_STATE_MAX_CONN;
  conn->num   = ATCD_CONN_NO_NUM;
  return;

  /*if(i == ATCD_CONN_MAX_NUMBER)
  {
    atcd_dbg_warn("CONN neprirazeno - vycerpan max pocet...\r\n");
    conn->state = ATCD_CONN_STATE_MAX_CONN;
    conn->num   = ATCD_CONN_NO_NUM;
    return;
  }*/
}
//------------------------------------------------------------------------------
void atcd_conn_write(atcd_conn_t *conn, uint8_t* data, uint16_t len)   //write data to connection
{
  atcd_dbg_inf("CONN: Ukladam data k odeslani do bufferu.\r\n");
  rbuff_write(&conn->tx_rbuff, data, len);
}
//------------------------------------------------------------------------------
void atcd_conn_close(atcd_conn_t *conn)                       //close connection
{
  atcd_dbg_inf("CONN: Nastavuji spojeni jako cekajici na uzavreni.\r\n");
  conn->state = ATCD_CONN_STATE_W_CLOSE;
  conn->timer = atcd_get_ms();
}
//------------------------------------------------------------------------------
void atcd_conn_free(atcd_conn_t *conn)                         //free connection
{
	if(conn->num < ATCD_CONN_MAX_NUMBER)
  {
    atcd_dbg_inf("CONN: Uvolnuji cislo spojeni.\r\n");
    atcd.conns.conn[conn->num] = NULL;

    conn->state = ATCD_CONN_STATE_CLOSE;
    conn->num   = ATCD_CONN_NO_NUM;
    atcd_atc_init(&conn->at_cmd);
    if(conn->callback != NULL && (conn->events & ATCD_CONN_EV_CLOSE) != 0) conn->callback(conn, ATCD_CONN_EV_CLOSE);
  }
  else atcd_dbg_err("CONN: Cislo spojeni k uvolneni je mimo rozsah!\r\n");
}
//------------------------------------------------------------------------------
