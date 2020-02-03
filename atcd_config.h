/*-----------------------------------------------------------------------------*/
/*
 * usart.h
 *
 *  Created on: Apr 9, 2011
 *      Author: Martin Polasek
 */
/*-----------------------------------------------------------------------------*/
#ifndef ATCD_CONFIG_H_INCLUDED
#define ATCD_CONFIG_H_INCLUDED

/* Includes ------------------------------------------------------------------*/


/* Exported functions ------------------------------------------------------- */

/* Defines -------------------------------------------------------------------*/
#define ATCD_ESP8266  1
#define ATCD_ESP32    2
#define ATCD_SIM800L  3
#define ATCD_GL865    4
#define ATCD_A6       5
#define ATCD_A7       6
#define ATCD_M590     7
#define ATCD_BGS5E    8
#define ATCD_BGS2     9
#define ATCD_ELS61    10

#include "atcd_user_config.h"

//#define ATCD_USE_DEVICE    ATCD_A6

//-----------------------------
#define ATCD_BUFF_SIZE 512


#define ATCD_FAIL 0
#define ATCD_OK   1
//-----------------------------

#ifndef ATCD_USE_DEVICE
  #define ATCD_USE_DEVICE  ATCD_A6
#endif /* ATCD_USE_DEVICE */

//-----------------------------
#if(ATCD_USE_DEVICE == ATCD_A6)
  #define ATCD_STR_START_SEQ        "AT Ready\r\n"
  #define ATCD_STR_DATA_RX          "+RECEIVE,"

  #define ATCD_STR_SIM_READY        "+CPIN:READY"
  #define ATCD_STR_SIM_PIN          "+CPIN:SIM PIN"

  #define ATCD_DATA_RX_NL
  #define ATCD_RX_NL_LEN  2

  #define ATCD_CONN_MAX_NUMBER 4
#endif /* ATCD_USE_DEVICE */
//----------------------------------------
#if(ATCD_USE_DEVICE == ATCD_BGS5E)
  #define ATCD_STR_START_SEQ        "^SYSSTART\r\n"
  #define ATCD_STR_DATA_RX          "+IPD,"

  #define ATCD_STR_SIM_READY        "+CPIN: READY"
  #define ATCD_STR_SIM_PIN          "+CPIN: SIM PIN"

  #define ATCD_RX_NL_LEN  2

  #define ATCD_CONN_MAX_NUMBER 4
#endif /* ATCD_BGS5E */
//----------------------------------------
#if(ATCD_USE_DEVICE == ATCD_ESP8266)
  #define ATCD_STR_START_SEQ        "ready\r\n"
  #define ATCD_STR_DATA_RX          "+IPD,"

  #define ATCD_RX_NL_LEN  2

  #define ATCD_CONN_MAX_NUMBER 4
#endif /* ATCD_ESP8266 */

//------------------------------------------------------------------------------
#endif /* ATCD_CONFIG_H_INCLUDED */
