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

// Stav SIM
typedef enum
{
  ATCD_SIM_STATE_NONE       = 0, //no sim
  ATCD_SIM_STATE_WAIT       = 2,
  ATCD_SIM_STATE_OK         = 3,
  ATCD_SIM_STATE_PIN        = 4,
  ATCD_SIM_STATE_PUK        = 5,
  ATCD_SIM_STATE_ERROR      = 6,
  ATCD_SIM_STATE_UNKNOWN    = 7  //after reset
} atcd_sim_state_e;

//------------------------------------------------------------------------------
typedef struct
{
  atcd_sim_state_e state;         //state
  char *pin;                      //PIN

} atcd_sim_t;

// Functions -------------------------------------------------------------------
void atcd_sim_init();
void atcd_sim_reset();

atcd_sim_state_e atcd_sim_state();

void atcd_sim_set_pin(char *pin);

void atcd_sim_proc();
//------------------------------------------------------------------------------
#endif /* ATCD_SIM_H_INCLUDED */
