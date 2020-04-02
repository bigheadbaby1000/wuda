
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include "rwble.h"
#include "reg_blecore.h"
#include "BK_reg_Protocol.h"
#include "ke_mem.h"
#include "ke_event.h"
#include "ke_timer.h"
#include "compiler.h"
#include "lld_util.h"
#include "llm_util.h"
#include "app.h"
#include "app_task.h"
//#include "appc.h"
//#include "appc_task.h"
//#include "master_app.h"
//#if CLI_CONSOLE
#include "cli.h"
//#endif
#include "wdt.h"
#include "nvds.h"
#include "uart.h"
#include "icu.h"
//#include "app_electric.h"
//#include "electric_task.h"
#include "app_api.h"
#include "app_uart.h"
//#include "reg_ble_em_tx_desc.h"
#include "rf.h"


ble_dev_info slave_device={
	0,   \
	0x11,0x22,0x33,0x44,0x55,0x66,  \
	0,  \
	0x79,0x41,0xDC,0x24,0x0E,0xE5,0xA9,0xE0,0x93,0xF3,0xA3,0xB5,0x01,0x00,0x40,0x6E,  \
	0x79,0x41,0xDC,0x24,0x0E,0xE5,0xA9,0xE0,0x93,0xF3,0xA3,0xB5,0x02,0x00,0x40,0x6E,  \
	0x79,0x41,0xDC,0x24,0x0E,0xE5,0xA9,0xE0,0x93,0xF3,0xA3,0xB5,0x03,0x00,0x40,0x6E,  \
};
//#define CLI_DEBUG1
uint8_t ble_dev_name[32] = "CTBLE-01";
//uint8 ble_adv_data[32] = "";
uint8_t scan_rsp_data[32] = "\x09\x08\x43\x54\x42\x4C\x45\x2D\x30\x31";///{0x0d,0x08,0x42,0x4B,0x33,0x34,0x33,0x35,0x2D,0x47,0x41,0x54,0x54,0x34};
//uint8_t dmo_channel = 0;

//uint16_t lesend_packet_len=0;
//uint16_t lesend_length_tailor=0;
//uint8_t lesend_packet[LESEND_MAX_LEN];
/* Built-in "help" command: prints all registered commands and their help
 * text string, if any.
 */

static void Delay_ms(int num) //sync from svn revision 18
{
	int x, y;
	for(y = 0; y < num; y ++ )
	{
		for(x = 0; x < 3260; x++);
	}

}

static uint8_t hex(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'z')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'Z')
        return c - 'A' + 10;
    return 0;
}

static void hexstr2bin(const char *macstr, uint8_t *mac, int len)
{
    int i;
    for (i=0;i < len && macstr[2 * i];i++) {
        mac[i] = hex(macstr[2 * i]) << 4;
        mac[i] |= hex(macstr[2 * i + 1]);
    }
}

static void ble_help_cmd(char *buf, int len, int argc, char **argv)
{
    int i, n;
    uint32_t build_in_count = sizeof(built_ins) / sizeof(struct cli_command);

    aos_cli_printf( "====Build-in Commands====\r\n" );
    for (i = 0, n = 0; i < MAX_COMMANDS && n < cli->num_commands; i++) {
        if (cli->commands[i]->name) {
            aos_cli_printf("%s: %s\r\n", cli->commands[i]->name,
                           cli->commands[i]->help ?
                           cli->commands[i]->help : "");
            n++;
            if ( n == build_in_count - USER_COMMANDS_NUM ) {
                aos_cli_printf("\r\n");
                aos_cli_printf("====User Commands====\r\n");
            }
        }
    }
}






#if 0

static void echo_cmd(char *buf, int len, int argc, char **argv)
{
    if (argc == 1) {
        aos_cli_printf("Usage: echo on/off. Echo is currently %s\r\n",
                       cli->echo_disabled ? "Disabled" : "Enabled");
        return;
    }

    if (!strcmp(argv[1], "on")) {
        aos_cli_printf("Enable echo\r\n");
        cli->echo_disabled = 0;
    } else if (!strcmp(argv[1], "off")) {
        aos_cli_printf("Disable echo\r\n");
        cli->echo_disabled = 1;
    }
}

void change_heard_end(uint8_t *src, uint8_t *dec, uint16_t len)
{
    for(int i=0; i < len; i++)
    {
        dec[i] = src[len-1-i];
    }
}

int8_t channel_search(uint8_t *mac)
{
	for(int i = 0;i < APPC_IDX_MAX;i++)
	{			
		if((ke_state_get(KE_BUILD_ID(TASK_APPC,i)) == APPC_LINK_CONNECTED) ||(ke_state_get(KE_BUILD_ID(TASK_APPC,i)) == APPC_SDP_DISCOVERING)\
				||(ke_state_get(KE_BUILD_ID(TASK_APPC,i)) == APPC_SERVICE_CONNECTED) )
		{
			if(memcmp(mac,appc_env[i]->con_dev_addr.addr.addr,GAP_BD_ADDR_LEN)==0)
			{
				//UART_PRINTF("i:%x\r\n",i);
				return i;
			}
		}
	}
	return -1;
}










#if CLI_CONSOLE






#if 0
void enter_DMO_mode(uint8_t channal)
{
    cliexit = 1;
	uart_ringbuf_clean();
	dmo_channel = channal;
    aos_cli_printf("\r\nOK\r\n");
}

void quit_DMO_mode(void)
{
    cliexit = 0;
    uart_ringbuf_clean();
    aos_cli_printf("\r\nOK\r\n");
}

static void dmo_cmd(char *buf, int len, int argc, char **argv)
{
#ifdef CLI_DEBUG	
	int i;
	aos_cli_printf("%s argc %d\r\n",__func__,argc);
	for(i=0;i<argc;i++)
	{
		aos_cli_printf("%s\r\n",argv[i]);
	}
#endif
    
  if (argc != 2)goto EXIT;
	
	cliexit = 1;
	uart_ringbuf_clean();
	dmo_channel = argv[1][0]-'0';
	aos_cli_printf("\r\nOK\r\n");
	
	return;
	
EXIT:
	aos_cli_printf("\r\nERROR\r\n");
}

static void qdmo_cmd(char *buf, int len, int argc, char **argv)
{
    aos_cli_printf("\r\nERROR\r\n");
}

#endif



uint8_t free_channel_search(void)
{
    uint8_t num = 0;
    
    for(int i = 0;i < APPC_IDX_MAX;i++)
    {
        
        if((ke_state_get(KE_BUILD_ID(TASK_APPC,i)) == APPC_LINK_CONNECTED) ||(ke_state_get(KE_BUILD_ID(TASK_APPC,i)) == APPC_SDP_DISCOVERING)\
            ||(ke_state_get(KE_BUILD_ID(TASK_APPC,i)) == APPC_SERVICE_CONNECTED) )
        {
            num++; 
        }
        else break;
    }
    return num;
}

