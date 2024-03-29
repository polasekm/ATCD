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

#define ATCD_DBG 1

#if(ATCD_DBG == 1)
  #define ATCD_DBG_BUFF_OVERRUN         atcd_dbg_warn("ATCD: ", "V bufferu pro ATCD doslo misto!\r\n");
  #define ATCD_DBG_ATC_LN_BUFF_OV       atcd_dbg_warn("ATCD: ", "Nastavuji overrun u ATC??????\r\n");
  #define ATCD_DBG_BOOT_SEQ             atcd_dbg_inf("ATCD: ", "Detekovana bootovaci sekvence.\r\n");
  #define ATCD_DBG_STARTING             atcd_dbg_inf("ATCD: ", "Spusteni zarizeni.\r\n");
  #define ATCD_DBG_CREG                 atcd_dbg_inf("ATCD: GSM:", " +CREG: x detect.\r\n");
  #define ATCD_DBG_CREG_ERR             atcd_dbg_warn("ATCD: GSM:", " +CREG: x je mimo rozah.\r\n");
  #define ATCD_DBG_ATC_BUFF_OV          atcd_dbg_warn("ATCD: ", "V cilovem bufferu ATC neni dost mista!\r\n");
  #define ATCD_DBG_RESET                atcd_dbg_warn("ATCD: ", "Reset zarizeni.\r\n");

  #define ATCD_DBG_SW_ERR               atcd_dbg_err("ATCD: ", "Neocekavana hodnota v SW hlavni smycky\r\n");

  #define ATCD_DBG_PIN_NONE             atcd_dbg_inf("ATCD: INIT: ", "PIN neni treba.\r\n");
  #define ATCD_DBG_PIN_REQ              atcd_dbg_inf("ATCD: INIT: ", "Je treba zadat PIN.\r\n");
  #define ATCD_DBG_PUK_REQ              atcd_dbg_inf("ATCD: INIT: ", "Je treba zadat PUK.\r\n");
  #define ATCD_DBG_PIN_ERR              atcd_dbg_err("ATCD: INIT: ", "Na dotaz na PIN prisla neocekavana odpoved - zacinam znovu!\r\n");

  #define ATCD_DBG_STAT_DONE            atcd_dbg_inf("ATCD: STAT: ", "Dotazovani na stav modemu bylo dokonceno.\r\n");
  
  #define ATCD_DBG_ATC_SEND_DATA        atcd_dbg_inf("ATCD: ATC: ", "Odesilani dat bylo zahajeno.\r\n");
  #define ATCD_DBG_ATC_SEND_DATA_ERR    atcd_dbg_err("ATCD: ATC: ", "ATC nema zadna data k odeslani, ackoli se to cekalo!\r\n");
  #define ATCD_DBG_ATC_SEND_CMD         atcd_dbg_inf("ATCD: ATC: ", "Odesilani prikazu bylo zahajeno.\r\n");

  #define ATCD_DBG_ATC_EXE              atcd_dbg_inf("ATCD: ATC: ", "Fronta je volna - spoustim AT prikaz.\r\n");
  #define ATCD_DBG_ATC_WAIT_P           atcd_dbg_inf("ATCD: ATC: ", "Ve fronte neni zadny dalsi AT prikaz ale parser je zamestnan - cekam.\r\n");
  #define ATCD_DBG_ATC_WAIT_Q           atcd_dbg_inf("ATCD: ATC: ", "Je fronta - cekam na konci.\r\n");

  #define ATCD_DBG_ATC_W_ECHO           atcd_dbg_inf("ATCD: ATC: ", "Odesilani bylo dokoceno - prechazime na W_ECHO...\r\n");
  #define ATCD_DBG_ATC_W_END            atcd_dbg_inf("ATCD: ATC: ", "Odesilani bylo dokoceno - prechazime na W_END...\r\n");
  #define ATCD_DBG_ATC_TX_PEND          atcd_dbg_inf("ATCD: ATC: ", "Prechazime na zadavani dat...\r\n");
  #define ATCD_DBG_ATC_QUEUE_EXE        atcd_dbg_inf("ATCD: ATC: ", "Ve fronte je cekajici AT prikaz - menim jeho stav a odesilam.\r\n");
  #define ATCD_DBG_ATC_TIM              atcd_dbg_warn("ATCD: ATC: ", "Probihajicimu AT prikazu vyprsel timeout.\r\n");
  #define ATCD_DBG_ATC_ESC_DATA         atcd_dbg_warn("ATCD: ATC: ", "Odesilam rn... pro eventualni ukonceni datoveho rezimu.\r\n");
  #define ATCD_DBG_ATC_QUEUE_END        atcd_dbg_inf("ATCD: ATC: ", "Ve fronte neni zadny dalsi cekajici AT prikaz - aktualizuji konec fronty.\r\n");

  #define ATCD_DBG_ATC_ECHO_DET         atcd_dbg_inf("ATCD: ATC: ", "ECHO detected.\r\n");
  #define ATCD_DBG_ATC_ECHO_T_FAIL      atcd_dbg_warn("ATCD: ATC: ", "ECHO test FAIL.\r\n");
  #define ATCD_DBG_ATC_OK_DET           atcd_dbg_inf("ATCD: ATC: ", "OK detected.\r\n");
  #define ATCD_DBG_ATC_ERR_DET          atcd_dbg_warn("ATCD: ATC: ", "ERROR detected.\r\n");
  #define ATCD_DBG_ATC_CMS_ERR_DET      atcd_dbg_warn("ATCD: ATC: ", "CMS ERROR detected.\r\n");
  #define ATCD_DBG_ATC_CME_ERR_DET      atcd_dbg_warn("ATCD: ATC: ", "CME ERROR detected.\r\n");
  #define ATCD_DBG_ATC_FAIL_DET         atcd_dbg_warn("ATCD: ATC: ", "FAIL detected.\r\n");
  #define ATCD_DBG_ATC_PROMT_DET        atcd_dbg_inf("ATCD: ATC: ", "Prompt \">\" detected.\r\n");

  #define ATCD_DBG_ATC_CANCELL          atcd_dbg_inf("ATCD: ATC: ", "Rusim AT prikaz.\r\n");
  #define ATCD_DBG_ATC_CANCELL_EQ       atcd_dbg_warn("ATCD: ATC: ", "Ruseny AT prikaz neni ve fronte.\r\n");
  #define ATCD_DBG_ATC_CANCELL_ALL      atcd_dbg_inf("ATCD: ATC: ", "Budu zruseny vsechny AT prikazy fe fronte.\r\n");

  #define ATCD_DBG_SEQ_ERR              atcd_dbg_warn("ATCD: ATC_SEQ: ", "Krok sekvence skoncil chybou!\r\n");
  #define ATCD_DBG_SEQ_ERR_MAX          atcd_dbg_warn("ATCD: ATC_SEQ: ", "Dosazano maximalniho poctu chyb sekvence!\r\n");
  #define ATCD_DBG_SEQ_NEXT             atcd_dbg_inf("ATCD: ATC_SEQ: ", "Spoustim dalsi krok sekvence.\r\n");
  #define ATCD_DBG_SEQ_STEP             atcd_dbg_inf("ATCD: ATC_SEQ: ", "Odesilam dalsi krok sekvence.\r\n");

  #define ATCD_DBG_IPD_SMS_TIM          atcd_dbg_err("ATCD: ", "Vyprsel timeout na IPD nebo SMS!\r\n");
  #define ATCD_DBG_START_TIM            atcd_dbg_err("ATCD: ", "Vyprsel timeout na start!\r\n");
  #define ATCD_DBG_INIT_TIM             atcd_dbg_err("ATCD: ", "Vyprsel timeout na inicializaci!\r\n");

  #define ATCD_DBG_INIT_DONE            atcd_dbg_inf3("ATCD: INIT: ", "Sekvence byla dokoncena.\r\n");
  #define ATCD_DBG_INIT_ERR_R           atcd_dbg_err("ATCD: INIT: ", "atcd.init_seq je  mimo rozsah - zacinam znovu!\r\n");
  #define ATCD_DBG_INIT_START           atcd_dbg_inf("ATCD: INIT: ", "Zacina inicializace zarizeni.\r\n");
  #define ATCD_DBG_INIT_STEP            atcd_dbg_inf("ATCD: INIT: ", "Provadim krok inicializace zarizeni.\r\n");
  #define ATCD_DBG_INIT_ERR             atcd_dbg_err("ATCD: INIT: ", "Inicializace selhala!\r\n");
  #define ATCD_DBG_INIT_OK              atcd_dbg_inf("ATCD: INIT: ", "Inicializace zarizeni byla dokoncena.\r\n");
 
  #define ATCD_DBG_STAT_START           atcd_dbg_inf("ATCD: STAT: ", "Dotazuji se na stav modemu.\r\n");
  #define ATCD_DBG_STAT_STEP            atcd_dbg_inf("ATCD: STAT: ", "Provadim krok dotazovani se na stav modemu\r\n");
  #define ATCD_DBG_STAT_ERR             atcd_dbg_err("ATCD: STAT: ", "Dotazovani na stav modemu selhalo.\r\n");
  #define ATCD_DBG_STAT_OK              atcd_dbg_inf("ATCD: STAT: ", "Dotazuji na stav modemu je hotovo.\r\n");

  #define ATCD_DBG_GPRS_SEQ_STEP       atcd_dbg_inf("ATCD: GPRS: SEQ: ", "Provadim krok sekvence AT prikazu GPRS.\r\n");
  #define ATCD_DBG_GPRS_SEQ_ERR        atcd_dbg_warn("ATCD: GPRS: SEQ: ", "Prekrocen pocet neuspesnych pokusu, zkousim o opravu odpojenim a znovu pripojenim.\r\n");
  #define ATCD_DBG_GPRS_SEQ_OK         atcd_dbg_inf("ATCD: GPRS: SEQ: ", "Sekvence AT prikazu byla dokoncena.\r\n");
  #define ATCD_DBG_GPRS_STATE_ERR      atcd_dbg_err("ATCD: GPRS: ", "Stav GPRS je mimo rozah.\r\n");

  #define ATCD_DBG_GPRS_STAT_START     atcd_dbg_inf("GPRS: STAT: ", "Kontrola stavu pripojeni.\r\n");
  #define ATCD_DBG_GPRS_STAT_CGATT     atcd_dbg_inf("GPRS: STAT: ", "Dotazuji se na stav GPRS.\r\n");
  
  #define ATCD_DBG_GPRS_CONN_SET       atcd_dbg_inf("GPRS: ", "Zahajuji pripojeni ke GPRS.\r\n");
  #define ATCD_DBG_GPRS_DISCONN_SET    atcd_dbg_inf("GPRS: ", "Zahajuji odpojeni od GPRS.\r\n");
  #define ATCD_DBG_GPRS_TIMEOUT        atcd_dbg_warn("GPRS: ", "Vyprsel cas pripojeni nebo odpojeni od GPRS.\r\n");

  #define ATCD_DBG_GPRS_INIT_START     atcd_dbg_inf("GPRS: INIT: ", "Zacinam s inicializaci GPRS.\r\n");
  #define ATCD_DBG_GPRS_INIT_OK        atcd_dbg_inf("GPRS: INIT: ", "Init sekvence dokoncena - cekam na pripojeni.\r\n");
  #define ATCD_DBG_GPRS_INIT_ERR       atcd_dbg_warn("GPRS: INIT: ", "Pri init sekvenci doslo k chybe - odpojuji se.\r\n");

  #define ATCD_DBG_GPRS_DEINIT_START   atcd_dbg_inf("GPRS: DEINIT: ", "Zacinam s deinicializaci GPRS.\r\n");
  #define ATCD_DBG_GPRS_DEINIT_OK      atcd_dbg_inf("GPRS: DEINIT: ", "Deinit sekvence dokoncena.\r\n");
  #define ATCD_DBG_GPRS_DEINIT_ERR     atcd_dbg_warn("GPRS: DEINIT: ", "Pri deinit sekvenci doslo k chybe - nastavuji odpojeno.\r\n");
  
  #define ATCD_DBG_PHONE_RING_DET      atcd_dbg_inf("ATCD: PHONE: ", "RING detect.\r\n");
  #define ATCD_DBG_PHONE_BUSY_DET      atcd_dbg_inf("ATCD: PHONE: ", "BUSY detect.\r\n");
  #define ATCD_DBG_PHONE_NO_CAR_DET    atcd_dbg_inf("ATCD: PHONE: ", "NO CARRIER detect.\r\n");

  #define ATCD_DBG_PHONE_SMS_DET       atcd_dbg_inf("ATCD: PHONE: ", "New SMS detected.\r\n");
  #define ATCD_DBG_PHONE_CALL_DET      atcd_dbg_inf("ATCD: PHONE: ", "Voice call detected.\r\n");
  #define ATCD_DBG_PHONE_CALL_N_DET    atcd_dbg_inf("ATCD: PHONE: ", "Voice call number detected.\r\n");
  #define ATCD_DBG_PHONE_CALL_N_DET_E  atcd_dbg_warn("ATCD: PHONE: ", "Voice call number detected error.\r\n");

  #define ATCD_DBG_PHONE_SMS_DET_ERR   atcd_dbg_warn("ATCD: PHONE: ", "New SMS detected - parsing error\r\n");
  #define ATCD_DBG_PHONE_SMS_BUFF_E    atcd_dbg_err("ATCD: PHONE: ", "V bufferu sms neni dostatek mista pro dalsi prijem dat!\r\n");
  #define ATCD_DBG_PHONE_SMS_END       atcd_dbg_inf("ATCD: PHONE: ", "Dosahli jsme konce SMS bloku.\r\n");

  #define ATCD_DBG_WIFI_CONN_DET       atcd_dbg_inf("ATCD: WIFI: ", "WIFI CONNECTED detect...\r\n");
  #define ATCD_DBG_WIFI_GOT_IP         atcd_dbg_inf("ATCD: WIFI: ", "WIFI GOT IP detect..\r\n");
  #define ATCD_DBG_WIFI_DISCONN_DET    atcd_dbg_inf("ATCD: WIFI: ", "WIFI DISCONNECT detect...\r\n");

  #define ATCD_DBG_CONN_TIM_WOP        atcd_dbg_warn("ATCD: CONN: ", "Spojeni vyprsel timeout/wop - rusim jej.\r\n"); //want open
  #define ATCD_DBG_CONN_TIM_OPE        atcd_dbg_warn("ATCD: CONN: ", "Spojeni vyprsel timeout/ope - rusim jej.\r\n"); //opening
  #define ATCD_DBG_CONN_TIM_CLO        atcd_dbg_warn("ATCD: CONN: ", "Spojeni vyprsel timeout/clo - rusim jej.\r\n"); //closing
  #define ATCD_DBG_CONN_ALLOC          atcd_dbg_inf("ATCD: CONN: ", "Spojeni prirazeno.\r\n");
  #define ATCD_DBG_CONN_ALLOC_ERR      atcd_dbg_warn("ATCD: CONN: ", "Spojeni neprirazeno - vycerpan max pocet.\r\n");

  #define ATCD_DBG_CONN_WRITE          atcd_dbg_inf("ATCD: CONN: ", "Ukladam data k odeslani do bufferu.\r\n");
  #define ATCD_DBG_CONN_CLOSE_W        atcd_dbg_inf("ATCD: CONN: ", "Nastavuji spojeni jako cekajici na uzavreni.\r\n");
  #define ATCD_DBG_CONN_FREE           atcd_dbg_inf("ATCD: CONN: ", "Uvolnuji cislo spojeni.\r\n");
  #define ATCD_DBG_CONN_FREE_ERR       atcd_dbg_err("ATCD: CONN: ", "Cislo spojeni k uvolneni je mimo rozsah!\r\n");

  #define ATCD_DBG_CONN_DATA_RX_DET    atcd_dbg_inf("ATCD: CONN: ", "ATCD_STR_DATA_RX detected.\r\n");
  #define ATCD_DBG_CONN_DATA_RX_LEN_E  atcd_dbg_err("ATCD: CONN: ", "ATCD_STR_DATA_RX ma velikost 0!\r\n");
  #define ATCD_DBG_CONN_DATA_RX_RNG_E  atcd_dbg_err("ATCD: CONN: ", "ATCD_STR_DATA_RX ma conn_id mimo rozsah!\r\n");

  #define ATCD_DBG_CONN_DET            atcd_dbg_inf("ATCD: CONN: ", "x, CONNECT OK detect.\r\n");
  #define ATCD_DBG_CONN_UNREG          atcd_dbg_err("ATCD: CONN: ", "Nelze zmenit stav neregistrovaneho spojeni!\r\n");
  #define ATCD_DBG_CONN_RNG_E          atcd_dbg_err("ATCD: CONN: ", "x, CONNECT detect - x mimo rozah.\r\n");

  #define ATCD_DBG_CONN                atcd_dbg_inf("ATCD: CONN: ", "Spojeni je navazano.\r\n");
  #define ATCD_DBG_CONN_OP_ERR         atcd_dbg_warn("ATCD: CONN: ", "Neznama operace.\r\n");

  #define ATCD_DBG_CONN_CLOSE_DET      atcd_dbg_inf("ATCD: CONN: ", "x, CLOSED detect.\r\n");
  #define ATCD_DBG_CONN_CLOSE_RNG_E    atcd_dbg_err("ATCD: CONN: ", "x, CLOSED detect - x mimo rozah.\r\n");

  #define ATCD_DBG_CONN_FAIL_DET      atcd_dbg_inf("ATCD: CONN: ", "x, CONNECT FAIL detect.\r\n");
  #define ATCD_DBG_CONN_FAIL_RNG_E    atcd_dbg_err("ATCD: CONN: ", "x, CONNECT FAIL detect - x mimo rozah.\r\n");

  #define ATCD_DBG_CONN_CLOSE_OK_DET   atcd_dbg_inf("ATCD: CONN: ", "x, CLOSE OK detect.\r\n");
  #define ATCD_DBG_CONN_CLOSE_OK_RE    atcd_dbg_err("ATCD: CONN: ", "x, CLOSE OK detect - x mimo rozah.\r\n");

  #define ATCD_DBG_CONN_OPENING        atcd_dbg_inf("ATCD: CONN: ", "Otevitam spojeni.\r\n");
  #define ATCD_DBG_CONN_PROT_ERR       atcd_dbg_err("ATCD: CONN: ", "Pokus otevrit spojeni nepodporovanym protokolem!\r\n");
  #define ATCD_DBG_CONN_W_CONN         atcd_dbg_inf("ATCD: CONN: ", "Prikaz pro otevreni spojeni dokoncen.\r\n");               //TODO Prejmenovat, uz to nema mit v nazvu wait... _W
  #define ATCD_DBG_CONN_OPENING_ERR    atcd_dbg_warn("ATCD: CONN: ", "Prikaz pro otevreni spojeni skoncil chybou!\r\n");
  #define ATCD_DBG_CONN_OPEN_FAIL      atcd_dbg_warn("ATCD: CONN: ", "Pokus otevrit spojeni selhal - bude zruseno!\r\n");
  #define ATCD_DBG_CONN_SEND           atcd_dbg_inf("ATCD: CONN: ", "Odesilam prikaz pro odeslani dat.\r\n");
  #define ATCD_DBG_CONN_SEND_ERR       atcd_dbg_warn("ATCD: CONN: ", "Prikaz pro odeslani dat skoncil chybou.\r\n");
  #define ATCD_DBG_CONN_SEND_FAIL      atcd_dbg_warn("ATCD: CONN: ", "Pokus o zapis do spojeni selhal - spojeni bude uzavreno!\r\n");

  #define ATCD_DBG_CONN_CLOSING        atcd_dbg_inf("ATCD: CONN: ", "Zaviram spojeni.\r\n");
  #define ATCD_DBG_CONN_CLOSING_ERR    atcd_dbg_warn("ATCD: CONN: ", "Prikaz pro ukoceni spojeni skoncil chybou!\r\n");
  #define ATCD_DBG_CONN_CLOSE_FAIL     atcd_dbg_warn("ATCD: CONN: ", "Pokus zavrit spojeni selhal - bude zruseno!\r\n");
  #define ATCD_DBG_CONN_W_CLOSE        atcd_dbg_inf("ATCD: CONN: ", "Prikaz pro uzavreni spojeni dokoncen.\r\n");              //TODO Prejmenovat, uz to nema mit v nazvu wait... _W

  #define ATCD_DBG_CONN_SISW_DET       atcd_dbg_inf("ATCD: CONN: ", "^SISW: x detected.\r\n");
  #define ATCD_DBG_CONN_SISW_P         atcd_dbg_inf("ATCD: CONN: ", "Prompt \"AT^SISW=x,x,x\" detected.\r\n");
  #define ATCD_DBG_CONN_SISW_P_DE      atcd_dbg_err("ATCD: CONN: ", "ATC nema zadna data k odeslani ackoli se ocekavalo ze ma!!\r\n");
  #define ATCD_DBG_CONN_SISW_RNG_E     atcd_dbg_err("ATCD: CONN: ", "^SISW: x detect - x mimo rozah.\r\n");

  #define ATCD_DBG_CONN_SIS_DET        atcd_dbg_inf("ATCD: CONN: ", "SIS: x,y... detect.\r\n");
  #define ATCD_DBG_CONN_SIS_CLOSE      atcd_dbg_inf("ATCD: CONN: ", "Spojeni je zruseno.\r\n");
  #define ATCD_DBG_CONN_SIS_PARAM_YE   atcd_dbg_warn("ATCD: CONN: ", "SIS: x,y - spatna hodnota parametru y.\r\n");
  #define ATCD_DBG_CONN_SIS_PARAM_XE   atcd_dbg_err("ATCD: CONN: ", "SIS: x,y... detect - x mimo rozah.\r\n");

  #define ATCD_DBG_CONN_SEND_OK_DET    atcd_dbg_inf("ATCD: CONN: ", "x, SEND OK.\r\n");
  #define ATCD_DBG_CONN_ALRD_CON_DET   atcd_dbg_err("ATCD: CONN: ", "x, ALREADY CONNECT.\r\n");
  #define ATCD_DBG_CONN_DNS_FAIL_DET   atcd_dbg_inf("ATCD: CONN: ", "DNS Fail detect.\r\n");

  #define ATCD_DBG_CONN_BUFF_E         atcd_dbg_warn("ATCD: CONN: ", "V bufferu spojeni neni dostatek mista pro dalsi prijem dat!\r\n");
  #define ATCD_DBG_CONN_IPD_END        atcd_dbg_inf("ATCD: CONN: ", "Dosahli jsme konce IPD bloku.\r\n");
  #define ATCD_DBG_CONN_IPD_ERR        atcd_dbg_err("ATCD: CONN: ", "Je rezim IPD, ale nenastaveno prijimajici spojeni! - Prechazim do rezimu ATC.\r\n");

  #define ATCD_DBG_GPS_SENTECE         atcd_dbg_inf("ATCD: GPS: ", "$G detect - NMEA veta.\r\n");
  #define ATCD_DBG_GPS_USENTECE        atcd_dbg_warn("ATCD: GPS: ", "Veta nebyla rozpoznana / neni podporovana.\r\n");
  #define ATCD_DBG_GPS_SENTECE_OFF     atcd_dbg_inf("ATCD: GPS: ", "Byla prijata NMEA veta ale GPS je vypnuta / vypina se.\r\n");

  #define ATCD_DBG_GPS_RMC             atcd_dbg_inf("ATCD: GPS: ", "RMC sentecne detect\r\n");
  #define ATCD_DBG_GPS_GSA             atcd_dbg_inf("ATCD: GPS: ", "GSA sentecne detect\r\n");
  #define ATCD_DBG_GPS_GGA             atcd_dbg_inf("ATCD: GPS: ", "GGA sentecne detect\r\n");
  #define ATCD_DBG_GPS_GSV             atcd_dbg_inf("ATCD: GPS: ", "GSV sentecne detect\r\n");
  #define ATCD_DBG_GPS_VTG             atcd_dbg_inf("ATCD: GPS: ", "VTG sentecne detect\r\n");
  #define ATCD_DBG_GPS_ACC             atcd_dbg_inf("ATCD: GPS: ", "ACCURACY sentecne detect\r\n");
  #define ATCD_DBG_GPS_BIAS            atcd_dbg_inf("ATCD: GPS: ", "HWBIAS sentecne detect\r\n");

  #define ATCD_DBG_GPS_CS_ERR          atcd_dbg_err("ATCD: GPS: ", "Chybny checksum vety\r\n");

  #define ATCD_DBG_GPS_RMC_ERR         atcd_dbg_err("ATCD: GPS: ", "Chyba parsovani vety RMC\r\n");
  #define ATCD_DBG_GPS_GSA_ERR         atcd_dbg_err("ATCD: GPS: ", "Chyba parsovani vety GSA\r\n");
  #define ATCD_DBG_GPS_GGA_ERR         atcd_dbg_err("ATCD: GPS: ", "Chyba parsovani vety GGA\r\n");
  #define ATCD_DBG_GPS_GSV_ERR         atcd_dbg_err("ATCD: GPS: ", "Chyba parsovani vety GSV\r\n");
  #define ATCD_DBG_GPS_VTG_ERR         atcd_dbg_err("ATCD: GPS: ", "Chyba parsovani vety VTG\r\n");
  #define ATCD_DBG_GPS_ACC_ERR         atcd_dbg_err("ATCD: GPS: ", "Chyba parsovani vety ACCURACY\r\n");
  #define ATCD_DBG_GPS_BIAS_ERR        atcd_dbg_err("ATCD: GPS: ", "Chyba parsovani vety HWBIAS\r\n");

  #define ATCD_DBG_GPS_ENABLING        atcd_dbg_inf("ATCD: GPS: ", "Enabling GPS receiver\r\n");
  #define ATCD_DBG_GPS_ENABLED         atcd_dbg_inf("ATCD: GPS: ", "Enabled GPS receiver\r\n");
  #define ATCD_DBG_GPS_DIABLING        atcd_dbg_inf("ATCD: GPS: ", "Disabling GPS receiver\r\n");
  #define ATCD_DBG_GPS_DIABLED         atcd_dbg_inf("ATCD: GPS: ", "Disabled GPS receiver\r\n");

