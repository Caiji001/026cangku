#include "stm32f10x_lib.h"
#include "flash.h"


void DelayX(unsigned int time)
{
  while(time--);
}

/**********************************************
功能描述：初始化SPI和GPIO
参数输入：无

    返回：无
***********************************************/
void SPI_GPIO_Init(void)
{
  SPI_InitTypeDef SPI_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
  
  /*Enable SPI1 and GPIO clocks*/
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE);
  
  /* Configure SPI1 pins: SCK, MOSI and  MISO*/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7 | GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; //推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  /*GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);*/
  
  /* Configure I/O for Flash Chip select and reset */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  
  /*GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);*/
  
  SPI_CS_HIGH();
  FLASH_RESET_LOW();
  DelayX(50);
  FLASH_RESET_HIGH();
  
  /* SPI1 configuration */
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  //SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;//SPI Mode 0,上升沿数据输入从机，下降沿数据从从机输出
  //SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;//SPI Mode 3,上升沿数据输入从机，下降沿数据从从机输出
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(SPI1, &SPI_InitStructure);

  /* Enable SPI1  */
  SPI_Cmd(SPI1, ENABLE); 
   
}
   
 /**********************************************
功能描述：通过SPI发送一个字节
参数输入：unsigned char byte欲发送的字节

返    回：无
***********************************************/
unsigned char SPI_WriteByte(unsigned char byte)
{
  /* Wait until the transmit buffer is empty */
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
  /* Send the byte */
  SPI_I2S_SendData(SPI1, byte);
 
  /* Wait to receive a byte */
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
  /* Return the byte read from the SPI bus */
  return SPI_I2S_ReceiveData(SPI1);

}

/**********************************************
功能描述：通过SPI读取一个字节
参数输入：无

返    回：收到的字节
***********************************************/
unsigned char SPI_ReadByte(void)
{
  return (SPI_WriteByte(0xff));
}

/**********************************************
功能描述：读取FLASH的状态
参数输入：无

返    回：状态寄存器的内容
***********************************************/
unsigned char StatusRegisterRead(void)
{
  unsigned char status;
  SPI_CS_LOW();
  SPI_WriteByte(Status_Read_Cmd);
  status = SPI_ReadByte();
  SPI_CS_HIGH();
  
  return status;
}

/**********************************************
功能描述：数据先传输到缓冲器再传到主存中的指定页（带内部檫除）
参数输入：unsigned char BufferNumber选择缓冲器（1或2）
          unsigned char PageAddr主存中指定页(0~4095)
          unsigned char ByteAddr页中指定字节地址(0~527）
          unsigned char *Data欲写入的数据
          unsigned char ByteNum写入数据字节数（1~528）

返    回：PASSED:成功     FAILED：失败
***********************************************/
TestStatus  DataToPage_ViaBuffer_WithErase_OneWay(unsigned char BufferNumber,
                                           unsigned int PageAddr,unsigned int ByteAddr,
                                           unsigned char *Data,unsigned int ByteNum)
{
  if((ByteNum<=(528-ByteAddr))&&(ByteNum>0))
  {
    Buffer_Write(BufferNumber,ByteAddr,Data,ByteNum);
    BufferToMMPage_WithErase(BufferNumber,PageAddr);
    return PASSED;
  } 
  return FAILED;
}


TestStatus  DataToPage_ViaBuffer_WithErase_SecWay(unsigned char BufferNumber,unsigned int PageAddr,unsigned int ByteAddr,
                                                   unsigned char *Data,unsigned int ByteNum)
 {
   unsigned int i;
   if((ByteNum<=(528-ByteAddr))&&(ByteNum>0))
   {
      while(!(StatusRegisterRead()&0x80));
      SPI_CS_LOW();
      switch(BufferNumber)
      {
      case 1: SPI_WriteByte(0x82);break;
      case 2: SPI_WriteByte(0x85);break;
      }
      SPI_WriteByte((unsigned char)(PageAddr>>6));
      SPI_WriteByte((unsigned char)((PageAddr<<2)|(ByteAddr>>8)));
      SPI_WriteByte((unsigned char)ByteAddr);
      for(i=0;i<ByteNum;i++)
      {
         SPI_WriteByte(Data[i]);
      }
      SPI_CS_HIGH();
      return PASSED;
  }
  return FAILED; 
}


