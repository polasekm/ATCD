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
#define ATCD_SIM_STATE_NONE         0
#define ATCD_SIM_STATE_WAIT         2
#define ATCD_SIM_STATE_OK           3
#define ATCD_SIM_STATE_PIN          4
#define ATCD_SIM_STATE_PUK          5
#define ATCD_SIM_STATE_ERROR        6
#define ATCD_SIM_STATE_UNKNOWN      7

//------------------------------------------------------------------------------
typedef struct
{
  uint8_t state;                  //state
  char *pin;                      //PIN

} atcd_sim_t;

// Functions -------------------------------------------------------------------
void atcd_sim_init();
void atcd_sim_reset();

uint8_t atcd_sim_state();

void atcd_sim_set_pin(char *pin);

void atcd_sim_proc();
//------------------------------------------------------------------------------
#endif /* ATCD_SIM_H_INCLUDED */
