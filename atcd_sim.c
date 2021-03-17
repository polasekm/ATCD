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
  atcd.sim.pin_state = ATCD_PIN_STATE_UNKNOWN;
  atcd.sim.pin = NULL;
}
//------------------------------------------------------------------------------
void atcd_sim_reset()
{
  atcd.sim.pin_state = ATCD_PIN_STATE_UNKNOWN;
}
//------------------------------------------------------------------------------
uint8_t atcd_sim_get_pin_state()
{
  return atcd.sim.pin_state;
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
