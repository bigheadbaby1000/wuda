

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include "ke_mem.h"
#include "ke_event.h"
#include "compiler.h"
#include "wdt.h"
#include "nvds.h"
#include "uart.h"
#include "icu.h"
#include "app.h"
#include "lld_util.h"
#include "app_api.h"
#include "app_uart.h"
#include "app_fff0.h"
#include "cli.h"
#include "rwip_config.h"
#include "app_task.h"
#include "app_fff0.h"
#include "gpio.h"



uint16_t lesend_packet_len=0;
uint16_t lesend_length_tailor=0;
uint8_t lesend_packet[TOTAL_BUF_NUM];
uint8_t send_data_enable_flag = 0;

volatile uint16_t pbuff_write = 0;
volatile uint16_t pbuff_read = 0;

//extern uint8_t dmo_channel;

void uart_data_to_lesend_pack(void)
{
	lesend_packet_len = pbuff_write;
	memcpy(lesend_packet, uart_rx_buf, lesend_packet_len);
	lesend_length_tailor = 0;
	send_data_enable_flag = 1;
}
void uart_data_to_rw_schedule(void)
{
	ke_event_set(KE_EVENT_AOS_CLI);
//	UART_PRINTF("-");
}

uint8_t lesend_data_process(void)
{
    uint16_t bt_rx_data_length_temp;//static
    uint16_t len;
    uint8_t data_buf_temp[TOTAL_BUF_NUM];
	
    //if((send_data_enable_flag==0)&&(uart_rx_done==0))return;
	if(send_data_enable_flag==0)
	{
		set_rts_busy(0);
		return APPM_ERROR_NO_ERROR; 
	}

    //uart_rx_done = 0;
    bt_rx_data_length_temp = lesend_packet_len;
    
    if(lesend_length_tailor < bt_rx_data_length_temp)
    {
		UART_PRINTF("lesend_packet_len = %d, mtu_size = %d\r\n", lesend_packet_len, app_env.mtu_size);
        if((bt_rx_data_length_temp - lesend_length_tailor) >= app_env.mtu_size)
        {
            len = app_env.mtu_size;
            memcpy(data_buf_temp, (const void*)(lesend_packet + lesend_length_tailor), len);
        }
        else
        {
            len = bt_rx_data_length_temp - lesend_length_tailor;
            memcpy(data_buf_temp, (const void*)(lesend_packet + lesend_length_tailor), len);
        }
//		for(int i = 0;i < APPC_IDX_MAX;i++)
		{			
			if(ke_state_get(KE_BUILD_ID(TASK_APP, 0)) == APPM_CONNECTED  )
			{
				if(app_fff1_send_lvl(0, data_buf_temp, len) == APPM_ERROR_NO_ERROR)
				{
//					UART_PRINTF("lensend = %d\r\n", len);
					send_data_enable_flag = 0;
					set_rts_busy(0);
//					aos_cli_printf("+DATASEND\r\n");
	//				uart_ringbuf_clean();
				}
				else
				{
//					UART_PRINTF("send error\r\n");
					lesend_packet_len = 0;
					lesend_length_tailor = 0;
					return APPM_ERROR_STATE;
				}
//				break;
			}
			else
			{
	//			aos_cli_printf("+ERROR\r\n");
			}
		}
        lesend_length_tailor += len;
        if(lesend_length_tailor == bt_rx_data_length_temp)
        {
            lesend_packet_len = 0;
            lesend_length_tailor = 0;
			uart_ringbuf_clean();
        }
    }

    return APPM_ERROR_NO_ERROR;
}



/*******************************************************************************
 * Function: store_uart_ringbuf_data
 * Description: store data into loop buffer
 * Input: uint8_t*
 * Input: uint16_t
 * Output: void
 * Return: uint8_t
 * Others: void
*******************************************************************************/