uint8_t mac_search(uint8_t *bdaddr)
{
    uint8_t num = 0;
    
    for(int i = 0;i < APPC_IDX_MAX;i++)
    {
        
        if((ke_state_get(KE_BUILD_ID(TASK_APPC,i)) == APPC_LINK_CONNECTED) ||(ke_state_get(KE_BUILD_ID(TASK_APPC,i)) == APPC_SDP_DISCOVERING)\
            ||(ke_state_get(KE_BUILD_ID(TASK_APPC,i)) == APPC_SERVICE_CONNECTED) )
        {
            num += 0x10;
            if(memcmp((const void *)&(appc_env[i]->con_dev_addr.addr.addr[0]),(const void *)&(bdaddr[0]),6)==0)
            {
                num++; 
                break;
            } 
        }        
    }
    return num;
} 


static void connect_cmd(char *buf, int len, int argc, char **argv)
{
	uint8_t addr_type;

	struct gap_bdaddr bdaddr;
	
#ifdef CLI_DEBUG1	
	uint8_t i;
	aos_cli_printf("%s argc %d\r\n",__func__,argc);
	for(i=0;i<argc;i++)
	{
		aos_cli_printf("%s\r\n",argv[i]);
	}
#endif
    
	if ((argc != 5)&&(argc != 2))
    {
        //aos_cli_printf("Usage: parm1:mac+type,parm2:service uuid,parm3:write uuid,parm4:notify uuid\r\n");
		goto EXIT;
	}
    
	addr_type = argv[1][strlen(argv[1])-1] - '0';
    
	if(addr_type <= 1)slave_device.type = addr_type;
	else goto EXIT;
    
	if(strlen(argv[1]) == 13)
    {
        uint8_t mac[6];
        hexstr2bin(argv[1], mac, strlen(argv[1])-1);//mac
        for(int i = 0; i < 6; i++)
        {
            slave_device.mac[i] = mac[5-i];
        }
    }
    else goto EXIT;	
    if(argc == 5)
    {
        if((strlen(argv[2]) <= 32) && (strlen(argv[3]) <= 32) && (strlen(argv[4]) <= 32))
        {
            uint8_t tempbuf[35];
            hexstr2bin(argv[2], tempbuf, strlen(argv[2]));//service uuid
            change_heard_end(tempbuf, slave_device.serv_uuid, 16);
            hexstr2bin(argv[3], tempbuf,strlen(argv[3]));//write uuid
            change_heard_end(tempbuf, slave_device.write_uuid, 16);
            hexstr2bin(argv[4], tempbuf, strlen(argv[4]));//write uuid
            change_heard_end(tempbuf, slave_device.notify_uuid, 16);
        }
        else goto EXIT;	
    }
	
	if(mac_search(slave_device.mac) & 0x0f) goto EXIT;
	//add connect function	
	
#ifdef CLI_DEBUG	
	aos_cli_printf("%s argc %d\r\n",__func__,argc);	
	for(i=1;i<argc;i++)
	{
		aos_cli_printf("%s\r\n",argv[i]);
	}
	aos_cli_printf("change :\r\n");
	for(i=0;i<6;i++)
	{
		aos_cli_printf("%x ",slave_device.mac[i]);
	}aos_cli_printf("\r\n");
	for(i=0;i<16;i++)
	{
		aos_cli_printf("%x ",slave_device.serv_uuid[i]);
	}aos_cli_printf("\r\n");
	for(i=0;i<16;i++)
	{
		aos_cli_printf("%x ",slave_device.write_uuid[i]);
	}aos_cli_printf("\r\n");
	for(i=0;i<16;i++)
	{
		aos_cli_printf("%x ",slave_device.notify_uuid[i]);
	}aos_cli_printf("\r\n");
#endif

    //hexstr2bin(argv[1], bdaddr.addr.addr, GAP_BD_ADDR_LEN);
    memcpy(bdaddr.addr.addr, slave_device.mac, GAP_BD_ADDR_LEN);
    bdaddr.addr_type = addr_type;
    
    if(appm_start_connencting( bdaddr) != APPM_ERROR_NO_ERROR)
    {
        goto EXIT;
    }
    appm_env.recon_num = APPM_RECONNENCT_DEVICE_NUM;
    aos_cli_printf("+GATTSTAT=%d,2\r\n",free_channel_search());
    
    return;
    
EXIT:
    aos_cli_printf("\r\nERROR\r\n");	
}


static void connect(char *buf, int len, int argc, char **argv)
{    
    UART_PRINTF("%s\r\n",__func__);
    struct gap_bdaddr bdaddr;
    uint8_t mac[6];
    if (argc != 2)goto EXIT;

    hexstr2bin(argv[1], mac, GAP_BD_ADDR_LEN);
    for(int i = 0; i < 6; i++)
    {
        bdaddr.addr.addr[i] = mac[5-i];
    }
    if(strlen(argv[1]) == 13) slave_device.type = argv[1][12] - '0';
    if(mac_search(bdaddr.addr.addr) & 0x0f) goto EXIT;
    
    UART_PRINTF("con address: %02x-%02x-%02x-%02x-%02x-%02x\r\n",
            bdaddr.addr.addr[0], bdaddr.addr.addr[1], bdaddr.addr.addr[2], bdaddr.addr.addr[3], bdaddr.addr.addr[4], bdaddr.addr.addr[5]);
    bdaddr.addr_type = slave_device.type;///ADDR_PUBLIC;

    if(appm_start_connencting( bdaddr) != APPM_ERROR_NO_ERROR)goto EXIT;
    
    aos_cli_printf("+GATTSTAT=%d,2\r\n",free_channel_search());
 
	return;
EXIT:
    aos_cli_printf("\r\nERROR\r\n");   
}





static void check_info_cmd(char *buf, int len, int argc, char **argv)
{	
#ifdef CLI_DEBUG1	
	int i;
	aos_cli_printf("%s argc %d\r\n",__func__,argc);
	for(i=0;i<argc;i++)
	{
		aos_cli_printf("%s\r\n",argv[i]);
	}
#endif    
    appc_connect_device_info_get();	
}








#if 0
static void ble_write(char *buf, int len, int argc, char **argv)
{
    //aos_cli_printf("==== ble_write====\r\n");
    uint16_t handle;
    uint8_t conidx;
    
    uint8_t buf_len;
    uint8_t buf1[20];
    hexstr2bin(argv[2],&conidx,1);
    hexstr2bin(argv[3],(uint8_t *)&handle,2);
    hexstr2bin(argv[4],&buf_len,1);
    
    hexstr2bin(argv[5],buf1,buf_len);
    
    if (!strcmp(argv[1], "val")) 
    {
        appc_write_service_data_req(conidx,handle,buf_len,buf1);
    }
    
    if (!strcmp(argv[1], "ntf")) 
    {
        appc_write_service_ntf_cfg_req(conidx,handle,buf1[0]);
    }   
}

