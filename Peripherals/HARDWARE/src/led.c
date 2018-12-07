#include "led.h"
//初始化 PF9 和 PF10 为输出口.并使能这两个口的时钟
//LED IO 初始化
void LED_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);//使能 GPIOF 时钟
	//GPIOF9,F10 初始化设置
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14 | GPIO_Pin_15;//LED0 和 LED1 对应 IO 口
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
	GPIO_Init(GPIOC, &GPIO_InitStructure);//初始化 GPIOSTM32F4 开发指南(库函数版)
	GPIO_SetBits(GPIOC,GPIO_Pin_14 | GPIO_Pin_15);//GPIOF9,F10 设置高，灯灭
}
