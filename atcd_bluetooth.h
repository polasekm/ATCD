/*-----------------------------------------------------------------------------*/
/*
 *
 *
 *  Created on: Sep 19, 2023
 *      Author: Yevhenii Basov
 */
/*-----------------------------------------------------------------------------*/
#ifndef ATCD_BLUETOOTH_H_INCLUDED
#define ATCD_BLUETOOTH_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "atcd_config.h"

typedef enum{
  ATCD_BLE_STATE_OFF,
  ATCD_BLE_STATE_ON,
  ATCD_BLE_STATE_TO_OFF,
  ATCD_BLE_STATE_TO_ON,
  ATCD_BLE_STATE_ADD_SERV,
  ATCD_BLE_STATE_ADD_CHAR
} atcd_ble_state_e;

typedef enum{
  ATCD_BLE_DEVNAME_STATE_OUTDATED,
  ATCD_BLE_DEVNAME_STATE_UPDATED
} atcd_ble_devname_state_e;

typedef enum{
	ATCD_BLE_RESP_STATE_COMM,
	ATCD_BLE_RESP_STATE_NOTIFY
} atcd_ble_resp_state_e;

typedef enum{
	ATCD_BLE_PROP_STATE_OUTDATED,
	ATCD_BLE_PROP_STATE_UPDATED
} atcd_ble_prop_state_e;

//typedef struct
//{
//	int char_index;
//
//	char char_uuid[33];
//
//	unsigned int prop;
//
//	unsigned int permission;
//
//	int char_handle;
//
//} atcd_ble_service_characteristic_t;

//typedef struct
//{
//	int service_index;
//
//	char char_uuid[33];
//
//	unsigned int prop;
//
//	unsigned int permission;
//
//} atcd_ble_service_charac_buff_t;

//typedef struct
//{
//	int service_index;
//
//	char uuid[33];
//
//	bool is_primary;
//
//	atcd_ble_service_characteristic_t characteristic[5];
//
//} atcd_ble_service_t;

//typedef struct
//{
//	char uuid[33];
//
//	bool is_primary;
//
//} atcd_ble_service_buff_t;

typedef struct
{
  atcd_ble_state_e state;

  atcd_ble_devname_state_e devname_state;

  atcd_ble_resp_state_e response_state;

  atcd_ble_prop_state_e properties_state;

  char resp_buff[50];
  char mess[200];

  char device_name[50];

  int server_index;

  char user_id[50];

  int inst;

  char passwd[8];

  uint32_t init;

  int registered;

  int8_t power;
  uint8_t freq;
  uint8_t alarm_freq;
  uint8_t accel_sens;

  //atcd_ble_service_t service[5];

  //atcd_ble_service_buff_t serv_create_buff;

  //atcd_ble_charac_buff_t charac_create_buff;

  //int service_index[20];

  //int char_index[50];

} atcd_ble_t;

void atcd_ble_init();

void atcd_ble_enable();

void atcd_ble_disable();

void atcd_ble_change_name(const char * name);

//int atcd_ble_add_service(const char * uuid, bool is_primary);

//int atcd_ble_add_charac(int serv_idx, int char_uuid, unsigned int prop, unsigned int permission);

char atcd_ble_asc_msg();

#endif /* ATCD_BLUETOOTH_H_INCLUDED */
