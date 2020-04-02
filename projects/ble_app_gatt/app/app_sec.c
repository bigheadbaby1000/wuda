/**
 ****************************************************************************************
 *
 * @file app_sec.c
 *
 * @brief Application Security Entry Point
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

#include "rwip_config.h"


#include <string.h>
#include "co_math.h"
#include "gapc_task.h"      // GAP Controller Task API Definition
#include "gap.h"            // GAP Definition
#include "gapc.h"           // GAPC Definition
#include "prf_types.h"
#include "llc_llcp.h"

#include "app.h"            // Application API Definition
#include "app_sec.h"        // Application Security API Definition
#include "app_task.h"       // Application Manager API Definitionde 
#include "ke_timer.h"
#include "nvds.h"           // NVDS API Definitions
#include "co_utils.h"
#include "uart.h"
/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */


/// Application Security Environment Structure
struct app_sec_env_tag app_sec_env;



bool app_get_bonding_flag(void);

bool app_sec_get_enc_flag(void);


/*
 * GLOBAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */

void app_sec_init()
{
	uint8_t length = NVDS_LEN_PERIPH_BONDED;
	if (nvds_get(NVDS_TAG_PERIPH_BONDED, &length, (uint8_t *)&app_sec_env.bonded) != NVDS_OK)
	{
		// If read value is invalid, set status to not bonded
		if ((app_sec_env.bonded != true) && (app_sec_env.bonded != false))
		{
			app_sec_env.bonded = false;
		}
	}
	app_sec_env.peer_pairing = false;
	app_sec_env.peer_encrypt = false;	
//	app_sec_env.bond_lost = false;
//	app_sec_env.pairing_fail = false;

    
//    UART_PRINTF("app_sec_env.bonded = %d\r\n", app_sec_env.bonded);
}

bool app_sec_get_bond_status(void)
{
    return app_sec_env.bonded;
}


bool app_get_bonding_flag(void)
{
	UART_PRINTF("peer pairing = %d\r\n",app_sec_env.peer_pairing);
	return app_sec_env.peer_pairing;
}


bool app_get_encrypt_flag(void)
{
	UART_PRINTF("peer encrypt = %d\r\n",app_sec_env.peer_encrypt);
	return app_sec_env.peer_encrypt;
}


void app_sec_remove_bond(void)
{
    UART_PRINTF("%s\r\n", __func__);
    if (app_sec_env.bonded == true)
    {
        app_sec_env.bonded = false;
        if (nvds_put(NVDS_TAG_PERIPH_BONDED, NVDS_LEN_PERIPH_BONDED,
                     (uint8_t *)&app_sec_env.bonded) != NVDS_OK)
            ASSERT_ERR(0);

    }
	app_sec_env.bonded = false;
	app_sec_env.peer_pairing = false;
	app_sec_env.peer_encrypt = false;
}

void app_sec_send_security_req(uint8_t conidx)
{
//	if(!app_get_encrypt_flag() && !app_get_bonding_flag())
	{
		UART_PRINTF("%s\r\n", __func__);
		
		// Send security request
	    struct gapc_security_cmd *cmd = KE_MSG_ALLOC(GAPC_SECURITY_CMD,
	                                                 KE_BUILD_ID(TASK_GAPC, conidx), TASK_APP,
	                                                 gapc_security_cmd);

	    cmd->operation = GAPC_SECURITY_REQ;
		
		if(app_env.enc_en)
	  	  cmd->auth      = GAP_AUTH_REQ_MITM_BOND;
		else
			cmd->auth	   = GAP_AUTH_REQ_NO_MITM_NO_BOND;
	    // Send the message
	    ke_msg_send(cmd);
	}
//	else
//	{
//		ke_timer_clear(APP_SEND_SECURITY_REQ,TASK_APP); 
//	}
}

/*
 * MESSAGE HANDLERS
 ****************************************************************************************
 */