static void devname_cmd(char *buf, int len, int argc, char **argv)
{
    UART_PRINTF("device name: %s\r\n","BK3435");
}

static void reboot_cmd(char *buf, int len, int argc, char **argv)
{
    aos_cli_printf("reboot cmd\r\n");
	
	wdt_reset(2000);

}
//#endif
static void delete_cmd(char *buf, int len, int argc, char **argv)
{  
  // nvds_deinit();
   UART_PRINTF("delete cmp,please reboot system!!\r\n");
}

void sys_memory_Check(void);
static void mem_check_cmd(char *buf, int len, int argc, char **argv)
{  
   UART_PRINTF("mem_check\r\n");
  // sys_memory_Check();
   
}

static void bk_reg_cmd(char *buf, int len, int argc, char **argv)
{    
   TRAhcit_UART_Rx();
}
#include "appm.h"

static void scan(char *buf, int len, int argc, char **argv)
{    
    aos_cli_printf("%s\r\n",__func__);
    if (argc == 1)
    {       
            aos_cli_printf("Usage: Scan on/off. Scan is currently %s\r\n",
                       APPM_GET_FIELD(SCAN_EN) ? "On" : "Off");
    }else if(argc == 2)
    {
        if (!strcmp(argv[1], "on")) 
        {
            if(APPM_GET_FIELD(SCAN_EN))
            {
                aos_cli_printf("scan currently is on\r\n");
            }else
            {
                aos_cli_printf("scan set on\r\n");
                uint8_t  cur_state = ke_state_get(TASK_APPM);
                APPM_SET_FIELD(SCAN_EN,1);
                aos_cli_printf("scan set on cur_state:%d\r\n",cur_state);
                appm_scan_adv_con_schedule();
            }       
        } 
        else if (!strcmp(argv[1], "off"))
        {
            if(APPM_GET_FIELD(SCAN_EN) == 0)
            {
                aos_cli_printf("scan currently is off\r\n");
            }else
            {
                APPM_SET_FIELD(SCAN_EN,0);
                uint8_t  cur_state = ke_state_get(TASK_APPM);
                appm_scan_adv_con_schedule();
                aos_cli_printf("scan set off cur_state:%d\r\n",cur_state);
            }         
        }
    
    }
	return;
EXIT:
		aos_cli_printf("\r\nERROR\r\n");		
}


static void appmstate(char *buf, int len, int argc, char **argv)
{    
    UART_PRINTF("%s state:%d\r\n",__func__,ke_state_get(TASK_APPM));
    
    uint8_t state;
    if (argc == 1)
    {       
        UART_PRINTF("Usage: Appm  currently is %d\r\n",
           ke_state_get(TASK_APPM));
    }else
    {
        hexstr2bin(argv[1], &state, 1);
        ke_state_set(TASK_APPM,state);
        UART_PRINTF("Usage: Appm state set %d\r\n",state);
    }
    
}
#endif






void ble_scan_set(uint8_t type)//0:stop 1:PASSIVE 2:ACTIVE
{
    extern uint8_t gapm_scan_type;
    uint8_t  cur_state = ke_state_get(TASK_APPM);

    if(type)
    {    
        if(!APPM_GET_FIELD(SCAN_EN))//
        {
            if(type == 0x01)gapm_scan_type = GAPM_SCAN_PASSIVE;
            if(type == 0x02)gapm_scan_type = GAPM_SCAN_ACTIVE;
            //USER_PRINTF("gapm_scan_type:%x\r\n",gapm_scan_type);
            //aos_cli_printf("scan set on\r\n");            
            APPM_SET_FIELD(SCAN_EN,1);
            //aos_cli_printf("scan set on cur_state:%d\r\n",cur_state);
            appm_scan_adv_con_schedule();
        } 
    }
    else
    {
        if(APPM_GET_FIELD(SCAN_EN))
        {
            APPM_SET_FIELD(SCAN_EN,0);
            appm_scan_adv_con_schedule();
            //aos_cli_printf("scan set off cur_state:%d\r\n",cur_state);
        }  
    }
}

/*
ble device 1 mac, addr type(pub or random),device name
ble device 2 mac, addr type(pub or random),device name
......
ble device n mac, addr type(pub or random),device name
*/
static void ble_scan_cmd(char *buf, int len, int argc, char **argv)
{
	
#ifdef CLI_DEBUG1
	int i;
	aos_cli_printf("%s argc %d\r\n",__func__,argc);
	for(i=0;i<argc;i++)
	{
		aos_cli_printf("%s\r\n",argv[i]);
	}
#endif
	if(argc == 2)
	{
		ble_scan_set(argv[1][0] - 0x30);
		aos_cli_printf("+SET SCAN=%d\r\n", argv[1][0] - 0x30);
	}
	else
	{
		goto EXIT;
	}
		return;
	EXIT:
		aos_cli_printf("+ERROR\r\n");

	//aos_cli_printf("\r\n+SCAN{\r\n112233445566,0,bk_ble_D01\r\n223344556677,1,bk_ble_D02\r\n+SCAN}\r\n");

}




#endif

extern uint16_t USER_MTU_SIZE;
void ble_data_rx(uint8_t *data, uint8_t len, uint8_t channel)
{
    uint8_t timerout = 20;
//    uint8_t *send_buf = (uint8_t *)data;
//    uint16_t send_len = len; 

    if(current_len[channel] == 0)
    {
        snprintf((char *)ble_rx_data_buffer[channel],15,"\r\n+DATA=%d,%03d,",channel,current_len[channel]);
        current_len[channel] = 14;
    }
    if(len < USER_MTU_SIZE)timerout = 1;
    else timerout = 15;
    switch(channel)
    {
        case 0x00:
            ke_timer_set(RX_DATA_TIMEOUT_RSP(0), TASK_APPM, timerout);
            break;
        case 0x01:
            ke_timer_set(RX_DATA_TIMEOUT_RSP(1), TASK_APPM, timerout);
            break;
        case 0x02:
            ke_timer_set(RX_DATA_TIMEOUT_RSP(2), TASK_APPM, timerout);
            break;
        case 0x03:
            ke_timer_set(RX_DATA_TIMEOUT_RSP(3), TASK_APPM, timerout);
            break;
        case 0x04:
            ke_timer_set(RX_DATA_TIMEOUT_RSP(4), TASK_APPM, timerout);
            break;
    } 

    if((current_len[channel] + len) < BLE_RX_DATA_MAX)
    {
        memcpy(ble_rx_data_buffer[channel]+current_len[channel], data, len);
        current_len[channel] += len;
    }
}