uint8_t store_uart_ringbuf_data(uint8_t *buf,uint16_t len)
{
	uint16_t free_cnt;
	uint8_t status ;
	
	//Calculates the number of empty buffer in the circular queue (the 
	//data stored in this queue is encoded)
	if(pbuff_write >= pbuff_read)
	{
		free_cnt = (TOTAL_BUF_NUM - pbuff_write + pbuff_read);
	}else
	{
		free_cnt = pbuff_read - pbuff_write;
	}
	//UART_PRINTF("free cnt: %d,store len : %d\r\n", free_cnt,len);
	
	//If there are at least two empty buffer in the loop queue, the current 
	//encoded data will be stored in buffer. 
	if(free_cnt >= len) 
	{
		memcpy((uint8_t *)&uart_rx_buf[pbuff_write],buf,len);
		

		pbuff_write = ((pbuff_write + len )% TOTAL_BUF_NUM);
		status = 1;
		//UART_PRINTF("enough data write!! : pbuff_write : %d, pbuff_read :%d\r\n",pbuff_write,pbuff_read);	
	}else
	{
		UART_PRINTF("no enough buff fill data %d,%d!!!\r\n",pbuff_write,pbuff_read); // for test show
		status = 0;
	}
		
	return status;
}


 /*******************************************************************************
 * Function: read_uart_ringbuf_data
 * Description: read data from loop buffer
 * Input: uint8_t*
 * Output: void
 * Return: uint8_t readed cnt
 * Others: void
*******************************************************************************/
uint8_t  read_uart_ringbuf_data(uint8_t *buf,uint16_t len)
{
	uint16_t unread_cnt;
	//Read 20 encode data from loop buffer to designated buffer 
	if(pbuff_write >= pbuff_read)
	{
		unread_cnt = pbuff_write - pbuff_read;
		
		//UART_PRINTF("read 0x%x\r\n",pbuff_read);	
	}else
	{
		unread_cnt = (TOTAL_BUF_NUM - pbuff_read  + pbuff_write);
		//UART_PRINTF("buff empty!!0x%x\r\n",pbuff_read);	
	} 
	//UART_PRINTF("unread_cnt : %d,read len :%d\r\n", unread_cnt,len); 
	
	
	if(unread_cnt >= len)
	{
        memcpy(buf,(uint8_t *)&uart_rx_buf[pbuff_read],len);

        if(0){
            UART_PRINTF("bufR %d:",uart_rx_index);
            for(int i =0;i < len;i++)
            {
            UART_PRINTF("%02x ",buf[i]);
            }
            UART_PRINTF("%\r\n");
        }
					//Update the buffer index of the data 
		//(in fact, the index is between 0-255)
        pbuff_read = ((pbuff_read + len )% TOTAL_BUF_NUM);
      //  UART_PRINTF("enough data read!! pbuff_write : %d,pbuff_read :%d\r\n",pbuff_write,pbuff_read);	
        return len;
	}else
	{
       // UART_PRINTF("buff no enough data read!!pbuff_write :%d,pbuff_read :%d\r\n",pbuff_write,pbuff_read);	
        return 0;
	}
	
}
#if 0
void app_uart_clean(void)
{
    memset((uint8_t *)uart_tx_buf,0,sizeof(uart_tx_buf));
		//memset((uint8_t *)uart_rx_buf,0,sizeof(uart_rx_buf));
		uart_rx_index = 0;///uart_rx_buff_header = 0;
		uart_rx_buff_tailor = 0;
}
#endif
 /*******************************************************************************
 * Function: uart_ringbuf_clean
 * Description: read data from loop buffer
 * Input: uint8_t*
 * Output: void
 * Return: uint8_t readed cnt
 * Others: void
*******************************************************************************/
void uart_ringbuf_clean(void)
{
    pbuff_write = pbuff_read = 0;
    uart_rx_index = 0;
	#if 0
	  memset((void *)uart_rx_buf,0,TOTAL_BUF_NUM);
	  memset((uint8_t *)uart_tx_buf,0,TOTAL_BUF_NUM);
	#endif
}



