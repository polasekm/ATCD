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

// Stav registrace zarizeni
typedef enum
{
  ATCD_REG_STATE_OFF          = 0,
  ATCD_REG_STATE_HOME         = 1,
  ATCD_REG_STATE_SEARCHING    = 2,
  ATCD_REG_STATE_DENIED       = 3,
  ATCD_REG_STATE_UNKNOWN      = 4,
  ATCD_REG_STATE_ROAMING      = 5,
  ATCD_REG_STATE_6            = 6,
  ATCD_REG_STATE_7            = 7,
  ATCD_REG_STATE_EMERGENCY    = 8,
  ATCD_REG_STATE_9            = 9,
  ATCD_REG_STATE_10           =10,
  ATCD_REG_STATE__MAX = 10} atcd_reg_state_e;

//Udalosti GSM zarizeni
#define ATCD_GSM_EV_NONE            0x00
#define ATCD_GSM_EV_PIN             0b00000001
#define ATCD_GSM_EV_REG             0b00000010
#define ATCD_GSM_EV_ALL             0xFF

//------------------------------------------------------------------------------
typedef struct
{
  atcd_reg_state_e state;         //registration state

  uint8_t cb_events;              //GSM events
  void (*callback)(uint8_t);      //events callback

  int8_t gsm_sig; //-1, 0..100
} atcd_gsm_t;

// Functions -------------------------------------------------------------------
void atcd_gsm_init();
void atcd_gsm_reset();
void atcd_gsm_proc();
uint8_t atcd_gsm_asc_msg();

atcd_reg_state_e atcd_gsm_state();
int8_t atcd_gsm_sig();
//------------------------------------------------------------------------------
#endif /* ATCD_GSM_H_INCLUDED */
