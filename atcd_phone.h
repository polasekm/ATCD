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

typedef enum {
  ATCD_PHONE_STATE_IDLE             = 0,
  ATCD_PHONE_STATE_RING,       //prisel RING
  ATCD_PHONE_STATE_RING_WA,    //posli ATA
  ATCD_PHONE_STATE_CALL,       //vetsinou CALL_IN ale ne vzdy; odpovida +CPAS: 4
  ATCD_PHONE_STATE_HANG_W,     //posli ATH
  ATCD_PHONE_STATE_DIAL,       //poslano ATD
  ATCD_PHONE_STATE_DIAL_W      //posli ATD...
} atcd_phone_state_t;


#define ATCD_PHONE_EV_NONE          0x00
#define ATCD_PHONE_EV_REG           0b00000001
#define ATCD_PHONE_EV_UNREG         0b00000010
#define ATCD_PHONE_EV_RING          0b00000100
#define ATCD_PHONE_EV_RING_END      0b00001000 //never happens
#define ATCD_PHONE_EV_SMS_IN        0b00010000 //+CMT: only, sms incomplete, atcd.phone.callback; next come atcd.phone.sms.callback(ATCD_SMS_EV_SMS_IN)
#define ATCD_PHONE_EV_CALL          0b00100000 //muze chodit opakovane pro in i out
#define ATCD_PHONE_EV_CALL_END      0b01000000 //muze chodit opakovane pro in i out
#define ATCD_PHONE_EV_ALL           0xFF

// SMS
typedef enum {
  ATCD_PHONE_SMS_STATE_IDLE         = 0,
  ATCD_PHONE_SMS_STATE_SEND_W,
  ATCD_PHONE_SMS_STATE_SENDING,
  ATCD_PHONE_SMS_STATE_SEND,
  ATCD_PHONE_SMS_STATE_TIMEOUT,
  ATCD_PHONE_SMS_STATE_ERROR,
  ATCD_PHONE_SMS_STATE_UNKNOWN
} atcd_phone_sms_state_t;

#define ATCD_SMS_EV_NONE          0x00
#define ATCD_SMS_EV_SEND          0b00000001
#define ATCD_SMS_EV_FAIL          0b00000010
#define ATCD_SMS_EV_SMS_IN        0b00010000 //+CMT only   //tohle prejmenovat
#define ATCD_SMS_EV_ALL           0xFF

//------------------------------------------------------------------------------
typedef struct atcd_sms_ts atcd_sms_t;

struct atcd_sms_ts
{
  char *sender;                   //Message sender
  char *datetime;                 //Message date time
  char *message;                  //Message text
  uint16_t len;                   //Message length

  uint16_t index;                 //Message memory index
    
  atcd_phone_sms_state_t state;   //SMS state
  uint8_t result;                 //SMS result
  
  uint16_t timeout;               //timeout in s

  uint8_t cb_events;              //AT commands events
  void (*callback)(uint8_t event, const atcd_sms_t *sms);      //events callback
  
  atcd_sms_t *next;               //next SMS in queue
};
//------------------------------------------------------------------------------
typedef struct
{
  atcd_phone_state_t state;       //phone state
  uint8_t state_call_out;         //bitmaska call id 8..1 aby se to lip mazalo//mozna jako jeden ze stavu modemu ale zatim takhle
  uint8_t state_call_in;          //bitmaska call id 8..1

  char dtmf_rx_tone;              //DTMF TX tone
  char dtmf_tx_tone;              //DTMF TX tone
  uint8_t dtmf_tx_dur;            //DTMF TX tone duration

  char number[21];                //src/dst phone number
  int numbertype;                 //incoming, from +CLIP
  uint16_t ring_cnt;              //ring counter
  uint16_t miss_call_cnt;         //missing call counter

  atcd_sms_t sms;                 //RX SMS struct for internal usage
  atcd_sms_t sms_tx;              //TX SMS struct for internal usage

  char sms_sender_buff[16];       //rx sms number buff
  char sms_datetime_buff[32];     //rx sms datetime buff
  char sms_message_buff[161];     //rx sms text buff

  uint8_t cb_events;              //phone events
  void (*callback)(uint8_t evt, char const *info);      //events callback

} atcd_phone_t;

// Functions -------------------------------------------------------------------
// Phone
void atcd_phone_init();                      //inializace telefonu
void atcd_phone_set_callback(uint8_t enable_events, void (*callback)(uint8_t event, char const *info));
void atcd_sms_set_callback(uint8_t doesNotUnderstand, void (*sms_callback)(uint8_t event, const atcd_sms_t *sms));
void atcd_smstx_set_callback(uint8_t doesNotUnderstand, void (*sms_callback)(uint8_t event, const atcd_sms_t *sms));

//nejde dat do init ani do reset
//sms.cb_events funguji nejak divne, sam si to nastavuje

void atcd_phone_call(const char *number);          //vytocit hovor
void atcd_phone_call_answer();               //zvednout hovor
void atcd_phone_call_hang_up();              //polozit hovor

uint8_t atcd_phone_are_phones_equal(const char *p1, int t1, const char *p2, int t2); //0734275077,129 == +46734275077,145

void atcd_phone_send_sms(char *number, char *msg); //poslat SMS

// PHONE
void atcd_phone_proc();                      //phone processing
void atcd_phone_reset();                     //phone state reset

uint8_t atcd_phone_asc_msg();
uint8_t atcd_phone_sms_proc(char ch);

atcd_phone_state_t atcd_phone_state();
atcd_phone_t *atcd_phone_fullstate();
uint16_t atcd_phone_ring_cnt();
const char *atcd_phone_ring_number(int *ntyp/*=nullptr*/);        //nikdy nevraci NULL ale muze ""
//------------------------------------------------------------------------------
#endif /* ATCD_PHONE_H_INCLUDED */
