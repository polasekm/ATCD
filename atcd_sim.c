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
void atcd_sim_init(atcd_sim_t *atcd_sim)
{
  atcd_sim->pin_state = ATCD_PIN_STATE_UNKNOWN;
  atcd_sim->pin = NULL;
}
//------------------------------------------------------------------------------
uint8_t atcd_sim_get_pin_state(atcd_sim_t *atcd_sim)
{
  return atcd_sim->pin_state;
}
//------------------------------------------------------------------------------
void atcd_sim_set_pin(atcd_sim_t *atcd_sim, char *pin)
{
  atcd_sim->pin = pin;
}
//------------------------------------------------------------------------------
void atcd_sim_proc(atcd_sim_t *atcd_sim)
{

}
//------------------------------------------------------------------------------