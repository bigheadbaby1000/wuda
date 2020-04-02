#include "rwip_config.h" // RW SW configuration
#include "arch.h"      // architectural platform definitions
#include <stdlib.h>    // standard lib functions
#include <stddef.h>    // standard definitions
#include <stdint.h>    // standard integer definition
#include <stdbool.h>   // boolean definition
#include "rwip.h"      // RW SW initialization
#include "syscntl.h"   // System control initialization
#include "emi.h"       // EMI initialization
#include "icu.h"
#include "uart.h"
#include "att.h"
#include "BK3435_reg.h"

volatile uint32_t  *p_APB_XVER_ADDR = (volatile uint32_t*) APB_XVER_BASE  ;

void XVR_Initial(void)
{
	p_APB_XVER_ADDR[0x0]=0xC3A03210;
	p_APB_XVER_ADDR[0x1]=0x8295C100;
	p_APB_XVER_ADDR[0x2]=0x2F42AF00;
	p_APB_XVER_ADDR[0x3]=0x6A5C9C62;
	p_APB_XVER_ADDR[0x4]=0x7F915B33;
	p_APB_XVER_ADDR[0x5]=0x48285500;
	p_APB_XVER_ADDR[0x6]=0x88D68C00;
	p_APB_XVER_ADDR[0x7]=0x18809FE5;
	p_APB_XVER_ADDR[0x8]=0x2FB84005;
	p_APB_XVER_ADDR[0x9]=0x70203C08;
	p_APB_XVER_ADDR[0xA]=0x1C07ED35;
	p_APB_XVER_ADDR[0xB]=0xD3DD240C;
	p_APB_XVER_ADDR[0xC]=0x8000D008;
	p_APB_XVER_ADDR[0xD]=0x84413F23;
	p_APB_XVER_ADDR[0xE]=0x00309350;
	p_APB_XVER_ADDR[0xF]=0x3126E978;
	p_APB_XVER_ADDR[0x10]=0x00073435;
	p_APB_XVER_ADDR[0x11]=0x17520820;
	p_APB_XVER_ADDR[0x12]=0x00000100;
	p_APB_XVER_ADDR[0x13]=0x53000000;
	p_APB_XVER_ADDR[0x14]=0x00080000;
	p_APB_XVER_ADDR[0x15]=0x00000000;
	p_APB_XVER_ADDR[0x16]=0x00000000;
	p_APB_XVER_ADDR[0x17]=0x00000000;
	p_APB_XVER_ADDR[0x18]=0x000001FF;
	p_APB_XVER_ADDR[0x19]=0x00000000;
	p_APB_XVER_ADDR[0x1A]=0x00000000;
	p_APB_XVER_ADDR[0x1B]=0x00000000;
	p_APB_XVER_ADDR[0x1C]=0x00000000;
	p_APB_XVER_ADDR[0x1D]=0x00000000;
	p_APB_XVER_ADDR[0x1E]=0x00000000;
	p_APB_XVER_ADDR[0x1F]=0x00000000;
	p_APB_XVER_ADDR[0x20]=0x0295E5DC;
	p_APB_XVER_ADDR[0x21]=0x96000000;
	p_APB_XVER_ADDR[0x22]=0x78000000;
	p_APB_XVER_ADDR[0x23]=0xA0000000;
	p_APB_XVER_ADDR[0x24]=0x40000782;
	p_APB_XVER_ADDR[0x25]=0x00002400;
	p_APB_XVER_ADDR[0x26]=0x10202500;
	p_APB_XVER_ADDR[0x27]=0x0008C900;
	p_APB_XVER_ADDR[0x28]=0x01011010;
	p_APB_XVER_ADDR[0x29]=0x7C104E00;
	p_APB_XVER_ADDR[0x2A]=0x12083B6E;
	p_APB_XVER_ADDR[0x2B]=0x00000408;
	p_APB_XVER_ADDR[0x2D]=0x082AC441;
	p_APB_XVER_ADDR[0x2E]=0x00000000;
	p_APB_XVER_ADDR[0x2F]=0x00000000;
	p_APB_XVER_ADDR[0x30]=0x10010001;
	p_APB_XVER_ADDR[0x31]=0x00000000;
	p_APB_XVER_ADDR[0x32]=0x00000000;
	p_APB_XVER_ADDR[0x33]=0x00000000;
	p_APB_XVER_ADDR[0x34]=0x00000000;
	p_APB_XVER_ADDR[0x35]=0x00000000;
	p_APB_XVER_ADDR[0x36]=0x00000000;
	p_APB_XVER_ADDR[0x37]=0x00000000;
	p_APB_XVER_ADDR[0x38]=0x00000000;
	p_APB_XVER_ADDR[0x39]=0x00000000;
	p_APB_XVER_ADDR[0x3A]=0x00028000;
	p_APB_XVER_ADDR[0x3B]=0x00003048;
	p_APB_XVER_ADDR[0x3C]=0x01FF3C00;
	p_APB_XVER_ADDR[0x3D]=0x00000000;
	p_APB_XVER_ADDR[0x3E]=0x00000000;
	p_APB_XVER_ADDR[0x3F]=0x00000000;
	p_APB_XVER_ADDR[0x40]=0x01000000;
	p_APB_XVER_ADDR[0x41]=0x07050402;
	p_APB_XVER_ADDR[0x42]=0x120F0C0A;
	p_APB_XVER_ADDR[0x43]=0x221E1A16;
	p_APB_XVER_ADDR[0x44]=0x35302B26;
	p_APB_XVER_ADDR[0x45]=0x4B45403A;
	p_APB_XVER_ADDR[0x46]=0x635D5751;
	p_APB_XVER_ADDR[0x47]=0x7C767069;
	p_APB_XVER_ADDR[0x48]=0x968F8983;
	p_APB_XVER_ADDR[0x49]=0xAEA8A29C;
	p_APB_XVER_ADDR[0x4A]=0xC5BFBAB4;
	p_APB_XVER_ADDR[0x4B]=0xD9D4CFCA;
	p_APB_XVER_ADDR[0x4C]=0xE9E5E1DD;
	p_APB_XVER_ADDR[0x4D]=0xF5F3F0ED;
	p_APB_XVER_ADDR[0x4E]=0xFDFBFAF8;
	p_APB_XVER_ADDR[0x4F]=0xFFFFFFFE;

}

