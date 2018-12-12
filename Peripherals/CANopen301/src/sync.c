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
** @file   sync.c
** @author Edouard TISSERANT and Francis DUPIN
** @date   Tue Jun  5 09:32:32 2007
**
** @brief
**
**
*/

#include "data.h"
#include "sync.h"
#include "canfestival.h"
#include "sysdep.h"
#include "ObjectDictionary.h"
#include "stm32f4xx_it.h"
//#include "global.h"
//#include "TestSlave.h"


//int numsynctx=0;
/*!                                                                                                
**                                                                                                 
**                                                                                                 
** @param d                                                                                        
** @param id                                                                                       
**/   
/* 发送同步报文定时器事件被触发的回调函数 */
/* 该函数被作为定时器回调函数被周期调用 */
void SyncAlarm(CO_Data* d, UNS32 id)
{
	sendSYNC(d);
}

/*!                                                                                                
** This is called when Index 0x1005 is updated.                                                                                                
**                                                                                                 
** @param d                                                                                        
** @param unsused_indextable                                                                       
** @param unsused_bSubindex                                                                        
**                                                                                                 
** @return                                                                                         
**/  
UNS32 OnCOB_ID_SyncUpdate(CO_Data* d, const OD_OI_typeDef * unsused_indextable, UNS8 unsused_bSubindex)
{
	startSYNC(d);
	return 0;
}

/*!                                                                                                
**                                                                                                 
**                                                                                                 
** @param d
**/
/* 启动同步报文 */
void startSYNC(CO_Data* d)
{
	/* 如果同步定时器已被设定要先停止 */
	if(d->syncTimer != TIMER_NONE)
	{
		stopSYNC(d);
	}

	/* 设定索引为0x1005和0x1006的对象的回调函数 */
	RegisterSetODentryCallBack(d, 0x10050000, &OnCOB_ID_SyncUpdate);/* startSYNC *//*Flaxin COB_ID同步消息，即该节点设备发送同步帧的COB_ID,本程序中设置的是0x80 */
	RegisterSetODentryCallBack(d, 0x10060000, &OnCOB_ID_SyncUpdate);/* startSYNC *//* 通信循环周期 *//* 周期发送的SYNC的时间间隔，单位是微秒，UNS32表示 */

	if(*d->COB_ID_Sync & 0x40000000ul && *d->Sync_Cycle_Period)/* 参考对象0x1005,其位30为gen.位，当gen.位为1的时候生成同步消息 */
	{                                                            /* Sync_Cycle_Period的单位为微秒 */
		/* 设定周期发送同步报文的定时器事件 */
		d->syncTimer = SetAlarm(d,
								0 /*No id needed*/,
								&SyncAlarm,/* 该函数的功能是先广播同步帧，然后处理同步报文，因为每个canopen网络中只允许有一同步生产者
		                                      既然该节点的这个函数先广播了一个同步帧，说明它就是同步生产者，生产者发送同步帧其实也可以看做它自己也收到了一个同步帧
		                                      只不过这个同步帧是自己发的（其实并没有接收到实际的同步帧，只是网络上出现同步帧就标志着
                                              所有节点的同步，当然可以看做同步生产者也接收到了一个同步帧		*/
								US_TO_TIMEVAL(*d->Sync_Cycle_Period), /* 通信循环周期，对象0x1006 */
								US_TO_TIMEVAL(*d->Sync_Cycle_Period));/* 通信循环周期，对象0x1006 */
        __HAL_CAN_ENABLE_IT(d, CAN_IT_RX_FIFO0_MSG_PENDING);//该函数现在可允许被中断打断
	}
}

/*!                                                                                                
**                                                                                                 
**                                                                                                 
** @param d                                                                                        
**/
/* 停止同步报文 */
void stopSYNC(CO_Data* d)
{
	/* 清空索引为0x1005和0x1006的对象的回调函数 */
    RegisterSetODentryCallBack(d, 0x10050000, NULL);
    RegisterSetODentryCallBack(d, 0x10060000, NULL);
	/* 删除周期发送同步报文的定时器事件 */
	d->syncTimer = DelAlarm(d->syncTimer, &SyncAlarm, d);
    __HAL_CAN_ENABLE_IT(d, CAN_IT_RX_FIFO0_MSG_PENDING);//该函数现在可允许被中断打断
}


/*!                                                                                                
**                                                                                                 
**                                                                                                 
** @param d                                                                                        
** @param cob_id                                                                                   
**                                                                                                 
** @return                                                                                         
**/  
/* 发送同步报文 */
/* 发送一帧不带数据字节的同步非远程帧报文，该帧的COB_ID来自对象0x1005,本程序中设置为0x80 */
/* 该函数在定时器回调函数中被周期调用 */
int test_sync_slave = 0;
int test_sync_master = 0;
UNS8 sendSYNCMessage(CO_Data* d)
{
  Message m;
  
  MSG_WAR(0x3001, "sendSYNC ", 0);
  
  m.cob_id = (UNS16)UNS16_LE(*d->COB_ID_Sync);
  m.rtr = NOT_A_REQUEST;
  m.len = 0;
  if(d->can_port_num ==1)
  {
      test_sync_slave++;
  }
  
  if(d->can_port_num ==2)
  {
      test_sync_master++;
  }
  
  return canSend(d, &m);
}


/*!                                                                                                
**                                                                                                 
**                                                                                                 
** @param d                                                                                        
** @param cob_id                                                                                   
**                                                                                                 
** @return                                                                                         
**/  
/* 发送同步报文 */
/* 该函数在定时器回调函数中被周期调用 */
UNS8 sendSYNC(CO_Data* d)
{
  UNS8 res;
  /* 发送同步报文消息 */
  res = sendSYNCMessage(d);/* 广播一帧同步帧，非远程帧，但不带数据字节 */
  /* 处理同步报文 */
	d->numsynctx++;
  proceedSYNC(d);
  return res;
}

/*!                                                                                                
**                                                                                                 
**                                                                                                 
** @param d                                                                                        
** @param m                                                                                        
**                                                                                                 
** @return                                                                                         
**/ 
/* 该函数需要在节点设备接收到同步帧后调用，由于同步生产者不可能接收到实际的同步帧
所以同步生产者就在它发送了同步帧之后马上调用该函数，也即同步生产者的发送看作是它自己的接收
所以会有两个地方需要调用该函数，一个是作为同步消费者的节点需要在candispatch函数中调用
另外一个就是同步生产者在定时器回调函数中调用，以弥补生产者永远接收不到同步帧的问题*/
UNS8 proceedSYNC(CO_Data* d)
{
  UNS8 res;
  
  MSG_WAR(0x3002, "SYNC received. Proceed. ", 0);
  
  (*d->post_sync)(d); /* Flaxin 空函数 */
  
  /* only operational state allows PDO transmission */
  if(! d->CurrentCommunicationState.csPDO)
    return 0;

  res = _sendPDOevent(d, 1 /*isSyncEvent*/ );
  
  /*Call user app callback*/
  (*d->post_TPDO)(d); /* Flaxin 空函数 */

  return res;
  
  
}


void _post_sync(CO_Data* d)
{

}
void _post_TPDO(CO_Data* d)
{
    
}
