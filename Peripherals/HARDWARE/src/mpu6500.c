#include "mpu6500.h"
#include "sys.h"
#include "delay.h"
#include "iic.h"

//#define MPU6500_SPI  //通信方式选择SPI
#define MPU6500_I2C

int ACC_ORIGINAL_X,ACC_ORIGINAL_Y,ACC_ORIGINAL_Z;
int GYRO_ORIGINAL_X,GYRO_ORIGINAL_Y,GYRO_ORIGINAL_Z;

//初始化MPU6500
//返回值:0,成功
//    其他,错误代码
u8 MPU_Init(void)
{ 
	u8 res;
	IIC_Init();//初始化IIC总线
	MPU_Write_Byte(MPU_PWR_MGMT1_REG,0X80);	//复位MPU6050
    delay_ms(100);
	MPU_Write_Byte(MPU_PWR_MGMT1_REG,0X00);	//唤醒MPU6050 
	MPU_Set_Gyro_Fsr(3);					//陀螺仪传感器,±500dps
	MPU_Set_Accel_Fsr(0);					//加速度传感器,±2g
	MPU_Set_Rate(50);
//	MPU_Write_Byte(MPU_SIGPATH_RST_REG,0x07);//重置加速度计、陀螺仪、温度传感
	MPU_Write_Byte(MPU_INT_EN_REG,0X00);	//关闭所有中断
	MPU_Write_Byte(MPU_USER_CTRL_REG,0X00);	//I2C主模式关闭
	MPU_Write_Byte(MPU_FIFO_EN_REG,0X00);	//关闭FIFO
	MPU_Write_Byte(MPU_INTBP_CFG_REG,0X80);	//INT引脚低电平有效
	res=MPU_Read_Byte(MPU_DEVICE_ID_REG);
	if(res==0x71)//器件ID正确
	{
		MPU_Write_Byte(MPU_PWR_MGMT1_REG,0X01);	//设置CLKSEL,PLL X轴为参考
		MPU_Write_Byte(MPU_PWR_MGMT2_REG,0X00);	//加速度与陀螺仪都工作
		MPU_Set_Rate(50);						//设置采样率为50Hz
 	}
	else 
		return 1;
	return 0;
}
//设置MPU6500陀螺仪传感器满量程范围
//fsr:0,±250dps;1,±500dps;2,±1000dps;3,±2000dps
//返回值:0,设置成功
//    其他,设置失败 
u8 MPU_Set_Gyro_Fsr(u8 fsr)
{
	return MPU_Write_Byte(MPU_GYRO_CFG_REG,fsr<<3);//设置陀螺仪满量程范围  
}
//设置MPU6500加速度传感器满量程范围
//fsr:0,±2g;1,±4g;2,±8g;3,±16g
//返回值:0,设置成功
//    其他,设置失败 
u8 MPU_Set_Accel_Fsr(u8 fsr)
{
	return MPU_Write_Byte(MPU_ACCEL_CFG_REG,fsr<<3);//设置加速度传感器满量程范围  
}
//设置MPU6500的陀螺仪和温度传感器低通数字滤波器  
//lpf:数字低通滤波频率(Hz)
//返回值:0,设置成功
//    其他,设置失败 
u8 MPU_Set_LPF(u16 lpf)
{
	u8 data=0;
	if(lpf>=184)data=1;
	else if(lpf>=92)data=2;
	else if(lpf>=41)data=3;
	else if(lpf>=20)data=4;
	else if(lpf>=10)data=5;
	else data=6; 
	return MPU_Write_Byte(MPU_CFG_REG,data);//设置陀螺仪和温度传感器低通数字滤波器  
}
//设置MPU6500的加速度计低通数字滤波器  
//lpf:数字低通滤波频率(Hz)
//返回值:0,设置成功
//    其他,设置失败 
u8 MPU_Set_Acc_LPF(u16 lpf)
{
	u8 data=0;
	if(lpf>=184)data=1;
	else if(lpf>=92)data=2;
	else if(lpf>=41)data=3;
	else if(lpf>=20)data=4;
	else if(lpf>=10)data=5;
	else data=6; 
	return MPU_Write_Byte(MPU_ACCEL_CFG2_REG,data);//设置加速度计低通数字滤波器  
}