#else
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



#ifndef ATCD_USE_DEVICE
  #define ATCD_USE_DEVICE  ATCD_A6
#endif /* ATCD_USE_DEVICE */

//-----------------------------
#if(ATCD_USE_DEVICE == ATCD_A6)
  #define ATCD_STR_START_SEQ        "AT Ready\r\n"
  #define ATCD_STR_DATA_RX          "+RECEIVE,"

  #define ATCD_STR_SIM_READY        "+CPIN:READY"
  #define ATCD_STR_SIM_PIN          "+CPIN:SIM PIN"
  #define ATCD_STR_SIM_PIN2         "+CPIN:SIM PIN2"
  #define ATCD_STR_SIM_PUK          "+CPIN:SIM PUK"
  #define ATCD_STR_SIM_PUK2         "+CPIN:SIM PUK2"

  #define ATCD_DATA_RX_NL
  #define ATCD_RX_NL_LEN  2

  #define ATCD_CONN_MAX_NUMBER 4
#endif /* ATCD_USE_DEVICE */
//-----------------------------
#if(ATCD_USE_DEVICE == ATCD_A7)
  #define ATCD_STR_START_SEQ        "^CINIT: 1, 0, 0\r\n"
  #define ATCD_STR_DATA_RX          "+RECEIVE,"

  #define ATCD_STR_SIM_READY        "+CPIN:READY"
  #define ATCD_STR_SIM_PIN          "+CPIN:SIM PIN"
  #define ATCD_STR_SIM_PIN2         "+CPIN:SIM PIN2"
  #define ATCD_STR_SIM_PUK          "+CPIN:SIM PUK"
  #define ATCD_STR_SIM_PUK2         "+CPIN:SIM PUK2"

  #define ATCD_DATA_RX_NL
  #define ATCD_RX_NL_LEN  2

  #define ATCD_CONN_MAX_NUMBER 4
