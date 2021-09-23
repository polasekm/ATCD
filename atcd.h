/*-----------------------------------------------------------------------------*/
/*
 * usart.h
 *
 *  Created on: Apr 9, 2011
 *      Author: Martin Polasek
 */
/*-----------------------------------------------------------------------------*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef ATCD_H_INCLUDED
#define ATCD_H_INCLUDED

#ifdef __cplusplus
 extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>     /* atoi */
#include <stdio.h>

#include "../Libs/rbuff/rbuff.h"

#include "atcd_config.h"
#include "atcd_hw.h"

#include "atcd_atc.h"
#include "atcd_parser.h"

#include "atcd_sim.h"
#include "atcd_gsm.h"
#include "atcd_phone.h"
#include "atcd_gprs.h"
#include "atcd_wifi.h"
#include "atcd_gps.h"
#include "atcd_conn.h"
#include "atcd_conns.h"

/* Exported functions ------------------------------------------------------- */

/* Defines -------------------------------------------------------------------*/


//---------------------------------------
// Stav samotneho zarizeni. Proihlaseni do site, wifi, ci gprs je v prislusnych 
// modulech
typedef enum
{
  ATCD_STATE_OFF      = 0,
  ATCD_STATE_STARTING = 1,
  ATCD_STATE_NO_INIT  = 2,
  //ATCD_STATE_SLEEP    = 3,
  ATCD_STATE_ON       = 4

} atcd_state_t;

typedef enum
{
  ATCD_SM_W_OFF       = 0,
  ATCD_SM_OFF         = 1,
  ATCD_SM_W_AUTO      = 2,
  ATCD_SM_AUTO        = 3,
  ATCD_SM_W_MANUAL    = 4,
  ATCD_SM_MANUAL      = 5

} atcd_sleep_mode_t;

typedef enum
{
  ATCD_SS_AWAKE        = 0,
  //ATCD_SS_AWAKE_FORECE = 1,
  ATCD_SS_W_WAKE       = 2,
  ATCD_SS_SLEEP        = 3

} atcd_sleep_state_t;

// Udalosti zarizeni
#define ATCD_EV_NONE                0x00
#define ATCD_EV_STATE               0b00000001  //kdyz prijde "RDY" // to ma byt init, ne?
#define ATCD_EV_ASYNC_MSG           0b00000010  //nezpracovana asynchronni zprava
#define ATCD_EV_STATE_RESET         0b00000100  //kazdy reset modemu
#define ATCD_EV_ALL                 0xFF

// Stav odesilani dat
typedef enum
{
  ATCD_P_TX_COMPLETE  = 0,
  ATCD_P_TX_ONGOING   = 1,
  ATCD_P_TX_PENDING   = 2

} atcd_tx_state_t;

//------------------------------------------------------------------------------
// Switch base offsets
#define ATCD_SB_INIT                0
#define ATCD_SB_STAT                100

#define ATCD_SB_PHONE               200

#define ATCD_SB_POWERSAVE           300

#define ATCD_SB_GPRS_INIT           400
#define ATCD_SB_GPRS_DEINIT         500

#define ATCD_SB_CONN_OPEN           600
#define ATCD_SB_CONN_WRITE          700
#define ATCD_SB_CONN_READ           800
#define ATCD_SB_CONN_CLOSE          900

#define ATCD_SB_GPS_START           1000
#define ATCD_SB_GPS_STOP            1100

#define ATCD_SB_END                 1200

#define ATCD_SO_ERR                 98
#define ATCD_SO_END                 99

//------------------------------------------------------------------------------
/*typedef enum
{
  atcd_pwrsSave,
  atcd_pwrsFull

} atcd_powersave_req_t;*/
//------------------------------------------------------------------------------
typedef struct
{
  uint32_t rx_ov;
  uint32_t reset;
  uint32_t warn;
  uint32_t err;

  uint32_t wake_time;
  uint32_t awake_time_acc;

} atcd_stat_t;

typedef struct
{
  atcd_state_t state;                          //device state
  uint32_t timer;                              //current operation timer

  atcd_tx_state_t tx_state;                    //transmission state
  //uint8_t  tx_pending;                         //priznak cekajicicho vysilani
  //uint8_t  tx_conn_num;
  uint16_t tx_data_len;
  rbuff_t  tx_rbuff;

  uint16_t proc_step;                          //aktualni krok v sekvencnim automatu ovladani modemu
  uint16_t err_cnt;                            //pocitadlo chyb
  uint16_t err_max;                            //maximalni pocet chyb
  
  atcd_sleep_mode_t  sleep_mode;               //nastaveny mod rezimu spanku
  atcd_sleep_state_t sleep_state;              //aktualni stav rezimu spanku
  uint8_t            sleep_disable;            //docasne zakazani spanku
  uint32_t           sleep_timer;              //casovac spanku

  atcd_parser_t parser;                        //parser prichozich dat

  atcd_at_cmd_t at_cmd;                        //AT cmd for internal usage
  char at_cmd_buff[64];                        //TODO: kurva drat jakej imbecil NIKDE nekontroluje delku!!!!! buffer pro sestaveny AT prikaz - pohlidat delku a mozne preteceni... //musi se tam vejit SMS nebo ne?
  char at_cmd_result_buff[32];                 //buffer pro nestandardni OK odpoved... spis regex ale to zase jindy
  rbuff_t at_cmd_data;                         //kruhovy buffer pro volitelna data k at prikazu

  atcd_conns_t conns;                          //TCP/UDP conections

  // Podmineny preklad
  atcd_sim_t sim;                       //
  atcd_gsm_t gsm;                       //
  atcd_phone_t phone;                   //
  atcd_gprs_t gprs;                     //
  atcd_gps_t gps;                       //
  atcd_wifi_t wifi;                     //

  uint8_t cb_events;                    //device callback events
  void (*callback)(uint8_t);            //events callback
  void (*state_update_callback)();      //called every 30s after state is updated

  atcd_stat_t stat;                     //statistiky

  /*struct
  {
    unsigned int atcd_atc_complete;
    unsigned int atcd_atc_send;
  } errors;*/

} atcd_t;

// Functions -------------------------------------------------------------------
// ATC Device
void atcd_init();                //init AT command device
void atcd_reset();               //reset AT command device
void atcd_start();               //start AT command device

//void atcd_set_powersave(atcd_powersave_req_t mode);   //enable power saving
//void atcd_set_powersave_hwsetter(void (*powersave_hwsetter)(uint8_t awake));
void atcd_set_sleep_mode(atcd_sleep_mode_t mode);
atcd_sleep_mode_t atcd_sleep_mode();

void atcd_set_system_callback(uint8_t eventmask, void (*system_callback)(uint8_t event));

void atcd_rx_data(uint8_t *data, uint16_t len);  //zpracuje prijata data
void atcd_rx_str(char *ch);                      //zpracuje prijaty retezec
void atcd_rx_ch(char ch);                        //zpracuje prijaty znak

void atcd_tx_complete();         //call on tx data complete
void atcd_sw_reset();            //SW reset
void atcd_state_reset();

void atcd_proc();                //data processing 

uint8_t atcd_state();
uint32_t atcd_awaketime();
//------------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif /* ATCD_H_INCLUDED */
