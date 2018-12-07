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

/* ��ýڵ��NMT״̬ */
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
/* ������ʱʱ���ã���������ʱ�ص����� */
/* ������������ʱ���¼�
�ڵ���Ϊ���������ߣ����ÿһ�����Ӷ�����м�ʱ�������վû���ڹ涨��ʱ���ڷ����������ģ��Ͷ϶��ó�����ߣ���֪ͨӦ�ó���*/
void ConsumerHeartbeatAlarm(CO_Data* d, UNS32 id)
{
  /* ����id�ҳ�nodeId���ο�����0x1016ֵ���� */
  UNS8 nodeId = (UNS8)(((d->ConsumerHeartbeatEntries[id]) & (UNS32)0x00FF0000) >> (UNS8)16);
  /*MSG_WAR(0x00, "ConsumerHearbeatAlarm", 0x00);*/

  /* timer have been notified and is now free (non periodic)*/
  /* -> avoid deleting re-assigned timer if message is received too late*/
  /* ����ö�ʱ����ɾ�� */
  d->ConsumerHeartBeatTimers[id]=TIMER_NONE;
  
  /* set node state */
  /* �Ѹôӽڵ�����Ϊ�Ͽ�����״̬ */
  d->NMTable[nodeId] = Disconnected;
  /*! call heartbeat error with NodeId */
  /* ִ����������ص����� */
  (*d->heartbeatError)(d, nodeId);/* �պ��� */
}



/*! The Producer Timer Callback
**
**
** @param d
** @param id
 * @ingroup heartbeato
**/
/* �����߶�ʱ���ص����������ڷ���������������������*/
/* �ڵ���Ϊ���������ߣ��ᶨʱ�����߷����������� */
void ProducerHeartbeatAlarm(CO_Data* d, UNS32 id)
{
  /* �������ʱ�䲻Ϊ�㣬���������� */
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
	/* ����ɾ���ö�ʱ�� */
	else 
	{
		d->ProducerHeartBeatTimer = DelAlarm(d->ProducerHeartBeatTimer, &ProducerHeartbeatAlarm, d);
        CAN_ITConfig(d->canHandle,CAN_IT_FMP0, ENABLE);//�ú������ڿ������жϴ��
	}
}


 /* �жϽ��յ��ı������������Ļ��ǽڵ�໤���� */
 /* ������յ��ڵ�໤���ģ�����Է�����һ֡���ݱ����Լ��Ľڵ�״̬ */
