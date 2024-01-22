/*-----------------------------------------------------------------------------*/
/*
 * usart.h
 *
 *  Created on: Mar 3, 2023
 *      Author: Milan Vandrovec
 */
/*-----------------------------------------------------------------------------*/
#ifndef ATCD_SETUP_H_INCLUDED
#define ATCD_SETUP_H_INCLUDED

/* Includes ------------------------------------------------------------------*/
//#include <stdlib.h>     /* atoi */
//#include <stdio.h>

#include "atcd_config.h"

//#include "atcd_atc.h"
//#include "atcd_conn.h"

/* Exported functions ------------------------------------------------------- */

/* Defines -------------------------------------------------------------------*/

//------------------------------------------------------------------------------
typedef struct
{
  uint8_t clean; //0..need write 1..written 2..failed write
  
  uint8_t clvl;
  struct
  {
    uint16_t np; //nonlinear processsing
    uint16_t ae; //acoustic echo cancellation
    uint16_t nr; //noise reduction
    uint16_t ns; //noise suppression
    uint8_t can; //enable
    uint8_t mirror_can; //what I think the module has now
  } echo;
  uint8_t cmic;
  uint8_t cagcset;
  uint8_t crsl;
  uint8_t sidet0;
} atcd_setup_t;

// Functions -------------------------------------------------------------------
void atcd_setup_init();
void atcd_setup_reset();
//void atcd_setup_proc();

//uint8_t atcd_setup_asc_msg();
void atcd_setup_clvl(uint8_t clvl);
void atcd_setup_echo(uint16_t np, uint16_t ae, uint16_t nr, uint16_t ns, uint8_t can);
void atcd_setup_cmic(uint8_t cmic);
void atcd_setup_cagcset(uint8_t mic_agc_en);
void atcd_setup_crsl(uint8_t crsl);
void atcd_setup_sidet(uint8_t sidet0);

//------------------------------------------------------------------------------
#endif /* ATCD_SETUP_H_INCLUDED */
