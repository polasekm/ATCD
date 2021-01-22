/*-----------------------------------------------------------------------------*/
/*
 * usart.h
 *
 *  Created on: Apr 9, 2011
 *      Author: Martin Polasek
 */
/*-----------------------------------------------------------------------------*/
#ifndef ATCD_PHONE_H_INCLUDED
#define ATCD_PHONE_H_INCLUDED

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>     /* atoi */
#include <stdio.h>

#include "atcd_config.h"

#include "atcd_atc.h"

/* Exported functions ------------------------------------------------------- */

/* Defines -------------------------------------------------------------------*/

// Phone
#define ATCD_PHONE_STATE_IDLE       0
#define ATCD_PHONE_STATE_RING       1
#define ATCD_PHONE_STATE_RING_WA    2
#define ATCD_PHONE_STATE_CALL       3
#define ATCD_PHONE_STATE_HANG_W     4
#define ATCD_PHONE_STATE_DIAL       5
#define ATCD_PHONE_STATE_DIAL_W     6
//#define ATCD_PHONE_STATE_GPRS       7

#define ATCD_PHONE_EV_NONE          0x00
#define ATCD_PHONE_EV_REG           0b00000001
#define ATCD_PHONE_EV_UNREG         0b00000010
#define ATCD_PHONE_EV_RING          0b00000100
#define ATCD_PHONE_EV_RING_END      0b00001000
#define ATCD_PHONE_EV_SMS_IN        0b00010000
#define ATCD_PHONE_EV_CALL          0b00100000
#define ATCD_PHONE_EV_CALL_END      0b01000000
#define ATCD_PHONE_EV_ALL           0xFF

// SMS
#define ATCD_SMS_EV_NONE          0x00
#define ATCD_SMS_EV_SEND          0b00000001
#define ATCD_SMS_EV_FAIL          0b00000010
#define ATCD_SMS_EV_CALL_IN       0b00000100
#define ATCD_SMS_EV_CALL_DW       0b00001000
#define ATCD_SMS_EV_SMS_IN        0b00010000
#define ATCD_SMS_EV_ALL           0xFF

//------------------------------------------------------------------------------
typedef struct atcd_sms
{
  char *sender;                   //Message sender
  char *datetime;                 //Message date time
  char *message;                  //Message text
  uint16_t len;                   //Message length

  uint16_t index;                 //Message memory index
    
  uint8_t state;                  //SMS state
  uint8_t result;                 //SMS result
  
  uint16_t timeout;               //timeout in s

  uint8_t cb_events;              //AT commands events
  void (*callback)(uint8_t);      //events callback
  
  struct atcd_sms *next;          //next SMS in queue
  
} atcd_sms_t;
//------------------------------------------------------------------------------
typedef struct
{
  uint8_t state;                  //phone state
  char *pin;                      //PIN

  char dtmf_rx_tone;              //DTMF TX tone
  char dtmf_tx_tone;              //DTMF TX tone
  uint8_t dtmf_tx_dur;            //DTMF TX tone duration

  char number[16];                //src/dst phone number
  uint8_t ring_cnt;               //ring counter
  uint8_t miss_call_cnt;          //missing call counter

  atcd_sms_t sms;                 //SMS struct for internal usage
  char sms_sender_buff[16];       //sms number buff
  char sms_datetime_buff[32];     //sms datetime buff)
  char sms_message_buff[161];     //sms datetime buff

  uint8_t cb_events;              //phone events
  void (*callback)(uint8_t);      //events callback

} atcd_phone_t;

// Functions -------------------------------------------------------------------
// Phone
void atcd_phone_init();                      //inializace telefonu

void atcd_phone_set_pin(char *pin);          //set PIN

void atcd_phone_call(char *number);          //vytocit hovor
void atcd_phone_call_answer();               //zvednout hovor
void atcd_phone_call_hang_up();              //polozit hovor

void atcd_phone_send_sms();                  //poslat SMS
void atcd_phone_call_down();                 //polozit hovor

// PHONE
void atcd_phone_proc();                      //phone processing
void atcd_phone_reset();                     //phone state reset

uint8_t atcd_phone_asc_msg();
uint8_t atcd_phone_sms_proc(char ch);
//------------------------------------------------------------------------------
#endif /* ATCD_PHONE_H_INCLUDED */
