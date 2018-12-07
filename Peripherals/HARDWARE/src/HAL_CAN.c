#include "HAL_CAN.h"
#include "stm32f4xx_can.h"

int CAN1_Config(unsigned int bitrate)
{
	GPIO_InitTypeDef  		GPIO_InitStructure;
	CAN_InitTypeDef 		CAN_InitStructure;
	CAN_FilterInitTypeDef 	CAN_FilterInitStructure;
	NVIC_InitTypeDef 		NVIC_InitStructure;

	CAN_DeInit(CAN1);    

	/* GPIO clock enable */
	RCC_AHB1PeriphClockCmd(CAN1_GPIO_CLK, ENABLE);
	/* CANx Periph clock enable */
	RCC_APB1PeriphClockCmd(CAN1_CLK, ENABLE);

	GPIO_PinAFConfig(CAN1_GPIO_PORT,CAN1_RX_SOURCE,CAN1_AF_PORT);
	GPIO_PinAFConfig(CAN1_GPIO_PORT,CAN1_TX_SOURCE,CAN1_AF_PORT);

	/* Configure CAN pin: RX */
	//GPIO_PinRemapConfig(GPIO_Remap1_CAN1 , ENABLE);
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = CAN1_TX_PIN | CAN1_RX_PIN ; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_Init(CAN1_GPIO_PORT, &GPIO_InitStructure);

	/* CAN register init */
    //	CAN_DeInit(CAN1);/* 通过 RCC_APB1RSTR 寄存器来复位 CAN1 */
	CAN_StructInit(&CAN_InitStructure);

	/* CAN cell init */
	CAN_InitStructure.CAN_TTCM = DISABLE;
	CAN_InitStructure.CAN_ABOM = DISABLE;
	CAN_InitStructure.CAN_AWUM = DISABLE;
	CAN_InitStructure.CAN_NART = DISABLE;
	CAN_InitStructure.CAN_RFLM = DISABLE;
	CAN_InitStructure.CAN_TXFP = ENABLE;/* 优先级由请求顺序决定 */
	CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;
	
	/*CAN的时钟是APB1，APB1=42M ，CAN波特率=RCC_APNB1PeriphClock/(CAN_SJW+CAN_BS1+CAN_BS2)/CAN_Prescaler*/
    switch(bitrate)
	{
		case 1000000://1Mbps
			CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
			CAN_InitStructure.CAN_BS1 = CAN_BS1_9tq;
			CAN_InitStructure.CAN_BS2 = CAN_BS2_4tq;
			CAN_InitStructure.CAN_Prescaler = 3; 				
		break;			
		case 500000://500Kbps
			CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
			CAN_InitStructure.CAN_BS1 = CAN_BS1_9tq;
			CAN_InitStructure.CAN_BS2 = CAN_BS2_4tq;
			CAN_InitStructure.CAN_Prescaler = 6; 				
		break;
		case 250000://250Kbps
			CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
			CAN_InitStructure.CAN_BS1 = CAN_BS1_9tq;
			CAN_InitStructure.CAN_BS2 = CAN_BS2_4tq;
			CAN_InitStructure.CAN_Prescaler = 12; 				
		break;
		default:;
    }
	CAN_Init(CAN1, &CAN_InitStructure);
    
	CAN_FilterInitStructure.CAN_FilterNumber = 0;
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
	CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;
	CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;															   	
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0;
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
	CAN_FilterInit(&CAN_FilterInitStructure);
	
	CAN_ITConfig(CAN1,CAN_IT_FMP0, ENABLE);
	/* Configure one bit for preemption priority */
    //	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);
	/* Enable CAN1 RX0 interrupt IRQ channel */
	NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);	
	return 0;
}

int CAN2_Config(unsigned int bitrate)
{
	GPIO_InitTypeDef  		GPIO_InitStructure;
	CAN_InitTypeDef 		CAN_InitStructure;
	CAN_FilterInitTypeDef 	CAN_FilterInitStructure;
	NVIC_InitTypeDef 		NVIC_InitStructure;

	CAN_DeInit(CAN2);  
	return 0;
}    
