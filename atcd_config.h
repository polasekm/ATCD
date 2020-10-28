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
#define ATCD_SIM868   4
#define ATCD_SIM7000  5
#define ATCD_GL865    6
#define ATCD_A6       7
#define ATCD_A7       8
#define ATCD_M590     9
#define ATCD_BGS5E    10
#define ATCD_BGS2     11
#define ATCD_ELS61    12

#include "atcd_user_config.h"

//#define ATCD_USE_DEVICE    ATCD_A6

#define ATCD_DBG 1

#if(ATCD_DBG == 1)
  #define ATCD_DBG_BUFF_OVERRUN         atcd_dbg_warn("ATCD: V bufferu pro ATCD doslo misto!\r\n");
  #define ATCD_DBG_ATC_LN_BUFF_OV       atcd_dbg_warn("ATCD: Nastavuji overrun u ATC??????\r\n");
  #define ATCD_DBG_BOOT_SEQ             atcd_dbg_inf("ATCD: Detekovana bootovaci sekvence.\r\n");
  #define ATCD_DBG_STARTING             atcd_dbg_inf("ATCD: Spusteni zarizeni.\r\n");
  #define ATCD_DBG_CREG                 atcd_dbg_inf("ATCD: GSM +CREG: x detect.\r\n");
  #define ATCD_DBG_CREG_ERR             atcd_dbg_warn("ATCD: GSM +CREG: x je mimo rozah.\r\n");
  #define ATCD_DBG_ATC_BUFF_OV          atcd_dbg_warn("ATCD: V cilovem bufferu ATC neni dost mista!\r\n");
  #define ATCD_DBG_RESET                atcd_dbg_warn("ATCD: Reset zarizeni.\r\n");

  #define ATCD_DBG_PIN_NONE             atcd_dbg_inf("ATCD: INIT: PIN neni treba.\r\n");
  #define ATCD_DBG_PIN_REQ              atcd_dbg_inf("ATCD: INIT: Je treba zadat PIN.\r\n");
  #define ATCD_DBG_PIN_ERR              atcd_dbg_err("ATCD: INIT: Na dotaz na PIN prisla neocekavana odpoved - zacinam znovu!\r\n");

  #define ATCD_DBG_INIT_DONE            atcd_dbg_inf("ATCD: INIT: Sekvence byla dokoncena.\r\n");
  #define ATCD_DBG_INIT_ERR_R           atcd_dbg_err("ATCD: INIT: atcd.init_seq je  mimo rozsah - zacinam znovu!\r\n"); 
  
  #define ATCD_DBG_SEQ_ERR              atcd_dbg_warn("ATCD: SEQ: Krok sekvence skoncil chybou!\r\n");
  #define ATCD_DBG_SEQ_ERR_MAX          atcd_dbg_warn("ATCD: ATC_SEQ: Dosazano maximalniho poctu chyb sekvence!\r\n");
  #define ATCD_DBG_SEQ_NEXT             atcd_dbg_inf("ATCD: ATC_SEQ: Spoustim dalsi krok sekvence.\r\n");
  #define ATCD_DBG_SEQ_STEP             atcd_dbg_inf("ATCD: ATC_SEQ: Odesilam dalsi krok sekvence.\r\n");

#else if
  #define ATCD_DBG_BUFF_OVERRUN
  #define ATCD_DBG_ATC_LN_BUFF_OV
  #define ATCD_DBG_BOOT_SEQ
  #define ATCD_DBG_STARTING
  #define ATCD_DBG_CREG
  #define ATCD_DBG_CREG_ERR
  #define ATCD_DBG_ATC_BUFF_OV
  #define ATCD_DBG_RESET

  #define ATCD_DBG_PIN_NONE
  #define ATCD_DBG_PIN_REQ
  #define ATCD_DBG_PIN_ERR


#endif /* ATCD_DBG */

//#define COUNTOF(__BUFFER__)   (sizeof(__BUFFER__) / sizeof(*(__BUFFER__)))

//-----------------------------
#define ATCD_P_BUFF_SIZE 512

/*
Casem smazat...

#define ATCD_FAIL 0
#define ATCD_OK   1*/
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
