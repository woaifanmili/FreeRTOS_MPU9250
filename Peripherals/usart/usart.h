#ifndef _USART_H
#define _USART_H
#include "sys.h"
#include "stdio.h"	
#include "stm32f4xx_hal_conf.h"

#include "delay.h"
#include "mpu6500.h"

extern UART_HandleTypeDef huart1;

/*发送数据到匿名上位机*/
void usart1_send_char(u8 data);
void usart1_niming_report(u8 fun,u8*data,u8 len);
void mpu9250_send_rawdata(IMUData_t imu);
void usart1_report_imu(Triplet_t acc,Triplet_t gyro,short roll,short pitch,short yaw);


#endif