static int gapc_bond_req_ind_handler(ke_msg_id_t const msgid,
                                     struct gapc_bond_req_ind const *param,
                                     ke_task_id_t const dest_id,
                                     ke_task_id_t const src_id)
{
	UART_PRINTF("========== %s req = %x =========\r\n",__func__ , param->request);
//	  GAPC_PAIRING_REQ, 0
//    GAPC_PAIRING_RSP,1
//    GAPC_PAIRING_SUCCEED,2
//    GAPC_PAIRING_FAILED,3
//    GAPC_TK_EXCH,4
//    GAPC_IRK_EXCH,5
//    GAPC_CSRK_EXCH,6
//    GAPC_LTK_EXCH,7
    struct gapc_bond_cfm *cfm = KE_MSG_ALLOC(GAPC_BOND_CFM,
                                             src_id, TASK_APP,
                                             gapc_bond_cfm);

    switch (param->request)
    {
        case (GAPC_PAIRING_REQ):
        {
//	    	UART_PRINTF("gapc_pairing req\r\n");
	    	app_sec_env.peer_pairing = true;
            cfm->request = GAPC_PAIRING_RSP;

            cfm->accept  = false;

#if (!HID_CONNECT_ANY)
            // Check if we are already bonded (Only one bonded connection is supported)
            if (!app_sec_env.bonded)
#endif				
            {
                cfm->accept  = true;

                // Pairing Features
				if(app_env.enc_en)
				{
					cfm->data.pairing_feat.auth 	 = GAP_AUTH_REQ_MITM_BOND;
					cfm->data.pairing_feat.iocap	 = GAP_IO_CAP_DISPLAY_ONLY;
				}
				else
				{
					cfm->data.pairing_feat.auth 	 = GAP_AUTH_REQ_NO_MITM_NO_BOND;
					cfm->data.pairing_feat.iocap	 = GAP_IO_CAP_NO_INPUT_NO_OUTPUT;
				}
                cfm->data.pairing_feat.key_size  = 16;
                cfm->data.pairing_feat.oob       = GAP_OOB_AUTH_DATA_NOT_PRESENT;
                cfm->data.pairing_feat.sec_req   = GAP_NO_SEC;
                cfm->data.pairing_feat.ikey_dist = GAP_KDIST_ENCKEY| GAP_KDIST_IDKEY;
                cfm->data.pairing_feat.rkey_dist = GAP_KDIST_ENCKEY| GAP_KDIST_IDKEY;
//                cfm->data.pairing_feat.ikey_dist =  GAP_KDIST_IDKEY |GAP_KDIST_SIGNKEY;
//                cfm->data.pairing_feat.rkey_dist =  GAP_KDIST_IDKEY | GAP_KDIST_SIGNKEY;
            }
        } break;

        case (GAPC_LTK_EXCH):
        {
//			UART_PRINTF("gapc_ltk req\r\n");
            // Counter
            uint8_t counter;
            cfm->accept  = true;
            cfm->request = GAPC_LTK_EXCH;

#if (0)
			// Generate all the values
            cfm->data.ltk.ediv = 0x1234;

            for (counter = 0; counter < RAND_NB_LEN; counter++)
            {
                cfm->data.ltk.ltk.key[counter] = 0x11;
                cfm->data.ltk.randnb.nb[counter] = 0x11;
            }

            for (counter = RAND_NB_LEN; counter < KEY_LEN; counter++)
            {
                cfm->data.ltk.ltk.key[counter] = 0x11;
            }
#else
            // Generate all the values
            cfm->data.ltk.ediv = (uint16_t)co_rand_word();

            for (counter = 0; counter < RAND_NB_LEN; counter++)
            {
                cfm->data.ltk.ltk.key[counter]    = (uint8_t)co_rand_word();
                cfm->data.ltk.randnb.nb[counter] = (uint8_t)co_rand_word();
            }

            for (counter = RAND_NB_LEN; counter < KEY_LEN; counter++)
            {
                cfm->data.ltk.ltk.key[counter]    = (uint8_t)co_rand_word();
            }
#endif
			if(app_env.enc_num_cur < app_env.enc_num_max)
			 {
				if (nvds_put(NVDS_TAG_LTK, NVDS_LEN_LTK,
							 (uint8_t *)&cfm->data.ltk) != NVDS_OK)
				{
					ASSERT_ERR(0);
				}
				app_env.enc_num_cur++;
			 }
			else
				UART_PRINTF("no nvds tag can be used\r\n");
        } break;


        case (GAPC_IRK_EXCH):
        {
//			UART_PRINTF("gapc_lrk exch\r\n");

            cfm->accept  = true;
            cfm->request = GAPC_IRK_EXCH;

            // Load IRK
            memcpy(cfm->data.irk.irk.key, app_env.loc_irk, KEY_LEN);
            // load device address
            cfm->data.irk.addr.addr_type = ADDR_PUBLIC;
            memcpy(cfm->data.irk.addr.addr.addr,(uint8_t *)&co_default_bdaddr,BD_ADDR_LEN);
        } break;

        case (GAPC_TK_EXCH):
        {
            // Generate a PIN Code- (Between 100000 and 999999)
            
//            app_env.pin_code = 123456;//(100000 + (co_rand_word()%900000));

            cfm->accept  = true;
            cfm->request = GAPC_TK_EXCH;

            // Set the TK value
            memset(cfm->data.tk.key, 0, KEY_LEN);

            cfm->data.tk.key[0] = (uint8_t)((app_env.pin_code & 0x000000FF) >>  0);
            cfm->data.tk.key[1] = (uint8_t)((app_env.pin_code & 0x0000FF00) >>  8);
            cfm->data.tk.key[2] = (uint8_t)((app_env.pin_code & 0x00FF0000) >> 16);
            cfm->data.tk.key[3] = (uint8_t)((app_env.pin_code & 0xFF000000) >> 24);
        } break;

        default:
        {
            ASSERT_ERR(0);
        } break;
    }

    // Send the message
    ke_msg_send(cfm);

    return (KE_MSG_CONSUMED);
}


