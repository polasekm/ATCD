/*-----------------------------------------------------------------------------*/
/*
 * atcd_conn.h
 *
 *  Created on: Apr 9, 2011
 *      Author: Martin Polasek
 */
/*-----------------------------------------------------------------------------*/
#ifndef ATCD_CONN_H_INCLUDED
#define ATCD_CONN_H_INCLUDED

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>     /* atoi */
#include <stdio.h>

#include "../Libs/rbuff/rbuff.h"

#include "atcd_atc.h"

/* Exported functions ------------------------------------------------------- */

/* Defines -------------------------------------------------------------------*/

// Stav spojeni
typedef enum
{
	ATCD_CONN_STATE_CLOSE       =0,
    ATCD_CONN_STATE_FAIL        =1,
	ATCD_CONN_STATE_TIMEOUT     =2,
	ATCD_CONN_STATE_MAX_CONN    =3,
	ATCD_CONN_STATE_CLOSING     =4,
	ATCD_CONN_STATE_W_CLOSE     =5,
	ATCD_CONN_STATE_OPENING     =6,
	ATCD_CONN_STATE_W_OPEN1     =7,
	ATCD_CONN_STATE_W_OPENFAILED=8,
	ATCD_CONN_STATE_OPEN        =9
} atcd_conn_state_e;

// Udalosti spojeni
#define ATCD_CONN_EV_NONE           0x00
#define ATCD_CONN_EV_NEW_DATA       0b00000001
#define ATCD_CONN_EV_SEND_OK        0b00000010
#define ATCD_CONN_EV_OVERRUN        0b00000100
#define ATCD_CONN_EV_OVERWRITE      0b00001000
#define ATCD_CONN_EV_OPEN           0b00010000
#define ATCD_CONN_EV_CLOSE          0b00100000
#define ATCD_CONN_EV_FAIL           0b01000000
#define ATCD_CONN_EV_ALL            0xFF

// Typ spojeni
typedef enum
{
	ATCD_CONN_T_TCP             =0,
	ATCD_CONN_T_UDP             =1
} atcd_conn_type_e;

// Cislo spojeni
#define ATCD_CONN_NO_NUM            0xFF

//------------------------------------------------------------------------------
typedef struct atcd_conn
{
  uint8_t num;                    //number
  atcd_conn_state_e state;                  //state
  
  atcd_conn_type_e protocol;               //protocol (TCP/UDP)
  const char *host;                     //destination host
  uint16_t port;                  //destinaion port
  
  uint16_t timeout;               //timeout in ms
  uint32_t timer;                 //connection timer
  
  rbuff_t rx_rbuff;               //kruhovy buffer pro prijimana data
  rbuff_t tx_rbuff;               //kruhovy buffer pro odesilana data
  
  uint8_t cb_events;              //connection events
  void (*callback)(struct atcd_conn*, uint8_t);  //events callback
  uint8_t dontPrint; //nevypisuj na terminal, nestiha vypisovat firmware

} atcd_conn_t;

// Functions -------------------------------------------------------------------

// Connections
void atcd_conn_init(atcd_conn_t *conn, uint8_t *rx_buff, uint16_t rx_buff_size, uint8_t *tx_buff, uint16_t tx_buff_size);  //init connection
void atcd_conn_open(atcd_conn_t *conn, const char *dest, uint16_t port, atcd_conn_type_e type); //open conenction
void atcd_conn_write(atcd_conn_t *conn, const uint8_t* data, uint16_t len);            //write data to connection
uint32_t atcd_conn_write_rb(atcd_conn_t *conn, rbuff_t *data);
void atcd_conn_close(atcd_conn_t *conn);                                         //close connection
void atcd_conn_free(atcd_conn_t *conn);                                          //free connection

atcd_conn_state_e atcd_conn_state(const atcd_conn_t *conn);

// Connections
void atcd_conn_proc();                    //connections processing
void atcd_conn_reset_all();               //force close all active connections

uint8_t atcd_conn_ipd_tst();
uint8_t atcd_conn_asc_msg();
uint8_t atcd_conn_data_proc(char ch);
//------------------------------------------------------------------------------
#endif /* ATCD_CONN_H_INCLUDED */
