/*-----------------------------------------------------------------------------*/
/*
 * usart.h
 *
 *  Created on: Apr 9, 2011
 *      Author: Martin Polasek
 */
/*-----------------------------------------------------------------------------*/
#ifndef ATCD_ATC_H_INCLUDED
#define ATCD_ATC_H_INCLUDED

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>     /* atoi */
#include <stdio.h>

#include "../rbuff/rbuff.h" 

/* Exported functions ------------------------------------------------------- */

/* Defines -------------------------------------------------------------------*/

// Povoleni vstupu dat v AT prikazu
#define ATCD_ATC_PROMPT_OFF         0
#define ATCD_ATC_PROMPT_ON          1

// Stav zpracovavani AT prikazu
#define ATCD_ATC_STATE_DONE         0
#define ATCD_ATC_STATE_WAIT         1
#define ATCD_ATC_STATE_TX           3
#define ATCD_ATC_STATE_W_ECHO       4
#define ATCD_ATC_STATE_W_END        5
#define ATCD_ATC_STATE_FREE         6

// Vysledny stav zpracovani AT prikazu
#define ATCD_ATC_RESULT_UNKNOWN     0
#define ATCD_ATC_RESULT_OK          1
#define ATCD_ATC_RESULT_ERROR       2
#define ATCD_ATC_RESULT_FAIL        3
#define ATCD_ATC_RESULT_TIMEOUT     4
#define ATCD_ATC_RESULT_CANCELL     5
#define ATCD_ATC_RESULT_OVERRUN     6
#define ATCD_ATC_RESULT_OVERWRITE   7

// Udalosti AT prikazu
#define ATCD_ATC_EV_NONE            0
#define ATCD_ATC_EV_ECHO            0b00000001
#define ATCD_ATC_EV_DONE            0b00000010
#define ATCD_ATC_EV_TIMEOUT         0b00000100
#define ATCD_ATC_EV_OVERRUN         0b00001000
#define ATCD_ATC_EV_OVERWRITE       0b00010000

//------------------------------------------------------------------------------
typedef struct atcd_at_cmd
{
  char *cmd;                      //AT command

  char *resp;                     //buffer to response
  uint16_t resp_len;              //response lenth
  uint16_t resp_buff_size;        //buffer size

  uint8_t state;                  //execute state
  uint8_t result;                 //AT command result

  uint8_t prompt;                 //prompt enable - neni duplcita se zadanymi daty?
  rbuff_t *data;                  //optional tx data in cmd body
  uint16_t data_len;              //optional tx data lenth

  uint16_t timeout;               //timeout in ms

  uint8_t events;                 //AT commands events
  void (*callback)(uint8_t);      //events callback

  struct atcd_at_cmd *next;       //next AT cmd in queue

} atcd_at_cmd_t;
//------------------------------------------------------------------------------


// Functions -------------------------------------------------------------------

// AT Commands
void atcd_atc_init(atcd_at_cmd_t *at_cmd);    //init AT command
void atcd_atc_exec(atcd_at_cmd_t *at_cmd);    //execute AT command
void atcd_atc_cancell(atcd_at_cmd_t *at_cmd); //cancell execute AT command

void atcd_atc_send_cmd();                     //send AT command
void atcd_atc_send_data();                    //send AT command data
//------------------------------------------------------------------------------
#endif /* ATCD_ATC_H_INCLUDED */
