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

#include "../Libs/rbuff/rbuff.h"

/* Exported functions ------------------------------------------------------- */

/* Defines -------------------------------------------------------------------*/

// Navratove hodnoty
typedef enum
{
  ATCD_OK        = 0,
  ATCD_ERR       = 1,
  ATCD_ERR_LOCK  = 2

} atcd_r_t;

// Stav zpracovavani AT prikazu
typedef enum
{
  ATCD_ATC_STATE_DONE     = 0,
  ATCD_ATC_STATE_WAIT     = 1,
  ATCD_ATC_STATE_TX       = 3,
  ATCD_ATC_STATE_W_ECHO   = 4,
  ATCD_ATC_STATE_W_PROMPT = 5,
  ATCD_ATC_STATE_TX_DATA  = 6,
  ATCD_ATC_STATE_W_END    = 7

} atcd_atc_state_t;

// Vysledny stav zpracovani AT prikazu
typedef enum
{
  ATCD_ATC_RESULT_UNKNOWN   = 0,
  ATCD_ATC_RESULT_OK        = 1,
  ATCD_ATC_RESULT_ERROR     = 2,
  ATCD_ATC_RESULT_FAIL      = 3,
  ATCD_ATC_RESULT_TIMEOUT   = 4,
  ATCD_ATC_RESULT_CANCELL   = 5,
  ATCD_ATC_RESULT_OVERRUN   = 6,
  ATCD_ATC_RESULT_OVERWRITE = 7

} atcd_atc_result_t;

// Vysledny stav zpracovani odpovedi AT prikazu
/*#define ATCD_ATC_RESP_FLUSH         0
#define ATCD_ATC_RESP_OK            1
#define ATCD_ATC_RESP_OVERRUN       2*/

// Udalosti AT prikazu
#define ATCD_ATC_EV_NONE            0x00
#define ATCD_ATC_EV_ECHO            0b00000001
#define ATCD_ATC_EV_DONE            0b00000010
#define ATCD_ATC_EV_TIMEOUT         0b00000100
#define ATCD_ATC_EV_OVERRUN         0b00001000
#define ATCD_ATC_EV_ALL             0xFF

//------------------------------------------------------------------------------
typedef struct atcd_at_cmd
{
  char *cmd;                      //AT command
  char *result_str;               //OK result string (ATC, ktere pri uspechu nekonci "OK")

  char *resp;                     //response buffer
  uint16_t resp_len;              //response size
  uint16_t resp_buff_size;        //response buffer size

  atcd_atc_state_t state;         //execute state
  atcd_atc_result_t result;       //AT command result
  uint16_t result_code;           //result code - pouze pokud chyba +CME ERROR/+CMS ERROR

  rbuff_t *data;                  //optional tx data in AT command body
  uint16_t data_len;              //optional tx data size

  uint16_t timeout;               //timeout in ms

  uint8_t cb_events;              //enabled AT command callback events
  void (*callback)(uint8_t);      //events callback

  struct atcd_at_cmd *next;       //next AT cmd in queue

} atcd_at_cmd_t;
//------------------------------------------------------------------------------


// Functions -------------------------------------------------------------------

// AT Commands
   void atcd_atc_init(atcd_at_cmd_t *at_cmd);           //init AT command
//   void atcd_atc_check_queue(atcd_at_cmd_t *at_cmd);    //check AT command in queue
uint8_t atcd_atc_check_success(atcd_at_cmd_t *at_cmd);  //check AT command state and result

atcd_r_t atcd_atc_exec(atcd_at_cmd_t *at_cmd);                   //execute AT command
atcd_r_t atcd_atc_exec_cmd(atcd_at_cmd_t *at_cmd, char *cmd);    //execute and set AT command
atcd_r_t atcd_atc_exec_cmd_res(atcd_at_cmd_t *at_cmd, char *cmd, char *res);   //execute and set AT command with result string

uint8_t atcd_atc_cancell(atcd_at_cmd_t *at_cmd);                //cancell execute AT command
void atcd_atc_cancel_all();                             //cancel all AT commands in queue

void atcd_atc_send_cmd_top();                    //send AT command at top queue
void atcd_atc_send_data_top();                   //send AT command data at top queue

   void atcd_atc_complete(atcd_at_cmd_t *at_cmd);  //AT command complete after result change

   void atcd_atc_proc();                         //AT commands state machine processing
uint8_t atcd_atc_ln_proc();                      //terminal line processing
uint8_t atcd_atc_prompt_tst();                   //terminal prompt test processing

atcd_r_t atcd_atc_set_defaults(atcd_at_cmd_t *at_cmd);                                 //set default AT commands values
    void atcd_atc_set_cmd(atcd_at_cmd_t *at_cmd, char *cmd);                           //set AT command
    void atcd_atc_set_resp_buf(atcd_at_cmd_t *at_cmd, uint8_t *buff, uint16_t len);    //set AT command
    void atcd_atc_set_timeout(atcd_at_cmd_t *at_cmd, uint16_t timeout);

uint8_t atcd_atc_get_state(atcd_at_cmd_t *at_cmd);    //get AT command state
uint8_t atcd_atc_get_result(atcd_at_cmd_t *at_cmd);   //get AT command result
//------------------------------------------------------------------------------
#endif /* ATCD_ATC_H_INCLUDED */
