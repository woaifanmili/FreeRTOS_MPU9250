/*
  This file is part of CanFestival, a library implementing CanOpen
  Stack.

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
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
  USA
*/

/*!
** @file   lifegrd.c
** @author Edouard TISSERANT
** @date   Mon Jun  4 17:19:24 2007
**
** @brief
**
**
*/

#include <data.h>
#include "lifegrd.h"
#include "canfestival.h"
//#include "dcf.h"
#include "sysdep.h"
#include "ObjectDictionary.h"

void ConsumerHeartbeatAlarm(CO_Data* d, UNS32 id);
void ProducerHeartbeatAlarm(CO_Data* d, UNS32 id);
UNS32 OnHearbeatProducerUpdate(CO_Data* d, const OD_OI_TypeDef * unused_indextable, UNS8 unused_bSubindex);

void GuardTimeAlarm(CO_Data* d, UNS32 id);
UNS32 OnNodeGuardUpdate(CO_Data* d, const OD_OI_typeDef * unused_indextable, UNS8 unused_bSubindex);

/* 获得节点的NMT状态 */
e_nodeState getNodeState (CO_Data* d, UNS8 nodeId)
{
  e_nodeState networkNodeState = Unknown_state; 
  #if NMT_MAX_NODE_ID>0
  if(nodeId < NMT_MAX_NODE_ID)
    networkNodeState = d->NMTable[nodeId];
  #endif
  return networkNodeState;
}

/*! 
** The Consumer Timer Callback
**
** @param d
** @param id
 * @ingroup heartbeato
**/
/* 心跳超时时调用，即心跳超时回调函数 */
/* 消费者心跳定时器事件
节点作为心跳消费者，会对每一个监视对象进行计时，如果从站没有在规定的时间内发送心跳报文，就断定该程序掉线，并通知应用程序*/
void ConsumerHeartbeatAlarm(CO_Data* d, UNS32 id)
{
  /* 根据id找出nodeId，参考对象0x1016值定义 */
  UNS8 nodeId = (UNS8)(((d->ConsumerHeartbeatEntries[id]) & (UNS32)0x00FF0000) >> (UNS8)16);
  /*MSG_WAR(0x00, "ConsumerHearbeatAlarm", 0x00);*/

  /* timer have been notified and is now free (non periodic)*/
  /* -> avoid deleting re-assigned timer if message is received too late*/
  /* 避免该定时器被删除 */
  d->ConsumerHeartBeatTimers[id]=TIMER_NONE;
  
  /* set node state */
  /* 把该从节点设置为断开连接状态 */
  d->NMTable[nodeId] = Disconnected;
  /*! call heartbeat error with NodeId */
  /* 执行心跳错误回调函数 */
  (*d->heartbeatError)(d, nodeId);/* 空函数 */
}



/*! The Producer Timer Callback
**
**
** @param d
** @param id
 * @ingroup heartbeato
**/
/* 生产者定时器回调函数，周期发送心跳包，即心跳报文*/
/* 节点作为心跳生产者，会定时向总线发送心跳报文 */
void ProducerHeartbeatAlarm(CO_Data* d, UNS32 id)
{
  /* 如果周期时间不为零，发送心跳包 */
	if(*d->ProducerHeartBeatTime)
	{
		Message msg;
		/* Time expired, the heartbeat must be sent immediately
		** generate the correct node-id: this is done by the offset 1792
		** (decimal) and additionaly
		** the node-id of this device.
		*/
		UNS16 tmp = *d->bDeviceNodeId + 0x700;
		msg.cob_id = UNS16_LE(tmp);
		msg.len = (UNS8)0x01;
		msg.rtr = 0;
		msg.data[0] = d->nodeState; /* No toggle for heartbeat !*/
		/* send the heartbeat */
		MSG_WAR(0x3130, "Producing heartbeat: ", d->nodeState);
		canSend(d, &msg );
	}
	/* 否则删除该定时器 */
	else 
	{
		d->ProducerHeartBeatTimer = DelAlarm(d->ProducerHeartBeatTimer, &ProducerHeartbeatAlarm, d);
        CAN_ITConfig(d->canHandle,CAN_IT_FMP0, ENABLE);//该函数现在可允许被中断打断
	}
}


 /* 判断接收到的报文是心跳报文还是节点监护报文 */
 /* 如果接收到节点监护报文，就向对方返回一帧数据报告自己的节点状态 */
