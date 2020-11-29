/*-----------------------------------------------------------------------------*/
/*
 * usart.h
 *
 *  Created on: Apr 9, 2011
 *      Author: Martin Polasek
 */
/*-----------------------------------------------------------------------------*/
#ifndef ATCD_ATC_SEQ_H_INCLUDED
#define ATCD_ATC_SEQ_H_INCLUDED

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>     /* atoi */
#include <stdio.h>

#include "../rbuff/rbuff.h" 

#include "atcd_atc.h"

/* Exported functions ------------------------------------------------------- */

/* Defines -------------------------------------------------------------------*/

// Stav zpracovavani AT prikazu
#define ATCD_ATC_SEQ_STATE_DONE         0
#define ATCD_ATC_SEQ_STATE_WAIT         1
#define ATCD_ATC_SEQ_STATE_RUN          3
#define ATCD_ATC_SEQ_STATE_ERROR        4

//------------------------------------------------------------------------------
typedef struct
{
   uint8_t state;
   uint8_t step;
   //atcd_at_cmd_t at_cmd;

   uint8_t err_cnt;
   uint8_t err_max;

   atcd_at_cmd_t *at_cmd;
   //void (*make_step)(uint8_t);

} atcd_at_cmd_seq_t;
//------------------------------------------------------------------------------


// Functions -------------------------------------------------------------------

void atcd_atc_seq_init(atcd_at_cmd_seq_t *at_cmd_seq);  
void atcd_atc_seq_run(atcd_at_cmd_seq_t *at_cmd_seq);   
void atcd_atc_seq_proc(atcd_at_cmd_seq_t *at_cmd_seq);         
//------------------------------------------------------------------------------
#endif /* ATCD_ATC_SEQ_H_INCLUDED */