//设置MPU6500的采样率(假定Fs=1KHz)
//rate:4~1000(Hz)
//返回值:0,设置成功
//    其他,设置失败 
u8 MPU_Set_Rate(u16 rate)
{
	u8 data;
	if(rate>1000)rate=1000;
	if(rate<4)rate=4;
	data=1000/rate-1;
	data=MPU_Write_Byte(MPU_SAMPLE_RATE_REG,data);	//设置数字低通滤波器
 	return MPU_Set_LPF(rate/2);	//自动设置LPF为采样率的一半
}

//得到温度值
//返回值:温度值(扩大了100倍)
short MPU_Get_Temperature(void)
{
  u8 buf[2]; 
  short raw;
	float temp;
	MPU_Read_Len(MPU_ADDR,MPU_TEMP_OUTH_REG,2,buf); 
  raw=((u16)buf[0]<<8)|buf[1];  
  temp=36.53+((double)raw)/340;  
  return temp*100;;
}

//得到加速度计和陀螺仪的值（原始值）
//accData:加速度计x,y,z轴的原始读读数（带符号）
//gyroData:陀螺仪x,y,z轴的原始读数(带符号)
//返回值:0,成功
//    其他,错误代码
u8 MPU_ReadData(int16_t *accData,int16_t *gyroData)
{
  uint8_t buf[14],res;
  res=MPU_Read_Len(MPU_ADDR,MPU_ACCEL_XOUTH_REG,14,buf);
  if(res==0)
  {
    accData[0] = (buf[0] << 8) | buf[1];
    accData[1] = (buf[2] << 8) | buf[3];
    accData[2] = (buf[4] << 8) | buf[5];

    gyroData[0] = (buf[8] << 8) | buf[9];
    gyroData[1] = (buf[10] << 8) | buf[11];
    gyroData[2] = (buf[12] << 8) | buf[13];
  }
  ACC_ORIGINAL_X=(int)(accData[0]*1000/16384);
  ACC_ORIGINAL_Y=(int)(accData[1]*1000/16384);
  ACC_ORIGINAL_Z=(int)(accData[2]*1000/16384);
  GYRO_ORIGINAL_X=(int)(gyroData[0]*1000/65.5);
  GYRO_ORIGINAL_Y=(int)(gyroData[1]*1000/65.5);
  GYRO_ORIGINAL_Z=(int)(gyroData[2]*1000/65.5);
  return res;;
}

