/*
 * usart.c
 *
 *  Created on: Apr 9, 2011
 *      Author: Martin
 */
//------------------------------------------------------------------------------
#include "atcd_wifi.h"
#include "atcd.h"

extern atcd_t atcd;

//------------------------------------------------------------------------------
void atcd_wifi_init()
{


}
//------------------------------------------------------------------------------
void atcd_wifi_reset()
{

}
//------------------------------------------------------------------------------
void atcd_wifi_proc()
{


}
//------------------------------------------------------------------------------
uint8_t atcd_wifi_asc_msg()
{
  if(strncmp(atcd.parser.buff + atcd.parser.line_pos, "WIFI CONNECTED\r\n", strlen("WIFI CONNECTED\r\n")) == 0)
  {
    ATCD_DBG_WIFI_CONN_DET
    atcd.wifi.state = ATCD_WIFI_STATE_W_DHCP;
    atcd.parser.buff_pos = atcd.parser.line_pos;
    if(atcd.wifi.callback != NULL && (atcd.wifi.events & ATCD_WIFI_EV_CONN) != 0) atcd.wifi.callback(ATCD_WIFI_EV_CONN);
    return 1;
  }
  
  if(strncmp(atcd.parser.buff + atcd.parser.line_pos, "WIFI GOT IP\r\n", strlen("WIFI GOT IP\r\n")) == 0)
  {
    ATCD_DBG_WIFI_GOT_IP
    atcd.wifi.state = ATCD_WIFI_STATE_CONN;
    atcd.parser.buff_pos = atcd.parser.line_pos;
    if(atcd.wifi.callback != NULL && (atcd.wifi.events & ATCD_WIFI_EV_GOT_IP) != 0) atcd.wifi.callback(ATCD_WIFI_EV_GOT_IP);
    return 1;
  }

  if(strncmp(atcd.parser.buff + atcd.parser.line_pos, "WIFI DISCONNECT\r\n", strlen("WIFI DISCONNECT\r\n")) == 0)
  {
    ATCD_DBG_WIFI_DISCONN_DET
    atcd.wifi.state = ATCD_WIFI_STATE_DISCONN;
    atcd.parser.buff_pos = atcd.parser.line_pos;
    atcd_conn_reset_all();
    if(atcd.wifi.callback != NULL && (atcd.wifi.events & ATCD_WIFI_EV_DISCONN) != 0) atcd.wifi.callback(ATCD_WIFI_EV_DISCONN);
    return 1;
  }

  return 0;
}
//------------------------------------------------------------------------------
