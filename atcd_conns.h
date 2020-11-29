/*-----------------------------------------------------------------------------*/
/*
 * usart.h
 *
 *  Created on: Apr 9, 2011
 *      Author: Martin Polasek
 */
/*-----------------------------------------------------------------------------*/
#ifndef ATCD_CONNS_H_INCLUDED
#define ATCD_CONNS_H_INCLUDED

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>     /* atoi */
#include <stdio.h>

#include "atcd_config.h"

#include "atcd_atc.h"
#include "atcd_conn.h"

/* Exported functions ------------------------------------------------------- */

/* Defines -------------------------------------------------------------------*/

//------------------------------------------------------------------------------
typedef struct
{
  atcd_conn_t *conn[ATCD_CONN_MAX_NUMBER]; //Pointers to connection structs
  uint8_t conn_num_proc;          //number of processing connections

  atcd_at_cmd_t at_cmd;           //AT cmd for internal usage
  char at_cmd_buff[75];           //buffer pro sestaveny AT prikaz
  
  uint32_t timer;                 //connection timer

} atcd_conns_t;


// Functions -------------------------------------------------------------------
void atcd_conns_reset();

//------------------------------------------------------------------------------
#endif /* ATCD_CONNS_H_INCLUDED */
