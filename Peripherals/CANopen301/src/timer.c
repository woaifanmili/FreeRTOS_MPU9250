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
/* ����ARR��ֵ���ö�ʱ����value΢��֮������жϷ����� */
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
/* �����������жϹ�ȥ�˶೤ʱ�� */
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
** @return  row_number��˵�����뵽��row_number�Žṹ��;
** @return  TIMER_NONE: ˵�����нṹ�嶼���¼�ռ����;
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
    __HAL_CAN_DISABLE_IT(d,CAN_IT_RX_FIFO0_MSG_PENDING);//�ú����������жϴ��
    
	TIMER_HANDLE row_number;//#define TIMER_HANDLE INTEGER16
	s_timer_entry *row;
    

	/* in order to decide new timer setting we have to run over all timer rows */
	/* ���������¼���� *//* ���ܻ���ǰ�������ҵ�row->state == TIMER_FREE��rowΪֹ */
	for(row_number=0, row=d->timers; (row_number <= d->last_timer_raw + 1) && (row_number < MAX_NB_TIMER); row_number++, row++)
	{
		/* �¼����û�б�ռ�ã��������¼���ӽ���� */
		if (callback && 	/* if something to store */
		   row->state == TIMER_FREE) /* and empty row *//* �ҵ���һ��row->state == TIMER_FREE��row������ */
		{	/* just store */
			TIMEVAL real_timer_value;//�ñ������ڱ�������ʹ��
			TIMEVAL elapsed_time;//�ñ������ڱ�������ʹ��
            /* ����һ������¼���ں� */
			if (row_number == d->last_timer_raw + 1)
            {
				d->last_timer_raw++; /* last_timer_raw��SetAlarm�����м�1����DelAlarm�����м�һ�������������ģ�ɾ�����ǽṹ���������¼��Ż��������ֵΪ-1 */
                                    /* ���������Ҳ���������ģ�ֻ��Ҫ�ñ�������Ǹ��ṹ��Ż�ӣ���ΪDelAlarmɾ�Ļ�ɾ�����м�Ĳ������������Ļ��ͻ���ӣ�����Ҳ������ */     
//                if((d->last_timer_raw == -1) && (gMC.target_position_reached_timer == 0))
//                {
//                    settimer_bug++; // used to position bug in previous version, not used now.
//                }
            }

			/* ��ȡ��ʱ���Ѿ���ȥ��ʱ�� */
			elapsed_time = getElapsedTime(d);
			/* set next wakeup alarm if new entry is sooner than others, or if it is alone */
			/* ���¼����֮�󴥷� */
			real_timer_value = value;
			/* ���ó������ʱ�� */
			real_timer_value = min_val(real_timer_value, TIMEVAL_MAX);

			/* ���¼�����ʱ��С�����������¼��Ĵ���ʱ�䣬�򽫶�ʱ���Ĵ���ʱ�����Ϊ���¼��Ĵ���ʱ�� */
			if (d->total_sleep_time > elapsed_time && d->total_sleep_time - elapsed_time > real_timer_value)
			{
				/* ��ʱ�����ߵ�����ֵΪtotal_sleep_timeʱ���� */
				d->total_sleep_time = elapsed_time + real_timer_value;/* ͬһ��ʱ�����ϣ���һ����������ģ�total_sleep_time�ͱ���ֵΪ��һ�� */
				/* ���ö�ʱ����real_timer_value΢��֮�󣬼�����ֵΪtotal_sleep_timeʱ���� */
				setTimer(real_timer_value, d);
			}
			/* ���ø��¼��Ļص����� */
			row->callback = callback;
			/* ���ø��¼��Ķ����ֵ� */
			row->d = d;
			/* ���øûص�����id */
			row->id = id;
			/* ����ʱ��� */
			row->val = value + elapsed_time;
			/* ���ڴ�����0Ϊ�����ڴ��� */
			row->interval = period;
			/* ����ڱ��¼�ռ�� */
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
/* �Ӷ�ʱ�������ɾ��һ���¼���������¼�ɾ�������ֵ��øú���ɾ��һ�Σ�һ�㲻��������ɾһ�ξ����������ֻ��ִ�е����һ�䣨return TIMER_NONE;��*/
/* ����ǵ�һ��ɾ��֮ǰ���ù����¼����ͻ�ִ�е��ú��������д��루��Ϊ���ù����¼�handle������TIMER_NONE����-1��ֻ����0~7��Щ��������Ϊ����
SetAlarm���ö�ʱ���¼���ʱ�����ķ���ֵ�����������뵽�Ľṹ���ţ� */
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
    __HAL_CAN_DISABLE_IT(d,CAN_IT_RX_FIFO0_MSG_PENDING);//�ú����������жϴ��
	/* Quick and dirty. system timer will continue to be trigged, but no action will be preformed. */
	MSG_WAR(0x3320, "DelAlarm. handle = ", handle);
	if(handle != TIMER_NONE)/*#define TIMER_NONE -1 ���ù��Ķ�ʱ���¼��������������������ΪSetAlarm�ķ���ֵΪ�ṹ���ţ�������Ϊ0~7�� */
	{

				/* ����һ�������¼���ں� */
				if(handle == d->last_timer_raw)
        {
            d->last_timer_raw--;
            
        }
			
		/* ���¼������Ϊ����״̬ */
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
/* ɨ�������¼��Ƿ���Ҫ���� */
void TimeDispatch(CO_Data* d)
{
	TIMER_HANDLE i;
	/* ��һ�δ���ʱ�� */
	TIMEVAL next_wakeup = TIMEVAL_MAX + 500; /* used to compute when should normaly occur next wakeup */
	/* First run : change timer state depending on time */
	/* Get time since timer signal */
	/* �Ӷ�ʱ���жϵ��������е�����һ�����е�ʱ�䣬����У׼��ʱ������ֵ */
	UNS32 overrun = (UNS32)getElapsedTime(d);
    /* ��ǰ��ʱ���Ѿ����ߵ�ʱ�� */
	TIMEVAL real_total_sleep_time = d->total_sleep_time + overrun;

	s_timer_entry *row;

	/* ���������¼���� */
	for(i=0, row = d->timers; i <= d->last_timer_raw; i++, row++)
	{
		/* �������ڱ��¼�ռ�� */
		if (row->state & TIMER_ARMED) /* if row is active */
		{
			/* ���˴���ʱ��� *//* Flaxin��ʵ�ǵ�����һ���¼��Ĵ���ʱ�䣬ֻ�ǳ���������Ҫʱ�䣬���е�����row->val�պû��real_total_sleep_timeСoverrun���� */
			if (row->val <= real_total_sleep_time) /* to be trigged */
			{
				/* ��������������¼� */
				if (!row->interval) /* if simply outdated */
				{
					/* ���󴥷� */
					row->state = TIMER_TRIG; /* ask for trig */
				}
				/* ������������¼� ���������һ�δ�����ʱ��㣨��Ҫ����ʱ�����overrun��*/
				else /* or period have expired */
				{
					/* set val as interval, with 32 bit overrun correction, */
					/* modulo for 64 bit not available on all platforms     */
					/* ��interval��У��ֵ��Ϊval */
					row->val = row->interval - (overrun % (UNS32)row->interval);
					/* ���������Դ��� */
					row->state = TIMER_TRIG_PERIOD; /* ask for trig, periodic */
					/* Check if this new timer value is the soonest */
					/* ������һ�λ��ѵ����ʱ�� */
					if(row->val < next_wakeup)
						next_wakeup = row->val;/* ���еĶ�ʱ���¼��ṹ�嶼��ô����һ�飬next_wakeup�ͳ�Ϊ����һ�δ���ʱ����̵��¼���ʱ�� */
				}
			}
			/* ��û������ʱ�䣬�����봥����ʣ��೤ʱ�� */
			else
			{
				/* Each armed timer value in decremented. */
				/* ��ȥ�Ѿ���ȥ��ʱ��*/
				row->val -= real_total_sleep_time;

				/* Check if this new timer value is the soonest */
				/* ������һ�δ���ʱ�� */
				if(row->val < next_wakeup)
					next_wakeup = row->val;/* ���еĶ�ʱ���¼��ṹ�嶼��ô����һ�飬next_wakeup�ͳ�Ϊ����һ�δ���ʱ����̵��¼���ʱ�� */
			}
		}
	}
	/* Remember how much time we should sleep. */
	/* ��ʱ�����ߵ�����ֵΪtotal_sleep_timeʱ���� */
	d->total_sleep_time = next_wakeup;
    /* total_sleep_time��next_wakeup����������
	total_sleep_time�������ж��ĸ��¼���������ģ�������ʱ����ʱ��Ϳ��Կ���������������¼��Ĵ���ʱ���
	next_wakeup�����ж���Щʱ��ʣ���ʱ���ĸ���̣���ʣ��ʱ������¼���ʣ��ʱ��*/
	/* Set timer to soonest occurence */
	/* ���ö�ʱ�� */
	setTimer(next_wakeup, d);/* Ϊ�˱��������ڻ�û�ж�ʱ���¼�����ʱ��ARR��Ϊ�������һ����С��ֵ */

	/* Then trig them or not. */
	/* �����������״̬���ж��Ƿ񴥷��ص����� */
	for(i=0, row = d->timers; i <= d->last_timer_raw; i++, row++)
	{
		if (row->state & TIMER_TRIG)
		{
			/* �������״̬�����δ����ͻ��Ϊ���У����ڴ����������ڴ��� */
			row->state &= ~TIMER_TRIG; /* reset trig state (will be free if not periodic) */
			/* ���� */
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