//int8_t test_num = 1;
uint8_t app_send_ble_data(uint8_t device_id,uint16_t packet_len,uint8_t* packet)
{
	if((device_id >= BLE_CONNECTION_MAX) || (appc_env[device_id] == NULL)) goto EXIT;

	if(appc_env[device_id]->role == ROLE_SLAVE)
    {
        if(app_electric_tx_send(device_id,packet_len,packet) != APPM_ERROR_NO_ERROR)goto EXIT;
    }
    else 
    {        
        if(appc_write_service_data_req(device_id,appc_env[device_id]->svc_write_handle,packet_len,packet)!= APPM_ERROR_NO_ERROR)
            goto EXIT;

            //test_num++;

        //UART_PRINTF("\r\ntest:%d\r\n",test_num);
    }
    
	return APPM_ERROR_NO_ERROR;
    
EXIT:	
	return APPM_ERROR_STATE;
}

#endif

void soft_reset(void)
{
    wdt_reset(1000);
    //while(1);//0612
}

static void ble_reboot_cmd(char *buf, int len, int argc, char **argv)
{
#ifdef CLI_DEBUG1
  aos_cli_printf("reboot cmd\r\n");
#endif
	
	aos_cli_printf("+REBOOT\r\n");
		soft_reset();

}

static void ble_adv_interval_read(void)
{
	uint16_t interval = 0;
	uint8_t lenth = sizeof(interval);
	if(nvds_get(NVDS_TAG_BLE_ADV_INTERVAL, &lenth, (uint8_t *)&interval) != NVDS_OK)
	{
		interval = APP_ADV_INT_MIN;
	}
	aos_cli_printf("+ADV INTV=%d\r\n", interval);
}

static uint8_t ble_adv_interval_set(const uint16_t interval)
{
	if(interval<32 || interval>8000)	return APPM_ERROR_STATE;
	if(nvds_put(NVDS_TAG_BLE_ADV_INTERVAL,sizeof(interval),(uint8_t *)&interval) != NVDS_OK)
	{
		return APPM_ERROR_STATE;
	}
 	appm_stop_advertising();
	return APPM_ERROR_NO_ERROR;
}

static void ble_adv_interval_cmd(char *buf, int len, int argc, char **argv)
{
#ifdef CLI_DEBUG1	
	int i;
	aos_cli_printf("%s argc %d\r\n",__func__,argc);
	for(i=0;i<argc;i++)
	{
		aos_cli_printf("%s\r\n",argv[i]);
	}
#endif
	if(argc == 1)
	{
		ble_adv_interval_read();
	}
	else if(argc == 2)
	{
		uint16_t interval=0;
		interval = atoi(argv[1]);
		if(ble_adv_interval_set(interval) != APPM_ERROR_NO_ERROR)goto EXIT;
		aos_cli_printf("+SET ADV INTV=%d\r\n", interval);
	}
	else
	{
		goto EXIT;
	}
		
	return;
EXIT:
	aos_cli_printf("+ERROR\r\n");
}


static void ble_conn_interval_read(void)
{
	uint16_t interval=BLE_UAPDATA_DFT_INTVALUE;
	uint8_t lenth=sizeof(interval);

	for(int i = 0;i < APPC_IDX_MAX;i++)
	{			
//		if((ke_state_get(KE_BUILD_ID(TASK_APPC,i)) == APPC_LINK_CONNECTED) ||(ke_state_get(KE_BUILD_ID(TASK_APPC,i)) == APPC_SDP_DISCOVERING)\
//				||(ke_state_get(KE_BUILD_ID(TASK_APPC,i)) == APPC_SERVICE_CONNECTED) )
		if(ke_state_get(KE_BUILD_ID(TASK_APP, 0)) == APPM_CONNECTED  )
		{

			aos_cli_printf("+CONN INTV=%d\r\n", app_env.conn_intv);
			return;
		}
	}
	if(nvds_get(NVDS_TAG_BLE_CONN_INTERVAL, &lenth, (uint8_t *)&interval) == NVDS_OK)
	{
	}

		aos_cli_printf("+CONN INTV=%d\r\n", interval);


}

static uint8_t ble_conn_interval_set(const uint16_t intv)
{

	if(intv<6 || intv>1600)
			return	APPM_ERROR_STATE;
//	for(int i = 0;i < APPC_IDX_MAX;i++)
	{			
//		if((ke_state_get(KE_BUILD_ID(TASK_APPC,i)) == APPC_LINK_CONNECTED) ||(ke_state_get(KE_BUILD_ID(TASK_APPC,i)) == APPC_SDP_DISCOVERING)\
//				||(ke_state_get(KE_BUILD_ID(TASK_APPC,i)) == APPC_SERVICE_CONNECTED) )
		if(ke_state_get(KE_BUILD_ID(TASK_APP, 0)) == APPM_CONNECTED  )
		{

//            appc_update_param(i,intv,intv,0,1000);
			struct gapc_conn_param param;
			param.intv_min = intv;
			param.intv_max = intv;
			param.latency = 0;
			param.time_out = 1000;
			appm_update_param(&param);
//			break;
		}
	}
	if(nvds_put(NVDS_TAG_BLE_CONN_INTERVAL, sizeof(intv), (uint8_t *)&intv) != NVDS_OK)
	{
		return APPM_ERROR_STATE;

	}
//	appm_env.conn_intv = intv;
	return APPM_ERROR_NO_ERROR;
}

static void ble_conn_interval_cmd(char *buf, int len, int argc, char **argv)
{
#ifdef CLI_DEBUG1
		int i;
		aos_cli_printf("%s argc %d\r\n",__func__,argc);
		for(i=0;i<argc;i++)
		{
			aos_cli_printf("%s\r\n",argv[i]);
		}	
#endif

    if (argc == 1)
    {    
        ble_conn_interval_read();
    }
    else if(argc == 2)
    {
		uint16_t conn_intv = atoi(argv[1]);
        if(ble_conn_interval_set(conn_intv) != APPM_ERROR_NO_ERROR) goto EXIT;
		aos_cli_printf("+SET CONN INTV=%d\r\n", conn_intv);
    }
    else
    {
        goto EXIT;
    }
	return;
EXIT:
    aos_cli_printf("+ERROR\r\n");		
}

#if 1

