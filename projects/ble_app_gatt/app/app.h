/**
 ****************************************************************************************
 *
 * @file app.h
 *
 * @brief Application entry point
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */

#ifndef APP_H_
#define APP_H_

/**
 ****************************************************************************************
 * @addtogroup APP
 * @ingroup RICOW
 *
 * @brief Application entry point.
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"     // SW configuration

#if (BLE_APP_PRESENT)

#include <stdint.h>          // Standard Integer Definition
#include <co_bt.h>           // Common BT Definitions
#include "arch.h"            // Platform Definitions
#include "gapc.h"            // GAPC Definitions
#include "nvds.h"

/*
 * DEFINES
 ****************************************************************************************
 */
/// Maximal length of the Device Name value
#define APP_DEVICE_NAME_MAX_LEN      (29)

/// Default Advertising duration - 30s (in multiple of 10ms)
#define APP_DFLT_ADV_DURATION        (3000)
#define APPC_IDX_MAX 3


extern uint8_t ble_dev_name[32];
extern uint8_t scan_rsp_data[32];

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
*/
enum appm_error
{
	APPM_ERROR_NO_ERROR,
	APPM_ERROR_LINK_LOSS,
	APPM_ERROR_STATE,
	APPM_ERROR_NTFIND_DISABLE,
	APPM_ERROR_LINK_MAX
};	

/// List of Application NVDS TAG identifiers
enum app_nvds_tag
{
    /// BLE Application Advertising data
    NVDS_TAG_APP_BLE_ADV_DATA           = 0x0B,
    NVDS_LEN_APP_BLE_ADV_DATA           = 32,

    /// BLE Application Scan response data
    NVDS_TAG_APP_BLE_SCAN_RESP_DATA     = 0x0C,
    NVDS_LEN_APP_BLE_SCAN_RESP_DATA     = 32,
#if 0
    /// Mouse Sample Rate
    NVDS_TAG_MOUSE_SAMPLE_RATE          = 0x38,
    NVDS_LEN_MOUSE_SAMPLE_RATE          = 1,
        /// Mouse NTF Cfg
    NVDS_TAG_MOUSE_NTF_CFG              = 0x3A,
    NVDS_LEN_MOUSE_NTF_CFG              = 2,
        /// Mouse Timeout value
    NVDS_TAG_MOUSE_TIMEOUT              = 0x3B,
    NVDS_LEN_MOUSE_TIMEOUT              = 2,
    /// Peer Device BD Address
    NVDS_TAG_PEER_BD_ADDRESS            = 0x3C,
    NVDS_LEN_PEER_BD_ADDRESS            = 7,
    /// Mouse Energy Safe
    NVDS_TAG_MOUSE_ENERGY_SAFE          = 0x3D,
    NVDS_LEN_MOUSE_SAFE_ENERGY          = 2,
    /// PAIRING
    NVDS_TAG_PAIRING                    = 0x3E,
    NVDS_LEN_PAIRING                    = 54,
#endif
    NVDS_TAG_UART_BAUDRATE              = 0x10,
    NVDS_LEN_UART_BAUDRATE              = 4,
    /// Peripheral Bonded
    NVDS_TAG_PERIPH_BONDED              = 0x30,
    NVDS_LEN_PERIPH_BONDED              = 1,
    
    NVDS_TAG_ENC_NUM_MAX              = 0x31,
    NVDS_LEN_ENC_NUM_MAX              = 1,

    NVDS_TAG_ENC_NUM_CUR              = 0x32,
    NVDS_LEN_ENC_NUM_CUR              = 1,
    
    NVDS_TAG_POWER              = 0x33,
    NVDS_LEN_POWER              = 1,
    /// EDIV (2bytes), RAND NB (8bytes),  LTK (16 bytes), Key Size (1 byte)
    NVDS_TAG_LTK                        = 0x34,
    NVDS_LEN_LTK                        = 28,
};

