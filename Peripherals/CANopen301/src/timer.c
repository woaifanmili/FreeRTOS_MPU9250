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
/*!
** @file   timer.c
** @author Edouard TISSERANT and Francis DUPIN
** @date   Tue Jun  5 09:32:32 2007
**
** @brief
**
**
*/

/* #define DEBUG_WAR_CONSOLE_ON */
/* #define DEBUG_ERR_CONSOLE_ON */

#include <applicfg.h>
#include "timer.h"
#include "TestSlave.h"
#include "CANopenMaster.h"
#include "TIM.h"
//#include "stm32f4xx_hal_tim.h"

int watch_overrun=0;

#define min_val(a,b) ((a<b)?a:b)


/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  setTimer
  * @param  value:Set time value 0x0000-0xffff
  * @retval NONE
  */
/* 设置ARR的值，让定时器在value微秒之后进入中断服务函数 */
void setTimer(TIMEVAL value, CO_Data* d)
{
	uint32_t timer = __HAL_TIM_GET_COUNTER(d);
	__HAL_TIM_SET_AUTORELOAD(d, (value + timer) % (0xFFFFFFFF));
}
/**
  * @brief  getElapsedTime
  * @param  NONE
	* @retval TIMEVAL:Return current timer value
  */
/* 计算距离进入中断过去了多长时间 */
TIMEVAL getElapsedTime(CO_Data* d)
{
	uint32_t timer = __HAL_TIM_GET_COUNTER(d);
    
    if(timer < d->last_time_set)
    {
        d->getElapsedTime_Error++;
    }

	return timer > d->last_time_set ? timer - d->last_time_set : d->last_time_set - timer;
}
/*!
** -------  Use this to declare a new alarm ------
**
** @param d
** @param id
** @param callback
** @param value
** @param period
**
** @return  row_number：说明申请到了row_number号结构体;
** @return  TIMER_NONE: 说明所有结构体都有事件占用着;
**/
int SetAlarm_TargetReachedAlarm=0;
int set_last_timer_raw = 0;
int set_last_timer_raw_other = 0;
int settimer_bug = 0;

int FollowingErrorAlarm_settimerbug = 0;
int TargetReachedAlarm_settimerbug = 0;
//int SyncAlarm_settimerbug = 0;
int SDOTimeoutAlarm_settimerbug = 0;

UNS32 set_clock_idle_times     = 0;
UNS32 set_clocK_start_times    = 0;
UNS32 set_clock_high_times     = 0;
UNS32 set_clock_low_times      = 0;
UNS32 set_clock_monoflop_times = 0;


