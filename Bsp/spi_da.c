/**********************************************************************************
**********************************************************************************/
#include "spi_da.h"
#include "stm32f10x.h"

//delay 28ns------------------
static void delay(void)
{
  int i;
  for (i = 0; i < 2; i++)
    ;
}

//delay 1us------------------
static void delayus(void)
{
  int i;
  for (i = 0; i < 71; i++)
    ;
}

static void AD5754_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  //--------------Digital Output-------------------------------
  RCC_APB2PeriphClockCmd(SYNC_CLK, ENABLE);
  GPIO_InitStructure.GPIO_Pin = SYNC_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; //推挽输出
  GPIO_Init(SYNC_PORT, &GPIO_InitStructure);

  RCC_APB2PeriphClockCmd(SDIN_CLK, ENABLE);
  GPIO_InitStructure.GPIO_Pin = SDIN_PIN | LDAC_PIN | CLR_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; //推挽输出
  GPIO_Init(SDIN_PORT, &GPIO_InitStructure);

  //--------------Digital Input_-------------------------------
  RCC_APB2PeriphClockCmd(SDO_CLK, ENABLE);
  GPIO_InitStructure.GPIO_Pin = SDO_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(SDO_PORT, &GPIO_InitStructure);
}
/********************************************************************************
 *	@brief	控制端口初始化化
 *	@param	None
 *	@retval	None
 ********************************************************************************/
void AD5754_init(void)
{
  u8 DATx[3];
  u8 DARx[3];

  AD5754_GPIO_Init();

  delayus();
  CLR_H;
  LDAC_L;

  delay();
  DATx[0] = 0x0C; //设置量程
  DATx[1] = 0x00;
  DATx[2] = 0x04; //0:5v 1:10v 2:10.8v 3:+-5v 4:+-10v 5:+-10.8v
  AD5754_Write(DATx, DARx);

  delayus();
  DATx[0] = 0x10; //打开电源
  DATx[1] = 0x00;
  DATx[2] = 0x0F;
  AD5754_Write(DATx, DARx);

  delayus();
  DATx[0] = 0x04; //初始化输出为0v
  DATx[1] = 0x00;
  DATx[2] = 0x00;
  AD5754_Write(DATx, DARx);
}

/********************************************************************************
 *	@brief	控制端口初始化化
 *	@param	nChannel: 通道号
 *          sVal：需要写入的值
 *	@retval	读到的参数值
 ********************************************************************************/
void AD5754_Write(u8 *ptrTx, u8 *ptrRx)
{
  static u8 i, j;
  static u8 data;

  SYNC_L;
  SCLK_H;
  delay();

  for (j = 0; j < 3; j++)
  {
    data = *ptrTx;
    for (i = 0; i < 8; i++)
    {
      if (data & 0x80 == 0)
        SDIN_L;
      else
        SDIN_H;
      delay();
      SCLK_L;
      delay();

      if (SDO == 0)
        *ptrRx &= 0xFE;
      else
        *ptrRx |= 0x01;

      data <<= 1;
      *ptrRx <<= 1;
      SCLK_H;
    }
    ptrRx++;
    ptrTx++;
  }

  delay();
  SYNC_H;
  delay();
}

/*----------------------------------END OF FILE------------------------*/
