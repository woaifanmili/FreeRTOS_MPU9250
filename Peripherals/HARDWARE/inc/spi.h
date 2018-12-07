#ifndef __SPI_H
#define __SPI_H

#include "sys.h"

#define MPU_CS  PAout(15)
#define MPU_SPI  SPI3 
void SPI3_Init(void);
u8 SPI_RW( SPI_TypeDef* SPIx, u8 WriteByte );
void SPI_WriteByte( SPI_TypeDef* SPIx, u8 WriteByte );
u8 SPI_ReadByte( SPI_TypeDef* SPIx );
void MPU_WriteReg( uint8_t WriteAddr, uint8_t WriteData );
uint8_t MPU_ReadReg( uint8_t ReadAddr );
void MPU_WriteBuf( uint8_t WriteAddr, uint8_t *WriteBuf, uint8_t Bytes );
void MPU_ReadBuf( uint8_t ReadAddr, uint8_t *ReadBuf, uint8_t Bytes );
#endif