static int gapc_bond_ind_handler(ke_msg_id_t const msgid,
                                 struct gapc_bond_ind const *param,
                                 ke_task_id_t const dest_id,
                                 ke_task_id_t const src_id)
{
    UART_PRINTF("%s param->info = 0x%x\r\n",__func__,param->info);
//    GAPC_PAIRING_REQ,0
//    GAPC_PAIRING_RSP,1
//    GAPC_PAIRING_SUCCEED,2
//    GAPC_PAIRING_FAILED,3
//    GAPC_TK_EXCH,4
//    GAPC_IRK_EXCH,5
//    GAPC_CSRK_EXCH,6
//    GAPC_LTK_EXCH,7
//    GAPC_REPEATED_ATTEMPT,8
//    GAPC_OOB_EXCH,9
    
    switch (param->info)
    {
        case (GAPC_PAIRING_SUCCEED):
        {
	    	UART_PRINTF("gapc pairing success\r\n");
//			app_sec_env.bond_lost = false;
//			app_sec_env.pairing_fail = false;
#if (HID_CONNECT_ANY)
			app_sec_env.bonded = false;
#else
            app_sec_env.bonded = true;
#endif
		
            // Update the bonding status in the environment
            if (nvds_put(NVDS_TAG_PERIPH_BONDED, NVDS_LEN_PERIPH_BONDED,
                         (uint8_t *)&app_sec_env.bonded) != NVDS_OK)
            {
                // An error has occurred during access to the NVDS
                ASSERT_ERR(0);
            }
			#if 0
            // Set the BD Address of the peer device in NVDS
            if (nvds_put(NVDS_TAG_PEER_BD_ADDRESS, NVDS_LEN_PEER_BD_ADDRESS,
                         (uint8_t *)gapc_get_bdaddr(0, SMPC_INFO_PEER)) != NVDS_OK)
            {
                // An error has occurred during access to the NVDS
                ASSERT_ERR(0);
            }
			#endif
        } break;

        case (GAPC_REPEATED_ATTEMPT):
        {
			app_sec_env.peer_pairing = false;
	    	app_sec_env.peer_encrypt = false;
//			app_sec_env.pairing_fail = true;
            appm_disconnect();
        } break;

        case (GAPC_IRK_EXCH):
        {
           	// Store peer identity in NVDS
           	if (nvds_put(NVDS_TAG_PEER_IRK, NVDS_LEN_PEER_IRK, (uint8_t *)&param->data.irk) != NVDS_OK)
           	{
               	ASSERT_ERR(0);
           	}
					 
			memcpy(&app_env.peer_irk,&param->data.irk.irk.key[0],sizeof(struct gapc_irk));
//			uart_printf_str("irk.key = ",  &param->data.irk.irk.key[0], sizeof(struct gap_sec_key), UART_PRINTF);
			#if 1
			UART_PRINTF("IRK = ");
			for(int i = 0;i<sizeof(struct gap_sec_key);i++)
			{
				UART_PRINTF("%x",param->data.irk.irk.key[i]);
			}
			UART_PRINTF("\r\n");
			#endif
			UART_PRINTF("addr.type = %x\r\n",param->data.irk.addr.addr_type);
//			uart_printf_str("addr.addr = ",  &param->data.irk.addr.addr.addr[0], 6, UART_PRINTF);
			#if 1
			UART_PRINTF("addr.addr = ");
			for(int i = 0;i<sizeof(struct bd_addr);i++)
			{
				UART_PRINTF("0x%x ",param->data.irk.addr.addr.addr[i]);
			}
			UART_PRINTF("\r\n");
			#endif
        } break;

        case (GAPC_PAIRING_FAILED):
        {
	    	UART_PRINTF("gapc pairing failed\r\n");
	    	app_sec_env.peer_pairing = false;
	    	app_sec_env.peer_encrypt = false;
//			app_sec_env.pairing_fail = true;
	    	appm_disconnect();
        } break;
        case (GAPC_LTK_EXCH):
        {
			UART_PRINTF("LTK = ");
			for(int i = 0;i<sizeof(struct gap_sec_key);i++)
			{
				UART_PRINTF("%x",param->data.ltk.ltk.key[i]);
			}
			UART_PRINTF("\r\n");
#if 0
//			 for(uint8_t i=0; i<app_env.enc_num_max; i++)
			if(app_env.enc_num_cur < app_env.enc_num_max)
			 {
				 if(nvds_put(NVDS_TAG_LTK+app_env.enc_num_cur, NVDS_LEN_LTK,
							  (uint8_t *)&param->data.ltk) != NVDS_OK)
				 {
					 ASSERT_ERR(0);
				 }
				app_env.enc_num_cur++;
			 }
			else
				UART_PRINTF("no nvds tag can be used\r\n");
#endif

        } break;
        default:
        {
            ASSERT_ERR(0);
        } break;
    }

    return (KE_MSG_CONSUMED);
}

