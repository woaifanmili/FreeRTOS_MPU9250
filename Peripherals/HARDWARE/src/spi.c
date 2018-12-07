#include "spi.h"

//SPI 口初始化
//这里针是对 SPI3 的初始化

void SPI3_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef SPI_InitStructure;
    
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);//使能 GPIOC 时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);//使能 GPIOC 时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);// 使能 SPI3 时钟
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12;//PC10~12
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;//100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
    GPIO_Init(GPIOC, &GPIO_InitStructure);// 初始化
    
        
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;// CS PA15
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    //配置引脚复用映射
    GPIO_PinAFConfig(GPIOC,GPIO_PinSource10,GPIO_AF_SPI3); //PC10 复用为 SPI1
    GPIO_PinAFConfig(GPIOC,GPIO_PinSource11,GPIO_AF_SPI3); //PC11 复用为 SPI2
    GPIO_PinAFConfig(GPIOC,GPIO_PinSource12,GPIO_AF_SPI3); //PC12 复用为 SPI3
    
        //这里只针对 SPI 口初始化
 //   RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI3,ENABLE);//复位 SPI1
  //  RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI3,DISABLE);//停止复位 SPI1
    
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex; //设置 SPI 全双工
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master; //设置 SPI 工作模式:主 SPI
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b; //设置 SPI 的数据大小: 8 位帧结构
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;//串行同步时钟的空闲状态为高电平
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge; //数据捕获于第二个时钟沿
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft; //NSS 信号由硬件管理
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8; //预分频 256
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB; //数据传输从 MSB 位开始
    SPI_InitStructure.SPI_CRCPolynomial = 7; //CRC 值计算的多项式
    SPI_Init(SPI3, &SPI_InitStructure); //根据指定的参数初始化外设 SPIx 寄存器
    
    SPI_Cmd(SPI3, ENABLE); //使能 SPI1

}

/**********************************************************************
**函數 : SPI_RW
**功能 : Receive 1Byte Data
**輸入 : SPIx
**輸出 : None
**使用 : Read = SPI_RW(SPI1, 0xFF);
**********************************************************************/
u8 SPI_RW( SPI_TypeDef* SPIx, u8 WriteByte )
{
    while((SPIx->SR & SPI_I2S_FLAG_TXE) == (u16)RESET);
    SPIx->DR = WriteByte;
    while((SPIx->SR & SPI_I2S_FLAG_RXNE) == (u16)RESET);

    return SPIx->DR;
}

/**********************************************************************
**函數 : SPI_WriteByte
**功能 : Transmit 1Byte Data
**輸入 : SPIx, WriteByte
**輸出 : None
**使用 : SPI_WriteByte(SPI1, 0xFF);
**********************************************************************/
void SPI_WriteByte( SPI_TypeDef* SPIx, u8 WriteByte )
{
    while((SPIx->SR & SPI_I2S_FLAG_TXE) == (u16)RESET);
    SPIx->DR = WriteByte;
    while((SPIx->SR & SPI_I2S_FLAG_RXNE) == (u16)RESET);
    SPIx->DR;
}

/**********************************************************************
**函數 : SPI_ReadByte
**功能 : Receive 1Byte Data
**輸入 : SPIx
**輸出 : None
**使用 : Read = SPI_ReadByte(SPI1);
**********************************************************************/
u8 SPI_ReadByte( SPI_TypeDef* SPIx )
{
    while((SPIx->SR & SPI_I2S_FLAG_TXE) == (u16)RESET);
    SPIx->DR = 0xFF;
    while((SPIx->SR & SPI_I2S_FLAG_RXNE) == (u16)RESET);

    return SPIx->DR;
}


void MPU_WriteReg( uint8_t WriteAddr, uint8_t WriteData )
{
    MPU_CS = 0;
    SPI_WriteByte(MPU_SPI, WriteAddr);
    SPI_WriteByte(MPU_SPI, WriteData);
    MPU_CS = 1;
}

uint8_t MPU_ReadReg( uint8_t ReadAddr )
{
    uint8_t ReadData;
    
    MPU_CS = 0;
 
    ReadAddr|=0x80;//The first bit of the first byte:Read(1) Write(0)
    SPI_WriteByte(MPU_SPI, ReadAddr);
    ReadData = SPI_ReadByte(MPU_SPI);
    
    MPU_CS = 1;

    return ReadData;
}

void MPU_WriteBuf( uint8_t WriteAddr, uint8_t *WriteBuf, uint8_t Bytes )
{
    uint8_t i;

    MPU_CS = 0;

    SPI_WriteByte(MPU_SPI, WriteAddr);

    for(i=0; i<Bytes; i++)
        SPI_WriteByte(MPU_SPI, WriteBuf[i]);

    MPU_CS = 1;
}

void MPU_ReadBuf( uint8_t ReadAddr, uint8_t *ReadBuf, uint8_t Bytes )
{
    uint8_t i = 0;

    MPU_CS = 0;
    
    ReadAddr|=0x80;//The first bit of the first byte:Read(1) Write(0)
    SPI_WriteByte(MPU_SPI, ReadAddr);

    for(i=0; i<Bytes; i++)
        ReadBuf[i] = SPI_ReadByte(MPU_SPI);

    MPU_CS = 1;
}