TIMER_HANDLE SetAlarm(CO_Data* d, UNS32 id, TimerCallback_t callback, TIMEVAL value, TIMEVAL period)//#define TIMEVAL UNS32
{
    __HAL_CAN_DISABLE_IT(d,CAN_IT_RX_FIFO0_MSG_PENDING);//该函数不允许被中断打断
    
	TIMER_HANDLE row_number;//#define TIMER_HANDLE INTEGER16
	s_timer_entry *row;
    

	/* in order to decide new timer setting we have to run over all timer rows */
	/* 遍历所有事件入口 *//* 可能会提前结束，找到row->state == TIMER_FREE的row为止 */
	for(row_number=0, row=d->timers; (row_number <= d->last_timer_raw + 1) && (row_number < MAX_NB_TIMER); row_number++, row++)
	{
		/* 事件入口没有被占用，即将该事件添加进入口 */
		if (callback && 	/* if something to store */
		   row->state == TIMER_FREE) /* and empty row *//* 找到第一个row->state == TIMER_FREE的row给用上 */
		{	/* just store */
			TIMEVAL real_timer_value;//该变量就在本函数中使用
			TIMEVAL elapsed_time;//该变量就在本函数中使用
            /* 更新一下最大事件入口号 */
			if (row_number == d->last_timer_raw + 1)
            {
				d->last_timer_raw++; /* last_timer_raw在SetAlarm函数中加1，在DelAlarm函数中减一（减是有条件的，删除的是结构体编号最大的事件才会减），初值为-1 */
                                    /* 就像这里加也是有条件的，只有要用编号最大的那个结构体才会加，因为DelAlarm删的话删的是中间的不会减，到这里的话就会填坑，所以也不用增 */     
//                if((d->last_timer_raw == -1) && (gMC.target_position_reached_timer == 0))
//                {
//                    settimer_bug++; // used to position bug in previous version, not used now.
//                }
            }

			/* 获取定时器已经过去的时间 */
			elapsed_time = getElapsedTime(d);
			/* set next wakeup alarm if new entry is sooner than others, or if it is alone */
			/* 该事件多久之后触发 */
			real_timer_value = value;
			/* 不得超过最大时间 */
			real_timer_value = min_val(real_timer_value, TIMEVAL_MAX);

			/* 该事件触发时间小于其他所有事件的触发时间，则将定时器的触发时间更新为该事件的触发时间 */
			if (d->total_sleep_time > elapsed_time && d->total_sleep_time - elapsed_time > real_timer_value)
			{
				/* 定时器休眠到计数值为total_sleep_time时触发 */
				d->total_sleep_time = elapsed_time + real_timer_value;/* 同一个时间轴上，哪一个是最紧急的，total_sleep_time就被赋值为哪一个 */
				/* 设置定时器在real_timer_value微秒之后，即计数值为total_sleep_time时触发 */
				setTimer(real_timer_value, d);
			}
			/* 设置该事件的回调函数 */
			row->callback = callback;
			/* 设置该事件的对象字典 */
			row->d = d;
			/* 设置该回调函数id */
			row->id = id;
			/* 触发时间点 */
			row->val = value + elapsed_time;
			/* 周期触发，0为非周期触发 */
			row->interval = period;
			/* 该入口被事件占用 */
			row->state = TIMER_ARMED;
			return row_number;
		}
	}

	return TIMER_NONE;
}

/*!
**  -----  Use this to remove an alarm ----
**
** @param handle
**
** @return TIMER_NONE
**/
/* 从定时器入口中删除一个事件，如果该事件删除过，又调用该函数删除一次（一般不会这样，删一次就行了嘛），就只会执行到最后一句（return TIMER_NONE;）*/
/* 如果是第一次删除之前设置过的事件，就会执行到该函数的所有代码（因为设置过的事件handle不会是TIMER_NONE，即-1，只会是0~7这些整数，因为调用
SetAlarm设置定时器事件的时候函数的返回值就是他们申请到的结构体编号） */
//int DelAlarm_TargetReachedAlarm = 0;
//int delete_last_timer_raw = 0;
//int delete_last_timer_raw_other = 0;
//int deltimer_bug = 0;

//int FollowingErrorAlarm_deltimerbug = 0;
//int TargetReachedAlarm_deltimerbug = 0;
//int SyncAlarm_deltimerbug = 0;
//int SDOTimeoutAlarm_deltimerbug = 0;

int bugle = 0;
TIMER_HANDLE DelAlarm(TIMER_HANDLE handle,TimerCallback_t callback, CO_Data* d)
{
    __HAL_CAN_DISABLE_IT(d,CAN_IT_RX_FIFO0_MSG_PENDING);//该函数不允许被中断打断
	/* Quick and dirty. system timer will continue to be trigged, but no action will be preformed. */
	MSG_WAR(0x3320, "DelAlarm. handle = ", handle);
	if(handle != TIMER_NONE)/*#define TIMER_NONE -1 设置过的定时器事件都能满足这个条件，因为SetAlarm的返回值为结构体编号（本工程为0~7） */
	{

				/* 更新一下最大的事件入口号 */
				if(handle == d->last_timer_raw)
        {
            d->last_timer_raw--;
            
        }
			
		/* 该事件入口设为空闲状态 */
		//timers[handle].state = TIMER_FREE;
		d->timers[handle].state = TIMER_FREE;
	}
	return TIMER_NONE;
}

