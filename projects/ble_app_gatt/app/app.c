/**
 ****************************************************************************************
 *
 * @file app.c
 *
 * @brief Application entry point
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup APP
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"             // SW configuration

#if (BLE_APP_PRESENT)
#include <string.h>
//#include "rwapp_config.h"
#include "app_task.h"                // Application task Definition
#include "app.h"                     // Application Definition
#include "gap.h"                     // GAP Definition
#include "gapm_task.h"               // GAP Manager Task API
#include "gapc_task.h"               // GAP Controller Task API

#include "co_bt.h"                   // Common BT Definition
#include "co_math.h"                 // Common Maths Definition
#include "ke_timer.h"
#include "app_fff0.h"                 // Application security Definition
#include "app_dis.h"                 // Device Information Service Application Definitions
#include "app_batt.h"                // Battery Application Definitions
#include "app_oads.h"                 // Application oads Definition
#include "nvds.h"                    // NVDS Definitions
#include "rf.h"
#include "uart.h"
#include "adc.h"
#include "gpio.h"
#include "wdt.h"
#include "app_api.h"
#include "app_sec.h"

/*
 * DEFINES
 ****************************************************************************************
 */
#define APP_DEVICE_NAME_LENGTH_MAX      (18)


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

typedef void (*appm_add_svc_func_t)(void);

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// List of service to add in the database
enum appm_svc_list
{
    APPM_SVC_FFF0,
    APPM_SVC_DIS,
    APPM_SVC_BATT,
	APPM_SVC_OADS,
    APPM_SVC_LIST_STOP ,
};

/*
 * LOCAL VARIABLES DEFINITIONS
 ****************************************************************************************
 */

/// Application Task Descriptor
static const struct ke_task_desc TASK_DESC_APP = {NULL, &appm_default_handler,
                                                  appm_state, APPM_STATE_MAX, APP_IDX_MAX};

/// List of functions used to create the database
static const appm_add_svc_func_t appm_add_svc_func_list[APPM_SVC_LIST_STOP] =
{
    (appm_add_svc_func_t)app_fff0_add_fff0s,
    (appm_add_svc_func_t)app_dis_add_dis,
    (appm_add_svc_func_t)app_batt_add_bas,
	(appm_add_svc_func_t)app_oad_add_oads,
};

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Application Environment Structure
struct app_env_tag app_env;

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */


void appm_nvds_env_init(void)
{
    uint8_t key_len = KEY_LEN;
    uint8_t encnumlen = NVDS_LEN_ENC_NUM_MAX;
	//generate init value if not stored in nvds
    if (nvds_get(NVDS_TAG_LOC_IRK, &key_len, app_env.loc_irk) != NVDS_OK)
    {
		UART_PRINTF("no loc irk\r\n");
        uint8_t counter;
        for (counter = 0; counter < KEY_LEN; counter++)
        {
            app_env.loc_irk[counter]    = (uint8_t)co_rand_word();
        }
        if (nvds_put(NVDS_TAG_LOC_IRK, KEY_LEN, (uint8_t *)&app_env.loc_irk) != NVDS_OK)
        {
            ASSERT_INFO(0, NVDS_TAG_LOC_IRK, 0);
        }
    }

    if (nvds_get(NVDS_TAG_ENC_NUM_MAX, &encnumlen, &app_env.enc_num_max) != NVDS_OK)
    {
        app_env.enc_num_max = 5;
		UART_PRINTF("no enc num max record\r\n");
        if (nvds_put(NVDS_TAG_ENC_NUM_MAX, (encnumlen), (uint8_t *)&app_env.enc_num_max) != NVDS_OK)
        {
            ASSERT_ERR(0);
        }
    }
    if (nvds_get(NVDS_TAG_ENC_NUM_CUR, &encnumlen, &app_env.enc_num_cur) != NVDS_OK)
    {
		UART_PRINTF("no enc num cur record\r\n");
        app_env.enc_num_cur = 0;
        if (nvds_put(NVDS_TAG_ENC_NUM_CUR, (encnumlen), (uint8_t *)&app_env.enc_num_cur) != NVDS_OK)
        {
            ASSERT_ERR(0);
        }
    }

}