void proceedNODE_GUARD(CO_Data* d, Message* m )
{
  UNS8 nodeId = (UNS8) GET_NODE_ID((*m));
	 /* 当节点接收到主站节点保护报文之后，立即做出回应 */
  /* 如果这个报文是主节点询问从节点状态的报文 */
  if(m->rtr == 1)
    /*!
    ** Notice that only the master can have sent this
    ** node guarding request
    */
    {
      /*!
      ** Receiving a NMT NodeGuarding (request of the state by the
      ** master)
      ** Only answer to the NMT NodeGuarding request, the master is
      ** not checked (not implemented)
      */
	  /* 和自己的ID进行比较，如果符合就将自己的状态回复给主站 */
      if (nodeId == *d->bDeviceNodeId)
        {
          Message msg;
          UNS16 tmp = *d->bDeviceNodeId + 0x700;
          msg.cob_id = UNS16_LE(tmp);
          msg.len = (UNS8)0x01;
          msg.rtr = 0;
          msg.data[0] = d->nodeState;
			/*Flaxin msg.data[0]的最高位每次都翻转 */
          if (d->toggle)
            {
              msg.data[0] |= 0x80 ;
              d->toggle = 0 ;
            }
          else
            d->toggle = 1 ;
          /* send the nodeguard response. */
          MSG_WAR(0x3130, "Sending NMT Nodeguard to master, state: ", d->nodeState);
          canSend(d, &msg );
        }
    }
	 /* 当主节点接收到从节点回复之后，就重新将从站的寿命恢复为最大值，如果发现从节点状态改变了，需要上报应用程序 */
	/* 从站上报状态报文 */
	else{/* Not a request CAN */
      /* The state is stored on 7 bit */
	  /* 取出从站节点状态 */
      e_nodeState newNodeState = (e_nodeState) ((*m).data[0] & 0x7F);

      MSG_WAR(0x3110, "Received NMT nodeId : ", nodeId);
      
      /*!
      ** Record node response for node guarding service
      */
	  /* 将寿命值还原为LifeTimeFactor */
      d->nodeGuardStatus[nodeId] = *d->LifeTimeFactor;
      /* 如果这个节点状态有变化，上报应用程序并改变节点运行状态表 */
      if (d->NMTable[nodeId] != newNodeState)
      {
        (*d->post_SlaveStateChange)(d, nodeId, newNodeState);/* 空函数 */
        /* the slave's state receievd is stored in the NMTable */
        d->NMTable[nodeId] = newNodeState;
      }

      /* Boot-Up frame reception */
	  /* 如果该从节点处于初始化状态，表明该节点进入预运行状态 */
      if ( d->NMTable[nodeId] == Initialisation )
      {
          /*
          ** The device send the boot-up message (Initialisation)
          ** to indicate the master that it is entered in
          ** pre_operational mode
          */
          MSG_WAR(0x3100, "The NMT is a bootup from node : ", nodeId);
          /* call post SlaveBootup with NodeId */
		  /* 上报应用程序该从节点启动 */
		  (*d->post_SlaveBootup)(d, nodeId);/* 空函数 */
      }
	  /* 如果主站在规定的时间内都接收到从站发来的心跳报文，就对从站重新进行计时 */
      /* 节点保护报文和心跳保护报文只能使用一个 */
	  /* 如果该从节点处于未知运行状态，说明没有启用节点保护报文，说明该报文是心跳报文 */
      if( d->NMTable[nodeId] != Unknown_state )
	  {
        UNS8 index, ConsumerHeartBeat_nodeId ;
		/* 遍历消费者的所有心跳入口 */
        for( index = (UNS8)0x00; index < *d->ConsumerHeartbeatCount; index++ )
          {
            ConsumerHeartBeat_nodeId = (UNS8)( ((d->ConsumerHeartbeatEntries[index]) & (UNS32)0x00FF0000) >> (UNS8)16 );
            if ( nodeId == ConsumerHeartBeat_nodeId )
              {
                TIMEVAL time = ( (d->ConsumerHeartbeatEntries[index]) & (UNS32)0x0000FFFF );
                /* Renew alarm for next heartbeat. */
				/* 如果收到心跳包，就对该节点重新进行掉线计时 */
                DelAlarm(d->ConsumerHeartBeatTimers[index], &ConsumerHeartbeatAlarm, d);
                CAN_ITConfig(d->canHandle,CAN_IT_FMP0, ENABLE);//该函数现在可允许被中断打断
                d->ConsumerHeartBeatTimers[index] = SetAlarm(d, index, &ConsumerHeartbeatAlarm, MS_TO_TIMEVAL(time), 0);
                CAN_ITConfig(d->canHandle,CAN_IT_FMP0, ENABLE);//该函数现在可允许被中断打断
              }
          }
      }
    }
}