void proceedNODE_GUARD(CO_Data* d, Message* m )
{
  UNS8 nodeId = (UNS8) GET_NODE_ID((*m));
	 /* ���ڵ���յ���վ�ڵ㱣������֮������������Ӧ */
  /* ���������������ڵ�ѯ�ʴӽڵ�״̬�ı��� */
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
	  /* ���Լ���ID���бȽϣ�������Ͼͽ��Լ���״̬�ظ�����վ */
      if (nodeId == *d->bDeviceNodeId)
        {
          Message msg;
          UNS16 tmp = *d->bDeviceNodeId + 0x700;
          msg.cob_id = UNS16_LE(tmp);
          msg.len = (UNS8)0x01;
          msg.rtr = 0;
          msg.data[0] = d->nodeState;
			/*Flaxin msg.data[0]�����λÿ�ζ���ת */
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
	 /* �����ڵ���յ��ӽڵ�ظ�֮�󣬾����½���վ�������ָ�Ϊ���ֵ��������ִӽڵ�״̬�ı��ˣ���Ҫ�ϱ�Ӧ�ó��� */
	/* ��վ�ϱ�״̬���� */
	else{/* Not a request CAN */
      /* The state is stored on 7 bit */
	  /* ȡ����վ�ڵ�״̬ */
      e_nodeState newNodeState = (e_nodeState) ((*m).data[0] & 0x7F);

      MSG_WAR(0x3110, "Received NMT nodeId : ", nodeId);
      
      /*!
      ** Record node response for node guarding service
      */
	  /* ������ֵ��ԭΪLifeTimeFactor */
      d->nodeGuardStatus[nodeId] = *d->LifeTimeFactor;
      /* �������ڵ�״̬�б仯���ϱ�Ӧ�ó��򲢸ı�ڵ�����״̬�� */
      if (d->NMTable[nodeId] != newNodeState)
      {
        (*d->post_SlaveStateChange)(d, nodeId, newNodeState);/* �պ��� */
        /* the slave's state receievd is stored in the NMTable */
        d->NMTable[nodeId] = newNodeState;
      }

      /* Boot-Up frame reception */
	  /* ����ôӽڵ㴦�ڳ�ʼ��״̬�������ýڵ����Ԥ����״̬ */
      if ( d->NMTable[nodeId] == Initialisation )
      {
          /*
          ** The device send the boot-up message (Initialisation)
          ** to indicate the master that it is entered in
          ** pre_operational mode
          */
          MSG_WAR(0x3100, "The NMT is a bootup from node : ", nodeId);
          /* call post SlaveBootup with NodeId */
		  /* �ϱ�Ӧ�ó���ôӽڵ����� */
		  (*d->post_SlaveBootup)(d, nodeId);/* �պ��� */
      }
	  /* �����վ�ڹ涨��ʱ���ڶ����յ���վ�������������ģ��ͶԴ�վ���½��м�ʱ */
      /* �ڵ㱣�����ĺ�������������ֻ��ʹ��һ�� */
	  /* ����ôӽڵ㴦��δ֪����״̬��˵��û�����ýڵ㱣�����ģ�˵���ñ������������� */
      if( d->NMTable[nodeId] != Unknown_state )
	  {
        UNS8 index, ConsumerHeartBeat_nodeId ;
		/* ���������ߵ������������ */
        for( index = (UNS8)0x00; index < *d->ConsumerHeartbeatCount; index++ )
          {
            ConsumerHeartBeat_nodeId = (UNS8)( ((d->ConsumerHeartbeatEntries[index]) & (UNS32)0x00FF0000) >> (UNS8)16 );
            if ( nodeId == ConsumerHeartBeat_nodeId )
              {
                TIMEVAL time = ( (d->ConsumerHeartbeatEntries[index]) & (UNS32)0x0000FFFF );
                /* Renew alarm for next heartbeat. */
				/* ����յ����������ͶԸýڵ����½��е��߼�ʱ */
                DelAlarm(d->ConsumerHeartBeatTimers[index], &ConsumerHeartbeatAlarm, d);
                CAN_ITConfig(d->canHandle,CAN_IT_FMP0, ENABLE);//�ú������ڿ������жϴ��
                d->ConsumerHeartBeatTimers[index] = SetAlarm(d, index, &ConsumerHeartbeatAlarm, MS_TO_TIMEVAL(time), 0);
                CAN_ITConfig(d->canHandle,CAN_IT_FMP0, ENABLE);//�ú������ڿ������жϴ��
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
 /* �ڵ㱣�����ģ�
    ��վ�ڳ�ʼ���ڵ㱨��֮������ѯ�ʴ�վ��ǰ״̬��ÿѯ��һ�ξͼ���һ�θýڵ������ֵ��
    һ������ֵ���ٵ�0�����ϱ�Ӧ�ó��򣬲��϶��ýڵ����*/
/* ��վѯ�ʴ�վ״̬��ʱ���ص����������ڵ�໤���� */
void GuardTimeAlarm(CO_Data* d, UNS32 id)
{
  /* �����վѯ�ʴ�վ����ʱ������Ϊ0 */
  if (*d->GuardTime)
  {
    UNS8 i;

    MSG_WAR(0x00, "Producing nodeguard-requests: ", 0);
    /* ��վһ��һ��ѯ��״̬ */
    for (i = 0; i < NMT_MAX_NODE_ID; i++) 
	  {
      /** Send node guard request to all nodes except this node, if the 
      * node state is not "Unknown_state"
      */
	  /* ֻҪ��վ����δ֪״̬ */
	  /* ��վ��ʼ��ʱ�ᷢ���������ģ�����ֻҪ��վ���������Ͳ���������״̬ */
		  if (d->NMTable[i] != Unknown_state && i != *d->bDeviceNodeId) 
		  {

			/** Check if the node has confirmed the guarding request within
			* the LifeTime (GuardTime x LifeTimeFactor)
			*/
			/* ���ô�վ�Ƿ�������ֵ�����û������ֵ��Ҫ���д���ص������������ڵ�״̬����Ϊδ����״̬ */
			if (d->nodeGuardStatus[i] <= 0)
			{
			  MSG_WAR(0x00, "Node Guard alarm for nodeId : ", i);

			  // Call error-callback function
			  if (*d->nodeguardError) 
			  {
				(*d->nodeguardError)(d, i);/* �պ��� */
			  }

			  // Mark node as disconnected
			  d->NMTable[i] = Disconnected;

			}
			/* ������ѯ�ʱ���֮�󽫸ô�վ����ֵ��һ */
			d->nodeGuardStatus[i]--;
			/* ���ڵ�ѯ�ʴ�վ״̬ */
			masterSendNMTnodeguard(d, i); /* ��վ����Զ��֡�����վ��״̬--�����˽ڵ�໤��ԭ�� */
		  }
      }
  }
  /* ѯ��ʱ����Ϊ�㣬��ɾ���ö�ʱ���¼� */
  else
  {
	d->GuardTimeTimer = DelAlarm(d->GuardTimeTimer, &GuardTimeAlarm, d);
    CAN_ITConfig(d->canHandle,CAN_IT_FMP0, ENABLE);//�ú������ڿ������жϴ��
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


/* ��ConsumerHeartbeatCount=0��ʱ�򣬾�˵���ýڵ㲻�������ߡ�
    ��ProducerHeartBeatTime=0��ʱ�򣬾�˵���ýڵ㲻�������ߡ�
    ���߶�Ϊ0����˵���ýڵ㲻������������*/
/* ��ʼ���������ĺ��� */
void heartbeatInit(CO_Data* d)
{
  /* ƫ��������ɨ���������б� */
  UNS8 index; /* Index to scan the table of heartbeat consumers */
  /* ����0x1017����������������ʱ���ı�ص����� */
  RegisterSetODentryCallBack(d, 0x10170000, &OnHeartbeatProducerUpdate);
   
  d->toggle = 0;
  /* ��������������������ڣ���һ�������߾���һ����� */
  for( index = (UNS8)0x00; index < *d->ConsumerHeartbeatCount; index++ )/* Flaxin ��Ϊ�����߼���ڶ����������� */
    {
      TIMEVAL time = (UNS16) ( (d->ConsumerHeartbeatEntries[index]) & (UNS32)0x0000FFFF ) ;/* �μ�����0x1016 */
	  /* ����������������ʱ���ص����� */
      if ( time )
        {
          d->ConsumerHeartBeatTimers[index] = SetAlarm(d, index, &ConsumerHeartbeatAlarm, MS_TO_TIMEVAL(time), 0);
          CAN_ITConfig(d->canHandle, CAN_IT_FMP0, ENABLE);//�ú������ڿ������жϴ��
        }
    }
  /* ����������ʱ�䲻Ϊ0������������������ʱ���ص����� */
  if ( *d->ProducerHeartBeatTime )/* Flaxin �����������Լ���ֻ����һ�� */
    {
      TIMEVAL time = *d->ProducerHeartBeatTime;/* ����0x1017 ������������ʱ */
      d->ProducerHeartBeatTimer = SetAlarm(d, 0, &ProducerHeartbeatAlarm, MS_TO_TIMEVAL(time), MS_TO_TIMEVAL(time));
      CAN_ITConfig(d->canHandle, CAN_IT_FMP0, ENABLE);//�ú������ڿ������жϴ��
    }
}
 /* ���������ֻҪ����վ��ʵ�־Ϳ����ˣ���վ�Ǳ������պͷ��͵ġ�
    ����վ��GuardTime��LifeTimeFactor��Ϊ0��ʱ�򣬾ͱ�ʾ������nodeguard����*/
/* ��ʼ�����״̬�������� */
void nodeguardInit(CO_Data* d)
{
  /* ����0x100C���໤���ڣ���0x100D�������������ӣ��ı�ص����� */
  RegisterSetODentryCallBack(d, 0x100C0000, &OnNodeGuardUpdate);
  RegisterSetODentryCallBack(d, 0x100D0000, &OnNodeGuardUpdate);
  /* �����վѯ��ʱ������Ϊ0 */
  /* ���Ҵ�վ��ʼ������ֵ��Ϊ0 */
  if (*d->GuardTime && *d->LifeTimeFactor) 
  {
    UNS8 i;
    /* ������վѯ�ʴ�վ״̬��ʱ���¼� */
    TIMEVAL time = *d->GuardTime;
    d->GuardTimeTimer = SetAlarm(d, 0, &GuardTimeAlarm, MS_TO_TIMEVAL(time), MS_TO_TIMEVAL(time));/*  */
    CAN_ITConfig(d->canHandle, CAN_IT_FMP0, ENABLE);//�ú������ڿ������жϴ��
    MSG_WAR(0x0, "GuardTime: ", time);
    /* Ϊ�ӽڵ��ʼ������ֵ */
    for (i = 0; i < NMT_MAX_NODE_ID; i++) 
	  {
		  /** Set initial value for the nodes */
		  /* �ڵ㲻Ϊδ֪״̬�����Ҹýڵ㲻���Լ� */
		  /* ��Ϊ��վ��ʼ����ʱ����ϱ���վ�������� */
		  if (d->NMTable[i] != Unknown_state && i != *d->bDeviceNodeId) 
		  {
			d->nodeGuardStatus[i] = *d->LifeTimeFactor;
		  }
	  }

    MSG_WAR(0x0, "Timer for node-guarding startet", 0);
  }
}

/* ͬʱ��ʼ���������ĺͽڵ㱣������ */
void lifeGuardInit(CO_Data* d)
{
  /* ��ʼ���������� */
  heartbeatInit(d);
  /* ��ʼ���ڵ㱣������ */
  nodeguardInit(d);
}



/* ֹͣ�����ֱ��ģ�ֻ��Ҫ����heartbeatStop��nodeguardStop�������� */

/* ɾ��������ʱ�¼� */
void heartbeatStop(CO_Data* d)
{
  UNS8 index;
  for( index = (UNS8)0x00; index < *d->ConsumerHeartbeatCount; index++ )
    {
      d->ConsumerHeartBeatTimers[index] = DelAlarm(d->ConsumerHeartBeatTimers[index], &ConsumerHeartbeatAlarm, d);
      CAN_ITConfig(d->canHandle,CAN_IT_FMP0, ENABLE);//�ú������ڿ������жϴ��
    }
  d->ProducerHeartBeatTimer = DelAlarm(d->ProducerHeartBeatTimer, &ProducerHeartbeatAlarm, d);
  CAN_ITConfig(d->canHandle,CAN_IT_FMP0, ENABLE);//�ú������ڿ������жϴ��
}
/* ɾ���ڵ㱣����ʱ�¼� */
void nodeguardStop(CO_Data* d)
{
  d->GuardTimeTimer = DelAlarm(d->GuardTimeTimer, &GuardTimeAlarm, d);
  CAN_ITConfig(d->canHandle, CAN_IT_FMP0, ENABLE);//�ú������ڿ������жϴ��
}

 /* ͬʱֹͣ�����ͽڵ㱣�� */
void lifeGuardStop(CO_Data* d)
{
  heartbeatStop(d);
  nodeguardStop(d);
}

void _heartbeatError(CO_Data* d, UNS8 heartbeatID){}
void _post_SlaveBootup(CO_Data* d, UNS8 SlaveID){}
void _post_SlaveStateChange(CO_Data* d, UNS8 nodeId, e_nodeState newNodeState){}
void _nodeguardError(CO_Data* d, UNS8 id){}

