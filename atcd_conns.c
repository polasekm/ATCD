/*
 * usart.c
 *
 *  Created on: Apr 9, 2011
 *      Author: Martin
 */
//------------------------------------------------------------------------------
#include "atcd_conns.h"
#include "atcd.h"

extern atcd_t atcd;

//------------------------------------------------------------------------------
void atcd_conns_reset()
{
  atcd_conn_reset_all();

  atcd.conns.timer = atcd_get_ms();
  atcd.conns.conn_num_proc = 0;
}
//------------------------------------------------------------------------------