void ble_adv_state_set(uint8_t state)
{
    uint8_t  cur_state = ke_state_get(TASK_APP);

    if(APPM_GET_FIELD(ADV_EN) && !state)
    {
        UART_PRINTF("adv currently is on\r\n");
        APPM_SET_FIELD(ADV_EN,0);
        appm_scan_adv_con_schedule();
        UART_PRINTF("adv set off cur_state:%d\r\n",cur_state);
    }    
    else if(!APPM_GET_FIELD(ADV_EN) && state)
    {        
        UART_PRINTF("adv set on cur_state:%d\r\n",cur_state);
        APPM_SET_FIELD(ADV_EN,1);
        appm_scan_adv_con_schedule();
    }
	else
        UART_PRINTF("no change is set\r\n");
}

static void ble_adv_cmd(char *buf, int len, int argc, char **argv)
{
    uint8_t state;
//    UART_PRINTF("%s\r\n",__func__);
    if (argc == 1)
    {       
        aos_cli_printf("+ADV=%s\r\n",
            APPM_GET_FIELD(ADV_EN) ? "1" : "0");
    }
    else if(argc == 2)
    {
        if(!strcmp(argv[1], "0"))
        {
           state = 0;     
        }
        else if(!strcmp(argv[1], "1"))
        {
            state = 1;
        }
		#if 1
		else if(!strcmp(argv[1], "2"))
        {
        	uint8_t str_len=32;
        	str_len = ble_txadvlen_getf(LLM_LE_ADV_IDX)-6;
        	char str[32];
//			struct ble_adv_data *ble_advdata = (struct ble_adv_data *) ke_malloc(sizeof(struct ble_adv_data), KE_MEM_NON_RETENTION);
			em_rd((void *)str, ble_txdataptr_get(LLM_LE_ADV_IDX), str_len);
			uart_printf_str("+ADVDATA=", (uint8_t*)str, str_len, KT_PRINTF);
//			ke_free(ble_advdata);
        }
		#endif
		else
			goto EXIT;
		if(state==1||!state)
		{
      	  ble_adv_state_set(state);
		  aos_cli_printf("+SET ADV=%d\r\n", state);
    	}
		
    }
	else if(argc == 3 && !strcmp(argv[1], "2"))
	{
		uint8_t adv_udata[32] = {0};
		uint8_t adv_ulen = 0;
		hexstr2bin(argv[2], adv_udata, strlen(argv[2]));
		adv_ulen = strlen(argv[2]) /2 ;
		if(strlen(argv[2])%2 == 1)
		{
//			UART_PRINTF("need input even data\r\n");
			goto EXIT;
		}
#if 0
		UART_PRINTF("set user adv data = ");
		for(uint8_t i=0;i<adv_ulen; i++)
		{
			UART_PRINTF("%x ", adv_udata[i]);
		}
		UART_PRINTF("\r\n");
#endif
		
		appm_update_adv_data(adv_udata, adv_ulen, adv_udata, adv_ulen);

//				appm_stop_advertising();
//				appm_user_start_advertising(adv_udata , adv_ulen);
		
		aos_cli_printf("+SET ADVDATA=");
		for(uint8_t i=0; i<adv_ulen; i++)
			aos_cli_printf("%02X", adv_udata[i]);
		aos_cli_printf("\r\n");
	}
    else 
		goto EXIT;
//    aos_cli_printf("\r\nOK\r\n");
	return;
EXIT:
    aos_cli_printf("+ERROR\r\n");
}


#endif
static void ble_name_read(void)
{
//    uint8_t length=NVDS_LEN_DEVICE_NAME;
	uint8_t str_buf[ADV_MAX_DATA_NUM] = {0};
//uint8_t ble_advdata[ADV_MAX_DATA_NUM];
    struct ble_adv_data *ble_advdata = (struct ble_adv_data *) ke_malloc(sizeof(struct ble_adv_data), KE_MEM_NON_RETENTION);
	#if 0
    if(nvds_get(NVDS_TAG_DEVICE_NAME,&length,(uint8_t *)ble_device_name)==NVDS_OK)
	{
//		snprintf((char *)ble_device_name, 11,"BK3435_BLE");
		aos_cli_printf("+NAME=%s\r\n", (char *)ble_device_name);
	}
	else
		aos_cli_printf("+NAME=%s\r\n", (char *)ble_dev_name);
	#else
	
	em_rd((void *)&ble_advdata->adv_data[0], ble_txdataptr_get(LLM_LE_ADV_IDX), ADV_MAX_DATA_NUM);
		
	#if 0
	aos_cli_printf("+ADVDATA=");
	for(uint8_t i=0; i<ADV_MAX_DATA_NUM; i++)
		aos_cli_printf("%02X", ble_advdata->adv_data[i]);
	aos_cli_printf("\r\n");
	#endif
	
	if(appm_adv_data_decode_name(ADV_MAX_DATA_NUM, (const uint8_t *)ble_advdata, str_buf))
	{
		aos_cli_printf("+NAME=%s\r\n",str_buf);
		ke_free(ble_advdata);
//		memset(str_buf,0,31);
	} 

	#endif
}

static uint8_t ble_name_set(uint8_t *blename, uint8_t ble_len)
{
    uint8_t len = co_min(strlen((char *)blename), ble_len);
    
    if(len > APP_DEVICE_NAME_MAX_LEN)return APPM_ERROR_STATE;
	memset(ble_dev_name,0,sizeof(ble_dev_name));
	memset(scan_rsp_data,0,sizeof(scan_rsp_data));
	
	memcpy(ble_dev_name, blename, len);
//	uart_printf_str("set adv=", ble_dev_name, len, UART_PRINTF);
    if(nvds_put(NVDS_TAG_DEVICE_NAME,len,ble_dev_name) != NVDS_OK)return APPM_ERROR_STATE;

	scan_rsp_data[0] = len + 1;
	scan_rsp_data[1] = 0x08;

	memcpy(scan_rsp_data+2, ble_dev_name, strlen((char *)ble_dev_name));
//	uart_printf_str("set scan resp=", scan_rsp_data, len+2, UART_PRINTF);
	if(nvds_put(NVDS_TAG_APP_BLE_SCAN_RESP_DATA, len+2, scan_rsp_data) != NVDS_OK)return APPM_ERROR_STATE;
	
	appm_stop_advertising();
    return APPM_ERROR_NO_ERROR;
}

static void ble_name_cmd(char *buf, int len, int argc, char **argv)
{
	
#ifdef CLI_DEBUG1
	int i;
	aos_cli_printf("%s argc %d\r\n",__func__,argc);
	for(i=0;i<argc;i++)
	{
		aos_cli_printf("%s\r\n",argv[i]);
	}	
#endif
    if (argc == 1)
    {    
		ble_name_read();
    }
	else if(argc == 2)
    {
        if(ble_name_set((uint8_t *)argv[1] , strlen(argv[1])) != APPM_ERROR_NO_ERROR)goto EXIT;
        aos_cli_printf("+SET NAME=%s\r\n",argv[1]);
    }
	else
	{
		goto EXIT;
	}
	return;
    
EXIT:
    aos_cli_printf("+ERROR\r\n");
}