void appm_init()
{
    // Reset the application manager environment
    memset(&app_env, 0, sizeof(app_env));

    // Create APP task
    ke_task_create(TASK_APP, &TASK_DESC_APP);

    // Initialize Task state
    ke_state_set(TASK_APP, APPM_INIT);


	appm_nvds_env_init();

    // Device Information Module
    app_dis_init();

    // Battery Module
    app_batt_init();
			
    app_oads_init();

	app_sec_init();

	APPM_SET_FIELD(ADV_EN, 0);
	ble_mac_read();
	uint8_t length = 1;
	uint8_t power = 0xf;
	if(nvds_get(NVDS_TAG_POWER, &length, &power) == NVDS_OK)
	{
		UART_PRINTF("POWER=%d\r\n", power);
	}
	set_ble_power(power);
	app_env.conn_intv = BLE_UAPDATA_DFT_INTVALUE;
	app_env.mtu_size = 20;
	app_env.pin_code = 123456;
	app_env.enc_en = 0;
}

bool appm_add_svc(void)
{
    // Indicate if more services need to be added in the database
    bool more_svc = false;

    // Check if another should be added in the database
    if (app_env.next_svc != APPM_SVC_LIST_STOP)
    {
        ASSERT_INFO(appm_add_svc_func_list[app_env.next_svc] != NULL, app_env.next_svc, 1);

        // Call the function used to add the required service
        appm_add_svc_func_list[app_env.next_svc]();

        // Select following service to add
        app_env.next_svc++;
        more_svc = true;
    }

    return more_svc;
}


/*Éè±¸Ö÷¶¯¶Ï¿ªÁ¬½Óº¯Êý*/
void appm_disconnect(void)
{
    struct gapc_disconnect_cmd *cmd = KE_MSG_ALLOC(GAPC_DISCONNECT_CMD,
                                                   KE_BUILD_ID(TASK_GAPC, app_env.conidx), TASK_APP,
                                                   gapc_disconnect_cmd);

    cmd->operation = GAPC_DISCONNECT;
    cmd->reason    = CO_ERROR_REMOTE_USER_TERM_CON;
	ke_state_set(TASK_APP, APPM_READY);
    // Send the message
    ke_msg_send(cmd);
}

