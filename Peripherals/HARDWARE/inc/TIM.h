#ifndef _TIM_H_
#define _TIM_H_

#include "stm32f4xx.h"

#define TIM_CANOPEN_SLAVE     TIM2
#define TIM_CANOPEN_MASTER    TIM4

#define TIM_CANopenSlaveIRQHandler   TIM2_IRQHandler
#define TIM_CANopenMasterIRQHandler  TIM4_IRQHandler

void TIM_CANopenSlaveConfig(void);
void TIM_CANopenMasterConfig(void);

#endif