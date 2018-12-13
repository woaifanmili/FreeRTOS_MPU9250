
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2018 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_hal.h"
#include "cmsis_os.h"

/* USER CODE BEGIN Includes */
#include "delay.h"
#include "iic.h"
#include "usart.h"

/*MPU9250 config and DMP lib port*/
#include "mpu9250.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"

/*CANopen301 port*/
#include "TestSlave.h"
#include "CANopenInit.h"
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
CAN_HandleTypeDef hcan1;
TIM_HandleTypeDef htim2;
UART_HandleTypeDef huart1;
osThreadId TaskIMUdataHandle;
osThreadId TaskUSARTTxHandle;
osThreadId TaskCANopenUpdateHandle;
osThreadId TaskCANRxHandle;
osThreadId TaskTimeDispatchHandle;
osMessageQId QueueUSARTTxHandle;
osMessageQId QueueCANopenUpdateHandle;
osMessageQId QueueCANRxHandle;
osSemaphoreId SemphoreTimeDispatchHandle;

/* Private variables ---------------------------------------------------------*/
extern int32_t EULER[];

int main(void)
{
  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();
  
  /* Configure the delay clock */
  delay_init(168);
  delay_ms(500);

  /* Initialize all configured peripherals */
  GPIO_Init();
  CAN1_Init();
  USART1_Init();
  
  /* Initialize CANopen and MPU9250 DMP */
  CanopenInit();
  mpu_dmp_init();

  /* Initialize FreeRTOS Thread, semphores, queue  */
  Freertos_Init();
 
  /* Start scheduler */
  osKernelStart();

  while (1)
  {}
}

void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

    /**Configure the main internal regulator output voltage 
    */
  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 15, 0);
}

void Freertos_Init(void)
{
  
  /* Create the rtos_semaphores*/
  osSemaphoreDef(SemphoreTimeDispatch);
  SemphoreTimeDispatchHandle = osSemaphoreCreate(osSemaphore(SemphoreTimeDispatch),1);
  
  /* Create the thread(s) */
  /* definition and creation of TaskIMUdata */
  osThreadDef(TaskIMUdata, FunctionIMUdata, osPriorityLow, 0, 128);
  TaskIMUdataHandle = osThreadCreate(osThread(TaskIMUdata), NULL);

  /* definition and creation of TaskUSARTTx */
  osThreadDef(TaskUSARTTx, FunctionUSARTTx, osPriorityBelowNormal, 0, 128);
  TaskUSARTTxHandle = osThreadCreate(osThread(TaskUSARTTx), NULL);

  /* definition and creation of TaskCANTx */
  osThreadDef(TaskCANopenUpdate, FunctionCANopenUpdate, osPriorityBelowNormal, 0, 128);
  TaskCANopenUpdateHandle = osThreadCreate(osThread(TaskCANopenUpdate), &EULER);

  /* definition and creation of TaskCANRx */
  osThreadDef(TaskCANRx, FunctionCANRx, osPriorityNormal, 0, 128);
  TaskCANRxHandle = osThreadCreate(osThread(TaskCANRx), NULL);

  /* definition and creation of TaskTimeDispatch */
  osThreadDef(TaskTimeDispatch, FunctionTimeDispatch, osPriorityNormal, 0, 128);
  TaskTimeDispatchHandle = osThreadCreate(osThread(TaskTimeDispatch), NULL);
  
  /* Create the queue(s) */
  /* definition and creation of QueueUSARTTx */
  osMessageQDef(QueueUSARTTx, 16, IMUData_t);
  QueueUSARTTxHandle = osMessageCreate(osMessageQ(QueueUSARTTx), NULL);

  /* definition and creation of QueueCANopenUpdate */
  osMessageQDef(QueueCANopenUpdate, 16, IMUData_t);
  QueueCANopenUpdateHandle = osMessageCreate(osMessageQ(QueueCANopenUpdate), NULL);
  
  /* definition and creation of QueueCANRx */
  osMessageQDef(QueueCANRx, 16, Message);
  QueueCANRxHandle = osMessageCreate(osMessageQ(QueueCANRx), NULL);
}

/* CAN1 init function */
static void CAN1_Init(void)
{

  hcan1.CANInstance = CAN1;
  hcan1.Init.Prescaler = 6;
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_9TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_4TQ;
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = DISABLE;
  hcan1.Init.AutoWakeUp = DISABLE;
  hcan1.Init.AutoRetransmission = DISABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }
  
  CAN_FilterTypeDef hcan1FilterConfig;
  
  hcan1FilterConfig.FilterActivation = ENABLE;
  hcan1FilterConfig.FilterFIFOAssignment = CAN_FILTER_FIFO0;
  hcan1FilterConfig.FilterIdHigh = 0x0000;
  hcan1FilterConfig.FilterIdLow = 0x0000;
  hcan1FilterConfig.FilterMaskIdHigh = 0x0000;
  hcan1FilterConfig.FilterMaskIdLow = 0x0000;
  hcan1FilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
  hcan1FilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
  if(HAL_CAN_ConfigFilter(&hcan1,&hcan1FilterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }  
  
  HAL_CAN_Start(&hcan1);
}