void appm_start_advertising(void)
{	
	UART_PRINTF("%s\r\n", __func__);
    if (ke_state_get(TASK_APP) == APPM_READY)
    {				
        // Prepare the GAPM_START_ADVERTISE_CMD message
        struct gapm_start_advertise_cmd *cmd = KE_MSG_ALLOC(GAPM_START_ADVERTISE_CMD,
                                                            TASK_GAPM, TASK_APP,
                                                            gapm_start_advertise_cmd);

        cmd->op.addr_src    = GAPM_STATIC_ADDR;
        cmd->channel_map    = APP_ADV_CHMAP;
		uint16_t interval = 0;
		uint8_t advint_len = sizeof(interval);
		if(nvds_get(NVDS_TAG_BLE_ADV_INTERVAL, &advint_len, (uint8_t *)&interval) == NVDS_OK)
		{
//			UART_PRINTF("adv_int = %d\r\n", interval);
			cmd->intv_min		= interval;
			cmd->intv_max		= interval; 
		}
		else
		{
			cmd->intv_min		= APP_ADV_INT_MIN;
			cmd->intv_max		= APP_ADV_INT_MAX;	
		}

        cmd->op.code        = GAPM_ADV_UNDIRECT;
		
        cmd->info.host.mode = GAP_GEN_DISCOVERABLE;

 		/*-----------------------------------------------------------------------------------
         * Set the Advertising Data and the Scan Response Data
         *---------------------------------------------------------------------------------*/
        // Flag value is set by the GAP
        cmd->info.host.adv_data_len       = ADV_DATA_LEN;
        cmd->info.host.scan_rsp_data_len  = SCAN_RSP_DATA_LEN;

        // Advertising Data
        if(nvds_get(NVDS_TAG_APP_BLE_ADV_DATA, &cmd->info.host.adv_data_len,
                    &cmd->info.host.adv_data[0]) != NVDS_OK)
        {
            //cmd->info.host.adv_data_len = 0;

            cmd->info.host.adv_data[0] = 2;// Length of ad type flags
            cmd->info.host.adv_data[1] = GAP_AD_TYPE_FLAGS;
            cmd->info.host.adv_data[2] = GAP_BR_EDR_NOT_SUPPORTED;
            // set mode in ad_type
            switch(cmd->info.host.mode)
            {
                // General discoverable mode
                case GAP_GEN_DISCOVERABLE:
                {
                    cmd->info.host.adv_data[2] |= GAP_LE_GEN_DISCOVERABLE_FLG;
                }
                break;
                // Limited discoverable mode
                case GAP_LIM_DISCOVERABLE:
                {
                    cmd->info.host.adv_data[2] |= GAP_LE_LIM_DISCOVERABLE_FLG;
                }
                break;
                default: break; // do nothing
            }
            cmd->info.host.adv_data_len=3;
            //Add list of UUID and appearance						
            memcpy(&cmd->info.host.adv_data[cmd->info.host.adv_data_len],
                   APP_FFF0_ADV_DATA_UUID, APP_FFF0_ADV_DATA_UUID_LEN);
            cmd->info.host.adv_data_len += APP_FFF0_ADV_DATA_UUID_LEN;
        }

		//  Device Name Length
        uint8_t device_name_length;
        uint8_t device_name_avail_space;
        uint8_t device_name_temp_buf[APP_DEVICE_NAME_LENGTH_MAX] = {0};

        device_name_avail_space = ADV_DATA_LEN  - cmd->info.host.adv_data_len - 2;

        // Check if data can be added to the Advertising data
        if (device_name_avail_space > 2)
        {
						
            device_name_length = NVDS_LEN_DEVICE_NAME;
            if (nvds_get(NVDS_TAG_DEVICE_NAME, &device_name_length,
                         &device_name_temp_buf[0]) != NVDS_OK)
            {
            #if 0
                device_name_length = strlen(APP_DFLT_DEVICE_NAME);
                // Get default Device Name (No name if not enough space)
                memcpy(&device_name_temp_buf[0], APP_DFLT_DEVICE_NAME, device_name_length);
				#else
                device_name_length = strlen((const char*)ble_dev_name);
				memcpy(&device_name_temp_buf[0], ble_dev_name, device_name_length);
				#endif
            }
//			uart_printf_str("temp buff=", device_name_temp_buf, sizeof(device_name_temp_buf), UART_PRINTF);
			UART_PRINTF("name len=%d, avail len=%d, name=%s\r\n",device_name_length, device_name_avail_space, device_name_temp_buf);

	     	if(device_name_length > 0)
            {
                device_name_length = co_min(device_name_length, device_name_avail_space);
                cmd->info.host.adv_data[cmd->info.host.adv_data_len]     = device_name_length + 1;
                cmd->info.host.adv_data[cmd->info.host.adv_data_len + 1] = '\x09';//complete name type = 0x09
                memcpy(&cmd->info.host.adv_data[cmd->info.host.adv_data_len + 2],     device_name_temp_buf, device_name_length);
                cmd->info.host.adv_data_len += (device_name_length + 2);
            }
          
        }

        // Scan Response Data
        if(nvds_get(NVDS_TAG_APP_BLE_SCAN_RESP_DATA, &cmd->info.host.scan_rsp_data_len,
                    &cmd->info.host.scan_rsp_data[0]) != NVDS_OK)
        {
            cmd->info.host.scan_rsp_data_len = 0;
#if 0
			memcpy(&cmd->info.host.scan_rsp_data[cmd->info.host.scan_rsp_data_len],
                   APP_SCNRSP_DATA, APP_SCNRSP_DATA_LEN);
            cmd->info.host.scan_rsp_data_len += APP_SCNRSP_DATA_LEN;
			#else
			memcpy(&cmd->info.host.scan_rsp_data[cmd->info.host.scan_rsp_data_len],     scan_rsp_data, strlen((const char*)scan_rsp_data));
            cmd->info.host.scan_rsp_data_len += strlen((const char*)scan_rsp_data);
			#endif
        }

        // Send the message
        ke_msg_send(cmd);

		wdt_enable(0x3fff);

        // Set the state of the task to APPM_ADVERTISING
        ke_state_set(TASK_APP, APPM_ADVERTISING);	

    }
	
    // else ignore the request
}

