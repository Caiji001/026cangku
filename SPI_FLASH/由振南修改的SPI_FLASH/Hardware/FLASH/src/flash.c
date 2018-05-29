#include "stm32f10x_lib.h"
#include "flash.h"


void DelayX(unsigned int time)
{
  while(time--);
}

/**********************************************
������������ʼ��SPI��GPIO
�������룺��

    ���أ���
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
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; //�������
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
  //SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;//SPI Mode 0,��������������ӻ����½������ݴӴӻ����
  //SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;//SPI Mode 3,��������������ӻ����½������ݴӴӻ����
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
����������ͨ��SPI����һ���ֽ�
�������룺unsigned char byte�����͵��ֽ�

��    �أ���
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
����������ͨ��SPI��ȡһ���ֽ�
�������룺��

��    �أ��յ����ֽ�
***********************************************/
unsigned char SPI_ReadByte(void)
{
  return (SPI_WriteByte(0xff));
}

/**********************************************
������������ȡFLASH��״̬
�������룺��

��    �أ�״̬�Ĵ���������
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
���������������ȴ��䵽�������ٴ��������е�ָ��ҳ�����ڲ��߳���
�������룺unsigned char BufferNumberѡ�񻺳�����1��2��
          unsigned char PageAddr������ָ��ҳ(0~4095)
          unsigned char ByteAddrҳ��ָ���ֽڵ�ַ(0~527��
          unsigned char *Data��д�������
          unsigned char ByteNumд�������ֽ�����1~528��

��    �أ�PASSED:�ɹ�     FAILED��ʧ��
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
����������������д�뵽��������
�������룺unsigned char BufferNumberѡ�񻺳�����������1��2��
          unsigned int ByteAddr��������д�����ݵ���ʼ��ַ(0~527)
          unsigned char *Data��д�������
          unsigned int ByteNumд������ֽ���(1~528)

��    �أ�PASSED����ȡ�ɹ� FAILED����ȡʧ��
***********************************************/
TestStatus  Buffer_Write(unsigned char BufferNumber,unsigned int ByteAddr,
                          unsigned char *Data,unsigned int ByteNum)
{
  unsigned int i;
  if(((528-ByteAddr)>=ByteNum)&&(ByteNum>0))//д����������ڻ�����ʣ��ķ�Χ��
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
������������Ԥ�߳��Ļ�����ȫ�����ݿ�����������ָ��ҳ(û�б��߳���
�������룺unsigned char BufferNumberѡ�񻺳�����1��2��
          unsigned char PageAddr������ҳ��ַ(0~4095)

��    �أ�PASSED:�ɹ�     FAILED��ʧ��
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
��������:��ָ����λ�ÿ�ʼ�����������ݣ�ֱ��CS�źŵ�������
�������룺unsigned int PageAddr:ҳ��ַ��0~4095��
          unsigned int ByteAddr:�ֽڵ�ַ��0~527��
          unsigned char *DataBuffer:���ڴ�Ŷ���������
          unsigned long ByteNum:�������������ֽ��� 
��    �أ�PASSED����ȡ�ɹ� FAILED����ȡʧ��
***********************************************/
TestStatus Continuous_Array_Read(unsigned int PageAddr,unsigned int ByteAddr,
                           unsigned char *DataBuffer,unsigned long ByteNum)
{
  unsigned long i;
  if(ByteNum<=2162688)//����ȡ��������Ƭ�� ,��Χ��4096*528 Bytes
  {
    while(!(StatusRegisterRead()&0x80));
    SPI_CS_LOW();
    SPI_WriteByte(Continuous_Array_Read_Cmd);
    SPI_WriteByte((unsigned char)(PageAddr>>6));
    SPI_WriteByte((unsigned char)((PageAddr<<2)|(ByteAddr>>8)));
    SPI_WriteByte((unsigned char)ByteAddr);
    SPI_WriteByte(0x00); //ʹ������03Hʱ���������������õ����磬������ΪE8H����4������0BH����1�������������������ɶ������ĳ�ʼ��
    
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


