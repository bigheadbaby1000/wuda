#ifndef __APP_API_H__
#define __APP_API_H__

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include "uart.h"

#define LESEND_MAX_LEN 640 //128+512
#define BUILD_INS_NUM 15

#define USER_COMMANDS_NUM (BUILD_INS_NUM-1)
#define ADV_MAX_DATA_NUM 31

#define YEAR ((((__DATE__ [7] - '0') * 10 + (__DATE__ [8] - '0')) * 10 \
+ (__DATE__ [9] - '0')) * 10 + (__DATE__ [10] - '0'))

#define MONTH (       \
  __DATE__ [2] == 'n' ? (__DATE__ [1] == 'a' ? 0 : 5) \
: __DATE__ [2] == 'b' ? 1 \
: __DATE__ [2] == 'r' ? (__DATE__ [0] == 'M' ? 2 : 3) \
: __DATE__ [2] == 'y' ? 4 \
: __DATE__ [2] == 'l' ? 6 \
: __DATE__ [2] == 'g' ? 7 \
: __DATE__ [2] == 'p' ? 8 \
: __DATE__ [2] == 't' ? 9 \
: __DATE__ [2] == 'v' ? 10 : 11)

#define DAY ((__DATE__ [4] == ' ' ? 0 : __DATE__ [4] - '0') * 10 \
+ (__DATE__ [5] - '0'))

#define DATE_AS_INT (((YEAR - 2000) * 12 + MONTH) * 31 + DAY)

typedef struct {
	uint8_t device_id;
	uint8_t mac[6];
	uint8_t type;
	uint8_t serv_uuid[16];
	uint8_t write_uuid[16];
	uint8_t notify_uuid[16];
}ble_dev_info;

struct ble_adv_data
{
	uint8_t adv_data[ADV_MAX_DATA_NUM];
};


extern uint16_t lesend_packet_len;
extern uint8_t lesend_packet[];
extern uint16_t lesend_length_tailor;
extern uint8_t send_data_enable_flag;

extern ble_dev_info slave_device;
extern struct gap_bdaddr conn_bdaddr;
uint8_t mac_search(uint8_t *bdaddr);
uint8_t free_channel_search(void);
int8_t channel_search(uint8_t *mac);
void soft_reset(void);
void ble_mac_read(void);



uint8_t app_send_ble_data(uint8_t device_id,uint16_t packet_len,uint8_t* packet);
void ble_data_rx(uint8_t *data, uint8_t len, uint8_t channel);
#endif