static int gapc_encrypt_req_ind_handler(ke_msg_id_t const msgid,
                                        struct gapc_encrypt_req_ind const *param,
                                        ke_task_id_t const dest_id,
                                        ke_task_id_t const src_id)
{
	if(ke_timer_active(APP_SEND_SECURITY_REQ, TASK_APP))
	{
		ke_timer_clear(APP_SEND_SECURITY_REQ, TASK_APP);
	}
	
	UART_PRINTF("%s\r\n",__func__);
	app_sec_env.peer_encrypt = true;
    // LTK value
    struct gapc_ltk ltk;
    // Length
    uint8_t length = NVDS_LEN_LTK;

    // Prepare the GAPC_ENCRYPT_CFM message
    struct gapc_encrypt_cfm *cfm = KE_MSG_ALLOC(GAPC_ENCRYPT_CFM,
                                                src_id, TASK_APP,
                                                gapc_encrypt_cfm);

    cfm->found    = false;

	
#if (!HID_CONNECT_ANY)
    if (app_sec_env.bonded)
#else
	if(1)
#endif		
    {
        // Retrieve the required informations from NVDS
        for(uint8_t i=0; i<app_env.enc_num_max; i++)
       	{
			if (nvds_get(NVDS_TAG_LTK+i, &length, (uint8_t *)&ltk) == NVDS_OK)
			{
				// Check if the provided EDIV and Rand Nb values match with the stored values
				UART_PRINTF("LTK = ");
				for(int i = 0;i<sizeof(struct gap_sec_key);i++)
				{
					UART_PRINTF("%x",ltk.ltk.key[i]);
				}
				UART_PRINTF("\r\n");
				if ((param->ediv == ltk.ediv) &&
					!memcmp(&param->rand_nb.nb[0], &ltk.randnb.nb[0], sizeof(struct rand_nb)))
				{
					cfm->found	  = true;
					cfm->key_size = 16;
					memcpy(&cfm->ltk, &ltk.ltk, sizeof(struct gap_sec_key));//send local LTK to master
					break;
				}
				else if(i==app_env.enc_num_max-1)
				{
					UART_PRINTF("ltk fit error\r\n");
					llc_llcp_reject_ind_pdu_send(0, LLCP_START_ENC_REQ_OPCODE, CO_ERROR_PIN_MISSING);
					appm_disconnect();
				}
			}
			else if(i==app_env.enc_num_max-1)
			{
				UART_PRINTF("read ltk fail\r\n");
				llc_llcp_reject_ind_pdu_send(0, LLCP_START_ENC_REQ_OPCODE, CO_ERROR_PIN_MISSING);
				appm_disconnect();
			}

		}

		// Send the message
    	ke_msg_send(cfm);
    }
	else
	{
	    UART_PRINTF("rc not bonded\r\n");
        llc_llcp_reject_ind_pdu_send(0, LLCP_START_ENC_REQ_OPCODE, CO_ERROR_PIN_MISSING);
        appm_disconnect();
	}
    
    return (KE_MSG_CONSUMED);
}


