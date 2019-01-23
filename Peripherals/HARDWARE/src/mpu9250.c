#include "mpu9250.h"
#include "sys.h"
#include "delay.h"
#include "iic.h"
#include "inv_mpu.h"
#include "math.h"
//�õ��¶�ֵ
//����ֵ:�¶�ֵ(������100��)
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

//�õ����ٶȼƺ������ǵ�ֵ��ԭʼֵ��
//accData:���ٶȼ�x,y,z���ԭʼ�������������ţ�
//gyroData:������x,y,z���ԭʼ����(������)
//����ֵ:0,�ɹ�
//    ����,�������
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

//�õ�������ֵ(ԭʼֵ)
//gx,gy,gz:������x,y,z���ԭʼ����(������)
//����ֵ:0,�ɹ�
//    ����,�������
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
//�õ����ٶ�ֵ(ԭʼֵ)
//gx,gy,gz:������x,y,z���ԭʼ����(������)
//����ֵ:0,�ɹ�
//    ����,�������
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

//mpu����д
//addr:������ַ 
//reg:�Ĵ�����ַ
//len:д�볤��
//buf:������
//����ֵ:0,����
//    ����,�������
u8 MPU_Write_Len(u8 addr,u8 reg,u8 len,u8 *buf)
{
	u8 i; 
  IIC_Start(); 
	IIC_Send_Byte((addr<<1)|0);//����������ַ+д����	
	if(IIC_Wait_Ack())	//�ȴ�Ӧ��
	{
		IIC_Stop();		 
		return 1;		
	}
    IIC_Send_Byte(reg);	//д�Ĵ�����ַ
    IIC_Wait_Ack();		//�ȴ�Ӧ��
	for(i=0;i<len;i++)
	{
		IIC_Send_Byte(buf[i]);	//��������
		if(IIC_Wait_Ack())		//�ȴ�ACK
		{
			IIC_Stop();	 
			return 1;		 
		}		
	}    
  IIC_Stop();	
	return 0;	
} 
//mpu������
//addr:������ַ
//reg:Ҫ��ȡ�ļĴ�����ַ
//len:Ҫ��ȡ�ĳ���
//buf:��ȡ�������ݴ洢��
//����ֵ:0,����
//    ����,�������
u8 MPU_Read_Len(u8 addr,u8 reg,u8 len,u8 *buf)
{ 
 	IIC_Start(); 
	IIC_Send_Byte((addr<<1)|0);//����������ַ+д����	
	if(IIC_Wait_Ack())	//�ȴ�Ӧ��
	{
		IIC_Stop();		 
		return 1;		
	}
  IIC_Send_Byte(reg);	//д�Ĵ�����ַ
  IIC_Wait_Ack();		//�ȴ�Ӧ��
  IIC_Start();
	IIC_Send_Byte((addr<<1)|1);//����������ַ+������	
  IIC_Wait_Ack();		//�ȴ�Ӧ�� 
	while(len)
	{
		if(len==1)
      *buf=IIC_Read_Byte(0);//������,����nACK 
		else 
      *buf=IIC_Read_Byte(1);		//������,����ACK  
		len--;
		buf++; 
	}    
  IIC_Stop();	//����һ��ֹͣ���� 
	return 0;	
}
//mpuдһ���ֽ� 
//reg:�Ĵ�����ַ
//data:����
//����ֵ:0,����
//    ����,�������
u8 MPU_Write_Byte(u8 reg,u8 data) 				 
{ 
  IIC_Start(); 
	IIC_Send_Byte((MPU_ADDR<<1)|0);//����������ַ+д����	
	if(IIC_Wait_Ack())	//�ȴ�Ӧ��
	{
		IIC_Stop();		 
		return 1;		
	}
  IIC_Send_Byte(reg);	//д�Ĵ�����ַ
  IIC_Wait_Ack();		//�ȴ�Ӧ�� 
	IIC_Send_Byte(data);//��������
	if(IIC_Wait_Ack())	//�ȴ�ACK
	{
		IIC_Stop();	 
		return 1;		 
	}		 
  IIC_Stop();	 
	return 0;
}
//mpu��һ���ֽ� 
//reg:�Ĵ�����ַ 
//����ֵ:����������
u8 MPU_Read_Byte(u8 reg)
{
	u8 res;
  IIC_Start(); 
	IIC_Send_Byte((MPU_ADDR<<1)|0);//����������ַ+д����	
	IIC_Wait_Ack();		//�ȴ�Ӧ�� 
  IIC_Send_Byte(reg);	//д�Ĵ�����ַ
  IIC_Wait_Ack();		//�ȴ�Ӧ��
  IIC_Start();
	IIC_Send_Byte((MPU_ADDR<<1)|1);//����������ַ+������	
  IIC_Wait_Ack();		//�ȴ�Ӧ�� 
	res=IIC_Read_Byte(0);//��ȡ����,����nACK 
  IIC_Stop();			//����һ��ֹͣ���� 
	return res;		
}