/**********************************************
功能描述：将数据写入到缓冲器中
参数输入：unsigned char BufferNumber选择缓冲器缓冲器（1或2）
          unsigned int ByteAddr缓冲器中写入数据的起始地址(0~527)
          unsigned char *Data需写入的数据
          unsigned int ByteNum写入的总字节数(1~528)

返    回：PASSED：读取成功 FAILED：读取失败
***********************************************/
TestStatus  Buffer_Write(unsigned char BufferNumber,unsigned int ByteAddr,
                          unsigned char *Data,unsigned int ByteNum)
{
  unsigned int i;
  if(((528-ByteAddr)>=ByteNum)&&(ByteNum>0))//写入的数据量在缓冲器剩余的范围内
  {
    while(!(StatusRegisterRead()&0x80));
    SPI_CS_LOW();
    switch(BufferNumber)
    {
    case 1: SPI_WriteByte(Buffer1_Write_Cmd);break;
    case 2: SPI_WriteByte(Buffer2_Write_Cmd);break;
    }
    SPI_WriteByte(0x00);
    SPI_WriteByte((unsigned char)(ByteAddr>>8));
    SPI_WriteByte((unsigned char)ByteAddr);
    
    for(i=0;i<ByteNum;i++)
    {
      SPI_WriteByte(Data[i]);
    }
    SPI_CS_HIGH();
    return PASSED;
  }
  return FAILED;
}

/**********************************************
功能描述：带预檫除的缓冲器全部数据拷贝到主存中指定页(没有被檫除）
参数输入：unsigned char BufferNumber选择缓冲器（1或2）
          unsigned char PageAddr主存中页地址(0~4095)

返    回：PASSED:成功     FAILED：失败
***********************************************/
TestStatus  BufferToMMPage_WithErase(unsigned char BufferNumber,unsigned int PageAddr)
{
  if(PageAddr<4096)
  {
    while(!(StatusRegisterRead()&0x80));
    SPI_CS_LOW();
    switch(BufferNumber)
    {
    case 1: SPI_WriteByte(Buffer1ToMMPage_WithErase_Cmd);break;
    case 2: SPI_WriteByte(Buffer2ToMMPage_WithErase_Cmd);break;
    }
    SPI_WriteByte((unsigned char)(PageAddr>>6));
    SPI_WriteByte((unsigned char)(PageAddr<<2));
    SPI_WriteByte(0x00);
    SPI_CS_HIGH();
    return PASSED;
  }
  return FAILED; 
}

/**********************************************
功能描述:从指定的位置开始连续读出数据，直到CS信号的上升沿
参数输入：unsigned int PageAddr:页地址（0~4095）
          unsigned int ByteAddr:字节地址（0~527）
          unsigned char *DataBuffer:用于存放读出的数据
          unsigned long ByteNum:欲读出的数据字节数 
返    回：PASSED：读取成功 FAILED：读取失败
***********************************************/
TestStatus Continuous_Array_Read(unsigned int PageAddr,unsigned int ByteAddr,
                           unsigned char *DataBuffer,unsigned long ByteNum)
{
  unsigned long i;
  if(ByteNum<=2162688)//欲读取的数据在片内 ,范围：4096*528 Bytes
  {
    while(!(StatusRegisterRead()&0x80));
    SPI_CS_LOW();
    SPI_WriteByte(Continuous_Array_Read_Cmd);
    SPI_WriteByte((unsigned char)(PageAddr>>6));
    SPI_WriteByte((unsigned char)((PageAddr<<2)|(ByteAddr>>8)));
    SPI_WriteByte((unsigned char)ByteAddr);
    SPI_WriteByte(0x00); //使用命令03H时，不用再输入无用的数剧，但命令为E8H（加4个）或0BH（加1个）需加无用数据以完成读操作的初始化
    
    for(i=0;i<ByteNum;i++)
    {
      DataBuffer[i] = SPI_ReadByte();
    }
    SPI_CS_HIGH(); 
    
    return  PASSED;
  }
  return FAILED;
}

unsigned char DeviceInformation_Read(void)
{
   volatile unsigned char a,b,c,d; 
   while(!(StatusRegisterRead()&0x80));
   SPI_CS_LOW();
   SPI_WriteByte(0x9f);
   a = SPI_ReadByte();
   b = SPI_ReadByte();
   c = SPI_ReadByte();
   d = SPI_ReadByte();
   SPI_CS_HIGH();
   
   return(a);
}


