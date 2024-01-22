#include "atcd_bluetooth.h"
#include "atcd.h"

extern atcd_t atcd;

void atcd_ble_init(){
	atcd.ble.init = 0;
	atcd.ble.registered = 0;
	atcd.ble.devname_state = ATCD_BLE_DEVNAME_STATE_UPDATED;
	atcd.ble.response_state = ATCD_BLE_RESP_STATE_COMM;
	atcd.ble.properties_state = ATCD_BLE_PROP_STATE_UPDATED;
	atcd.ble.server_index = -1;
	memcpy(atcd.ble.passwd, (unsigned char[]){0x7A,0x1A,0xE5,0x88,0x36,0x5B,0xD4,0xDD}, 8);
	for(int i = 0; i < 200; i++){
		atcd.ble.mess[i] = '\0';
	}
}

void atcd_ble_enable(){
	if(atcd.ble.state != ATCD_BLE_STATE_ON || atcd.ble.state != ATCD_BLE_STATE_TO_ON)
	atcd.ble.state = ATCD_BLE_STATE_TO_ON;
}

void atcd_ble_disable(){
	if(atcd.ble.state != ATCD_BLE_STATE_OFF || atcd.ble.state != ATCD_BLE_STATE_TO_OFF)
	atcd.ble.state = ATCD_BLE_STATE_TO_OFF;
}

void atcd_ble_change_name(const char * name){
	if(strcmp(atcd.ble.device_name, name) != 0){
		strcpy(atcd.ble.device_name, name);
		atcd.ble.devname_state = ATCD_BLE_DEVNAME_STATE_OUTDATED;
	}
}

void atcd_ble_send_data(char * data){
	//memcpy(atcd.ble.resp_buff, data, 8);
//
//	strcpy(atcd.ble.resp_buff, atoi((int)data[0]));
//	for(int i = 0; i < 7; i++){
//		strcat(atcd.ble.resp_buff, atoi((int)data[i]));
//	}
	sprintf(atcd.ble.resp_buff, "%d%d%d%d", (int)data[4],(int)data[5],(int)data[6],(int)data[7]);
	atcd.ble.response_state = ATCD_BLE_RESP_STATE_NOTIFY;
}

//int atcd_ble_add_service(const char * uuid, bool is_primary){
//	strcpy(atcd.ble.serv_create_buff.uuid, uuid);
//}

//int atcd_ble_add_characteristic(int serv_idx, int char_uuid, unsigned int prop, unsigned int permission);

char atcd_ble_asc_msg(){
	char *str = atcd.parser.buff + atcd.parser.line_pos;
	char buff[50];
	int buff_size = 0;
	char resp[50];
	int resp_len;
	if(strncmp(str, "+BLESWREQ", strlen("+BLESWREQ")) == 0){
		sscanf(str, "+BLESWREQ: %*8c,%*d,%*d,%*17c,%*d,%[^,],%*d,%*d,%*d", buff);
	}
	else return 0;
	//strcpy(str, atcd.parser.buff);
	if(!toHex(buff) || !(buff_size = toHex(buff))){
		return 0;
	}
	printf(buff);
	str[0] = '-';


	switch(buff[0]){
	case 0x4C:
		if(buff[1] == 0x8){
			if(strncmp(buff + 2, atcd.ble.passwd, 8) == 0){
				resp_len = HextoStr(resp, "CC0101", 6);
				atcd.ble.registered = 1;
			}
			else{
				resp_len = HextoStr(resp, "CC0100", 6);
			}
		}
		else{
			resp_len = HextoStr(resp, "CC0100", 6);
		}
		strncpy(atcd.ble.resp_buff, resp, resp_len);
		atcd.ble.response_state = ATCD_BLE_RESP_STATE_NOTIFY;
		atcd.parser.line_pos += strlen(buff);
		return 1;
	default:
		switch(buff[0]){
		case 0x53:
			if(!atcd.ble.registered){
				resp_len = HextoStr(resp, "CC0100", 6);
				strncpy(atcd.ble.resp_buff, resp, resp_len);
				atcd.ble.response_state = ATCD_BLE_RESP_STATE_NOTIFY;
			}
			else{
				if(buff[1] == 0x4){
					atcd.ble.power = buff[2];
					atcd.ble.freq = buff[3];
					atcd.ble.alarm_freq = buff[4];
					atcd.ble.accel_sens = buff[5];
					atcd.ble.properties_state = ATCD_BLE_PROP_STATE_OUTDATED;

					resp_len = HextoStr(resp, "D304", 4);
					char temp[8];
					HextoStr(temp, buff + 2, 4);
					resp_len += HextoStr(resp + 8, temp, 8);
				}
				else{
					resp_len = HextoStr(resp, "0", 1);
				}
			}
			strncpy(atcd.ble.resp_buff, resp, resp_len);
			atcd.ble.response_state = ATCD_BLE_RESP_STATE_NOTIFY;
			atcd.parser.line_pos += strlen(buff);
			return 1;
		case 0x50:
			if(!atcd.ble.registered){
				resp_len = HextoStr(resp, "CC0100", 6);
				strncpy(atcd.ble.resp_buff, resp, resp_len);
				atcd.ble.response_state = ATCD_BLE_RESP_STATE_NOTIFY;
			}
			else{
				if(buff[1] == 0x0){
					char data[4];
					data[0] = atcd.ble.power;
					data[1] = atcd.ble.freq;
					data[2] = atcd.ble.alarm_freq;
					data[3] = atcd.ble.accel_sens;

					resp_len = HextoStr(resp, "D004", 4);
					char temp[8];
					HextoStr(temp, data, 4);
					resp_len += HextoStr(resp + 8, temp, 8);
				}
				else{
					resp_len = HextoStr(resp, "0", 1);
				}
			}
			strncpy(atcd.ble.resp_buff, resp, resp_len);
			atcd.ble.response_state = ATCD_BLE_RESP_STATE_NOTIFY;
			atcd.parser.line_pos += strlen(buff);
			return 1;
		default:
			return 0;
		}
	}
}

int toHex(char *str){
	char buff;
	int size = strlen(str);
	if(size % 2 != 0) return 0;

	for(int i = 0; i < size / 2; i++){
		sscanf((str + i*2), "%2x%*s", &buff);
		str[i] = buff;
	}

	str[size / 2] = '\0';
	return size / 2;
}

int HextoStr(char* out, char* in, int in_len){
	for(int i = 0; i < in_len; i++){
		out[2*i] = (in[i] / 0x10) + 0x30;
		if((in[i] / 0x10) > 0x9) out[2*i] += 0x7;
		out[2*i+1] = (in[i] % 0x10) + 0x30;
		if((in[i] % 0x10) > 0x9) out[2*i+1] += 0x7;
	}
	out[in_len * 2] = '\0';
	return in_len * 2;
}