void Test_XVR_PN9_Rx_Ena(uint8_t channel)
{
	uint32_t wdata_temp;
	wdata_temp = (0xF<<7) + (2+channel*2) ;
	p_APB_XVER_ADDR[0x24] = wdata_temp;
	//recv_pn9=1,RX mode,radio power on
	p_APB_XVER_ADDR[0x25] = (0x1<<10);
	p_APB_XVER_ADDR[0x25] |= (0x1<<13);
}

void Test_XVR_Single_Carrier_Tx_Ena1(uint8_t channel, uint8_t tx_power)
{
	uint32_t wdata_temp;

	// Disable auto-chnn, auto-syncwin, auto-syncwrd
	wdata_temp = (tx_power<<7) + (channel*2+2) ;
	p_APB_XVER_ADDR[0x24]  = wdata_temp ; 
	// XVR-Reg0x25: [10]-pn9_recv_en, [11]-pn9_send_en, [12]-test_tmode, [13]-test_radio
	p_APB_XVER_ADDR[0x25]  = (0x1<<12) |(1<<11);
	p_APB_XVER_ADDR[0x25]  = (0x1<<13) |(0x1<<12)|(1<<11) ;
}

void LockPn9Test(int en_dis){
	if(en_dis){
		p_APB_XVER_ADDR[0x25] |= (1<<9);  //PN9 hold EN
	}else{
		p_APB_XVER_ADDR[0x25] &= ~(0x1 << 9);
	}
}

void GetBER(uint32_t*bc,uint32_t*be){
	*bc=(p_APB_XVER_ADDR[0x15]);
	*be=(p_APB_XVER_ADDR[0x16]);
}


void rw_pn9_enter(void)
{
	while(1)
	{

	}
}



static uint8_t tst_mode = 0;

uint8_t tx_rsp_buffer[7] = {0x04,0x0E,0x04,0x01,0x32,0x20,0x00};
uint8_t rx_rsp_buffer[7] = {0x04,0x0E,0x04,0x01,0x31,0x20,0x00};
uint8_t test_end_rsp_buffer[9] = {0x04,0x0E,0x06,0x01,0x33,0x20,0x00,0x00,0x00};
uint8_t error_cmd_rsp[7] = {0x04,0x0E,0x04,0x01,0x34,0x20,0x00};

void pn9_test_process(uint8_t* buffer, uint8_t len)
{
	uint8_t chnl;
	uint16_t ber;
	uint32_t bc, be;

	if((buffer[0] == 0x1) && (len >= 4))
	{
		XVR_Initial();
		if(buffer[1] == 0x31)  //RX
		{
			tst_mode = 1;
			chnl = buffer[4];
			Test_XVR_PN9_Rx_Ena(chnl);
			uart_send(rx_rsp_buffer, sizeof(rx_rsp_buffer));
		}
		else if(buffer[1] == 0x32)  //TX
		{
			tst_mode = 2;
			chnl = buffer[4];
			Test_XVR_Single_Carrier_Tx_Ena1(chnl, 0);
			uart_send(tx_rsp_buffer, sizeof(tx_rsp_buffer));
		}
		else if(buffer[1] == 0x33) //END
		{
			if(tst_mode == 0x1)
			{
				LockPn9Test(0x1);
				GetBER(&bc, &be);
				//uart_printf("bc = %d\r\n", bc);
				if(bc != 0)
				{
					ber = be*10000/bc;
					test_end_rsp_buffer[7] = LO_UINT16(ber);
					test_end_rsp_buffer[8] = HI_UINT16(ber);
				}
				else
				{
					test_end_rsp_buffer[7] = 0xfe;
					test_end_rsp_buffer[8] = 0xfe;
				}
				uart_send(test_end_rsp_buffer, sizeof(test_end_rsp_buffer));
				p_APB_XVER_ADDR[0x25] = 0;
			}
			else if(tst_mode == 0x2)
			{
				test_end_rsp_buffer[7] = 0x0;
				test_end_rsp_buffer[8] = 0x0;
				uart_send(test_end_rsp_buffer, sizeof(test_end_rsp_buffer));
				p_APB_XVER_ADDR[0x25] = 0;
			}
			else 
			{
				uart_send(error_cmd_rsp, sizeof(error_cmd_rsp));
			}
		}
	}
}

