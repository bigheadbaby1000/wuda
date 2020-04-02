# BK3435 & BK3431Q (BLE)
## 目录
- [ke_initial](#ke_initial) 
- [timer](#timer) 
- [flash](#flash) 
- [](#) 
- [](#) 
- [](#) 

## ke_initial
- 在ke内核完成初始化以前，**不应该**使用任何API来操作BLE或者和ke相关的功能，比如ke_timer或者发起关闭广播等等。                 
ke初始化过程会多次进入app_task.c函数gapm_cmp_evt_handler中，通过打印param->operation可以观察到GAPM_RESET-->GAPM_SET_DEV_CONFIG-->GAPM_PROFILE_TASK_ADD并最终完成BLE数据库的初始化。
```
        case (GAPM_PROFILE_TASK_ADD):
        {
            if (!appm_add_svc())
            {   //初始化完成，设置BLE状态，开广播并开启测试timer
                ke_state_set(TASK_APP, APPM_READY);
                appm_start_advertising();
                ke_timer_set(APP_PERIOD_TIMER, TASK_APP, 10);
            }
        }
```

## timer
内核自带timer，通常对精度要求不高的情况下请使用此timer。  
**开启timer**：
```
ke_timer_set(APP_PERIOD_TIMER, TASK_APP, 100);  
```
其中参数100是时间，1代表10ms，100即1s间隔。  
APP_PERIOD_TIMER是task的handle，计数超时以后会触发函数，在app_task.c
```
 static int app_period_timer_handler(ke_msg_id_t const msgid,
											void *param,
											ke_task_id_t const dest_id,
											ke_task_id_t const src_id)
  {
	  UART_PRINTF("%s\r\n", __func__);
  //  ke_timer_set(APP_PERIOD_TIMER, TASK_APP, 100);   //继续开启timer
	  return KE_MSG_CONSUMED;
  }
```
**取消timer**:
```
ke_timer_clear(APP_PERIOD_TIMER, TASK_APP);
```
>1. 通常BLE芯片上电到app_task.c中的gapm_cmp_evt_handler，case:GAPM_PROFILE_TASK_ADD中就初始化完成，可以在这里添加timer的启动。
```
        case (GAPM_PROFILE_TASK_ADD):
        {
            if (!appm_add_svc())
            {
                ke_state_set(TASK_APP, APPM_READY);
                ke_timer_set(APP_PERIOD_TIMER, TASK_APP, 10);
            }
        }
```

>2. 如果timer不够，可以在app_task.c中的appm_default_state中注册更多的callback函数。

## flash
flash分布图  
![](/img/3435/flash.png)

- 客户可以使用的分区是0x42000-0x44000这8k的flash空间,0x41000-0x42000这4k是给nvds来用的，如果nvds没有使用，也是可以使用flash的读写函数来操作的，但在不使用nvds读写的情况下优先使用unused区域的flash。
- 对擦除的动作要格外小心，因为flash的擦除是4k对齐的，所以擦除的时候需要找对起始地址以免发生误擦除动作。
#### API
读取：
```
uint8_t flash_read(uint8_t flash_space, uint32_t address, uint32_t len, uint8_t *buffer, void (*callback)(void))
```
写入：
```
uint8_t flash_write(uint8_t flash_space, uint32_t address, uint32_t len, uint8_t *buffer, void (*callback)(void))
```
擦除：
```
uint8_t flash_erase(uint8_t flash_type, uint32_t address, uint32_t len, void (*callback)(void))
```
下面给出一段测试代码，在0x42000起始地址写入1024+64字节数据以后读出，并擦除1024字节后打印所有1024+64字节数据。
```
void flash_read_test(void)
{
	uint8_t wudabuf_wr[64];
	uint8_t wudabuf_rd[64] = {0};
	memset(wudabuf_wr, 5, 64);
	flash_erase_sector(0x42000);
	for(uint16_t i=0;i<17; i++)
		flash_write(0, 0x42000+64*i, 64, wudabuf_wr, NULL);
	UART_PRINTF("read flash test = \r\n");
	for(uint16_t k=0;k<17; k++)
	{
		flash_read(0, 0x42000+64*k, 64, wudabuf_rd, NULL);
		for(uint16_t ki=0;ki<64; ki++)
		{
			UART_PRINTF("%x ", wudabuf_rd[ki]);
		}
	}
	UART_PRINTF("\r\n");
	
	flash_erase(0, 0x42000, 1024, NULL);
	UART_PRINTF(" flash erase test = \r\n");
	for(uint16_t k=0;k<17; k++)
	{
		flash_read(0, 0x42000+64*k, 64, wudabuf_rd, NULL);
		for(uint16_t ki=0;ki<64; ki++)
		{
			UART_PRINTF("%x ", wudabuf_rd[ki]);
		}
	}
	UART_PRINTF("\r\n");
}
```


















