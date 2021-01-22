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

  atcd.phone.sms.cb_events = ATCD_SMS_EV_ALL;
  atcd.phone.sms.callback = NULL;
}
//------------------------------------------------------------------------------
void atcd_phone_reset()                   //phone state reset
{
  atcd.phone.state = ATCD_PHONE_STATE_IDLE;
  atcd.phone.number[0] = 0;
  atcd.phone.ring_cnt = 0;
  atcd.phone.miss_call_cnt = 0;

  atcd.phone.dtmf_rx_tone = 0;
  atcd.phone.dtmf_tx_tone = 0;
  atcd.phone.dtmf_tx_dur = 1;

  atcd.phone.sms.sender = NULL;
  atcd.phone.sms.datetime = NULL;
  atcd.phone.sms.message = NULL;

  atcd.phone.sms.len = 0;
  atcd.phone.sms.index = 0;
  atcd.phone.sms.state = 0;
  atcd.phone.sms.result = 0;

  atcd.phone.sms.timeout = 0;
  atcd.phone.sms.next = NULL;

  atcd.phone.sms.sender = atcd.phone.sms_sender_buff;
  atcd.phone.sms.sender[0] = 0;
  atcd.phone.sms.datetime = atcd.phone.sms_datetime_buff;
  atcd.phone.sms.datetime[0] = 0;
  atcd.phone.sms.message = atcd.phone.sms_message_buff;
  atcd.phone.sms.message[0] = 0;
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
      if(atcd.phone.state != ATCD_PHONE_STATE_RING_WA && atcd.phone.state != ATCD_PHONE_STATE_HANG_W)
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
  
  if(strncmp(atcd.parser.buff + atcd.parser.line_pos, "+CMT:", strlen("+CMT:")) == 0)
  {
    ATCD_DBG_PHONE_SMS_DET
    // Bude nasledovat SMS - pocet znaku je uveden na konci...

    p    = atcd.parser.buff + atcd.parser.line_pos + strlen("+CMT:");
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
    atcd.parser.mode = ATCD_P_MODE_SMS;

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
      atcd.phone.number[0] = 0;

      if(atcd.phone.callback != NULL && (atcd.phone.cb_events & ATCD_PHONE_EV_CALL_END) != 0) atcd.phone.callback(ATCD_PHONE_EV_CALL_END);
    }
    return 1;
  }

  if(strncmp(atcd.parser.buff + atcd.parser.line_pos, "+CLIP:", strlen("+CLIP:")) == 0)
  {
    ATCD_DBG_PHONE_CALL_N_DET

    p    = atcd.parser.buff + atcd.parser.line_pos + strlen("+CLIP:");
    endl = atcd.parser.buff + atcd.parser.buff_pos;

    //Phone number str start
    np = (char*)memchr(p, '"', endl - p);
    if(np == NULL) goto skip_proc2;
    p = np + 1;

    //Phone number str end
    np = (char*)memchr(p, '"', endl - p);
    if(np == NULL) goto skip_proc2;
    memcpy(atcd.phone.number, p, np - p);
    atcd.phone.number[np - p] = 0;
    p = np + 1;

    atcd.parser.buff_pos = atcd.parser.line_pos;
    return 1;

  skip_proc2:
    ATCD_DBG_PHONE_CALL_N_DET_E
    atcd.parser.buff_pos = atcd.parser.line_pos;
    return 1;
  }

  if(strncmp(atcd.parser.buff + atcd.parser.line_pos, "BUSY\r\n", strlen("BUSY\r\n")) == 0)
  {
    ATCD_DBG_PHONE_BUSY_DET

    atcd.phone.state = ATCD_PHONE_STATE_IDLE;
    atcd.phone.ring_cnt = 0;
    atcd.phone.number[0] = 0;

    //neulozit nekam zmeskane cislo?
    //neinkrementovat pocet zmeskanych hovoru?

    atcd.parser.buff_pos = atcd.parser.line_pos;
    if(atcd.phone.callback != NULL && (atcd.phone.cb_events & ATCD_PHONE_EV_CALL_END) != 0) atcd.phone.callback(ATCD_PHONE_EV_CALL_END);
    return 1;
  }

  if(strncmp(atcd.parser.buff + atcd.parser.line_pos, "NO CARRIER\r\n", strlen("NO CARRIER\r\n")) == 0)
  {
    ATCD_DBG_PHONE_NO_CAR_DET

    atcd.phone.state = ATCD_PHONE_STATE_IDLE;
    atcd.phone.ring_cnt = 0;
    atcd.phone.number[0] = 0;

    //neindikovat odmitnuty hovor?

    atcd.parser.buff_pos = atcd.parser.line_pos;
    if(atcd.phone.callback != NULL && (atcd.phone.cb_events & ATCD_PHONE_EV_CALL_END) != 0) atcd.phone.callback(ATCD_PHONE_EV_CALL_END);
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
uint8_t atcd_phone_sms_proc(char ch)
{
  // If parser in SMS receiving mode
  if(atcd.parser.mode == ATCD_P_MODE_SMS)
  {
    // Pokud je v bufferu misto
    if(atcd.parser.buff_pos - atcd.parser.line_pos <= 160)
    {
      // Zapise prijaty byte do bufferu
      atcd.phone.sms.message[atcd.parser.buff_pos - atcd.parser.line_pos] = ch;
    }
    else
    {
      ATCD_DBG_PHONE_SMS_BUFF_E
      //conn->cb_events |=  ATCD_CONN_EV_OVERRUN;
    }

    atcd.parser.buff_pos++;

    // Pokud jsme dosahli komce IPD bloku
    //if(atcd.buff_pos >= atcd.parser.ipd_len)
    if(atcd.parser.buff_pos - atcd.parser.line_pos >= atcd.phone.sms.len)
    {
      ATCD_DBG_PHONE_SMS_END
      atcd.parser.mode = ATCD_P_MODE_ATC;

      atcd.parser.buff_pos = 0;
      atcd.parser.line_pos = 0;

      atcd.phone.sms.message[atcd.phone.sms.len + 1] = 0;

      atcd.phone.sms.cb_events |= ATCD_SMS_EV_SMS_IN;
      if(atcd.phone.sms.callback != NULL) atcd.phone.sms.callback(ATCD_SMS_EV_SMS_IN);
    }

    return 1;
  }

  return 0;
}
//------------------------------------------------------------------------------
