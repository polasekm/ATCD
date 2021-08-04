/*-----------------------------------------------------------------------------*/
/*
 * usart.h
 *
 *  Created on: Apr 9, 2011
 *      Author: Martin Polasek
 */
/*-----------------------------------------------------------------------------*/
#ifndef ATCD_GPRS_H_INCLUDED
#define ATCD_GPRS_H_INCLUDED

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>     /* atoi */
#include <stdio.h>

#include "atcd_atc.h"
#include "atcd_phone.h"

/* Exported functions ------------------------------------------------------- */
/* Defines -------------------------------------------------------------------*/

// GPRS State
#define ATCD_GPRS_STATE_DISCONN       0
#define ATCD_GPRS_STATE_DISCONNING    1
#define ATCD_GPRS_STATE_CONN          2
#define ATCD_GPRS_STATE_CONNECTING    3

// GPRS Events
#define ATCD_GPRS_EV_NONE             0

//------------------------------------------------------------------------------
typedef struct
{
  uint8_t state;                  //gprs state
  uint32_t timer;                 //current operation timer
  uint8_t autoclose_bearer;       //bool

  char *apn;                      //GPRS access point name
  char *name;                     //GPRS user name
  char *psswd;                    //GPRS password
  
  char *ip[16];                     //IP address

  uint8_t cb_events;              //gprs events
  void (*callback)(uint8_t);      //events callback

} atcd_gprs_t;

// Functions -------------------------------------------------------------------

// GPRS
void atcd_gprs_init();                           //inializace gprs
//--------------------------------------------------------------
void atcd_gprs_autoconn();
void atcd_gprs_autoclose_bearer(uint8_t autoclose_bearer); //zavirat bearer pokud neni zadne aktivni spojeni
void atcd_gprs_connect();                        //connect gprs
void atcd_gprs_disconnect();                     //disconnect gprs

void atcd_gprs_set_apn(char *apn, char *name, char *psswd);  //set apn, name and psswd

void atcd_gprs_proc();                           //gprs processing
void atcd_gprs_reset();                          //gprs state reset

uint8_t atcd_gprs_state();
//------------------------------------------------------------------------------
#endif /* ATCD_GPRS_H_INCLUDED */
