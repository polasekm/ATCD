/*
 * usart.c
 *
 *  Created on: Apr 9, 2011
 *      Author: Martin
 */
//------------------------------------------------------------------------------
#include "atcd_sim.h"
#include "atcd.h"

extern atcd_t atcd;

//------------------------------------------------------------------------------
void atcd_sim_init()
{
  atcd.sim.state = ATCD_SIM_STATE_UNKNOWN;
  atcd.sim.pin = NULL;
}
//------------------------------------------------------------------------------
void atcd_sim_reset()
{
  atcd.sim.state = ATCD_SIM_STATE_UNKNOWN;
}
//------------------------------------------------------------------------------
atcd_sim_state_e atcd_sim_state()
{
  return atcd.sim.state;
}
//------------------------------------------------------------------------------
void atcd_sim_set_pin(char *pin)
{
  atcd.sim.pin = pin;
}
//------------------------------------------------------------------------------
void atcd_sim_proc()
{

}
//------------------------------------------------------------------------------
