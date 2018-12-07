#include "spi.h"

//SPI �ڳ�ʼ��
//�������Ƕ� SPI3 �ĳ�ʼ��

void SPI3_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef SPI_InitStructure;
    
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);//ʹ�� GPIOC ʱ��
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);//ʹ�� GPIOC ʱ��
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);// ʹ�� SPI3 ʱ��
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12;//PC10~12
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
    GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;//100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
    GPIO_Init(GPIOC, &GPIO_InitStructure);// ��ʼ��
    
        
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;// CS PA15
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    //�������Ÿ���ӳ��
    GPIO_PinAFConfig(GPIOC,GPIO_PinSource10,GPIO_AF_SPI3); //PC10 ����Ϊ SPI1
    GPIO_PinAFConfig(GPIOC,GPIO_PinSource11,GPIO_AF_SPI3); //PC11 ����Ϊ SPI2
    GPIO_PinAFConfig(GPIOC,GPIO_PinSource12,GPIO_AF_SPI3); //PC12 ����Ϊ SPI3
    
        //����ֻ��� SPI �ڳ�ʼ��
 //   RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI3,ENABLE);//��λ SPI1
  //  RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI3,DISABLE);//ֹͣ��λ SPI1
    
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex; //���� SPI ȫ˫��
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master; //���� SPI ����ģʽ:�� SPI
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b; //���� SPI �����ݴ�С: 8 λ֡�ṹ
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;//����ͬ��ʱ�ӵĿ���״̬Ϊ�ߵ�ƽ
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge; //���ݲ����ڵڶ���ʱ����
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft; //NSS �ź���Ӳ������
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8; //Ԥ��Ƶ 256
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB; //���ݴ���� MSB λ��ʼ
    SPI_InitStructure.SPI_CRCPolynomial = 7; //CRC ֵ����Ķ���ʽ
    SPI_Init(SPI3, &SPI_InitStructure); //����ָ���Ĳ�����ʼ������ SPIx �Ĵ���
    
    SPI_Cmd(SPI3, ENABLE); //ʹ�� SPI1

}

/**********************************************************************
**���� : SPI_RW
**���� : Receive 1Byte Data
**ݔ�� : SPIx
**ݔ�� : None
**ʹ�� : Read = SPI_RW(SPI1, 0xFF);
**********************************************************************/
u8 SPI_RW( SPI_TypeDef* SPIx, u8 WriteByte )
{
    while((SPIx->SR & SPI_I2S_FLAG_TXE) == (u16)RESET);
    SPIx->DR = WriteByte;
    while((SPIx->SR & SPI_I2S_FLAG_RXNE) == (u16)RESET);

    return SPIx->DR;
}

/**********************************************************************
**���� : SPI_WriteByte
**���� : Transmit 1Byte Data
**ݔ�� : SPIx, WriteByte
**ݔ�� : None
**ʹ�� : SPI_WriteByte(SPI1, 0xFF);
**********************************************************************/
void SPI_WriteByte( SPI_TypeDef* SPIx, u8 WriteByte )
{
    while((SPIx->SR & SPI_I2S_FLAG_TXE) == (u16)RESET);
    SPIx->DR = WriteByte;
    while((SPIx->SR & SPI_I2S_FLAG_RXNE) == (u16)RESET);
    SPIx->DR;
}

/**********************************************************************
**���� : SPI_ReadByte
**���� : Receive 1Byte Data
**ݔ�� : SPIx
**ݔ�� : None
**ʹ�� : Read = SPI_ReadByte(SPI1);
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