enum app_loc_nvds_tag
{
    /// Audio mode 0 task
    NVDS_TAG_AM0_FIRST                  = NVDS_TAG_APP_SPECIFIC_FIRST, // 0x90
    NVDS_TAG_AM0_LAST                   = NVDS_TAG_APP_SPECIFIC_FIRST+16, // 0xa0

    /// Local device Identity resolving key
    NVDS_TAG_LOC_IRK,
    /// Peer device Resolving identity key (+identity address)
    NVDS_TAG_PEER_IRK,

    /// size of local identity resolving key
    NVDS_LEN_LOC_IRK                    = KEY_LEN,
    /// size of Peer device identity resolving key (+identity address)
    NVDS_LEN_PEER_IRK                   = sizeof(struct gapc_irk),
};

/// Application environment structure
struct app_env_tag
{
    /// Connection handle
    uint16_t conhdl;
    /// Connection Index
    uint8_t  conidx;

    /// Last initialized profile
    uint8_t next_svc;

    /// Bonding status
    bool bonded;

    /// Device Name length
    uint8_t dev_name_len;
    /// Device Name
    uint8_t dev_name[APP_DEVICE_NAME_MAX_LEN];

    /// Local device IRK
    uint8_t loc_irk[KEY_LEN];
    struct gapc_irk peer_irk;
	uint16_t mtu_size;
	
    uint16_t fields;
    uint16_t fields_bck;

	uint16_t conn_intv;
	uint8_t enc_num_max;
	uint8_t enc_num_cur;
	uint32_t pin_code;
	uint8_t enc_en;
};

enum app_fields
{
    /// ADV EN (1 when EN)
    APPM_ADV_EN = 0,
    APPM_ADV_EN_MASK = 0x01,

   /// SCAN EN (1 when EN)
    APPM_SCAN_EN = 1,
    APPM_SCAN_EN_MASK = 0x02,

    /// SDEV CON  FLAG
 //   APPM_SDEV_CON = 2,
  //  APPM_SDEV_CON_MASK = 0x04,
    
    /// MDEV CON  FLAG
 //   APPM_MDEV_CON = 3,
 //   APPM_MDEV_CON_MASK = 0x18,
};

#define APPM_SET_FIELD(field, value)\
    (app_env.fields) = ((app_env.fields) & (~APPM_##field##_MASK)) \
                                     | (((value) << APPM_##field) & (APPM_##field##_MASK))


/// Get appm configuration field
#define APPM_GET_FIELD(field)\
    (((app_env.fields) & (APPM_##field##_MASK)) >> APPM_##field)

/// appm configuration field
#define APPM_FIELD_SAVE_CLEAN()\
    app_env.fields_bck  =  app_env.fields;\
    app_env.fields = 0;


#define APPM_FIELD_RECOVER()\
    ( app_env.fields  =  app_env.fields_bck)
    
#define APPM_FIELD_CLEAN()\
    ( app_env.fields  =  0)



extern struct app_env_tag app_env;


//#ifdef UART_PRINTF_CLASS_DETAIL

extern struct p_appm_state appm_state_ins[];

//#endif







void appm_init(void);
bool appm_add_svc(void);
void appm_start_advertising(void);
void appm_stop_advertising(void);
void appm_update_param(struct gapc_conn_param *conn_param);
void appc_le_data_length_update_req(uint8_t conidx,uint16_t max_tx_octets,uint16_t max_tx_time);
void appc_gatt_mtu_change(uint8_t conidx);
void appm_disconnect(void);
uint8_t appm_get_dev_name(uint8_t* name);
void appm_scan_adv_con_schedule(void);
uint8_t appm_adv_data_decode_name(uint8_t len, const uint8_t *data,uint8_t *name_str);
void appm_update_adv_data( uint8_t* adv_buff, uint8_t adv_len, uint8_t* scan_buff, uint8_t scan_len);

#endif //(BLE_APP_PRESENT)

#endif // APP_H_