#endif /* ATCD_USE_DEVICE */
//-----------------------------
#if(ATCD_USE_DEVICE == ATCD_SIM868)
  #define ATCD_STR_START_SEQ        "RDY\r\n"//nemuze zacinat \r\n protoze minuly radek se zpracuje   "\r\nRDY\r\n"
  #define ATCD_STR_DATA_RX          "+RECEIVE,"

  #define ATCD_STR_SIM_READY        "+CPIN: READY"
  #define ATCD_STR_SIM_PIN          "+CPIN: SIM PIN"
  #define ATCD_STR_SIM_PIN2         "+CPIN: SIM PIN2"
  #define ATCD_STR_SIM_PUK          "+CPIN: SIM PUK"
  #define ATCD_STR_SIM_PUK2         "+CPIN: SIM PUK2"

  #define ATCD_DATA_RX_NL
  #define ATCD_RX_NL_LEN  2

  #define ATCD_CONN_MAX_NUMBER 6
#endif /* ATCD_USE_DEVICE */
//-----------------------------
#if(ATCD_USE_DEVICE == ATCD_SIM7000)
  #define ATCD_STR_START_SEQ        "\r\nRDY\r\n"
  #define ATCD_STR_DATA_RX          "+RECEIVE,"

  #define ATCD_STR_SIM_READY        "+CPIN: READY"
  #define ATCD_STR_SIM_PIN          "+CPIN: SIM PIN"
  #define ATCD_STR_SIM_PIN2         "+CPIN: SIM PIN2"
  #define ATCD_STR_SIM_PUK          "+CPIN: SIM PUK"
  #define ATCD_STR_SIM_PUK2         "+CPIN: SIM PUK2"

  #define ATCD_DATA_RX_NL
  #define ATCD_RX_NL_LEN  2

  #define ATCD_CONN_MAX_NUMBER 8
#endif /* ATCD_USE_DEVICE */
//----------------------------------------
#if(ATCD_USE_DEVICE == ATCD_BGS5E)
  #define ATCD_STR_START_SEQ        "^SYSSTART\r\n"
  #define ATCD_STR_DATA_RX          "+IPD,"

  #define ATCD_STR_SIM_READY        "+CPIN: READY"
  #define ATCD_STR_SIM_PIN          "+CPIN: SIM PIN"
  #define ATCD_STR_SIM_PUK          "+CPIN: SIM PUK"

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
