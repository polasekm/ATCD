/*-----------------------------------------------------------------------------*/
/*
 * usart.h
 *
 *  Created on: Apr 9, 2011
 *      Author: Martin Polasek
 */
/*-----------------------------------------------------------------------------*/
#ifndef ATCD_HW_H_INCLUDED
#define ATCD_HW_H_INCLUDED

/* Includes ------------------------------------------------------------------*/

#include "ring_buffer/ring_buffer.h"
/* Exported functions ------------------------------------------------------- */

/* Defines -------------------------------------------------------------------*/
#define ATCD_RB_RX_BUFF_LEN 512

#define ATCD_PWR_OFF   0
#define ATCD_PWR_ON    1
// Functions -------------------------------------------------------------------
void atc_dev_hw_init();           //init HW

void atcd_hw_pwr(uint8_t state);  //PWR on
void atcd_hw_reset();             //HW reset
void atcd_hw_igt();               //HW ignition (GSM and LTE modem only)

void atcd_hw_rx(uint8_t *data, uint16_t len);    //rx data
void atcd_hw_tx(rbuff_t *rbuff, uint16_t len);   //tx data

void atcd_dbg_inf(char *str);                //debug info print
void atcd_dbg_warn(char *str);                //debug info print
void atcd_dbg_err(char *str);                 //debug info print
void atcd_dbg_in(char *str, uint16_t len);    //debug info print
void atcd_dbg_out(char *str, uint16_t len);   //debug info print
void atcd_dbg_out_rb(rbuff_t *rbuff, uint16_t len);  //debug info print - ring buffer

void atcd_dbg_txt(char *str);                 //debug text print

void atcd_it_proc();              //Ostranit - pouzivat jen RX a TX fce
void atcd_log_it_proc();

uint32_t atcd_get_ms();           //get time in ms
//------------------------------------------------------------------------------
#endif /* ATCD_HW_H_INCLUDED */
