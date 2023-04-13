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

#include "../Libs/rbuff/rbuff.h"

//#include "atcd_config.h"
/* Exported functions ------------------------------------------------------- */

/* Defines -------------------------------------------------------------------*/

#define ATCD_PWR_OFF   0
#define ATCD_PWR_ON    1
// Functions -------------------------------------------------------------------
void atcd_hw_init();              //init HW
void atcd_hw_proc();              //HW processing

void atcd_hw_pwr(uint8_t state);  //PWR on
void atcd_hw_reset();             //HW reset
void atcd_hw_reset_sudden();      //turn off power to modem
void atcd_hw_igt();               //HW ignition (GSM and LTE modem only)

void atcd_hw_rx(uint8_t *data, uint16_t len);          //rx data
void atcd_hw_tx(rbuff_t *rbuff, uint16_t len);         //tx data
void atcd_hw_tx_esc(char *pattern, uint16_t len);   //tx escape data

void atcd_dbg_inf(const char *header, const char *str);   //debug info print
void atcd_dbg_inf2(const char *header, const char *str);  //info ale ne tak casto
void atcd_dbg_inf3(const char *header, const char *str);  //inf2 + poslat do TB
void atcd_dbg_warn(const char *header, const char *str);  //debug warning print
void atcd_dbg_err(const char *header, const char *str);   //debug error print
void atcd_dbg_err2(const char *header, const char *str);  //red text, no reporting
void atcd_dbg_in(const char *str, uint16_t len);    //debug data in print
void atcd_dbg_out(const char *str, uint16_t len);   //debug data out print
void atcd_dbg_out_rb(rbuff_t *rbuff, uint16_t len);  //debug data out print - ring buffer

void atcd_dbg_txt(const char *str);                 //debug text print

void atcd_it_proc();              //Ostranit - pouzivat jen RX a TX fce

uint32_t atcd_get_ms();           //get time in ms
//------------------------------------------------------------------------------
#endif /* ATCD_HW_H_INCLUDED */
