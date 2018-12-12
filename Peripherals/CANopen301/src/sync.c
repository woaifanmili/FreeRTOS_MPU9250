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
/* ����ͬ�����Ķ�ʱ���¼��������Ļص����� */
/* �ú�������Ϊ��ʱ���ص����������ڵ��� */
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
/* ����ͬ������ */
void startSYNC(CO_Data* d)
{
	/* ���ͬ����ʱ���ѱ��趨Ҫ��ֹͣ */
	if(d->syncTimer != TIMER_NONE)
	{
		stopSYNC(d);
	}

	/* �趨����Ϊ0x1005��0x1006�Ķ���Ļص����� */
	RegisterSetODentryCallBack(d, 0x10050000, &OnCOB_ID_SyncUpdate);/* startSYNC *//*Flaxin COB_IDͬ����Ϣ�����ýڵ��豸����ͬ��֡��COB_ID,�����������õ���0x80 */
	RegisterSetODentryCallBack(d, 0x10060000, &OnCOB_ID_SyncUpdate);/* startSYNC *//* ͨ��ѭ������ *//* ���ڷ��͵�SYNC��ʱ��������λ��΢�룬UNS32��ʾ */

	if(*d->COB_ID_Sync & 0x40000000ul && *d->Sync_Cycle_Period)/* �ο�����0x1005,��λ30Ϊgen.λ����gen.λΪ1��ʱ������ͬ����Ϣ */
	{                                                            /* Sync_Cycle_Period�ĵ�λΪ΢�� */
		/* �趨���ڷ���ͬ�����ĵĶ�ʱ���¼� */
		d->syncTimer = SetAlarm(d,
								0 /*No id needed*/,
								&SyncAlarm,/* �ú����Ĺ������ȹ㲥ͬ��֡��Ȼ����ͬ�����ģ���Ϊÿ��canopen������ֻ������һͬ��������
		                                      ��Ȼ�ýڵ����������ȹ㲥��һ��ͬ��֡��˵��������ͬ�������ߣ������߷���ͬ��֡��ʵҲ���Կ������Լ�Ҳ�յ���һ��ͬ��֡
		                                      ֻ�������ͬ��֡���Լ����ģ���ʵ��û�н��յ�ʵ�ʵ�ͬ��֡��ֻ�������ϳ���ͬ��֡�ͱ�־��
                                              ���нڵ��ͬ������Ȼ���Կ���ͬ��������Ҳ���յ���һ��ͬ��֡		*/
								US_TO_TIMEVAL(*d->Sync_Cycle_Period), /* ͨ��ѭ�����ڣ�����0x1006 */
								US_TO_TIMEVAL(*d->Sync_Cycle_Period));/* ͨ��ѭ�����ڣ�����0x1006 */
        __HAL_CAN_ENABLE_IT(d, CAN_IT_RX_FIFO0_MSG_PENDING);//�ú������ڿ������жϴ��
	}
}

/*!                                                                                                
**                                                                                                 
**                                                                                                 
** @param d                                                                                        
**/
/* ֹͣͬ������ */
void stopSYNC(CO_Data* d)
{
	/* �������Ϊ0x1005��0x1006�Ķ���Ļص����� */
    RegisterSetODentryCallBack(d, 0x10050000, NULL);
    RegisterSetODentryCallBack(d, 0x10060000, NULL);
	/* ɾ�����ڷ���ͬ�����ĵĶ�ʱ���¼� */
	d->syncTimer = DelAlarm(d->syncTimer, &SyncAlarm, d);
    __HAL_CAN_ENABLE_IT(d, CAN_IT_RX_FIFO0_MSG_PENDING);//�ú������ڿ������жϴ��
}


/*!                                                                                                
**                                                                                                 
**                                                                                                 
** @param d                                                                                        
** @param cob_id                                                                                   
**                                                                                                 
** @return                                                                                         
**/  
/* ����ͬ������ */
/* ����һ֡���������ֽڵ�ͬ����Զ��֡���ģ���֡��COB_ID���Զ���0x1005,������������Ϊ0x80 */
/* �ú����ڶ�ʱ���ص������б����ڵ��� */
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
/* ����ͬ������ */
/* �ú����ڶ�ʱ���ص������б����ڵ��� */
UNS8 sendSYNC(CO_Data* d)
{
  UNS8 res;
  /* ����ͬ��������Ϣ */
  res = sendSYNCMessage(d);/* �㲥һ֡ͬ��֡����Զ��֡�������������ֽ� */
  /* ����ͬ������ */
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
/* �ú�����Ҫ�ڽڵ��豸���յ�ͬ��֡����ã�����ͬ�������߲����ܽ��յ�ʵ�ʵ�ͬ��֡
����ͬ�������߾�����������ͬ��֮֡�����ϵ��øú�����Ҳ��ͬ�������ߵķ��Ϳ��������Լ��Ľ���
���Ի��������ط���Ҫ���øú�����һ������Ϊͬ�������ߵĽڵ���Ҫ��candispatch�����е���
����һ������ͬ���������ڶ�ʱ���ص������е��ã����ֲ���������Զ���ղ���ͬ��֡������*/
UNS8 proceedSYNC(CO_Data* d)
{
  UNS8 res;
  
  MSG_WAR(0x3002, "SYNC received. Proceed. ", 0);
  
  (*d->post_sync)(d); /* Flaxin �պ��� */
  
  /* only operational state allows PDO transmission */
  if(! d->CurrentCommunicationState.csPDO)
    return 0;

  res = _sendPDOevent(d, 1 /*isSyncEvent*/ );
  
  /*Call user app callback*/
  (*d->post_TPDO)(d); /* Flaxin �պ��� */

  return res;
  
  
}


void _post_sync(CO_Data* d)
{

}
void _post_TPDO(CO_Data* d)
{
    
}