//得到陀螺仪值(原始值)
//gx,gy,gz:陀螺仪x,y,z轴的原始读数(带符号)
//返回值:0,成功
//    其他,错误代码
u8 MPU_Get_Gyroscope(short *gx,short *gy,short *gz)
{
    u8 buf[6],res;  
	res=MPU_Read_Len(MPU_ADDR,MPU_GYRO_XOUTH_REG,6,buf);
	if(res==0)
	{
		*gx=((u16)buf[0]<<8)|buf[1];  
		*gy=((u16)buf[2]<<8)|buf[3];  
		*gz=((u16)buf[4]<<8)|buf[5];
	} 	
    return res;;
}
//得到加速度值(原始值)
//gx,gy,gz:陀螺仪x,y,z轴的原始读数(带符号)
//返回值:0,成功
//    其他,错误代码
u8 MPU_Get_Accelerometer(short *ax,short *ay,short *az)
{
    u8 buf[6],res;  
	res=MPU_Read_Len(MPU_ADDR,MPU_ACCEL_XOUTH_REG,6,buf);
	if(res==0)
	{
		*ax=((u16)buf[0]<<8)|buf[1];  
		*ay=((u16)buf[2]<<8)|buf[3];  
		*az=((u16)buf[4]<<8)|buf[5];
	} 	
    return res;;
}
//mpu连续写
//addr:器件地址 
//reg:寄存器地址
//len:写入长度
//buf:数据区
//返回值:0,正常
//    其他,错误代码
u8 MPU_Write_Len(u8 addr,u8 reg,u8 len,u8 *buf)
{
	u8 i; 
#if defined MPU6500_I2C
    IIC_Start(); 
	IIC_Send_Byte((addr<<1)|0);//发送器件地址+写命令	
	if(IIC_Wait_Ack())	//等待应答
	{
		IIC_Stop();		 
		return 1;		
	}
    IIC_Send_Byte(reg);	//写寄存器地址
    IIC_Wait_Ack();		//等待应答
	for(i=0;i<len;i++)
	{
		IIC_Send_Byte(buf[i]);	//发送数据
		if(IIC_Wait_Ack())		//等待ACK
		{
			IIC_Stop();	 
			return 1;		 
		}		
	}    
    IIC_Stop();	
#elif	defined MPU6500_SPI
      MPU_WriteBuf(reg,buf,len);
//	  cs_low();
//		HAL_SPI_Transmit(&hspi3,&reg,1,0xffff);
//		for(i=0; i<len; i++)
//			HAL_SPI_Receive(&hspi3,&buf[i],1,0xffff);
//		cs_high();
#else
#error "Please define a communication protocol"
#endif
	return 0;	
} 
//mpu连续读
//addr:器件地址
//reg:要读取的寄存器地址
//len:要读取的长度
//buf:读取到的数据存储区
//返回值:0,正常
//    其他,错误代码
u8 MPU_Read_Len(u8 addr,u8 reg,u8 len,u8 *buf)
{ 
//	u8 i;
#if defined MPU6500_I2C
 	IIC_Start(); 
	IIC_Send_Byte((addr<<1)|0);//发送器件地址+写命令	
	if(IIC_Wait_Ack())	//等待应答
	{
		IIC_Stop();		 
		return 1;		
	}
    IIC_Send_Byte(reg);	//写寄存器地址
    IIC_Wait_Ack();		//等待应答
    IIC_Start();
	IIC_Send_Byte((addr<<1)|1);//发送器件地址+读命令	
    IIC_Wait_Ack();		//等待应答 
	while(len)
	{
		if(len==1)*buf=IIC_Read_Byte(0);//读数据,发送nACK 
		else *buf=IIC_Read_Byte(1);		//读数据,发送ACK  
		len--;
		buf++; 
	}    
    IIC_Stop();	//产生一个停止条件 
#elif defined MPU6500_SPI
//	  cs_low();
//    reg|=0x80;//The first bit of the first byte:Read(1) Write(0)
//		HAL_SPI_Transmit(&hspi3,&reg,1,0xffff);
//		for(i=0; i<len; i++)
//			HAL_SPI_Receive(&hspi3,&buf[i],1,0xffff);
//		cs_high();
    MPU_ReadBuf(reg,buf,len);
#else
#error "Please define a communication protocol"
#endif
	return 0;	
}
//mpu写一个字节 
//reg:寄存器地址
//data:数据
//返回值:0,正常
//    其他,错误代码
u8 MPU_Write_Byte(u8 reg,u8 data) 				 
{ 
#if defined MPU6500_I2C
    IIC_Start(); 
	IIC_Send_Byte((MPU_ADDR<<1)|0);//发送器件地址+写命令	
	if(IIC_Wait_Ack())	//等待应答
	{
		IIC_Stop();		 
		return 1;		
	}
    IIC_Send_Byte(reg);	//写寄存器地址
    IIC_Wait_Ack();		//等待应答 
	IIC_Send_Byte(data);//发送数据
	if(IIC_Wait_Ack())	//等待ACK
	{
		IIC_Stop();	 
		return 1;		 
	}		 
    IIC_Stop();	 
#elif defined MPU6500_SPI
    MPU_WriteReg(reg,data);
//    cs_low();
//    HAL_SPI_Transmit(&hspi3, &reg,1,0xffff);
//    HAL_SPI_Transmit(&hspi3, &data,1,0xffff);
//		cs_high();
#else
#error "Please define a communication protocol"
#endif
	return 0;
}
//mpu读一个字节 
//reg:寄存器地址 
//返回值:读到的数据
u8 MPU_Read_Byte(u8 reg)
{
	u8 res;
#if defined MPU6500_I2C	
    IIC_Start(); 
	IIC_Send_Byte((MPU_ADDR<<1)|0);//发送器件地址+写命令	
	IIC_Wait_Ack();		//等待应答 
    IIC_Send_Byte(reg);	//写寄存器地址
    IIC_Wait_Ack();		//等待应答
    IIC_Start();
	IIC_Send_Byte((MPU_ADDR<<1)|1);//发送器件地址+读命令	
    IIC_Wait_Ack();		//等待应答 
	res=IIC_Read_Byte(0);//读取数据,发送nACK 
    IIC_Stop();			//产生一个停止条件 
#elif defined MPU6500_SPI	
  res=MPU_ReadReg(reg);
//    cs_low();
//    reg|=0x80;//The first bit of the first byte:Read(1) Write(0)
//		HAL_SPI_Transmit(&hspi3,&reg,1,0xffff);
//		HAL_SPI_Receive(&hspi3,&res,1,0xffff);
//		cs_high();
#else
#error "Please define a communication protocol"	
#endif
	return res;		
}


