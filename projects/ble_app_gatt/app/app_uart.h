/**
 ****************************************************************************************
 *
 * @file electric_uart_protocol.h
 *
 * @brief electric_uart Driver for HCI over UART operation.
 *
 * Copyright (C) Beken 2019-2022
 *
 *
 ****************************************************************************************
 */

#ifndef __APP_UART_PROTOCOL_H__
#define __APP_UART_PROTOCOL_H__
#include <stdint.h>



uint8_t lesend_data_process(void);
void uart_data_to_lesend_pack(void);
uint8_t store_uart_ringbuf_data(uint8_t *buf,uint16_t len);
uint8_t  read_uart_ringbuf_data(uint8_t *buf,uint16_t len);
void uart_ringbuf_clean(void);
void uart_data_to_rw_schedule(void);


#endif 
