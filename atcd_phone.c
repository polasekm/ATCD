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
void atcd_phone_init()   //inializace telefonu
{
  atcd.phone.state = ATCD_PHONE_STATE_UNREG;
  atcd.phone.pin = NULL;

  atcd.phone.flags = 0;
  atcd.phone.miss_call_cnt = 0;

  atcd.phone.cb_events = ATCD_PHONE_EV_ALL;
  atcd.phone.callback = NULL;
}
//------------------------------------------------------------------------------
void atcd_phone_reset()                   //phone state reset
{
  atcd.phone.state = ATCD_PHONE_STATE_UNREG;
}
//------------------------------------------------------------------------------
void atcd_phone_proc()                    //phone processing
{

}
//------------------------------------------------------------------------------
void atcd_phone_set_pin(char *pin)       //set PIN
{
  atcd.phone.pin = pin;
}
//------------------------------------------------------------------------------
uint8_t atcd_phone_asc_msg()
{
  if(strncmp(atcd.buff + atcd.line_pos, "RING\r\n", strlen("RING\r\n")) == 0)
  {
    atcd_dbg_inf("PHONE: RING detect.\r\n");
    //atcd.phone.state = ATCD_PHONE_STATE_REG_ROAM;
    atcd.buff_pos = atcd.line_pos;
    if(atcd.phone.callback != NULL && (atcd.phone.cb_events & ATCD_PHONE_EV_RING) != 0) atcd.phone.callback(ATCD_PHONE_EV_RING);
    return 1;
  }
  
  if(strncmp(atcd.buff + atcd.line_pos, "+CMT: ", strlen("+CMT: ")) == 0)
  {
    atcd_dbg_inf("PHONE: New SMS detected.\r\n");
    // Bude nasledovat SMS - pocet znaku je uveden na konci...

    //atcd.phone.state = ATCD_PHONE_STATE_REG_ROAM;
    atcd.buff_pos = atcd.line_pos;
    if(atcd.phone.callback != NULL && (atcd.phone.cb_events & ATCD_PHONE_EV_SMS_IN) != 0) atcd.phone.callback(ATCD_PHONE_EV_SMS_IN);
    return 0;
  }

  return 0;
}
//------------------------------------------------------------------------------