/* TIM2 init function */
static void MX_TIM2_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 84-1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 0XFFFFFFFF;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }
  
  HAL_TIM_Base_Start_IT(&htim2);
}

/* USART1 init function */
static void USART1_Init(void)
{

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 500000;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/** GPIO init function */
static void GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10|GPIO_PIN_11, GPIO_PIN_RESET);

  /*Configure GPIO pins : PC14 PC15 */
  GPIO_InitStruct.Pin = GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PB10 PB11 */
  GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}
void FunctionIMUdata(void const * argument)
{
  int16_t sLightcnt = 0;

  for(;;)
  {
    IMUData_t IMU;  
    taskENTER_CRITICAL();
    if(mpu_dmp_get_data(&IMU.Euler.pitch,&IMU.Euler.roll,&IMU.Euler.yaw) == 0)     //角度顺序有问题，尚不清楚原因，可能是芯片方向。约75ms可以采样到一次 
    {      
      /*sample raw data of temp,acc and gyro*/
      IMU.temp=MPU_Get_Temperature();	
      MPU_Get_Gyroscope(&IMU.gyro.x, &IMU.gyro.y, &IMU.gyro.z);	
      MPU_Get_Accelerometer(&IMU.acc.x, &IMU.acc.y, &IMU.acc.z);
      /*send data to usart queue*/
      xQueueSend(QueueUSARTTxHandle, &IMU, 10);
      xQueueSend(QueueCANopenUpdateHandle, &IMU, 10);       
      /*LED1 presents sample data working*/
      if(!(sLightcnt %= 10))
        HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_14);
      sLightcnt++;        
    } 
    taskEXIT_CRITICAL();    
    osDelay(1);
  }
}
void FunctionUSARTTx(void const * argument)
{
  BaseType_t xStatus;

  for(;;)
  {
    IMUData_t IMU;  
    /*the task will remain in the Blocked state to wait for data to be available*/
    xStatus=xQueueReceive(QueueUSARTTxHandle,&IMU,portMAX_DELAY);

    if(xStatus == pdPASS)
    {
      /* Data was successfully received from the queue*/
      mpu9250_send_rawdata(IMU);//用自定义帧发送加速度和陀螺仪原始数据
      usart1_report_imu(IMU.acc,IMU.gyro,(int)(-IMU.Euler.pitch*100),(int)(IMU.Euler.roll*100),(int)(IMU.Euler.yaw*10)); 
    }    
  }
}
void FunctionCANopenUpdate(void const * argument)
{
  BaseType_t xStatus;
  int32_t *euler = (int32_t *)argument;

  for(;;)
  {
    IMUData_t IMU; 
    /*the task will remain in the Blocked state to wait for data to be available*/
    xStatus=xQueueReceive(QueueCANopenUpdateHandle,&IMU,portMAX_DELAY);
    
    if(xStatus == pdPASS)
    {
      euler[0] = (int32_t)(IMU.Euler.pitch*100);
      euler[1] = (int32_t)(IMU.Euler.roll*100);
      euler[2] = (int32_t)(IMU.Euler.yaw*100);
    }      
  }
}
void FunctionCANRx(void const * argument)
{
  BaseType_t xStatus;

  for(;;)
  {
    Message m;  
    /*the task will remain in the Blocked state to wait for data to be available*/
    xStatus=xQueueReceive(QueueCANRxHandle, &m, portMAX_DELAY);

    if(xStatus == pdPASS)
    {
      /* Data was successfully received from the queue*/
      canDispatch(&TestSlave_Data, &m);
    }  
  }
}
void FunctionTimeDispatch(void const * argument)
{
  MX_TIM2_Init();
  BaseType_t xStatus;

  for(;;)
  {
    /*the task will remain in the Blocked state to wait for data to be available*/
    xStatus = xSemaphoreTake(SemphoreTimeDispatchHandle, portMAX_DELAY);

    if(xStatus == pdPASS)
    {    
      CANopen_slave_node();
    }
  }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{

  if (htim->Instance == TIM1) 
  {
    HAL_IncTick();
  }
  if (htim->Instance == TIM2) 
  {
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    xSemaphoreGiveFromISR(SemphoreTimeDispatchHandle, &xHigherPriorityTaskWoken); 
  }
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{ 
  /*Read CAN data and send to queue QueueCANRxHandle*/
  int i;
  CAN_RxHeaderTypeDef RxMessageHeader; 
  UNS8 Data[8];    
  Message m;
  
  portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

  HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxMessageHeader, Data);
  m.cob_id=RxMessageHeader.StdId;
    
  if(RxMessageHeader.RTR == CAN_RTR_REMOTE)
     m.rtr=1;
  else if(RxMessageHeader.RTR == CAN_RTR_DATA)
     m.rtr=0;
  m.len=RxMessageHeader.DLC;
  for(i = 0; i < RxMessageHeader.DLC; i++)
     m.data[i]=Data[i];
  
  xQueueSendFromISR(QueueCANRxHandle, &m, &xHigherPriorityTaskWoken);
}

void _Error_Handler(char *file, int line)
{
  /* User can add his own implementation to report the HAL error return state */
  while(1)
  {
  }
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
