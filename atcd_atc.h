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

// Stav zpracovavani AT prikazu
#define ATCD_ATC_STATE_DONE         0
#define ATCD_ATC_STATE_WAIT         1
#define ATCD_ATC_STATE_TX           3
#define ATCD_ATC_STATE_W_ECHO       4
#define ATCD_ATC_STATE_W_END        5
//#define ATCD_ATC_STATE_IDLE         6

// Vysledny stav zpracovani AT prikazu
#define ATCD_ATC_RESULT_UNKNOWN     0
#define ATCD_ATC_RESULT_OK          1
#define ATCD_ATC_RESULT_ERROR       2
#define ATCD_ATC_RESULT_FAIL        3
#define ATCD_ATC_RESULT_TIMEOUT     4
#define ATCD_ATC_RESULT_CANCELL     5
#define ATCD_ATC_RESULT_OVERRUN     6
#define ATCD_ATC_RESULT_OVERWRITE   7

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

  char *resp;                     //response buffer
  uint16_t resp_len;              //response size
  uint16_t resp_buff_size;        //response buffer size

  uint8_t state;                  //execute state
  uint8_t result;                 //AT command result

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
   void atcd_atc_check_queue(atcd_at_cmd_t *at_cmd);    //check AT command in queue
uint8_t atcd_atc_check_success(atcd_at_cmd_t *at_cmd);  //check AT command state and result

uint8_t atcd_atc_exec(atcd_at_cmd_t *at_cmd);                   //execute AT command
uint8_t atcd_atc_exec_cmd(atcd_at_cmd_t *at_cmd, char *cmd);    //execute and set AT command
uint8_t atcd_atc_cancell(atcd_at_cmd_t *at_cmd);                //cancell execute AT command

uint8_t atcd_atc_send_cmd();                     //send AT command
uint8_t atcd_atc_send_data();                    //send AT command data

uint8_t atcd_atc_ln_proc();                      //AT commant line processing
uint8_t atcd_atc_prompt_tst();                   //prompt test processing

uint8_t atcd_atc_set_defaults(atcd_at_cmd_t *at_cmd);                                 //set default AT commands values
   void atcd_atc_set_cmd(atcd_at_cmd_t *at_cmd, char *cmd);                           //set AT command
   void atcd_atc_set_resp_buf(atcd_at_cmd_t *at_cmd, uint8_t *buff, uint16_t len);    //set AT command
   void atcd_atc_set_timeout(atcd_at_cmd_t *at_cmd, uint16_t timeout);   

uint8_t atcd_atc_get_state(atcd_at_cmd_t *at_cmd);    //get AT command state
uint8_t atcd_atc_get_result(atcd_at_cmd_t *at_cmd);   //get AT command result
//------------------------------------------------------------------------------
#endif /* ATCD_ATC_H_INCLUDED */
