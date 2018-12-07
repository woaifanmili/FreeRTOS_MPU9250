#include "usart.h"


//����1����1���ַ� 
//c:Ҫ���͵��ַ�
void usart1_send_char(u8 data)
{
	while(__HAL_UART_GET_FLAG(&huart1,UART_FLAG_TC)==HAL_UART_STATE_RESET)
        ; 
    HAL_UART_Transmit(&huart1,&data,1,HAL_MAX_DELAY);   
} 

//�������ݸ�����������λ�����(V2.6�汾)
//fun:������. 0XA0~0XAF
//data:���ݻ�����,���28�ֽ�!!
//len:data����Ч���ݸ���
void usart1_niming_report(u8 fun,u8 *data,u8 len)
{
	u8 send_buf[32];
	u8 i;
  /*���28�ֽ�����*/
	if(len>28)  return;	   
	send_buf[len+3]=0;	//У��������
	send_buf[0]=0X88;	//֡ͷ
	send_buf[1]=fun;	//������
	send_buf[2]=len;	//���ݳ���
  /*��������,����У���*/
	for(i=0;i<len;i++)
    send_buf[3+i]=data[i];			
	for(i=0;i<len+3;i++)
    send_buf[len+3]+=send_buf[i];		
	/*�������ݵ�����1 */
  for(i=0;i<len+4;i++)
    usart1_send_char(send_buf[i]);	
}  

//���ͼ��ٶȴ��������ݺ�����������
//aacx,aacy,aacz:x,y,z������������ļ��ٶ�ֵ
//gyrox,gyroy,gyroz:x,y,z�������������������ֵ
void mpu9250_send_rawdata(IMUData_t imu)
{
	u8 tbuf[12]; 
	tbuf[0]=(imu.acc.x>>8)&0XFF;
	tbuf[1]=imu.acc.x&0XFF;
	tbuf[2]=(imu.acc.y>>8)&0XFF;
	tbuf[3]=imu.acc.y&0XFF;
	tbuf[4]=(imu.acc.z>>8)&0XFF;
	tbuf[5]=imu.acc.z&0XFF; 
	tbuf[6]=(imu.gyro.x>>8)&0XFF;
	tbuf[7]=imu.gyro.x&0XFF;
	tbuf[8]=(imu.gyro.y>>8)&0XFF;
	tbuf[9]=imu.gyro.y&0XFF;
	tbuf[10]=(imu.gyro.z>>8)&0XFF;
	tbuf[11]=imu.gyro.z&0XFF;
	usart1_niming_report(0XA1,tbuf,12);//�Զ���֡,0XA1
}	
//ͨ������1�ϱ���������̬���ݸ�����
//aacx,aacy,aacz:x,y,z������������ļ��ٶ�ֵ
//gyrox,gyroy,gyroz:x,y,z�������������������ֵ
//roll:�����.��λ0.01�ȡ� -18000 -> 18000 ��Ӧ -180.00  ->  180.00��
//pitch:������.��λ 0.01�ȡ�-9000 - 9000 ��Ӧ -90.00 -> 90.00 ��
//yaw:�����.��λΪ0.1�� 0 -> 3600  ��Ӧ 0 -> 360.0��
void usart1_report_imu(Triplet_t acc,Triplet_t gyro,short roll,short pitch,short yaw)
{
	u8 tbuf[28]; 
	u8 i;
	for(i=0;i<28;i++)tbuf[i]=0;//��0
	tbuf[0]=(acc.x>>8)&0XFF;
	tbuf[1]=acc.x&0XFF;
	tbuf[2]=(acc.y>>8)&0XFF;
	tbuf[3]=acc.y&0XFF;
	tbuf[4]=(acc.z>>8)&0XFF;
	tbuf[5]=acc.z&0XFF; 
	tbuf[6]=(gyro.x>>8)&0XFF;
	tbuf[7]=gyro.x&0XFF;
	tbuf[8]=(gyro.y>>8)&0XFF;
	tbuf[9]=gyro.y&0XFF;
	tbuf[10]=(gyro.z>>8)&0XFF;
	tbuf[11]=gyro.z&0XFF;	
	tbuf[18]=(roll>>8)&0XFF;
	tbuf[19]=roll&0XFF;
	tbuf[20]=(pitch>>8)&0XFF;
	tbuf[21]=pitch&0XFF;
	tbuf[22]=(yaw>>8)&0XFF;
	tbuf[23]=yaw&0XFF;
	usart1_niming_report(0XAF,tbuf,28);//�ɿ���ʾ֡,0XAF
} 

 




