/*
This file is part of CanFestival, a library implementing CanOpen Stack.

Copyright (C): Edouard TISSERANT and Francis DUPIN

See COPYING file for copyrights details.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef __timer_h__
#define __timer_h__

#include <timerscfg.h>
#include <applicfg.h>

#define TIMER_HANDLE INTEGER16


#include "data.h"

/* --------- types and constants definitions --------- */
#define TIMER_FREE 0
#define TIMER_ARMED 1
#define TIMER_TRIG 2
#define TIMER_TRIG_PERIOD 3

#define TIMER_NONE -1

//#define TIM_CANOPEN_SLAVE_CLK 	RCC_APB1Periph_TIM2
//#define TIM_CANOPEN_SLAVE			TIM2
//#define TIM_CANOPEN_SLAVE_IRQn 	TIM2_IRQn



//typedef void (*TimerCallback_t)(CO_Data* d, UNS32 id);

void TimeDispatch(CO_Data* d);

/**
 * @ingroup timer
 * @brief Set a timerfor a given time.
 * @param value The time value.
 */
void setTimer(TIMEVAL value, CO_Data* d);

/**
 * @ingroup timer
 * @brief Get the time elapsed since latest timer occurence.
 * @return time elapsed since latest timer occurence
 */
TIMEVAL getElapsedTime(CO_Data* d);

void TIM_CANopenSlaveStart(void);
void TIM_CANopenMasterStart(void);

#endif /* #define __timer_h__ */
