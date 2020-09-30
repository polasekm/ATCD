/*-----------------------------------------------------------------------------*/
/*
 * usart.h
 *
 *  Created on: Apr 9, 2011
 *      Author: Martin Polasek
 */
/*-----------------------------------------------------------------------------*/
#ifndef ATCD_SIM_H_INCLUDED
#define ATCD_SIM_H_INCLUDED

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>     /* atoi */
#include <stdio.h>

#include "atcd_config.h"

#include "atcd_atc.h"

/* Exported functions ------------------------------------------------------- */

/* Defines -------------------------------------------------------------------*/

// Stav GSM registrace zazeni
#define ATCD_PIN_STATE_NONE         0
#define ATCD_PIN_STATE_REQ          1
#define ATCD_PIN_STATE_WAIT         2
#define ATCD_PIN_STATE_OK           3
#define ATCD_PIN_STATE_WRONG        4
#define ATCD_PIN_STATE_UNKNOWN      5

//------------------------------------------------------------------------------
typedef struct
{
  uint8_t pin_state;              //PIN state
  char *pin;                      //PIN

} atcd_sim_t;

// Functions -------------------------------------------------------------------
void atcd_sim_init(atcd_sim_t *atcd_sim);

uint8_t atcd_sim_get_pin_state(atcd_sim_t *atcd_sim);
void atcd_sim_set_pin(atcd_sim_t *atcd_sim, char *pin);

void atcd_sim_proc(atcd_sim_t *atcd_sim);
//------------------------------------------------------------------------------
#endif /* ATCD_SIM_H_INCLUDED */
