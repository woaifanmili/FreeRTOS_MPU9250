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
** @file   states.c
** @author Edouard TISSERANT and Francis DUPIN
** @date   Tue Jun  5 09:32:32 2007
**
** @brief
**
**
*/

//#include "logic_debug.h"
#include "data.h"
#include "sysdep.h"
#include "ObjectDictionary.h"
#include "SDO.h"
#include "canfestival.h"
//#include "TestSlave.h"


UNS32 sizeUNS8 = sizeof(UNS8);
UNS32 sizeUNS16 = sizeof(UNS16);
UNS32 sizeUNS32 = sizeof(UNS32);

//int num_sync_rcv=0;
//int num_pdo_rcv=0;

#define SYNC_PERIOD_SLAVE 20000    // ��s
#define SYNC_PERIOD_MASTER 10000    // ��s
#define PRODUCER_HEARTBEAT_TIME 400  //ms


/** Prototypes for internals functions */
/*!                                                                                                
**                                                                                                 
**                                                                                                 
** @param d                                                                                        
** @param newCommunicationState                                                                    
**/     
void switchCommunicationState(CO_Data* d, 
	s_state_communication *newCommunicationState);
	
/*!                                                                                                
**                                                                                                 
**                                                                                                 
** @param d                                                                                        
**                                                                                                 
** @return                                                                                         
**/  
/* ��ȡ�ڵ�����״̬ */
e_nodeState getState(CO_Data* d)
{
  return d->nodeState;
}

/*!                                                                                                
**                                                                                                 
**                                                                                                 
** @param d                                                                                        
** @param m                                                                                        
**/  
/* ���ദ���� */
void canDispatch(CO_Data* d, Message *m)
{
	UNS16 cob_id = UNS16_LE(m->cob_id);
	 /* �жϱ��� */
	 switch(cob_id >> 7)
	{
		/* ������ͬ�����Ļ�������� */
		case SYNC:		/* can be a SYNC or a EMCY message */
			/* ͬ������ */
			if(cob_id == 0x080)	/* SYNC */
			{
				d->num_sync_rcv++;
				/* ��ǰ״̬֧��ͬ������ */
				if(d->CurrentCommunicationState.csSYNC)
					proceedSYNC(d);
			}
			/* �������� */
			else 		/* EMCY */
				/* ��ǰ״̬֧�ֽ������� */
				if(d->CurrentCommunicationState.csEmergency)
					proceedEMCY(d,m);
			break;
	    /* ʱ��� */
		/* case TIME_STAMP: */

		/* �������ݶ��� */
		case PDO1tx:
		case PDO1rx:
		case PDO2tx:
		case PDO2rx:
		case PDO3tx:
		case PDO3rx:
		case PDO4tx:
		case PDO4rx:
			d->num_pdo_rcv++;
			/* ��ǰ״̬֧�ֹ������ݶ����� */
			if (d->CurrentCommunicationState.csPDO)
				proceedPDO(d,m);
			break;
		/* �������ݶ��� */
		case SDOtx:
		case SDOrx:
		    /* ��ǰ״̬֧�ַ������ݶ����� */
			if (d->CurrentCommunicationState.csSDO)
				proceedSDO(d,m);
			break;
		/* �ڵ�״̬���� */
		case NODE_GUARD:
			/* ��ǰ״̬֧�ֽڵ�״̬���� */
			if (d->CurrentCommunicationState.csLifeGuard)
				proceedNODE_GUARD(d,m);
			break;
		/* ��������� */
		case NMT:
			/* ��ǰ�ڵ�Ϊ��վ */
			if (*(d->iam_a_slave))
			{
				proceedNMTstateChange(d,m);
			}
            break;
#ifdef CO_ENABLE_LSS
		case LSS:
			if (!d->CurrentCommunicationState.csLSS)break;
			if ((*(d->iam_a_slave)) && cob_id==MLSS_ADRESS)
			{
				proceedLSS_Slave(d,m);
			}
			else if(!(*(d->iam_a_slave)) && cob_id==SLSS_ADRESS)
			{
				proceedLSS_Master(d,m);
			}
			break;
#endif
	}
}

