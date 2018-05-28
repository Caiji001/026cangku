#include ""
#include "stm32f10x.h"
#include "stdio.h"
#include "flash.h"

#define BufferSize           (countof(Send_Data)-1) //Send_Data大小为所见字符数加一个潜在的结束符

#define countof(a) (sizeof(a) / sizeof(*(a)))

TestStatus Buffercmp(unsigned char* pBuffer1,unsigned char* pBuffer2,unsigned int BufferLength);
void RCC_Configuration(void);
void NVIC_Configuration(void);
unsigned char Send_Data[]="Welcome to join us";
unsigned char Receive_Data[BufferSize];

u8	testflash;


int main(void)
{  	volatile TestStatus flag=FAILED;
  

 // unsigned char ID;

  /* System clocks configuration */
  RCC_Configuration();
  
  /* NVIC configuration */
  NVIC_Configuration();
  
  SPI_GPIO_Init();
 
  //ID = DeviceInformation_Read(); //Atmel:0x1f(Test OK)
  //GPIO_Write(GPIOC,ID);
  
  //通过缓冲器2，从主存的第1页的第0字节开始写入数据
  //DataToPage_ViaBuffer_WithErase_OneWay(2,1,0,Send_Data,BufferSize);  //Test OK
  DataToPage_ViaBuffer_WithErase_SecWay(2,2,1,Send_Data,BufferSize); //Test OK
  Continuous_Array_Read(2,1,Receive_Data,BufferSize); 
  
  flag = Buffercmp(Send_Data,Receive_Data,BufferSize);
  
  if(flag==PASSED)
  {
    testflash=3; //读写成功
  }
  else
  {
    testflash=4;
  }  
  
  while (1)
  {
  }
}


/*******************************************************************************
* Function Name  : RCC_Configuration
* Description    : Configures the different system clocks.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RCC_Configuration(void)
{
  ErrorStatus HSEStartUpStatus;
  /* RCC system reset(for debug purpose) */
  RCC_DeInit();
  
  /* Enable HSE */
  RCC_HSEConfig(RCC_HSE_ON);
  
  /* Wait till HSE is ready */
  HSEStartUpStatus = RCC_WaitForHSEStartUp();
  
  if(HSEStartUpStatus==SUCCESS)
  {
     /* Enable Prefetch Buffer */
     FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
     
     /* Flash 2 wait state */
     FLASH_SetLatency(FLASH_Latency_2);
     
     /* HCLK = SYSCLK */
     RCC_HCLKConfig(RCC_SYSCLK_Div1);
     
     /* PCLK2 = HCLK */
     RCC_PCLK2Config(RCC_HCLK_Div1);
     
     /* PCLK1 = HCLK/2 */
     RCC_PCLK1Config(RCC_HCLK_Div2);
     
     /* PLLCLK = 8MHz * 9 = 72 MHz */
     RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);
     
     /* Enable PLL */
     RCC_PLLCmd(ENABLE);
     
     /* Wait till PLL is ready */
     while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
     {}
     
     /* Select PLL as system clock source */
     RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
     
     /* Wait till PLL is used as system clock source */
     while (RCC_GetSYSCLKSource() != 0x08)
     {}
     
  }
     /*Enable GPIOC for LED */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);//用作LED指示（看操作是否成功）
}  
     
/*******************************************************************************
* Function Name  : NVIC_Configuration
* Description    : Configures Vector Table base location.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void NVIC_Configuration(void)
{
#ifdef  VECT_TAB_RAM
  /* Set the Vector Table base location at 0x20000000 */
  NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0);
#else  /* VECT_TAB_FLASH  */
  /* Set the Vector Table base location at 0x08000000 */
  NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
#endif
}    
     
/*******************************************************************************
* Function Name  : Buffercmp
* Description    : Compares two buffers.
* Input          : - pBuffer1, pBuffer2: buffers to be compared.
*                : - BufferLength: buffer's length
* Output         : None
* Return         : PASSED: pBuffer1 identical to pBuffer2
*                  FAILED: pBuffer1 differs from pBuffer2
*******************************************************************************/
TestStatus Buffercmp(unsigned char* pBuffer1, unsigned char* pBuffer2, unsigned int BufferLength)
{
  while (BufferLength--)
  {
    if (*pBuffer1 != *pBuffer2)
    {
      return FAILED;
    }

    pBuffer1++;
    pBuffer2++;
  } 
   
  return PASSED;
}