static void ble_baud_read(void)
{
	uint32_t baudrate=0;
	uint8_t length=sizeof(baudrate);
	if(nvds_get(NVDS_TAG_UART_BAUDRATE, &length, (uint8_t *)&baudrate) == NVDS_OK)
	{
		aos_cli_printf("+BAUD=%d\r\n", baudrate);
	}
	else
	{
//		baudrate = 115200;
		aos_cli_printf("+BAUD=115200\r\n");
	}
}

static uint8_t ble_baud_set(uint32_t baudrate)
{
	if(uart_init(baudrate))
		return 	APPM_ERROR_STATE;
	if(nvds_put(NVDS_TAG_UART_BAUDRATE,sizeof(baudrate),(uint8_t *)&baudrate) != NVDS_OK)
		return 	APPM_ERROR_STATE;
	return APPM_ERROR_NO_ERROR;
}

static void ble_baud_cmd(char *buf, int len, int argc, char **argv)
{
#ifdef CLI_DEBUG1	
	int i;
	aos_cli_printf("%s argc %d\r\n",__func__,argc);
	for(i=0;i<argc;i++)
	{
		aos_cli_printf("%s\r\n",argv[i]);
	}
#endif	
	if(argc == 1)
	{
		ble_baud_read();
	}
	else if(argc == 2)
	{
		uint32_t baudrate = atoi(argv[1]); 
		aos_cli_printf("+SET BAUD=%d\r\n",baudrate);
		Delay_ms(20);
		if(ble_baud_set(baudrate) != APPM_ERROR_NO_ERROR)			goto EXIT;
	}
	else
	{
		goto EXIT;
	}
	
	return;
EXIT:
	aos_cli_printf("+ERROR\r\n");		
}

void ble_mac_read(void)
{
    uint32_t bdl,bdu;
    uint8_t *p_mac_l = (uint8_t *)&bdl;
    uint8_t *p_mac_u = (uint8_t *)&bdu;
    bdl = ble_bdaddrl_getf();
    bdu = ble_bdaddru_getf();
    aos_cli_printf("+MAC=%02X%02X%02X%02X%02X%02X\r\n",
    p_mac_u[1], p_mac_u[0], p_mac_l[3], p_mac_l[2], p_mac_l[1], p_mac_l[0]);
}

static uint8_t ble_mac_set(struct gap_bdaddr mac)
{
    struct bd_addr b_addr;
    
    for(int i=0;i<6;i++)
    {
        b_addr.addr[i] = mac.addr.addr[5-i];
    }
    if(nvds_put(NVDS_TAG_BD_ADDRESS,6,b_addr.addr) != NVDS_OK)///(uint8_t *)&mac[0]
    {            
        return APPM_ERROR_STATE;
    }
    appm_stop_advertising();
	#if 0
    uint8_t flag = 0;
    if(APPM_GET_FIELD(ADV_EN))//if adv on set adv off
    {
        uint8_t  cur_state = ke_state_get(TASK_APPM);
        APPM_SET_FIELD(ADV_EN,0);
        appm_scan_adv_con_schedule();
//        UART_PRINTF("adv set off cur_state:%d\r\n",cur_state);
        flag = 1;
    } 
	#endif
    //memcpy(b_addr.addr,mac,6);
//    if(mac.addr_type == ADDR_PUBLIC)
    {
//        co_default_bdaddr_type = GAPM_CFG_ADDR_PUBLIC;
        llm_util_set_public_addr(&b_addr);
        lld_util_set_bd_address(&b_addr,ADDR_PUBLIC);
    }
#if 0
    else
    {
        co_default_bdaddr_type = GAPM_CFG_ADDR_PRIVATE;
        llm_util_set_rand_addr(&b_addr);
        lld_util_set_bd_address(&b_addr,ADDR_RAND);
    }
#endif
    aos_cli_printf("+SET MAC=%02X%02X%02X%02X%02X%02X\r\n",
            mac.addr.addr[0], mac.addr.addr[1], mac.addr.addr[2], mac.addr.addr[3], mac.addr.addr[4], mac.addr.addr[5]);
          //mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]);
    #if 0
    if((!APPM_GET_FIELD(ADV_EN)) && flag)//adv on
    {
        uint8_t  cur_state = ke_state_get(TASK_APPM);
//        aos_cli_printf("adv set on cur_state:%d\r\n",cur_state);
//        APPM_SET_FIELD(ADV_EN,1);
//        appm_scan_adv_con_schedule();
    }
	#endif
    return APPM_ERROR_NO_ERROR;    
}


static void ble_mac_cmd(char *buf, int len, int argc, char **argv)
{
    struct gap_bdaddr bdaddr;
//    uint8_t mac[6];
//    uint8_t type;
#ifdef CLI_DEBUG1
		int i;
		aos_cli_printf("%s argc %d\r\n",__func__,argc);
		for(i=0;i<argc;i++)
		{
			aos_cli_printf("%s\r\n",argv[i]);
		}	
#endif

    if (argc == 1)
    {    
        ble_mac_read();
    }
    else if(argc == 2)
    { 
        hexstr2bin(argv[1], bdaddr.addr.addr, 6);
//        if((strlen(argv[1]) == 13) && ((0x30 <= argv[1][12])&&(argv[1][12] <= 0x31)))	//add type==0/1
		if(strlen(argv[1]) == 12)	//add type==0/1
        {
//            bdaddr.addr_type = argv[1][12] - '0';
        }
        else
			goto EXIT;
        if(ble_mac_set(bdaddr) != APPM_ERROR_NO_ERROR) goto EXIT;
    }
    else
    {
        goto EXIT;
    }
	return;
EXIT:
    aos_cli_printf("+ERROR\r\n");
}

static void ble_pwr_read(void)
{
	uint8_t length = 1;
	uint8_t power = 0;
	if(nvds_get(NVDS_TAG_POWER, &length, &power) == NVDS_OK)
	{
		aos_cli_printf("+POWER=%d\r\n", power);
	}
	else
	{
		ASSERT_ERR(0);
		aos_cli_printf("+ERROR\r\n");
	}
}

static uint8_t ble_pwr_set(uint8_t pwr)
{
	uint8_t power = pwr;
	if(nvds_put(NVDS_TAG_POWER, sizeof(power),(uint8_t *)&power) != NVDS_OK)
		return 	APPM_ERROR_STATE;
	set_ble_power(power);
	return APPM_ERROR_NO_ERROR;
}