/* �������߹ر�ͨ�Ŷ���ʱ������Ӧ�ĺ��� */
#define StartOrStop(CommType, FuncStart, FuncStop) \
	if(newCommunicationState->CommType && d->CurrentCommunicationState.CommType == 0){\
		MSG_WAR(0x9999,#FuncStart, 9999);\
		d->CurrentCommunicationState.CommType = 1;\
		FuncStart;\
	}else if(!newCommunicationState->CommType && d->CurrentCommunicationState.CommType == 1){\
		MSG_WAR(0x9999,#FuncStop, 9999);\
		d->CurrentCommunicationState.CommType = 0;\
		FuncStop;\
	}
#define None


	
/*!                                                                                                
**                                                                                                 
**                                                                                                 
** @param d                                                                                        
** @param newCommunicationState                                                                    
**/  
/* �л�����״̬ʱ���������߹ر�ͨ�Ŷ��� */	
void switchCommunicationState(CO_Data* d, s_state_communication *newCommunicationState)
{
#ifdef CO_ENABLE_LSS
	StartOrStop(csLSS,	startLSS(d),	stopLSS(d))
#endif
	/* �رշ������ݶ���ʱ����resetSDO���� */
	StartOrStop(csSDO,	None,		resetSDO(d))
	/* ����ͬ������ʱ����startSYNC���� */
	/* �ر�ͬ������ʱ����stopSYNC���� */
	StartOrStop(csSYNC,	startSYNC(d),		stopSYNC(d))
    /* �����ڵ�״̬����ʱ����lifeGuardInit���� */
	/* �رսڵ�״̬����ʱ����lifeGuardStop���� */
	StartOrStop(csLifeGuard,	lifeGuardInit(d),	lifeGuardStop(d))
	/* ������������ʱ����emergencyInit���� */
	/* �رս�������ʱ����emergencyStop���� */
	StartOrStop(csEmergency,	emergencyInit(d),	emergencyStop(d))
	/* �����������ݶ���ʱ����PDOInit���� */
	/* �رչ������ݶ���ʱ����PDOStop���� */
	StartOrStop(csPDO,	PDOInit(d),	PDOStop(d))
	/* �ر���������ʱ����slaveSendBootUp���� */
	StartOrStop(csBoot_Up,	None,	slaveSendBootUp(d))/* �����е���֣�csBoot_Up==1�����������slaveSendBootUp������0�ŵ����������������� */
}

/*!                                                                                                
**                                                                                                 
**                                                                                                 
** @param d                                                                                        
** @param newState                                                                                 
**                                                                                                 
** @return                                                                                         
**/  
/* ���ýڵ�����״̬ */
/* ���ýڵ�״̬����΢�����ˣ�����Ҫ��Ϊָ����״̬����Ҫ�ı䱨�ĵĿ����ԣ���������غ������д��� */
UNS8 setState(CO_Data* d, e_nodeState newState)
{
	/* �¾�״̬��һ��ʱ����Ҫ�л�״̬ */
	if(newState != d->nodeState)
	{
		/* �ж���״̬ */
		switch( newState )
		{
			/* ��ʼ��״̬ */
			case Initialisation:
			{
				s_state_communication newCommunicationState = {1, 0, 0, 0, 0, 0, 0};
				/* 301�ĵ�˵�õ�Initialisation״̬�����������ģ���csBoot_Up==1��ʱ��ִ�еĲ������������ĺ�����
				���ǵ�csBoot_Up==0ʱ�Ż�����������ĺ��������������ģ�Pre_operational״̬csBoot_Up==0��������Pre_operational״̬�Żᷢ���������� */
				
				/* ���ڵ�״̬����Ϊ��ʼ��״̬ */
				d->nodeState = Initialisation;
				/* �л�����״̬ʱ���������߹ر�ͨ�Ŷ�����Ҫ����һЩ���� */
				switchCommunicationState(d, &newCommunicationState); 
				/* call user app init callback now. */
				/* d->initialisation MUST NOT CALL SetState */
				/* �����û��ص������������û��ص������в��������setState���� */
				(*d->initialisation)(d);/* _initialisation */
			}/* ����ط����ǲ���Ҫbreak�����ýڵ��Զ��ɳ�ʼ��״̬ת��Ԥ����״̬ */

			/* Automatic transition - No break statement ! */
			/* Transition from Initialisation to Pre_operational */
			/* is automatic as defined in DS301. */
			/* App don't have to call SetState(d, Pre_operational) */
			/* Ԥ����״̬ */					
			case Pre_operational:
			{
				
				s_state_communication newCommunicationState = {0, 1, 1, 1, 1, 0, 1};
				d->nodeState = Pre_operational;
				switchCommunicationState(d, &newCommunicationState);
                (*d->preOperational)(d);/* _preOperational */
			}
//			break;/* ��һ��Ҳ�����Զ�״̬�л� */
			/* ����״̬ */					
			case Operational:
			/* ������ӳ�ʼ��״ֱ̬�ӱ������״̬ */
			if(d->nodeState == Initialisation) return 0xFF;
			{
				s_state_communication newCommunicationState = {0, 1, 1, 1, 1, 1, 0};
				d->nodeState = Operational;
				newState = Operational;
				switchCommunicationState(d, &newCommunicationState);
				(*d->operational)(d);
			}
			break;
			/* ֹͣ״̬ */			
			case Stopped:
			/* ������ӳ�ʼ��״ֱ̬�ӱ��ֹͣ״̬ */
			if(d->nodeState == Initialisation) return 0xFF;
			{
				s_state_communication newCommunicationState = {0, 0, 0, 0, 1, 0, 1};
				d->nodeState = Stopped;
				newState = Stopped;
				switchCommunicationState(d, &newCommunicationState);
				(*d->stopped)(d);
			}
			break;
			
			default:
			return 0xFF;

		}/* end switch case */
	
	}
	/* d->nodeState contains the final state */
	/* may not be the requested state */
    return d->nodeState;
}

/*!                                                                                                
**                                                                                                 
**                                                                                                 
** @param d                                                                                        
**                                                                                                 
** @return                                                                                         
**/ 
/* ��ȡ����ڵ�ID */
UNS8 getNodeId(CO_Data* d)
{
  return *d->bDeviceNodeId;
}

/*!                                                                                                
**                                                                                                 
**                                                                                                 
** @param d                                                                                        
** @param nodeId                                                                                   
**/  
/* ���ýڵ�ID */
/* ���ýڵ�ID�Ƚϸ��ӣ�����Ҫ�ѽڵ�ID���ó�ָ��ֵ����Ҫ�޸�TPDO��COB-ID,RPDO��COB-ID,SDO_SVR�Ľ��պͷ���COB-ID,EMCY��COB-ID */
void setNodeId(CO_Data* d, UNS8 nodeId)
{
//  UNS16 offset = d->firstIndex->SDO_SVR;
  UNS32 errorCode;

#ifdef CO_ENABLE_LSS
  d->lss_transfer.nodeID=nodeId;
  if(nodeId==0xFF){
  	*d->bDeviceNodeId = nodeId;
  	return;
  }
  else
#endif
  /* �ڵ�ID������ */
  if(!(nodeId>0 && nodeId<=127)){
	  MSG_WAR(0x2D01, "Invalid NodeID", nodeId);
	  return;
  }

//  if(offset)
  if (d->Find_Object(NULL,0x12000000,&errorCode, d)!= &NULL_OBJ)
  {
    /* Adjust COB-ID Client->Server (rx) only id already set to default value or id not valid (id==0xFF)*/
    /* �����˽���SDO������� */
//	if((*(UNS32*)d->objdict[offset].pSubindex[1].pObject == 0x600 + *d->bDeviceNodeId)||(*d->bDeviceNodeId==0xFF)){
    if((*(UNS32*)d->Find_Object(NULL, 0x12000001, &errorCode, d) -> p_content == 0x600 + *d->bDeviceNodeId)||(*d->bDeviceNodeId==0xFF)){
      /* cob_id_client = 0x600 + nodeId; */
	  /* �ͻ���->������cob_id = 0x600 + ������nodeId; */
//      *(UNS32*)d->objdict[offset].pSubindex[1].pObject = 0x600 + nodeId;
		*(UNS32*)d->Find_Object(NULL, 0x12000001, &errorCode, d) -> p_content = 0x600 + nodeId;
    }
    /* Adjust COB-ID Server -> Client (tx) only id already set to default value or id not valid (id==0xFF)*/
    /* �����˷���SDO������� */
//	if((*(UNS32*)d->objdict[offset].pSubindex[2].pObject == 0x580 + *d->bDeviceNodeId)||(*d->bDeviceNodeId==0xFF)){
	if((*(UNS32*)d->Find_Object(NULL, 0x12000002, &errorCode, d) -> p_content == 0x580 + *d->bDeviceNodeId)||(*d->bDeviceNodeId==0xFF)){
      /* cob_id_server = 0x580 + nodeId; */
      /* ������->�ͻ���cob_id = 0x580 + ������nodeId; */
//	  *(UNS32*)d->objdict[offset].pSubindex[2].pObject = 0x580 + nodeId;
	  *(UNS32*)d->Find_Object(NULL, 0x12000002, &errorCode, d) -> p_content = 0x580 + nodeId;
    }
  }

  /* 
   	Initialize the server(s) SDO parameters
  	Remember that only one SDO server is allowed, defined at index 0x1200	
 		
  	Initialize the client(s) SDO parameters 	
  	Nothing to initialize (no default values required by the DS 401)	
  	Initialize the receive PDO communication parameters. Only for 0x1400 to 0x1403 
  */
  /* ��ʼ������PDOͨ�Ų��� */
  {
    UNS8 i = 0;
//    UNS16 offset = d->firstIndex->PDO_RCV;     /* 0x1400 */
//    UNS16 lastIndex = d->lastIndex->PDO_RCV;
//	UNS32 cobID[] = {0x200, 0x300, 0x400, 0x500};
    UNS32 cobID[] = {0x180, 0x280, 0x380, 0x480};
//    if( offset ) 
	if(d->Find_Object(NULL, 0x14000000, &errorCode, d)!=&NULL_OBJ)
	  /* 0x1400-0x1403 */
//	  while( (offset <= lastIndex) && (i < 4)) 
	  while( i < 4) 
	   {
		  /* �����˽���PDO������� */
//          if((*(UNS32*)d->objdict[offset].pSubindex[1].pObject == cobID[i] + *d->bDeviceNodeId)||(*d->bDeviceNodeId==0xFF))
		    if((*(UNS32*)d->Find_Object(NULL, 0x14000001 | i<<16, &errorCode, d)->p_content == cobID[i] + *d->bDeviceNodeId)||(*d->bDeviceNodeId==0xFF))
	         /* RPD1=0x200+nodeId;RPD2=0x300+nodeId;RPD3=0x400+nodeId;RPD4=0x500+nodeId */
//		     *(UNS32*)d->objdict[offset].pSubindex[1].pObject = cobID[i] + nodeId;
//			*(UNS32*)d->Find_Object(NULL,0x14000001 | i<<16,&errorCode)->p_content = cobID[i] + nodeId;
			*(UNS32*)d->Find_Object(NULL,0x14000001 | i<<16, &errorCode, d)->p_content = cobID[i] + NodeID_OF_Opposite;
            i ++;
//          offset ++;
       }
  }
  /* ** Initialize the transmit PDO communication parameters. Only for 0x1800 to 0x1803 */
  /* ��ʼ������PDOͨ�Ų��� */
  {
    UNS8 i = 0;
//    UNS16 offset = d->firstIndex->PDO_TRS;   /* 0x1800 */
//    UNS16 lastIndex = d->lastIndex->PDO_TRS;
    UNS32 cobID[] = {0x180, 0x280, 0x380, 0x480};
    i = 0;
//    if( offset ) 
      if(d->Find_Object(NULL, 0x18000000, &errorCode, d)!=&NULL_OBJ)
	  /* 0x1800-0x1803 */
//	  while ((offset <= lastIndex) && (i < 4)) 
	  while ( i < 4)
		{
		  /* �����˷���PDO������� */
//		  if((*(UNS32*)d->objdict[offset].pSubindex[1].pObject == cobID[i] + *d->bDeviceNodeId)||(*d->bDeviceNodeId==0xFF))
		  if((*(UNS32*)d->Find_Object(NULL, 0x18000001 | i<<16, &errorCode, d)->p_content == cobID[i] + *d->bDeviceNodeId)||(*d->bDeviceNodeId==0xFF))
			 /* TPD1=0x180+nodeId;TPD2=0x280+nodeId;TPD3=0x380+nodeId;TPD4=0x480+nodeId */ 
//		     *(UNS32*)d->objdict[offset].pSubindex[1].pObject = cobID[i] + nodeId;
		     *(UNS32*)d->Find_Object(NULL, 0x18000001 | i<<16, &errorCode, d)->p_content = cobID[i] + nodeId;
		  i ++;
//		  offset ++;
		}
  }

  /* Update EMCY COB-ID if already set to default*/
  /* �����˽������ĵ������ */
  if((*d->error_cobid == *d->bDeviceNodeId + 0x80)||(*d->bDeviceNodeId==0xFF))
	/* EMCY=0x80+nodeId */
    *d->error_cobid = nodeId + 0x80;

  /* bDeviceNodeId is defined in the object dictionary. */
  /* ���ýڵ�ID */
  *d->bDeviceNodeId = nodeId;
}



void TestSlave_initialisation(CO_Data* d)
{

// test OD data
#ifdef	LOGIC_DEBUG_ON
    CANopenSlaveODdataInit(d);
#endif		
	
	
	/*Define SYNC Communicatram*/
	{	
//////////////////////////////////////debug		
//		UNS32 SYNC_COB_ID = 0x00000080|SYNC_COBID_gen_1|SYNC_COBID_frame_0;
//		UNS32 SyncCyclePeriod = SYNC_PERIOD_SLAVE;/* ��λΪus ���޷���32λ���ɱ�ʾ��ʱ�䷶ΧΪ1΢�뵽1.193Сʱ */

//		setODentry(d, 0x1005, 0x00, &SYNC_COB_ID, &sizeUNS32, OD_ACCESS_RW);
//		setODentry(d, 0x1006, 0x00, &SyncCyclePeriod, &sizeUNS32, OD_ACCESS_RW);
///////////////////////////////////////		
		
	}
	
//	/*Define Emergency COB ID*/
//	{
//		UNS32 EMCY_COB_ID = 0x00000080|EMCY_COBID_valid_0|EMCY_COBID_frame_0 + *d->bDeviceNodeId;

//		setODentry(d, 0x1014, 0x00, &EMCY_COB_ID, &sizeUNS32, OD_ACCESS_RW);
//	}
	
//	/* **************************************************************************************************************** */
//	/*Define Consumer HeartBeat time*/
//	{
//		UNS32 ConsumerHeartBeatTime = 65|(NodeID_OF_Opposite<<16);/* ��λΪms��������Ŀֻ��16λ������ʾ����ʱ�䣬���Կɱ�ʾ��ʱ�䷶ΧΪ1ms��65.535s */

//		setODentry(d, 0x1016, 0x01, &ConsumerHeartBeatTime, &sizeUNS32, OD_ACCESS_RW);
//	}

	/*Define Producer HeartBeat time*/
	{
		UNS16 ProducerHeartBeatTime = PRODUCER_HEARTBEAT_TIME;/* ��λΪms���޷���16λ���ɱ�ʾ��ʱ�䷶ΧΪ1ms��65.535s */

		setODentry(d, 0x1017, 0x00, &ProducerHeartBeatTime, &sizeUNS16, OD_ACCESS_RW);
	}
	
//	/* **************************************************************************************************************** */
	/*SSDO and CSDO Configuration*/
	{
		/*SSDO configure */
		
/////////////////////////////////////////////////////////////////////////////////////////////////////		
		{
			UNS32 COB_ID_CLIENT_TO_SERVER_RX = 0x600 + *d->bDeviceNodeId;
			UNS32 COB_ID_SERVER_TO_CLIENT_TX = 0x580 + *d->bDeviceNodeId;
            UNS8  NODE_ID_OF_SDO_ClIENT = NodeID_OF_Opposite;
            
			setODentry(d, 0x1200, 0x01, &COB_ID_CLIENT_TO_SERVER_RX, &sizeUNS32, OD_ACCESS_RW);
			setODentry(d, 0x1200, 0x02, &COB_ID_SERVER_TO_CLIENT_TX, &sizeUNS32, OD_ACCESS_RW);
            setODentry(d, 0x1200, 0x03, &NODE_ID_OF_SDO_ClIENT, &sizeUNS8, OD_ACCESS_RW);
		}		
/////////////////////////////////////////////////////////////////////////////////////////////////////		
		
//		/*CSDO configure */
//		{
//			UNS32 COB_ID_CLIENT_TO_SERVER_TX = 0x600 + NodeID_OF_Opposite;
//			UNS32 COB_ID_SERVER_TO_CLIENT_RX = 0x580 + NodeID_OF_Opposite;
//			UNS8  NODE_ID_OF_SDO_SERVER = NodeID_OF_Opposite;
//			
//			setODentry(d, 0x1280, 0x01, &COB_ID_CLIENT_TO_SERVER_TX, &sizeUNS32, OD_ACCESS_RW);
//			setODentry(d, 0x1280, 0x02, &COB_ID_SERVER_TO_CLIENT_RX, &sizeUNS32, OD_ACCESS_RW);
//			setODentry(d, 0x1280, 0x03, &NODE_ID_OF_SDO_SERVER, &sizeUNS8, OD_ACCESS_RW);
//		}
	}

	/* **************************************************************************************************************** */	
		/*TPDO1 configure */
		{
			UNS32 TPDO1_COBID = 0x180 + *d->bDeviceNodeId;
			UNS8  TPDO1TransmissionType = TRANS_EVERY_N_SYNC(1);		
			//UNS8  TPDO1TransmissionType = TRANS_EVENT_SPECIFIC;	
            UNS32 TPDO1_inhibit_time = 0;        // unit: 100��s
			UNS32 TPDO1_event_time = 20;         // unit: 1ms

			// �������(��)ϥ�ؽ�,��ʵ��λ�á�Ŀ��λ�÷���������
			if(( 2 == *d->bDeviceNodeId) || ( 4 == *d->bDeviceNodeId)) 
			{
				UNS32 TPDO1MappingValue1 = 0x60640020;		
				UNS32 TPDO1MappingValue2 = 0x60620020;	
				setODentry(d, 0x1800, 0x01, &TPDO1_COBID, &sizeUNS32, OD_ACCESS_RW);
				setODentry(d, 0x1800, 0x02, &TPDO1TransmissionType, &sizeUNS8, OD_ACCESS_RW);
				setODentry(d, 0x1800, 0x03, &TPDO1_inhibit_time, &sizeUNS32, OD_ACCESS_RW);				
				setODentry(d, 0x1800, 0x05, &TPDO1_event_time,   &sizeUNS32, OD_ACCESS_RW);
				setODentry(d, 0x1A00, 0x01, &TPDO1MappingValue1, &sizeUNS32, OD_ACCESS_RW);
				setODentry(d, 0x1A00, 0x02, &TPDO1MappingValue2, &sizeUNS32, OD_ACCESS_RW);				
			}
		}
		
	/*RPDO Configuration*/
	{	
		/*RPDO1 configure */
		{   // ��������Źؽڣ��������ϥ�ؽ�λ�ã���������Źؽڣ��������ϥ�ؽ�λ��
			if((1 == *d->bDeviceNodeId)) 
			{				
				UNS32 RPDO1_COBID = 0x182;
				UNS8  RPDO1TransmissionType = TRANS_EVENT_SPECIFIC;		
				UNS32 RPDO1MappingValue1 = 0x2F090020;							//��ȡ��ϥ�ؽ�ʵ��λ��	
				UNS32 RPDO1MappingValue2 = 0x2F050020;							//��ȡ��ϥ�ؽ�Ŀ��λ��
					
				setODentry(d, 0x1400, 0x01, &RPDO1_COBID, &sizeUNS32, OD_ACCESS_RW);
				setODentry(d, 0x1400, 0x02, &RPDO1TransmissionType, &sizeUNS8, OD_ACCESS_RW);
				setODentry(d, 0x1600, 0x01, &RPDO1MappingValue1, &sizeUNS32, OD_ACCESS_RW);
				setODentry(d, 0x1600, 0x02, &RPDO1MappingValue2, &sizeUNS32, OD_ACCESS_RW);
			}
			if( 3 == *d->bDeviceNodeId) 
			{
				UNS32 RPDO2_COBID = 0x184;
				UNS8  RPDO2TransmissionType = TRANS_EVENT_SPECIFIC;
				UNS32 RPDO2MappingValue1 = 0x2F100020;							//��ȡ��ϥ�ؽ�ʵ��λ��	
				UNS32 RPDO2MappingValue2 = 0x2F100020;							//��ȡ��ϥ�ؽ�Ŀ��λ��
					
				setODentry(d, 0x1401, 0x01, &RPDO2_COBID, &sizeUNS32, OD_ACCESS_RW);
				setODentry(d, 0x1401, 0x02, &RPDO2TransmissionType, &sizeUNS8, OD_ACCESS_RW);
				setODentry(d, 0x1601, 0x01, &RPDO2MappingValue1, &sizeUNS32, OD_ACCESS_RW);		
				setODentry(d, 0x1601, 0x02, &RPDO2MappingValue2, &sizeUNS32, OD_ACCESS_RW);
			}	
		}
		
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/*debug active control*/
		/*RPDO2 configure */		
       {
			UNS32 RPDO3_COBID = 0x18D;				
			UNS8  RPDO3TransmissionType = TRANS_EVENT_SPECIFIC;
			UNS32 RPDO3MappingValue1 = 0x2F500020;							//��ȡ�������ƿ����֣���������ģʽ����
				
			setODentry(d, 0x1402, 0x01, &RPDO3_COBID, &sizeUNS32, OD_ACCESS_RW);
			setODentry(d, 0x1402, 0x02, &RPDO3TransmissionType, &sizeUNS8, OD_ACCESS_RW);
			setODentry(d, 0x1602, 0x01, &RPDO3MappingValue1, &sizeUNS32, OD_ACCESS_RW);
			//setODentry(d, 0x1602, 0x01, &RPDO3MappingValue1, &sizeUNS32, OD_ACCESS_RW);		   
	   }
	}
}


void CANopenMaster_initialisation(CO_Data* d)
{
	
// test OD data
#ifdef	LOGIC_DEBUG_ON
    CANopenMasterODdataInit(d);
#endif		
	
	/*Define SYNC Communicatram*/
	{	
		UNS32 SYNC_COB_ID = 0x00000080|SYNC_COBID_gen_1|SYNC_COBID_frame_0;
		UNS32 SyncCyclePeriod = SYNC_PERIOD_MASTER;/* ��λΪus ���޷���32λ���ɱ�ʾ��ʱ�䷶ΧΪ1΢�뵽1.193Сʱ */

		setODentry(d, 0x1005, 0x00, &SYNC_COB_ID, &sizeUNS32, OD_ACCESS_RW);
		setODentry(d, 0x1006, 0x00, &SyncCyclePeriod, &sizeUNS32, OD_ACCESS_RW);
	}

//	/* **************************************************************************************************************** */
	/*SSDO and CSDO Configuration*/
	{
		/*SSDO configure */
		{
            UNS32 COB_ID_CLIENT_TO_SERVER_RX = 0x600 + *d->bDeviceNodeId;
            UNS32 COB_ID_SERVER_TO_CLIENT_TX = 0x580 + *d->bDeviceNodeId;
            UNS8  NODE_ID_OF_SDO_ClIENT = NodeID_OF_Opposite;
            
            setODentry(d, 0x1200, 0x01, &COB_ID_CLIENT_TO_SERVER_RX, &sizeUNS32, OD_ACCESS_RW);
            setODentry(d, 0x1200, 0x02, &COB_ID_SERVER_TO_CLIENT_TX, &sizeUNS32, OD_ACCESS_RW);
            setODentry(d, 0x1200, 0x03, &NODE_ID_OF_SDO_ClIENT, &sizeUNS8, OD_ACCESS_RW);
		}		
		
		/*CSDO configure */
		{
			UNS32 COB_ID_CLIENT_TO_SERVER_TX = 0x600 + NodeID_OF_Opposite;
			UNS32 COB_ID_SERVER_TO_CLIENT_RX = 0x580 + NodeID_OF_Opposite;
			UNS8  NODE_ID_OF_SDO_SERVER = NodeID_OF_Opposite;
			
			setODentry(d, 0x1280, 0x01, &COB_ID_CLIENT_TO_SERVER_TX, &sizeUNS32, OD_ACCESS_RW);
			setODentry(d, 0x1280, 0x02, &COB_ID_SERVER_TO_CLIENT_RX, &sizeUNS32, OD_ACCESS_RW);
			setODentry(d, 0x1280, 0x03, &NODE_ID_OF_SDO_SERVER, &sizeUNS8, OD_ACCESS_RW);
		}
	}
		
    /*TPDO1 configure */
    {
        UNS32 TPDO1_COBID = 0x180 + 7;
        UNS8  TPDO1TransmissionType = TRANS_EVERY_N_SYNC(1);
			
        //����CSPλ��
        UNS32 TPDO1MappingValue1 = 0x2F120020;      //CSPģʽ��desiredλ��			
        setODentry(d, 0x1800, 0x01, &TPDO1_COBID, &sizeUNS32, OD_ACCESS_RW);
        setODentry(d, 0x1800, 0x02, &TPDO1TransmissionType, &sizeUNS8, OD_ACCESS_RW);
        setODentry(d, 0x1A00, 0x01, &TPDO1MappingValue1, &sizeUNS32, OD_ACCESS_RW);	
    }	
	
	/*RPDO Configuration*/
	{	
		/*RPDO1 configure */
        //UNS32 RPDO1_COBID = 0x200 + NodeID_OF_Opposite;
		//UNS8  RPDO1TransmissionType = TRANS_EVERY_N_SYNC(2);		
        UNS32 RPDO1_COBID = 0x1A0;				
		UNS8  RPDO1TransmissionType = TRANS_EVENT_SPECIFIC;
		
		UNS32 RPDO1MappingValue1 = 0x60640020;							//��ȡʵ��λ��
		UNS32 RPDO1MappingValue2 = 0x60780020;							//��ȡʵ�ʵ���
			
		setODentry(d, 0x1400, 0x01, &RPDO1_COBID, &sizeUNS32, OD_ACCESS_RW);
		setODentry(d, 0x1400, 0x02, &RPDO1TransmissionType, &sizeUNS8, OD_ACCESS_RW);
		setODentry(d, 0x1600, 0x01, &RPDO1MappingValue1, &sizeUNS32, OD_ACCESS_RW);
		setODentry(d, 0x1600, 0x02, &RPDO1MappingValue2, &sizeUNS32, OD_ACCESS_RW);

		
		/*RPDO2 configure */
        UNS32 RPDO2_COBID = 0x2A0;				
		UNS8  RPDO2TransmissionType = TRANS_EVENT_SPECIFIC;			
		UNS32 RPDO2MappingValue1 = 0x60770010;							//��ȡʵ��Ť��
		UNS32 RPDO2MappingValue2 = 0x606C0020;							//��ȡʵ���ٶ�			
		setODentry(d, 0x1401, 0x01, &RPDO2_COBID, &sizeUNS32, OD_ACCESS_RW);
		setODentry(d, 0x1401, 0x02, &RPDO2TransmissionType, &sizeUNS8, OD_ACCESS_RW);
		setODentry(d, 0x1601, 0x01, &RPDO2MappingValue1, &sizeUNS32, OD_ACCESS_RW);
		setODentry(d, 0x1601, 0x02, &RPDO2MappingValue2, &sizeUNS32, OD_ACCESS_RW);	
		
		/*RPDO3 configure */
		UNS32 RPDO3_COBID = 0x3A0;		
		UNS8  RPDO3TransmissionType = TRANS_EVENT_SPECIFIC;
		UNS32 RPDO3MappingValue1 = 0x2F040010;							//��ȡ״̬��
		//UNS32 RPDO3MappingValue2 = 0x60410010;						//��ȡ״̬��
		UNS32 RPDO3MappingValue2 = 0x2F020010;							//��ȡ���ϴ���	
		
		setODentry(d, 0x1402, 0x01, &RPDO3_COBID, &sizeUNS32, OD_ACCESS_RW);
		setODentry(d, 0x1402, 0x02, &RPDO3TransmissionType, &sizeUNS8, OD_ACCESS_RW);
		setODentry(d, 0x1602, 0x01, &RPDO3MappingValue1, &sizeUNS32, OD_ACCESS_RW);
		setODentry(d, 0x1602, 0x02, &RPDO3MappingValue2, &sizeUNS32, OD_ACCESS_RW);	
	}
}


void _preOperational(CO_Data* d)
{
    if (!(*(d->iam_a_slave)))/* �������վ���㲥��NodeId����Ϊ0���ɣ�NMT��λ���λ���дӽڵ� */
    {
        masterSendNMTstateChange (d, 0, NMT_Start_Node);
    }
}

//Cost timetest3=
//		{
//			1,
//			8,
//			16,
//			11,
//			30,
//			19,
//			53,
//			20,            
//			4,
//			15,
//			36,
//			59
//		};
		
//Time timetest4=
//		{
//			2017,
//			7,
//			27,
//			4,
//			11,
//			47,
//			28,
//			137,
//			593,
//		};
				
void _operational(CO_Data* d)
{
//	SetAlarm(d,  0, (void*)canSend_alarm ,  1000,  10000);//#define TIMEVAL UNS32
//	UNS32 abortCode=0;
//	UNS32 AbortCode=0;
//	UNS32 sizeTime=Timelength;
//	UNS32 sizeCost=Costlength;
	
//	writeNetworkDictCallBackAI (d,  NodeID_OF_Opposite, 0x2004, 0x00, Timelength, TIME, &timetest1,  NULL,  0,  1);
//	while (getWriteResultNetworkDict (d, NodeID_OF_Opposite, &abortCode) == SDO_BLOCK_DOWNLOAD_IN_PROGRESS);
	/* �������������ôһ�䣬��Ҫ�����ǿ����ػ���һ��������ѡ������Ǹ�״̬��������Ϊ�ǿ����أ������õ���SDO_BLOCK_DOWNLOAD_IN_PROGRESS */
	/* û��������һ��ĺ���Ǵ���ͨ��d->transfers[line]����һ�δ���󲻻��ͷţ���һֱռ�ŵ��º���Ĵ���ʼ���ò�����һ������ͨ����
	�����Դ�˷ѣ��Ȼ��۵�һ���ĳ̶ȣ����еĴ���ͨ���������ˣ���Ҫ���д���Ļ��ͻ�ʧ�ܣ���Ϊ���ʱ����Ҳ������õĴ���ͨ�� */
//	writeNetworkDictCallBackAI (d,  NodeID_OF_Opposite, 0x2006, 0x00, Costlength, COST, &timetest3,  NULL,  0,  1);
//	while (getWriteResultNetworkDict (d, NodeID_OF_Opposite, &abortCode) == SDO_BLOCK_DOWNLOAD_IN_PROGRESS);
	
//	readNetworkDictCallbackAI (d, NodeID_OF_Opposite, 0x2005,  0x00,  TIME, NULL,  1);
//	while (getReadResultNetworkDict (d, NodeID_OF_Opposite, &timetest4, &sizeTime,&abortCode) == SDO_BLOCK_UPLOAD_IN_PROGRESS);
//	/* ͬ�����������������һ�䣬ͬ��Ҳ��û���Ͳ����ͷŴ���ͨ�� */

//	readNetworkDictCallbackAI (d, NodeID_OF_Opposite, 0x2006,  0x00,  COST, NULL,  1);
//	while (getReadResultNetworkDict (d, NodeID_OF_Opposite, &timetest3, &sizeCost,&abortCode) == SDO_BLOCK_UPLOAD_IN_PROGRESS);
//	/* ͬ�����������������һ�䣬ͬ��Ҳ��û���Ͳ����ͷŴ���ͨ�� */
}
void _stopped(CO_Data* d){}
