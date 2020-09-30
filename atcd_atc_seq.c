/*
 * usart.c
 *
 *  Created on: Apr 9, 2011
 *      Author: Martin
 */
//------------------------------------------------------------------------------
#include "atcd_atc_seq.h"
#include "atcd.h"

extern atcd_t atcd;

//------------------------------------------------------------------------------
/
//------------------------------------------------------------------------------
void atcd_atc_seq_init(atcd_at_cmd_seq_t *at_cmd_seq, void (*make_step)(uint8_t));   
{
  at_cmd_seq->state = ATCD_ATC_SEQ_STATE_DONE;
  at_cmd_seq->step = 0;

  at_cmd_seq->err_cnt = 0;
  at_cmd_seq->err_max = 0;

  at_cmd_seq->at_cmd = NULL;
  at_cmd_seq->make_step = make_step:

  //atcd_atc_init(at_cmd_seq->at_cmd);
}
//------------------------------------------------------------------------------
void atcd_atc_seq_proc(atcd_at_cmd_seq_t *at_cmd_seq)
{
  if(at_cmd_seq->state == ATCD_ATC_SEQ_STATE_RUN && at_cmd_seq->at_cmd.state == ATCD_ATC_STATE_DONE)
  {
    if(at_cmd_seq->at_cmd.result == ATCD_ATC_RESULT_OK)
    {
      at_cmd_seq->step++;

      atcd_dbg_inf("ATCD: SEQ: Spoustim dalsi krok sekvence.\r\n");
      at_cmd_seq->make_step(at_cmd_seq->step);
    }
    else
    {
      atcd_dbg_warn("ATCD: SEQ: Sekvence skoncila chybou!\r\n");
      at_cmd_seq->err_cnt++;

      if(at_cmd_seq->err_cnt > at_cmd_seq->err_max)
      {
        atcd_dbg_warn("ATCD: SEQ: Dosazano maximalniho poctu chyb sekvence!\r\n");
        at_cmd_seq->state = ATCD_ATC_SEQ_STATE_ERROR;
      }
    }
  }
}
//------------------------------------------------------------------------------