/* Éè±¸Ö÷¶¯Í£Ö¹¹ã²¥º¯Êý*/
void appm_stop_advertising(void)
{
	UART_PRINTF("%s\r\n", __func__);
    if (ke_state_get(TASK_APP) == APPM_ADVERTISING)
    {
        // Go in ready state
        ke_state_set(TASK_APP, APPM_READY);

        // Prepare the GAPM_CANCEL_CMD message
        struct gapm_cancel_cmd *cmd = KE_MSG_ALLOC(GAPM_CANCEL_CMD,
                                                   TASK_GAPM, TASK_APP,
                                                   gapm_cancel_cmd);
        cmd->operation = GAPM_CANCEL;

        // Send the message
        ke_msg_send(cmd);

		wdt_disable_flag = 1;
    }
    // else ignore the request
}

void appm_update_adv_data( uint8_t* adv_buff, uint8_t adv_len, uint8_t* scan_buff, uint8_t scan_len)
{
	if (ke_state_get(TASK_APP) == APPM_ADVERTISING 
                 && (adv_len <= ADV_DATA_LEN) && (scan_len <= ADV_DATA_LEN))
	{
		struct gapm_update_advertise_data_cmd *cmd =  KE_MSG_ALLOC(
            		            GAPM_UPDATE_ADVERTISE_DATA_CMD,
            		            TASK_GAPM,
            		            TASK_APP,
            		            gapm_update_advertise_data_cmd);

		cmd->operation = GAPM_UPDATE_ADVERTISE_DATA;
		cmd->adv_data_len = adv_len+2;//20191201 æŠŠç”¨æˆ·è‡ªå®šä¹‰æ•°æ®çš„æ ‡å‡†æ ¼å¼æ”¾è¿›æ¥ï¼Œç”¨æˆ·åªå¡«æ•°æ®ã€‚
		cmd->scan_rsp_data_len = scan_len+2;

		//memcpy
		cmd->adv_data[0] = adv_len+1;		
		cmd->adv_data[1] = 0xff;
		memcpy(&cmd->adv_data[2], adv_buff, adv_len);
		cmd->scan_rsp_data[0] = scan_len+1;
		cmd->scan_rsp_data[1] = 0xff;
		memcpy(&cmd->scan_rsp_data[2], scan_buff, scan_len);
        
		// Send the message
		ke_msg_send(cmd);
	}
}



void appm_update_param(struct gapc_conn_param *conn_param)
{
    struct gapc_param_update_cmd *cmd = KE_MSG_ALLOC(GAPC_PARAM_UPDATE_CMD,
                                                     KE_BUILD_ID(TASK_GAPC, app_env.conidx), TASK_APP,
                                                     gapc_param_update_cmd);

	 cmd->operation  = GAPC_UPDATE_PARAMS;
	 #if 0
	 cmd->intv_min	 = intv_min;
	 cmd->intv_max	 = intv_max;
	 cmd->latency	 = latency;
	 cmd->time_out	 = time_out;
#else
	cmd->intv_min	= conn_param->intv_min;
	cmd->intv_max	= conn_param->intv_max;
	cmd->latency	= conn_param->latency;
	cmd->time_out	= conn_param->time_out;
#endif
    cmd->ce_len_min = 0xFFFF;
    cmd->ce_len_max = 0xFFFF;
		
    UART_PRINTF("intv_min = %d,intv_max = %d,latency = %d,time_out = %d\r\n",cmd->intv_min,cmd->intv_max,cmd->latency,cmd->time_out);
	
    ke_msg_send(cmd);
}


uint8_t appm_get_dev_name(uint8_t* name)
{
    // copy name to provided pointer
    memcpy(name, app_env.dev_name, app_env.dev_name_len);
    // return name length
    return app_env.dev_name_len;
}






