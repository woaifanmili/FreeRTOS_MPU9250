#include "mpu9250.h"
#include "sys.h"
#include "delay.h"
#include "iic.h"
#include "inv_mpu.h"
#include "math.h"
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

u8 MPU_Get_Heading(short *mx, short *my, short *mz,double *heading)
{
  u8 res;  
  short buf[3];
  unsigned long sensor_timestamp;
  float mag_sensitivity=0.15;
  volatile float mag;
	res=mpu_get_compass_reg(buf,&sensor_timestamp);
	if(res==0)
	{
    *mx=((long)buf[0]*mag_sensitivity);
    *my=((long)buf[1]*mag_sensitivity);
    *mz=((long)buf[2]*mag_sensitivity); 
    *heading =	atan2( 
									(double) (   (int16_t)   (*mx +0)   ),
									(double) (   (int16_t)   (*my -0)  )
								)*(180/3.14159265)+180;
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
		if(len==1)
      *buf=IIC_Read_Byte(0);//读数据,发送nACK 
		else 
      *buf=IIC_Read_Byte(1);		//读数据,发送ACK  
		len--;
		buf++; 
	}    
  IIC_Stop();	//产生一个停止条件 
	return 0;	
}
//mpu写一个字节 
//reg:寄存器地址
//data:数据
//返回值:0,正常
//    其他,错误代码
u8 MPU_Write_Byte(u8 reg,u8 data) 				 
{ 
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
	return 0;
}
//mpu读一个字节 
//reg:寄存器地址 
//返回值:读到的数据
u8 MPU_Read_Byte(u8 reg)
{
	u8 res;
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
	return res;		
}