/**
 * @brief The guardTime - Timer Callback.
 * 
 * This function is called every GuardTime (OD 0x100C) ms <br>
 * On every call, a NodeGuard-Request is sent to all nodes which have a
 * node-state not equal to "Unknown" (according to NMTable). If the node has
 * not responded within the lifetime, the nodeguardError function is called and
 * the status of this node is set to "Disconnected"
 *
 * @param d 	Pointer on a CAN object data structure 
 * @param id
 * @ingroup nodeguardo
 */
 /* 节点保护报文：
    主站在初始化节点报文之后，周期询问从站当前状态，每询问一次就减少一次该节点的寿命值。
    一旦寿命值减少到0，就上报应用程序，并断定该节点掉线*/
/* 主站询问从站状态定时器回调函数，即节点监护报文 */
void GuardTimeAlarm(CO_Data* d, UNS32 id)
{
  /* 如果主站询问从站寿命时间间隔不为0 */
  if (*d->GuardTime)
  {
    UNS8 i;

    MSG_WAR(0x00, "Producing nodeguard-requests: ", 0);
    /* 主站一个一个询问状态 */
    for (i = 0; i < NMT_MAX_NODE_ID; i++) 
	  {
      /** Send node guard request to all nodes except this node, if the 
      * node state is not "Unknown_state"
      */
	  /* 只要从站不是未知状态 */
	  /* 从站初始化时会发送引导报文，所以只要从站运行起来就不会是运行状态 */
		  if (d->NMTable[i] != Unknown_state && i != *d->bDeviceNodeId) 
		  {

			/** Check if the node has confirmed the guarding request within
			* the LifeTime (GuardTime x LifeTimeFactor)
			*/
			/* 检查该从站是否还有寿命值，如果没有寿命值了要运行错误回调函数，并将节点状态设置为未连接状态 */
			if (d->nodeGuardStatus[i] <= 0)
			{
			  MSG_WAR(0x00, "Node Guard alarm for nodeId : ", i);

			  // Call error-callback function
			  if (*d->nodeguardError) 
			  {
				(*d->nodeguardError)(d, i);/* 空函数 */
			  }

			  // Mark node as disconnected
			  d->NMTable[i] = Disconnected;

			}
			/* 发送完询问报文之后将该从站寿命值减一 */
			d->nodeGuardStatus[i]--;
			/* 主节点询问从站状态 */
			masterSendNMTnodeguard(d, i); /* 主站发送远程帧请求从站的状态--体现了节点监护的原意 */
		  }
      }
  }
  /* 询问时间间隔为零，则删除该定时器事件 */
  else
  {
	d->GuardTimeTimer = DelAlarm(d->GuardTimeTimer, &GuardTimeAlarm, d);
    CAN_ITConfig(d->canHandle,CAN_IT_FMP0, ENABLE);//该函数现在可允许被中断打断
  }
}




/**
 * This function is called, if index 0x100C or 0x100D is updated to
 * restart the node-guarding service with the new parameters
 *
 * @param d 	Pointer on a CAN object data structure 
 * @param unused_indextable
 * @param unused_bSubindex
 * @ingroup nodeguardo
 */
UNS32 OnNodeGuardUpdate(CO_Data* d, const OD_OI_typeDef * unused_indextable, UNS8 unused_bSubindex)
{
  nodeguardStop(d);
  nodeguardInit(d);
  return 0;
}
/*! This is called when Index 0x1017 is updated.
**
**
** @param d
** @param unused_indextable
** @param unused_bSubindex
**
** @return
 * @ingroup heartbeato
**/
UNS32 OnHeartbeatProducerUpdate(CO_Data* d, const OD_OI_typeDef * unused_indextable, UNS8 unused_bSubindex)
{
  heartbeatStop(d);
  heartbeatInit(d);
  return 0;
}


/* 当ConsumerHeartbeatCount=0的时候，就说明该节点不是消费者。
    当ProducerHeartBeatTime=0的时候，就说明该节点不是生产者。
    两者都为0，就说明该节点不启用心跳报文*/
