#include "TIM.h"

#define TIM_PVT_CLK 	    RCC_APB1Periph_TIM3
#define TIM_PVT_IRQn 	    TIM3_IRQn

#define TIM_CANOPEN_SLAVE_CLK       RCC_APB1Periph_TIM2
#define TIM_CANOPEN_SLAVE_IRQn 	    TIM2_IRQn

#define TIM_CANOPEN_MASTER_CLK      RCC_APB1Periph_TIM4
#define TIM_CANOPEN_MASTER_IRQn     TIM4_IRQn


// config TIM2 for CANopenSlave, TIM2_IRQn to deal with the events of CANopenSlave
void TIM_CANopenSlaveConfig(void)
{
	NVIC_InitTypeDef         NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(TIM_CANOPEN_SLAVE_CLK, ENABLE);

    TIM_TimeBaseStructure.TIM_Prescaler =84-1; /* CNT递增的单位为1微秒 */
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
		
    TIM_TimeBaseStructure.TIM_Period =0xFFFFFFFF;              
	
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM_CANOPEN_SLAVE, &TIM_TimeBaseStructure);

	TIM_ITConfig(TIM_CANOPEN_SLAVE, TIM_IT_Update, ENABLE);

    TIM_Cmd(TIM_CANOPEN_SLAVE, ENABLE);
	
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);

    NVIC_InitStructure.NVIC_IRQChannel = TIM_CANOPEN_SLAVE_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}


// config TIM4 for CANopenMaster, TIM4_IRQn to deal with the events of CANopenMaster
void TIM_CANopenMasterConfig(void)
{
	NVIC_InitTypeDef         NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(TIM_CANOPEN_MASTER_CLK, ENABLE);

    TIM_TimeBaseStructure.TIM_Prescaler =84-1; /* CNT递增的单位为1微秒 */
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	
    TIM_TimeBaseStructure.TIM_Period =0xFFFF;              
	
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM_CANOPEN_MASTER, &TIM_TimeBaseStructure);

	TIM_ITConfig(TIM_CANOPEN_MASTER, TIM_IT_Update, ENABLE);

    TIM_Cmd(TIM_CANOPEN_MASTER, ENABLE);
	
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);

    NVIC_InitStructure.NVIC_IRQChannel = TIM_CANOPEN_MASTER_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}



