FreeRTOS_MPU9250
Sensor:MPU9250 using IIC
Microcontroller£ºSTM32F405RGT
OS£ºFreeRTOS V9.0.0
Communication protocol:USART CANopen301

Description:
1.Ported Inversense MPU-9250 DMP and FreeRTOS on the STM32f4 microcontroller using CubeMX
2.As for communication protocol, I have ported CANopen 301 using our previous code, only for SD0.
3.it can show on the zhengdianyuanzi niming shangweiji.
  
CANopen struct struct_CO_Data changed£º
1.CACAN_PORT CANInstance(used to be:canHandle)
    TIM_PORT Instance(used to be:can_timer_handle)
2.HAL lib macro define£º
        1).#define __HAL_CAN_DISABLE_IT(__HANDLE__, __INTERRUPT__) (((__HANDLE__)->CANInstance->IER) &= ~(__INTERRUPT__))
        used to be£º#define __HAL_CAN_DISABLE_IT(__HANDLE__, __INTERRUPT__) (((__HANDLE__)->Instance->IER) &= ~(__INTERRUPT__))
        2).#define __HAL_CAN_ENABLE_IT(__HANDLE__, __INTERRUPT__) (((__HANDLE__)->CANInstance->IER) |= (__INTERRUPT__))
        used to be£º#define __HAL_CAN_ENABLE_IT(__HANDLE__, __INTERRUPT__) (((__HANDLE__)->Instance->IER) |= (__INTERRUPT__))
        3).#define __HAL_CAN_CLEAR_FLAG(__HANDLE__, __FLAG__) \
  ((((__FLAG__) >> 8U) == 5U)? (((__HANDLE__)->CANInstance->TSR) = (1U << ((__FLAG__) & CAN_FLAG_MASK))): \
   (((__FLAG__) >> 8U) == 2U)? (((__HANDLE__)->CANInstance->RF0R) = (1U << ((__FLAG__) & CAN_FLAG_MASK))): \
   (((__FLAG__) >> 8U) == 4U)? (((__HANDLE__)->CANInstance->RF1R) = (1U << ((__FLAG__) & CAN_FLAG_MASK))): \
   (((__FLAG__) >> 8U) == 1U)? (((__HANDLE__)->CANInstance->MSR) = (1U << ((__FLAG__) & CAN_FLAG_MASK))): 0U)
        used to be£º#define __HAL_CAN_CLEAR_FLAG(__HANDLE__, __FLAG__) \
  ((((__FLAG__) >> 8U) == 5U)? (((__HANDLE__)->Instance->TSR) = (1U << ((__FLAG__) & CAN_FLAG_MASK))): \
   (((__FLAG__) >> 8U) == 2U)? (((__HANDLE__)->Instance->RF0R) = (1U << ((__FLAG__) & CAN_FLAG_MASK))): \
   (((__FLAG__) >> 8U) == 4U)? (((__HANDLE__)->Instance->RF1R) = (1U << ((__FLAG__) & CAN_FLAG_MASK))): \
   (((__FLAG__) >> 8U) == 1U)? (((__HANDLE__)->Instance->MSR) = (1U << ((__FLAG__) & CAN_FLAG_MASK))): 0U)
    Èý¡¢Other place using canHandle and can_timer_handle
    */
  