static void ble_pwr_cmd(char *buf, int len, int argc, char **argv)
{
#ifdef CLI_DEBUG1	
	int i;
	aos_cli_printf("%s argc %d\r\n",__func__,argc);
	for(i=0;i<argc;i++)
	{
		aos_cli_printf("%s\r\n",argv[i]);
	}
#endif	
	if(argc == 1)
	{
		ble_pwr_read();
	}
	else if(argc == 2)
	{
		uint8_t pwr = atoi(argv[1]);
		if(pwr>0x0f)goto EXIT;
		aos_cli_printf("+SET PWR=%d\r\n", pwr);
		if(ble_pwr_set(pwr) != APPM_ERROR_NO_ERROR)			goto EXIT;
	}
	else
	{
		goto EXIT;
	}
	
	return;
EXIT:
	aos_cli_printf("+ERROR\r\n");
}

static void ble_enc_num_read(void)
{
	uint8_t length = 1;
	if(nvds_get(NVDS_TAG_ENC_NUM_MAX, &length, (uint8_t *)&app_env.enc_num_max) == NVDS_OK)
	{
		aos_cli_printf("+ENCNUMMAX=%d\r\n", app_env.enc_num_max);
	}
	else
	{
		ASSERT_ERR(0);
		aos_cli_printf("+ERROR\r\n");
	}

	if(nvds_get(NVDS_TAG_ENC_NUM_CUR, &length, (uint8_t *)&app_env.enc_num_cur) == NVDS_OK)
	{
		aos_cli_printf("+ENCNUMCUR=%d\r\n", app_env.enc_num_cur);
	}
}

static uint8_t ble_enc_num_set(uint8_t encnum)
{
	if(encnum < app_env.enc_num_max)
	{
		for(uint8_t i=0; i<app_env.enc_num_max - encnum; i++)
		{
			if(nvds_del(NVDS_TAG_LTK+app_env.enc_num_max-1-i) != NVDS_OK)
			{
//				ASSERT_ERR(0);
//				return 	APPM_ERROR_STATE;
			}
			else
			{
				UART_PRINTF("del tag = %d\r\n", NVDS_TAG_LTK+app_env.enc_num_max-1-i);
			}
		}
	}
	if(encnum < app_env.enc_num_cur)
	{
		app_env.enc_num_cur = encnum;
	}
	app_env.enc_num_max = encnum;
	if(nvds_put(NVDS_TAG_ENC_NUM_MAX, sizeof(encnum),(uint8_t *)&encnum) != NVDS_OK)
		return 	APPM_ERROR_STATE;
	return APPM_ERROR_NO_ERROR;
}

static void ble_enc_num_cmd(char *buf, int len, int argc, char **argv)
{
#ifdef CLI_DEBUG1	
	int i;
	aos_cli_printf("%s argc %d\r\n",__func__,argc);
	for(i=0;i<argc;i++)
	{
		aos_cli_printf("%s\r\n",argv[i]);
	}
#endif	
	if(argc == 1)
	{
		ble_enc_num_read();
	}
	else if(argc == 2)
	{
		uint8_t encnum = atoi(argv[1]);
		if(encnum>10)goto EXIT;
		aos_cli_printf("+SET ENCNUMMAX=%d\r\n", encnum);
		if(ble_enc_num_set(encnum) != APPM_ERROR_NO_ERROR)			goto EXIT;
	}
	else
	{
		goto EXIT;
	}
	
	return;
EXIT:
	aos_cli_printf("+ERROR\r\n");
}

static void ble_enc_pincode_cmd(char *buf, int len, int argc, char **argv)
{
#ifdef CLI_DEBUG1	
	int i;
	aos_cli_printf("%s argc %d\r\n",__func__,argc);
	for(i=0;i<argc;i++)
	{
		aos_cli_printf("%s\r\n",argv[i]);
	}
#endif	
	if(argc == 1)
	{
		aos_cli_printf("+ENCPINCODE=%d\r\n", app_env.pin_code);
	}
	else if(argc == 2)
	{
		app_env.pin_code = atoi(argv[1]);
		aos_cli_printf("+SET ENCPINCODE=%d\r\n", app_env.pin_code);
	}
	else
	{
		goto EXIT;
	}
	
	return;
EXIT:
	aos_cli_printf("+ERROR\r\n");		
}

static void ble_enc_enable_cmd(char *buf, int len, int argc, char **argv)
{
#ifdef CLI_DEBUG1
	int i;
	aos_cli_printf("%s argc %d\r\n",__func__,argc);
	for(i=0;i<argc;i++)
	{
		aos_cli_printf("%s\r\n",argv[i]);
	}
#endif	
	if(argc == 1)	//read
	{
		aos_cli_printf("+ENCEN=%d\r\n", app_env.enc_en);
	}
	else if(argc == 2)	//set
	{
		app_env.enc_en = atoi(argv[1]);
		aos_cli_printf("+SET ENCEN=%d\r\n", app_env.enc_en);
	}
	else
	{
		goto EXIT;
	}
	
	return;
EXIT:
	aos_cli_printf("+ERROR\r\n");		
}

static uint8_t ble_version_set(const uint32_t version)
{    
#if 0
	uint32_t fwverion = 0x20191013;
//	uint8_t length=sizeof(fwverion);
	if(nvds_put(NVDS_TAG_APP_FW_VERION, sizeof(fwverion), (uint8_t *)&fwverion) != NVDS_OK)
	{
		goto EXIT;
	}
	EXIT:
		UART_PRINTF("fwversion error = %x\r\n", state);
#endif
	return APPM_ERROR_STATE;

}

static void ble_version_read(void)
{
    uint8_t fw_version[4];
    uint8_t hw_version[4]; 
    rwble_version(fw_version,hw_version);
	#if 0
    aos_cli_printf("+HW VERSION=v%d.%d.%d   build :%s\r\n",hw_version[3],hw_version[2],hw_version[1],(hw_version[0] == 0) ? "BLE v4.2":"BLE v4.2 + 2Mbps LE");
    aos_cli_printf("+FW VERSION=v%d.%d.%d.%d build :%d%02d%02d   %s\r\n",fw_version[3],fw_version[2],fw_version[1],fw_version[0],YEAR, MONTH + 1, DAY,__TIME__);
	#else
		aos_cli_printf("+HW VERSION=%d.%d.%d", hw_version[3],hw_version[2],hw_version[1]);
		aos_cli_printf("+FW VERSION=%d.%d.%d\r\n", fw_version[3],fw_version[2],fw_version[1]);
	#endif
}


