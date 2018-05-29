#ifndef __FLASH_H
#define __FLASH_H


#define SPI_CS_LOW()        GPIO_ResetBits(GPIOA,GPIO_Pin_4)
#define SPI_CS_HIGH()       GPIO_SetBits(GPIOA,GPIO_Pin_4)
#define FLASH_RESET_LOW()   GPIO_ResetBits(GPIOC,GPIO_Pin_4)
#define FLASH_RESET_HIGH()  GPIO_SetBits(GPIOC,GPIO_Pin_4)

typedef enum {FAILED = 0, PASSED = !FAILED} TestStatus;

//SPI和GPIO的初始化
void SPI_GPIO_Init(void);
void DelayX(unsigned int time);
unsigned char SPI_WriteByte(unsigned char byte);
unsigned char SPI_ReadByte(void);

//读取FLASH的状态寄存器
unsigned char StatusRegisterRead(void);

/**************操作指令码定义******************************/
//读指令

#define Buffer_1_Read_Cmd 0xD4  //读取缓冲器1(速率最高为66MHz）
//#define Buffer_1_Read_Cmd 0xD1  //（速率最高为33MHz）

#define Buffer_2_Read_Cmd 0xD6  //读取缓冲器2
//#define Buffer_2_Read_Cmd 0xD3  //

#define Page_Read_Cmd     0xD2  //读取一页

#define Continuous_Array_Read_Cmd 0x0B  //读取连续主存
//#define Continuous_Array_Read_Cmd 0xE8
//#define Continuous_Array_Read_Cmd 0x03

#define Status_Read_Cmd   0xD7

//写指令

#define Buffer1_Write_Cmd 0x84
#define Buffer2_Write_Cmd 0x87

#define Buffer1ToMMPage_WithErase_Cmd  0x83
#define Buffer2ToMMPage_WithErase_Cmd  0x86
#define Buffer1ToMMPage_WithoutErase_Cmd  0x88
#define Buffer2ToMMPage_WithoutErase_Cmd  0x89
#define Page_Erase_Cmd  0x81
#define Block_Erase_Cmd 0x50
//#define Sector_Erase_Cmd  0x7C

//读数据功能函数

TestStatus Continuous_Array_Read(unsigned int PageAddr,unsigned int ByteAddr,unsigned char *DataBuffer,unsigned long ByteNum);
unsigned char DeviceInformation_Read(void);

//写数据功能函数

TestStatus  Buffer_Write(unsigned char BufferNumber,unsigned int ByteAddr,unsigned char *Data,unsigned int ByteNum);
TestStatus  BufferToMMPage_WithErase(unsigned char BufferNumber,unsigned int PageAddr);
//人工组合功能连续写
TestStatus  DataToPage_ViaBuffer_WithErase_OneWay(unsigned char BufferNumber,unsigned int PageAddr,unsigned int ByteAddr,unsigned char *Data,unsigned int ByteNum);
//指令方式连续写
TestStatus  DataToPage_ViaBuffer_WithErase_SecWay(unsigned char BufferNumber,unsigned int PageAddr,unsigned int ByteAddr,unsigned char *Data,unsigned int ByteNum);

#endif