uint8_t appm_adv_data_decode_name(uint8_t len, const uint8_t *data,uint8_t *name_str)
{
    uint8_t find = 0;
    uint8_t index;
    for(index = 0; index < len;)
    {
        switch(data[index + 1])
        {
        case GAP_AD_TYPE_FLAGS:
        {
//            UART_PRINTF("AD_TYPE : ");
//            for(uint8_t len = 0; len < data[index] - 1; len++)
//            {
//                UART_PRINTF("%02x ",data[index + 2 + len]);
//            }
//            UART_PRINTF("\r\n");
            index +=(data[index] + 1);
        }
        break;
        case GAP_AD_TYPE_SHORTENED_NAME:
        case GAP_AD_TYPE_COMPLETE_NAME:
        {
//            if(strncmp((char*)&data[index + 2],find_str,str_len) == 0 )
//            {
//                find = 1;
//            }
//            UART_PRINTF("ADV_NAME : ");
            for(uint8_t len = 0; len < data[index] - 1; len++)
            {
               // UART_PRINTF("%c",data[index + 2 + len]);
                *name_str++ = data[index + 2 + len];
            }
//            UART_PRINTF("\r\n");
            index +=(data[index] + 1);
            
            find = 1;
        }
        break;
        case GAP_AD_TYPE_MORE_16_BIT_UUID:
        {
//            UART_PRINTF("UUID : ");
//            for(uint8_t len = 0; len < data[index] - 1;)
//            {
//                UART_PRINTF("%02x%02x  ",data[index + 2 + len],data[index + 3 + len]);
//                len+=2;
//            }
//            UART_PRINTF("\r\n");
            index +=(data[index] + 1);
        }
        break;
        default:
        {
            index +=(data[index] + 1);
        }
        break;
        }
    }
    return find;
}

void appc_le_data_length_update_req(uint8_t conidx,uint16_t max_tx_octets,uint16_t max_tx_time)
{
    
//    if((ke_state_get(KE_BUILD_ID(TASK_APPC,conidx)) == APPC_LINK_CONNECTED) ||(ke_state_get(KE_BUILD_ID(TASK_APPC,conidx)) == APPC_SDP_DISCOVERING)\
//                ||(ke_state_get(KE_BUILD_ID(TASK_APPC,conidx)) == APPC_SERVICE_CONNECTED) )
	if(ke_state_get(KE_BUILD_ID(TASK_APP, 0)) == APPM_CONNECTED  )
    {

        // Prepare the GAPC_SET_LE_PKT_SIZE_CMD message
        struct gapc_set_le_pkt_size_cmd *cmd = KE_MSG_ALLOC(GAPC_SET_LE_PKT_SIZE_CMD,
                                                     KE_BUILD_ID(TASK_GAPC, conidx),KE_BUILD_ID(TASK_APP,conidx) , //KE_BUILD_ID(TASK_APPM,conidx)
                                                     gapc_set_le_pkt_size_cmd);

        cmd->operation  = GAPC_SET_LE_PKT_SIZE;
        cmd->tx_octets   = max_tx_octets;
        cmd->tx_time   = max_tx_time;

        UART_PRINTF("%s tx_octets = %d,tx_time = %d\r\n", __func__, max_tx_octets,max_tx_time);

        // Send the message
        ke_msg_send(cmd);
    }else
    {
        UART_PRINTF("%s can't pro. cur state :%d\r\n",ke_state_get(KE_BUILD_ID(TASK_APP,conidx)));
    }
}

void appc_gatt_mtu_change(uint8_t conidx)
{
//    if((ke_state_get(KE_BUILD_ID(TASK_APPC,conidx)) == APPC_LINK_CONNECTED) ||(ke_state_get(KE_BUILD_ID(TASK_APPC,conidx)) == APPC_SDP_DISCOVERING)\
//                ||(ke_state_get(KE_BUILD_ID(TASK_APPC,conidx)) == APPC_SERVICE_CONNECTED) )
	if(ke_state_get(KE_BUILD_ID(TASK_APP, 0)) == APPM_CONNECTED  )
    {
        struct gattc_exc_mtu_cmd *cmd = KE_MSG_ALLOC(GATTC_EXC_MTU_CMD,
                                    KE_BUILD_ID(TASK_GATTC, conidx), KE_BUILD_ID(TASK_APP, conidx),  
                                    gattc_exc_mtu_cmd);

        cmd->operation = GATTC_MTU_EXCH;

        cmd->seq_num = 0;
        
        ke_msg_send(cmd);

    }else
    {
        UART_PRINTF("%s can't pro. cur state :%d\r\n",ke_state_get(KE_BUILD_ID(TASK_APP,conidx)));
    }
}

#ifdef UART_PRINTF_CLASS_DETAIL
struct p_appm_state{
	const char* appm_state_str;
};

struct p_appm_state appm_state_ins[APPM_STATE_MAX]={
	"APPM_INIT",
	"APPM_CREATE_DB",
	"APPM_READY",
	"APPM_ADVERTISING",
	"APPM_CONNECTED",
};

#endif




