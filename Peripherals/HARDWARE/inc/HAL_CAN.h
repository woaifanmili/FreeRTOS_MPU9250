#ifndef CHIP_HAL_CAN_H__
#define CHIP_HAL_CAN_H__

#define CAN1_CLK                    RCC_APB1Periph_CAN1
#define CAN1_RX_PIN                 GPIO_Pin_11
#define CAN1_TX_PIN                 GPIO_Pin_12
#define CAN1_GPIO_PORT              GPIOA
#define CAN1_GPIO_CLK               RCC_AHB1Periph_GPIOA
#define CAN1_AF_PORT                GPIO_AF_CAN1
#define CAN1_RX_SOURCE              GPIO_PinSource11
#define CAN1_TX_SOURCE              GPIO_PinSource12 

int CAN1_Config(unsigned int bitrate);
int CAN2_Config(unsigned int bitrate);	

#endif