/*!
** ------  TimeDispatch is called on each timer expiration ----
**
**/
//int tdcount=0;
/* 扫描所有事件是否需要触发 */
void TimeDispatch(CO_Data* d)
{
	TIMER_HANDLE i;
	/* 下一次触发时间 */
	TIMEVAL next_wakeup = TIMEVAL_MAX + 500; /* used to compute when should normaly occur next wakeup */
	/* First run : change timer state depending on time */
	/* Get time since timer signal */
	/* 从定时器中断到程序运行到这里一共运行的时间，用于校准定时器计数值 */
	UNS32 overrun = (UNS32)getElapsedTime(d);
    /* 当前定时器已经休眠的时间 */
	TIMEVAL real_total_sleep_time = d->total_sleep_time + overrun;

	s_timer_entry *row;

	/* 遍历所有事件入口 */
	for(i=0, row = d->timers; i <= d->last_timer_raw; i++, row++)
	{
		/* 如果该入口被事件占用 */
		if (row->state & TIMER_ARMED) /* if row is active */
		{
			/* 到了触发时间点 *//* Flaxin其实是到了这一个事件的触发时间，只是程序运行需要时间，运行到这里row->val刚好会比real_total_sleep_time小overrun个数 */
			if (row->val <= real_total_sleep_time) /* to be trigged */
			{
				/* 如果不是周期性事件 */
				if (!row->interval) /* if simply outdated */
				{
					/* 请求触发 */
					row->state = TIMER_TRIG; /* ask for trig */
				}
				/* 如果是周期性事件 ，则计算下一次触发的时间点（需要考虑时间误差overrun）*/
				else /* or period have expired */
				{
					/* set val as interval, with 32 bit overrun correction, */
					/* modulo for 64 bit not available on all platforms     */
					/* 把interval的校正值设为val */
					row->val = row->interval - (overrun % (UNS32)row->interval);
					/* 请求周期性触发 */
					row->state = TIMER_TRIG_PERIOD; /* ask for trig, periodic */
					/* Check if this new timer value is the soonest */
					/* 更新下一次唤醒的最短时间 */
					if(row->val < next_wakeup)
						next_wakeup = row->val;/* 所有的定时器事件结构体都这么处理一遍，next_wakeup就成为离下一次触发时间最短的事件的时间 */
				}
			}
			/* 还没到触发时间，计算离触发还剩余多长时间 */
			else
			{
				/* Each armed timer value in decremented. */
				/* 减去已经过去的时间*/
				row->val -= real_total_sleep_time;

				/* Check if this new timer value is the soonest */
				/* 更新下一次触发时间 */
				if(row->val < next_wakeup)
					next_wakeup = row->val;/* 所有的定时器事件结构体都这么处理一遍，next_wakeup就成为离下一次触发时间最短的事件的时间 */
			}
		}
	}
	/* Remember how much time we should sleep. */
	/* 定时器休眠到计数值为total_sleep_time时触发 */
	d->total_sleep_time = next_wakeup;
    /* total_sleep_time和next_wakeup的作用区别：
	total_sleep_time是用来判断哪个事件是最紧急的，创建定时器的时候就可以看出来，即最紧急事件的触发时间点
	next_wakeup用来判断这些时间剩余的时间哪个最短，即剩余时间最短事件的剩余时间*/
	/* Set timer to soonest occurence */
	/* 设置定时器 */
	setTimer(next_wakeup, d);/* 为了避免这里在还没有定时器事件设置时，ARR因为溢出会是一个很小的值 */

	/* Then trig them or not. */
	/* 遍历所有入口状态，判断是否触发回调函数 */
	for(i=0, row = d->timers; i <= d->last_timer_raw; i++, row++)
	{
		if (row->state & TIMER_TRIG)
		{
			/* 更新这个状态，单次触发就会变为空闲，周期触发还是周期触发 */
			row->state &= ~TIMER_TRIG; /* reset trig state (will be free if not periodic) */
			/* 触发 */
			if(row->callback)
            {
                (*row->callback)(row->d, row->id); /* trig ! */
            }
		}
	}
}

//// IRQ: CANopen_slave_node
void canTransmit(void)
{
		TestSlave_Data.last_time_set = __HAL_TIM_GET_COUNTER(&TIM_CANOPEN_SLAVE);

		TimeDispatch(&TestSlave_Data);    
}