/* 初始化心跳报文函数 */
void heartbeatInit(CO_Data* d)
{
  /* 偏移量用于扫描消费者列表 */
  UNS8 index; /* Index to scan the table of heartbeat consumers */
  /* 设置0x1017对象（生产者心跳超时）改变回调函数 */
  RegisterSetODentryCallBack(d, 0x10170000, &OnHeartbeatProducerUpdate);
   
  d->toggle = 0;
  /* 遍历所有消费者心跳入口，有一个生产者就有一个入口 */
  for( index = (UNS8)0x00; index < *d->ConsumerHeartbeatCount; index++ )/* Flaxin 作为消费者监测众多心跳生产者 */
    {
      TIMEVAL time = (UNS16) ( (d->ConsumerHeartbeatEntries[index]) & (UNS32)0x0000FFFF ) ;/* 参见对象0x1016 */
	  /* 设置消费者心跳定时器回调函数 */
      if ( time )
        {
          d->ConsumerHeartBeatTimers[index] = SetAlarm(d, index, &ConsumerHeartbeatAlarm, MS_TO_TIMEVAL(time), 0);
          CAN_ITConfig(d->canHandle, CAN_IT_FMP0, ENABLE);//该函数现在可允许被中断打断
        }
    }
  /* 生产者心跳时间不为0，设置生产者心跳定时器回调函数 */
  if ( *d->ProducerHeartBeatTime )/* Flaxin 生产者是它自己，只能有一个 */
    {
      TIMEVAL time = *d->ProducerHeartBeatTime;/* 对象0x1017 生产者心跳超时 */
      d->ProducerHeartBeatTimer = SetAlarm(d, 0, &ProducerHeartbeatAlarm, MS_TO_TIMEVAL(time), MS_TO_TIMEVAL(time));
      CAN_ITConfig(d->canHandle, CAN_IT_FMP0, ENABLE);//该函数现在可允许被中断打断
    }
}
 /* 这玩意儿，只要在主站中实现就可以了，从站是被动接收和发送的。
    当主站将GuardTime或LifeTimeFactor设为0的时候，就表示不启用nodeguard报文*/
/* 初始化结点状态保护函数 */
void nodeguardInit(CO_Data* d)
{
  /* 设置0x100C（监护周期），0x100D（生存周期因子）改变回调函数 */
  RegisterSetODentryCallBack(d, 0x100C0000, &OnNodeGuardUpdate);
  RegisterSetODentryCallBack(d, 0x100D0000, &OnNodeGuardUpdate);
  /* 如果主站询问时间间隔不为0 */
  /* 并且从站初始化寿命值不为0 */
  if (*d->GuardTime && *d->LifeTimeFactor) 
  {
    UNS8 i;
    /* 设置主站询问从站状态定时器事件 */
    TIMEVAL time = *d->GuardTime;
    d->GuardTimeTimer = SetAlarm(d, 0, &GuardTimeAlarm, MS_TO_TIMEVAL(time), MS_TO_TIMEVAL(time));/*  */
    CAN_ITConfig(d->canHandle, CAN_IT_FMP0, ENABLE);//该函数现在可允许被中断打断
    MSG_WAR(0x0, "GuardTime: ", time);
    /* 为从节点初始化寿命值 */
    for (i = 0; i < NMT_MAX_NODE_ID; i++) 
	  {
		  /** Set initial value for the nodes */
		  /* 节点不为未知状态，并且该节点不是自己 */
		  /* 因为从站初始化的时候会上报主站引导报文 */
		  if (d->NMTable[i] != Unknown_state && i != *d->bDeviceNodeId) 
		  {
			d->nodeGuardStatus[i] = *d->LifeTimeFactor;
		  }
	  }

    MSG_WAR(0x0, "Timer for node-guarding startet", 0);
  }
}

/* 同时初始化心跳报文和节点保护报文 */
void lifeGuardInit(CO_Data* d)
{
  /* 初始化心跳机制 */
  heartbeatInit(d);
  /* 初始化节点保护函数 */
  nodeguardInit(d);
}



/* 停止这两种报文，只需要调用heartbeatStop和nodeguardStop函数即可 */

/* 删除心跳定时事件 */
void heartbeatStop(CO_Data* d)
{
  UNS8 index;
  for( index = (UNS8)0x00; index < *d->ConsumerHeartbeatCount; index++ )
    {
      d->ConsumerHeartBeatTimers[index] = DelAlarm(d->ConsumerHeartBeatTimers[index], &ConsumerHeartbeatAlarm, d);
      CAN_ITConfig(d->canHandle,CAN_IT_FMP0, ENABLE);//该函数现在可允许被中断打断
    }
  d->ProducerHeartBeatTimer = DelAlarm(d->ProducerHeartBeatTimer, &ProducerHeartbeatAlarm, d);
  CAN_ITConfig(d->canHandle,CAN_IT_FMP0, ENABLE);//该函数现在可允许被中断打断
}
/* 删除节点保护定时事件 */
void nodeguardStop(CO_Data* d)
{
  d->GuardTimeTimer = DelAlarm(d->GuardTimeTimer, &GuardTimeAlarm, d);
  CAN_ITConfig(d->canHandle, CAN_IT_FMP0, ENABLE);//该函数现在可允许被中断打断
}

 /* 同时停止心跳和节点保护 */
void lifeGuardStop(CO_Data* d)
{
  heartbeatStop(d);
  nodeguardStop(d);
}

void _heartbeatError(CO_Data* d, UNS8 heartbeatID){}
void _post_SlaveBootup(CO_Data* d, UNS8 SlaveID){}
void _post_SlaveStateChange(CO_Data* d, UNS8 nodeId, e_nodeState newNodeState){}
void _nodeguardError(CO_Data* d, UNS8 id){}