static void ble_version_cmd(char *buf, int len, int argc, char **argv)
{
#ifdef CLI_DEBUG1	
	int i;
	aos_cli_printf("%s argc %d\r\n",__func__,argc);
	for(i=0;i<argc;i++)
	{
		aos_cli_printf("%s\r\n",argv[i]);
	}
#endif
	if(argc == 1)
	{
		ble_version_read();
	}
	else if(argc == 2)
	{
		uint32_t version=0;
		version = atoi(argv[1]);
		if(ble_version_set(version) != APPM_ERROR_NO_ERROR)goto EXIT;
		aos_cli_printf("+SET VERSION=%d \r\n", version);
	}
	else
	{
		goto EXIT;
	}
		
	return;
EXIT:
	aos_cli_printf("+ERROR\r\n");
}

static void ble_disconn_cmd(char *buf, int len, int argc, char **argv)
{
#ifdef CLI_DEBUG1	
	int i;
	aos_cli_printf("%s argc %d\r\n",__func__,argc);
	for(i=0;i<argc;i++)
	{
		aos_cli_printf("%s\r\n",argv[i]);
	}
#endif
	if((argc != 2)||((strlen(argv[1]) != 1)&&(strlen(argv[1]) != 12)))
	{
		//UART_PRINTF("argv[1]len:%x\r\n",strlen(argv[1]));
		goto EXIT;
	}
#if 0
	int8_t device_id;
	uint8_t mac[6];
	if(strlen(argv[1]) == 1)
	{
		device_id  = atoi(argv[1]);///-'0';
	}
	else
	{
        uint8_t mac_temp[6];
		hexstr2bin(argv[1],mac_temp,strlen(argv[1]));
        for(int i = 0; i < 6; i++)
        {
            mac[i] = mac_temp[5-i];
        }
		device_id = channel_search(mac);
		if(device_id == -1)goto EXIT;
	}
	#endif
	//add disconnect ble device function
	#if 0
	if(appm_disconnect(device_id) != APPM_ERROR_NO_ERROR)
    {
        goto EXIT;
    }
	#else
		appm_disconnect();
	aos_cli_printf("+DISCONN\r\n");
	#endif
	
	return;
EXIT:
	aos_cli_printf("+ERROR\r\n");	
}

static void ble_data_send_cmd(char *buf, int len, int argc, char **argv)
{
//	uint8_t device_id;
	
#ifdef CLI_DEBUG1	
	int i;
	aos_cli_printf("%s argc %d\r\n",__func__,argc);
	for(i=0;i<argc;i++)
	{
		aos_cli_printf("%s\r\n",argv[i]);
	}
#endif
	if((argc != 4) || (strlen(argv[1]) != 1) || (lesend_packet_len))
	{
		UART_PRINTF("%d %d %d\r\n",argc,strlen(argv[1]),lesend_packet_len);
		goto EXIT;
	}
    
//	dmo_channel = atoi(argv[1]);//device_id = atoi(argv[1]);// argv[1][0];///-'0';
//    if(BLE_WORK_STA[dmo_channel] != BLE_CONN){UART_PRINTF("BLE_WORK_STA[%d] = %s\r\n",dmo_channel,BLE_WORK_STA[dmo_channel]?"BLE_DISCONN":"BLE_CONN");goto EXIT;}
	lesend_packet_len = atoi(argv[2]);
//	aos_cli_printf("LEN= %d\r\n", lesend_packet_len);
    memcpy(lesend_packet, argv[3], lesend_packet_len);
    lesend_length_tailor = 0;
    send_data_enable_flag = 1;

//    if(app_send_ble_data(dmo_channel,lesend_packet_len,lesend_packet) != APPM_ERROR_NO_ERROR)goto EXIT;
//	lesend_packet_len = 0;

//	aos_cli_printf("+SENDDATA\r\n");
	return;
    
EXIT:
	aos_cli_printf("\r\nERROR\r\n");
}

#if 0
static void ble_ex_mtu_cmd(char *buf, int len, int argc, char **argv)
{
#ifdef CLI_DEBUG1	
	int i;
	aos_cli_printf("%s argc %d\r\n",__func__,argc);
	for(i=0;i<argc;i++)
	{
		aos_cli_printf("%s\r\n",argv[i]);
	}
#endif
	if(argc == 1)
	{
		appc_gatt_mtu_change(0);
	}
	else
	{
		goto EXIT;
	}
		
	return;
EXIT:
	aos_cli_printf("+ERROR\r\n");
}
#endif

const struct cli_command built_ins[BUILD_INS_NUM] = {
    {"AT+HELP","     ble_help_cmd",              ble_help_cmd},
//    {"AT+ECHO","     echo on/off",       echo_cmd},
	{"AT+BDBAUD","	 ble_baud_cmd", 		ble_baud_cmd},
	{"AT+NAME","	 ble_name_cmd", 			ble_name_cmd},
	{"AT+MAC","   ble_mac_cmd", 			ble_mac_cmd},
	{"AT+ADV","   ble_adv_cmd", 		ble_adv_cmd},
	{"AT+CONNINT","    ble_conn_interval_cmd",		ble_conn_interval_cmd},
	{"AT+ADVINT","	  ble_adv_interval_cmd",		ble_adv_interval_cmd},
	{"AT+VERSION","   ble_version_cmd", 	ble_version_cmd},
//	{"AT+EXMTU","   ble_ex_mtu_cmd", 	ble_ex_mtu_cmd},
	{"AT+LESEND","	 ble_data_send_cmd",		ble_data_send_cmd},
	{"AT+REBOOT","	 ble_reboot_cmd",			ble_reboot_cmd},
	{"AT+LEDISC","ble_disconn_cmd", 			ble_disconn_cmd},
	{"AT+ENCNUM","ble_enc_num_cmd", 			ble_enc_num_cmd},
	{"AT+ENCPINCODE","ble_enc_pincode_cmd", 			ble_enc_pincode_cmd},
	{"AT+ENCEN","ble_enc_enable_cmd", 			ble_enc_enable_cmd},
	{"AT+PWR","ble_pwr_cmd",			ble_pwr_cmd},
	
#if 0    
    {"exit","        CLI exit",          exit_cmd},
    /* others */
    {"devname","     print device name", devname_cmd},
    {"delete","      delete nvds info",  delete_cmd},   
    {"loglevel","    set log level",     log_cmd},
    {"memcheck","    show mem use",      mem_check_cmd},
    {"\x01","        bk reg w/r",        bk_reg_cmd},    
    {"appmstate","   appm state",        appmstate},
    {"ble_write","   ble_write",         ble_write},		
	
    {"AT+DMO","      Direct Mode",       dmo_cmd},
    {"AT+QDMO","     quit DMO",          qdmo_cmd},
    {"AT+CONN","     connect device",   	 connect},
    {"AT+LECCONN","  connect_cmd",			connect_cmd},
    {"AT+CHINFO","   check_info_cmd", 		check_info_cmd},
    {"AT+SCAN","     ble_scan_cmd",       	ble_scan_cmd},

	
#endif    


};

