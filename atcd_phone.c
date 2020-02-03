/*
 * usart.c
 *
 *  Created on: Apr 9, 2011
 *      Author: Martin
 */
//------------------------------------------------------------------------------
#include "atcd_phone.h"
#include "atcd.h"

extern atcd_t atcd;

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void atcd_phone_init(atcd_phone_t *phone)   //inializace telefonu
{
  phone->state = ATCD_PHONE_STATE_OFF;
  phone->pin = NULL;

  phone->flags = 0;           
  phone->miss_call_cnt = 0;  

  phone->events = ATCD_PHONE_EV_NONE;              
  phone->callback = NULL;
}
//------------------------------------------------------------------------------
void atcd_phone_proc()                    //phone processing
{

}
//------------------------------------------------------------------------------
void atcd_phone_reset()                   //phone state reset
{
  atcd.phone.state = ATCD_PHONE_STATE_OFF;
}
//------------------------------------------------------------------------------
void atcd_phone_set_pin(char *pin)       //set PIN
{
  atcd.phone.pin = pin;
}
//------------------------------------------------------------------------------
