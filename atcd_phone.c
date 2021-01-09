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
  atcd_phone_reset();
  atcd.phone.pin = NULL;

  atcd.phone.cb_events = ATCD_PHONE_EV_ALL;
  atcd.phone.callback = NULL;
}
//------------------------------------------------------------------------------
void atcd_phone_reset()                   //phone state reset
{
  atcd.phone.state = ATCD_PHONE_STATE_IDLE;
  atcd.phone.ring_cnt = 0;

  atcd.phone.dtmf_rx_tone = 0;
  atcd.phone.dtmf_tx_tone = 0;
  atcd.phone.dtmf_tx_dur = 1;

  atcd.phone.miss_call_cnt = 0;

  atcd.phone.sms.sender = atcd.phone.sms_sender_buff;
  atcd.phone.sms.sender[0] = 0;
  atcd.phone.sms.datetime = atcd.phone.sms_datetime_buff;
  atcd.phone.sms.datetime[0] = 0;
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
  uint8_t val;
  char *p, *np, *endl;

  if(strncmp(atcd.parser.buff + atcd.parser.line_pos, "RING\r\n", strlen("RING\r\n")) == 0)
  {
    ATCD_DBG_PHONE_RING_DET

    if(atcd.phone.state != ATCD_PHONE_STATE_RING)
    {
      if(atcd.phone.state != ATCD_PHONE_STATE_RING_WA)
      {
        atcd.phone.state = ATCD_PHONE_STATE_RING;
        atcd.phone.ring_cnt = 0;
      }
    }

    atcd.phone.ring_cnt++;

    atcd.parser.buff_pos = atcd.parser.line_pos;
    if(atcd.phone.callback != NULL && (atcd.phone.cb_events & ATCD_PHONE_EV_RING) != 0) atcd.phone.callback(ATCD_PHONE_EV_RING);
    return 1;
  }
  
  if(strncmp(atcd.parser.buff + atcd.parser.line_pos, "+CMT: ", strlen("+CMT: ")) == 0)
  {
    ATCD_DBG_PHONE_SMS_DET
    // Bude nasledovat SMS - pocet znaku je uveden na konci...

    p    = atcd.parser.buff + atcd.parser.line_pos + strlen("+CMT: ");
    endl = atcd.parser.buff + atcd.parser.buff_pos;

    //Sender phone number str start
    np = (char*)memchr(p, '"', endl - p);
    if(np == NULL) goto skip_proc;
    p = np + 1;

    //Sender phone number str end
    np = (char*)memchr(p, '"', endl - p);
    if(np == NULL) goto skip_proc;
    memcpy(atcd.phone.sms.sender, p, np - p);
    atcd.phone.sms.sender[np - p] = 0;
    p = np + 1;

    //Sender phone end
    np = (char*)memchr(p, ',', endl - p);
    if(np == NULL) goto skip_proc;
    p = np + 1;

    //? end
    np = (char*)memchr(p, ',', endl - p);
    if(np == NULL) goto skip_proc;
    p = np + 1;

    //Date time str start
    np = (char*)memchr(p, '"', endl - p);
    if(np == NULL) goto skip_proc;
    p = np + 1;

    //Date time str end
    np = (char*)memchr(p, '"', endl - p);
    if(np == NULL) goto skip_proc;
    memcpy(atcd.phone.sms.datetime, p, np - p);
    atcd.phone.sms.datetime[np - p] = 0;
    p = np + 1;

    //Date time end
    np = (char*)memchr(p, ',', endl - p);
    if(np == NULL) goto skip_proc;
    p = np + 1;

    //? end
    np = (char*)memchr(p, ',', endl - p);
    if(np == NULL) goto skip_proc;
    p = np + 1;

    //? end
    np = (char*)memchr(p, ',', endl - p);
    if(np == NULL) goto skip_proc;
    p = np + 1;

    //? end
    np = (char*)memchr(p, ',', endl - p);
    if(np == NULL) goto skip_proc;
    p = np + 1;

    //?end
    np = (char*)memchr(p, ',', endl - p);
    if(np == NULL) goto skip_proc;
    p = np + 1;

    //DST number end
    np = (char*)memchr(p, ',', endl - p);
    if(np == NULL) goto skip_proc;
    p = np + 1;

    //? end
    np = (char*)memchr(p, ',', endl - p);
    if(np == NULL) goto skip_proc;
    p = np + 1;

    //length

    atcd.phone.sms.len = atof(p);

    //atcd.phone.state = ATCD_PHONE_STATE_REG_ROAM;
    atcd.parser.buff_pos = atcd.parser.line_pos;
    if(atcd.phone.callback != NULL && (atcd.phone.cb_events & ATCD_PHONE_EV_SMS_IN) != 0) atcd.phone.callback(ATCD_PHONE_EV_SMS_IN);
    return 1;

  skip_proc:
    ATCD_DBG_PHONE_SMS_DET_ERR
    atcd.parser.buff_pos = atcd.parser.line_pos;
    return 1;
  }

  if(strncmp(atcd.parser.buff + atcd.parser.line_pos, "+CIEV: \"CALL\",", strlen("+CIEV: \"CALL\",")) == 0)
  {
    ATCD_DBG_PHONE_CALL_DET
    // stav je uveden na konci

    val = (uint8_t)atoi(atcd.parser.buff + atcd.parser.buff_pos - ATCD_RX_NL_LEN - 1);
    atcd.parser.buff_pos = atcd.parser.line_pos;

    if(val == 1)
    {
      atcd.phone.state = ATCD_PHONE_STATE_CALL;
      if(atcd.phone.callback != NULL && (atcd.phone.cb_events & ATCD_PHONE_EV_CALL) != 0) atcd.phone.callback(ATCD_PHONE_EV_CALL);
    }
    else
    {
      atcd.phone.state = ATCD_PHONE_STATE_IDLE;
      atcd.phone.ring_cnt = 0;

      if(atcd.phone.callback != NULL && (atcd.phone.cb_events & ATCD_PHONE_EV_CALL_END) != 0) atcd.phone.callback(ATCD_PHONE_EV_CALL_END);
    }
    return 1;
  }

  return 0;
}
//------------------------------------------------------------------------------
void atcd_phone_call_answer()
{
  if(atcd.phone.state == ATCD_PHONE_STATE_RING) atcd.phone.state = ATCD_PHONE_STATE_RING_WA;
}
//------------------------------------------------------------------------------
void atcd_phone_call(char *number)
{
  if(atcd.phone.state == ATCD_PHONE_STATE_IDLE)
  {
    strncpy(atcd.phone.number, number, 16);
    atcd.phone.number[15] = 0;
    atcd.phone.state = ATCD_PHONE_STATE_DIAL_W;
  }
}
//------------------------------------------------------------------------------
void atcd_phone_call_hang_up()
{
  if(atcd.phone.state != ATCD_PHONE_STATE_IDLE) atcd.phone.state = ATCD_PHONE_STATE_HANG_W;
  else if(atcd.phone.state != ATCD_PHONE_STATE_DIAL_W) atcd.phone.state = ATCD_PHONE_STATE_IDLE;
}
//------------------------------------------------------------------------------
