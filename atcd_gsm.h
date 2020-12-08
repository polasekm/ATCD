/*-----------------------------------------------------------------------------*/
/*
 * usart.h
 *
 *  Created on: Apr 9, 2011
 *      Author: Martin Polasek
 */
/*-----------------------------------------------------------------------------*/
#ifndef ATCD_GSM_H_INCLUDED
#define ATCD_GSM_H_INCLUDED

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>     /* atoi */
#include <stdio.h>

#include "atcd_config.h"

#include "atcd_atc.h"

/* Exported functions ------------------------------------------------------- */

/* Defines -------------------------------------------------------------------*/

// Stav registrace zazeni
#define ATCD_REG_STATE_OFF          0
#define ATCD_REG_STATE_HOME         1
#define ATCD_REG_STATE_SEARCHING    2
#define ATCD_REG_STATE_DENIED       3
#define ATCD_REG_STATE_UNKNOWN      4
#define ATCD_REG_STATE_ROAMING      5
#define ATCD_REG_STATE_6            6
#define ATCD_REG_STATE_7            7
#define ATCD_REG_STATE_EMERGENCY    8
#define ATCD_REG_STATE_9            9
#define ATCD_REG_STATE_10           10

//Udalosti GSM zarizeni
#define ATCD_GSM_EV_NONE            0x00
#define ATCD_GSM_EV_PIN             0b00000001
#define ATCD_GSM_EV_REG             0b00000010
#define ATCD_GSM_EV_ALL             0xFF

//------------------------------------------------------------------------------
typedef struct
{
  uint8_t state;                  //registration state
  //uint8_t roaming;              //roaming state
  //uint8_t pin_state;            //pin state

  char *pin;                      //PIN

  uint8_t cb_events;              //GSM events
  void (*callback)(uint8_t);      //events callback

} atcd_gsm_t;

// Functions -------------------------------------------------------------------
void atcd_gsm_init();
void atcd_gsm_reset();
void atcd_gsm_proc();
uint8_t atcd_gsm_asc_msg();

//------------------------------------------------------------------------------
#endif /* ATCD_GSM_H_INCLUDED */
