#ifndef __LED_H
#define __LED_H

#include "sys.h"

//LED �˿ڶ���
#define LED0 PCout(14) // DS0
#define LED1 PCout(15)// DS1

void LED_Init(void);//��ʼ��

#endif