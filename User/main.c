#include "stm32f10x.h"
#include <math.h>

#include "..\bsp\bsp_led.h"
#include "..\bsp\SysTick.h"
#include "..\bsp\bsp_exti.h"
#include "..\bsp\bsp_innerflash.h"
#include "..\bsp\spi_da.h"

#include "modbus_svr.h"

#define DA_REG_ADR 110

//--------------------------------------------------------------------------------
int main(void)
{
	u8 DATx[3], DARx[3];
	u8 nChn = 0;

	SysTick_Init();
	InternalFlashRead(wReg, 200);
	BOOTNUM++;
	bSaved = 1;

	LED_GPIO_Config();
	Modbus_init();

	SetTimer(1, 500);  //喂狗服务程序定时器
	SetTimer(2, 1000); //参数定时保存进程
	SetTimer(3, 100);  //DA口定时刷新进程
	SetTimer(4, 100);

	AD5754_init();
	
	IWDG_Configuration();
	LED1_OFF;

	DATx[0] = 0x0C; //设置量程
	DATx[1] = 0x00;
	DATx[2] = 0x00;
	AD5754_Write(DATx, DARx);

	DATx[0] = 0x10; //打开电源
	DATx[1] = 0x00;
	DATx[2] = 0x0F;
	AD5754_Write(DATx, DARx);

	DATx[0] = 0x04; //初始化
	DATx[1] = 0x00;
	DATx[2] = 0x00;
	AD5754_Write(DATx, DARx);

	while (1)
	{
		Modbus_task();

		//------------------------------------------------------------------
		if (GetTimer(1))
		{
			LED1_TOGGLE;
			IWDG_Feed();
		}

		if (GetTimer(2) && bSaved)
		{
			InternalFlashWrite(wReg, 200);
			bSaved = 0;
		}

		if (GetTimer(3)) //DA输出
		{
			DATx[0] = nChn;
			DATx[1] = (wReg[DA_REG_ADR + nChn] & 0xFF00) >> 8;
			DATx[2] = wReg[DA_REG_ADR + nChn] & 0x00FF;
			AD5754_Write(DATx, DARx);
			nChn = (nChn + 1) % 4 ;
		}
	}
}