static int gapc_encrypt_ind_handler(ke_msg_id_t const msgid,
                                    struct gapc_encrypt_ind const *param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{
    // encryption/ re-encryption succeeded
	UART_PRINTF("%s \r\n",__func__);

    return (KE_MSG_CONSUMED);
}

static int app_sec_msg_dflt_handler(ke_msg_id_t const msgid,
                                    void *param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{
    // Drop the message

    return (KE_MSG_CONSUMED);
}

/*
 * LOCAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Default State handlers definition
const struct ke_msg_handler app_sec_msg_handler_list[] =
{
    // Note: first message is latest message checked by kernel so default is put on top.
    {KE_MSG_DEFAULT_HANDLER,  (ke_msg_func_t)app_sec_msg_dflt_handler},

    {GAPC_BOND_REQ_IND,       (ke_msg_func_t)gapc_bond_req_ind_handler},
    {GAPC_BOND_IND,           (ke_msg_func_t)gapc_bond_ind_handler},

    {GAPC_ENCRYPT_REQ_IND,    (ke_msg_func_t)gapc_encrypt_req_ind_handler},
    {GAPC_ENCRYPT_IND,        (ke_msg_func_t)gapc_encrypt_ind_handler},
};

const struct ke_state_handler app_sec_table_handler =
    {&app_sec_msg_handler_list[0], (sizeof(app_sec_msg_handler_list)/sizeof(struct ke_msg_handler))};


/// @} APP
