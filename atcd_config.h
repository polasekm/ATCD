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
#define ATCD_SIM800   3
#define ATCD_SIM7000  4
#define ATCD_GL865    5
#define ATCD_A6       6
#define ATCD_A7       7
#define ATCD_M590     8
#define ATCD_BGS5E    9
#define ATCD_BGS2     10
#define ATCD_ELS61    11

#include "atcd_user_config.h"

//#define ATCD_USE_DEVICE    ATCD_A6

#define ATCD_DBG 1

#define ATCD_DBG_BUFF_OVERRUN         atcd_dbg_warn("ATCD: V bufferu pro ATCD doslo misto!\r\n");  
#define ATCD_DBG_ATC_LN_BUFF_OVERRUN  atcd_dbg_warn("ATCD: Nastavuji overrun u ATC??????\r\n");   
#define ATCD_DBG_BOOT_SEQ             atcd_dbg_inf("ATCD: Detekovana bootovaci sekvence.\r\n");
#define ATCD_DBG_STARTING             atcd_dbg_inf("ATCD: Spusteni zarizeni.\r\n");
#define ATCD_DBG_CREG                 atcd_dbg_inf("ATCD: GSM +CREG: x detect.\r\n");
#define ATCD_DBG_CREG_ERR             atcd_dbg_warn("ATCD: GSM +CREG: x je mimo rozah.\r\n");

//#define COUNTOF(__BUFFER__)   (sizeof(__BUFFER__) / sizeof(*(__BUFFER__)))

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
//-----------------------------
#if(ATCD_USE_DEVICE == ATCD_SIM7000)
  #define ATCD_STR_START_SEQ        "\r\nRDY\r\n"
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