void appm_scan_adv_con_schedule(void)
{
    uint8_t  cur_state = ke_state_get(TASK_APP);
	#ifdef UART_PRINTF_CLASS_DETAIL
  UART_PRINTF("schedule state = %s\r\n", appm_state_ins[cur_state].appm_state_str);
#endif
    switch(cur_state)
    {
        case APPM_CREATE_DB: // 1
        {
            if(APPM_GET_FIELD(ADV_EN))
            {
                ke_state_set(TASK_APP, APPM_READY);
                appm_start_advertising(); 
            }
//			else if(APPM_GET_FIELD(SCAN_EN))
//            {
//                ke_state_set(TASK_APPM,APPM_IDLE);
//                appm_start_scanning();
//            }
        }break;
        
        case APPM_READY: // 2
        {
            if(APPM_GET_FIELD(ADV_EN))
            {                
                appm_start_advertising();
            }
//			else if(APPM_GET_FIELD(SCAN_EN))
//            {
//                appm_start_scanning();
//            }
        }break;
       
        case APPM_ADVERTISING: // 3
        {
            if(!APPM_GET_FIELD(ADV_EN))
            {
                appm_stop_advertising();
            }
        }break;
#if 0
        case APPM_WAIT_ADVERTISTING_END: //4
        {                     
            UART_PRINTF("APPM_WAIT_ADVERTISTING_END\r\n");
        }break;  

        case APPM_ADVERTISTING_END: // 5
        {
            UART_PRINTF("APPM_ADVERTISTING_END\r\n");
            {

				if(APPM_GET_FIELD(ADV_EN))
                {
                    ke_state_set(TASK_APP,APPM_READY);
                    appm_start_advertising();
                }
//                else if(APPM_GET_FIELD(SCAN_EN))
//                {
//                    ke_state_set(TASK_APP,APPM_READY);
//                    appm_start_scanning();
//                }
//				else
//                {
//                    UART_PRINTF("adv && scan off\r\n");
//                    ke_state_set(TASK_APP, APPM_READY);
//                    if(appm_env.con_dev_flag == 1)
//                    {
//                        ke_state_set(TASK_APPM,APPM_IDLE);
//                        appm_start_connencting(appm_env.con_dev_addr);
//                    }
//                }
            
            }
            

        }break;  
			
        case APPM_SCANNING: //6
        {
            if(APPM_GET_FIELD(ADV_EN) ||!APPM_GET_FIELD(SCAN_EN))
            {              
                appm_stop_scanning();
            }
        }break; 
 

        case APPM_WAIT_SCAN_END: // 7
        {
            UART_PRINTF("APPM_WAIT_SCAN_END\r\n");
        }break;       
       
        case APPM_SCAN_END: // 8
        {           
            
            if(APPM_GET_FIELD(ADV_EN))
            {
                ke_state_set(TASK_APPM,APPM_IDLE);
                appm_start_advertising();
            }else if(APPM_GET_FIELD(SCAN_EN))
            {
                ke_state_set(TASK_APPM,APPM_IDLE);
                appm_start_scanning();             
            }else
            {
                UART_PRINTF("scan && adv off\r\n");
                ke_state_set(TASK_APPM,APPM_IDLE);
                if(appm_env.con_dev_flag == 1)
                {
                    ke_state_set(TASK_APPM,APPM_IDLE);
                    appm_start_connencting(appm_env.con_dev_addr);
                }
            }
        }break;
        
        case APPM_CONNECTING: // 9
        {           
           // appm_stop_connencting();
        }break;
        
        case APPM_LINK_CONNECTED: // a 10 
        {
            if(APPM_GET_FIELD(ADV_EN))
            {
                ke_state_set(TASK_APPM,APPM_IDLE);
                appm_start_advertising();
            }else if(APPM_GET_FIELD(SCAN_EN))
            {
                ke_state_set(TASK_APPM,APPM_IDLE);
                appm_start_scanning();
            }
             

        }break;
              
         case APPM_DISCONNECT:// d 13
        {
            if(APPM_GET_FIELD(ADV_EN))
            {
                ke_state_set(TASK_APPM,APPM_IDLE);
                appm_start_advertising();
            }else if(APPM_GET_FIELD(SCAN_EN))
            {
                ke_state_set(TASK_APPM,APPM_IDLE);
                appm_start_scanning();
            }
             
        }break;
#endif        
        

       default:break;
   }
    
}

#endif //(BLE_APP_PRESENT)

/// @} APP


