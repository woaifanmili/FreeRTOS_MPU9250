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
#include "pdo.h"
#include "objacces.h"
#include "canfestival.h"
#include "sysdep.h"
#include "ObjectDictionary.h"
/*!
** @file   pdo.c
** @author Edouard TISSERANT and Francis DUPIN
** @date   Tue Jun  5 09:32:32 2007
**
** @brief
**
**
*/

/*!
**
**
** @param d
** @param TPDO_com TPDO communication parameters OD entry
** @param TPDO_map TPDO mapping parameters OD entry
**
** @return  0���ɹ�����0��ʧ��
**/
/* ��TPDOӳ�����ӳ���ֵ����������ͨ���Ļ������� */
UNS8 buildPDO (CO_Data * d, UNS8 numPdo, Message * pdo)
{
	 UNS32 ErrorCode;
  /* PDOͨ�Ų��� */
  /* PDOӳ����� */
//  const OD_OI_TypeDef *TPDO_com = d->objdict + d->firstIndex->PDO_TRS + numPdo;/* 1800 */
//  const OD_OI_TypeDef *TPDO_map = d->objdict + d->firstIndex->PDO_TRS_MAP + numPdo;/* 1A00 */
	const OD_OI_TypeDef *TPDO_com = d->Find_Object(NULL, 0x18000000|(numPdo<<16), &ErrorCode, d);
	const OD_OI_TypeDef *TPDO_map = d->Find_Object(NULL, 0x1A000000|(numPdo<<16), &ErrorCode, d);
//	switch (numPdo)
//	{
//		case 1: TPDO_com = d->Find_Object("TPDO1 SUbCount", 0, &errorCode);TPDO_map = d->Find_Object("TPDO1 MappingCount", 0, &errorCode);break;
//		case 2: TPDO_com = d->Find_Object("TPDO2 SUbCount", 0, &errorCode);TPDO_map = d->Find_Object("TPDO2 MappingCount", 0, &errorCode);break;
//		case 3: TPDO_com = d->Find_Object("TPDO3 SUbCount", 0, &errorCode);TPDO_map = d->Find_Object("TPDO3 MappingCount", 0, &errorCode);break;
//		case 4: TPDO_com = d->Find_Object("TPDO4 SUbCount", 0, &errorCode);TPDO_map = d->Find_Object("TPDO4 MappingCount", 0, &errorCode);break;
//		case 5: TPDO_com = d->Find_Object("TPDO5 SUbCount", 0, &errorCode);TPDO_map = d->Find_Object("TPDO5 MappingCount", 0, &errorCode);break;
//		default:break;
//	}
  UNS8 prp_j = 0x00;
  UNS32 offset = 0x00000000;
  /* �������� */
//  const UNS8 *pMappingCount = (UNS8 *) TPDO_map->pSubindex[0].pObject;/* 1A00 */
    const UNS8 *pMappingCount = (UNS8 *)(d->Find_Object(NULL, 0x1A000000|(numPdo<<16), &ErrorCode, d))->p_content;
  /* cob_id */
//  pdo->cob_id = (UNS16) UNS16_LE(*(UNS32*)TPDO_com->pSubindex[1].pObject & 0x7FF);/* 1800 */
    pdo->cob_id = (UNS16) UNS16_LE(*(UNS32*)(d->Find_Object(NULL, 0x18000001|(numPdo<<16), &ErrorCode, d))->p_content);
  pdo->rtr = NOT_A_REQUEST;

  MSG_WAR (0x3009, "  PDO CobId is : ", *(UNS32 *) TPDO_com->pSubindex[1].pObject);
  MSG_WAR (0x300D, "  Number of objects mapped : ", *pMappingCount);

  do
    {
      UNS8 dataType;            /* Unused */
      UNS8 tmp[] = { 0, 0, 0, 0, 0, 0, 0, 0 };  /* temporary space to hold bits */

      /* pointer fo the var which holds the mapping parameter of an mapping entry  */
	  /* ӳ����е�ֵ */
//      UNS32 *pMappingParameter = (UNS32 *) TPDO_map->pSubindex[prp_j + 1].pObject;
	    UNS32 *pMappingParameter = (UNS32 *) ((d->Find_Object(NULL, 0x1A000000|(numPdo<<16)|(prp_j + 1), &ErrorCode, d))->p_content);
	  /* ���� */
      UNS16 index = (UNS16) ((*pMappingParameter) >> 16);
	  /* λ�� */
      UNS32 Size = (UNS32) (*pMappingParameter & (UNS32) 0x000000FF);     /* Size in bits */

      /* get variable only if Size != 0 and Size is lower than remaining bits in the PDO */
	  /* ��ഫ8���ֽڣ����˾ͻᶪ��������һ��PDOӳ��ĵı�����λ����Ҫ����64����8�ֽ� */
      if (Size && ((offset + Size) <= 64))
        {
		  /* λ��ת��Ϊ�ֽڣ�1-8bitΪһ���ֽ� */
          UNS32 ByteSize = 1 + ((Size - 1) >> 3);        /*1->8 => 1 ; 9->16 => 2, ... */
		  /* ������ */
          UNS8 subIndex = (UNS8) (((*pMappingParameter) >> (UNS8) 8) & (UNS32) 0x000000FF);

          MSG_WAR (0x300F, "  got mapping parameter : ", *pMappingParameter);
          MSG_WAR (0x3050, "    at index : ", TPDO_map->index);
          MSG_WAR (0x3051, "    sub-index : ", prp_j + 1);
          /* ���ֵ�����ȡ�����ݺ��������ͷŵ���ʱ���� */
//          if (getODentry (d, index, subIndex, tmp, &ByteSize, &dataType, 0) != OD_SUCCESSFUL)
			if (getODentry (d, index, subIndex, tmp, &ByteSize, &dataType, 0) != OD_SUCCESSFUL)
            {
              MSG_ERR (0x1013, " Couldn't find mapped variable at index-subindex-size : ", (UNS32) (*pMappingParameter));
              return 0xFF;
            }
          /* copy bit per bit in little endian */
		  /* ����ʱ����������ݿ�����PDO������ */
          CopyBits ((UNS8) Size, ((UNS8 *) tmp), 0, 0, (UNS8 *) & pdo->data[offset >> 3], (UNS8)(offset % 8), 0);
          /* ����ƫ���� */
          offset += Size;
        }
      prp_j++;
    }
  while (prp_j < *pMappingCount);
  /* һ��Ҫ������ֽ��� */
  pdo->len = (UNS8)(1 + ((offset - 1) >> 3));

  MSG_WAR (0x3015, "  End scan mapped variable", 0);

  return 0;
}

/*!
**
**
** @param d
** @param cobId
**
** @return  0x00: ����ɹ� ; 0xFF: ����ʧ��
**/
/* ��վ����PDO���� */
/* �������в�δʹ�� */
UNS8 sendPDOrequest (CO_Data * d, UNS16 RPDOIndex)
{
  UNS16 *pwCobId;
  UNS32 ErrorCode;
//  /* ָ���һ��PDOͨ�Ų�������0x1400 */
//  UNS16 offset = d->firstIndex->PDO_RCV;
//  /* ָ�����һ��PDOͨ�Ų������� */
//  UNS16 lastIndex = d->lastIndex->PDO_RCV;
    pwCobId = (UNS16*)(d->Find_Object(NULL, 0x00000001|RPDOIndex<<16, &ErrorCode, d)->p_content);	
  /* ��ǰ����״̬����PDO���� */
  if (!d->CurrentCommunicationState.csPDO)
    {
      return 0;
    }
  /* Sending the request only if the cobid have been found on the PDO
     receive */
  /* part dictionary */
  MSG_WAR (0x3930, "sendPDOrequest RPDO Index : ", RPDOIndex);
  /* RPDO��0x1400-0x15FF */
//  if (offset && RPDOIndex >= 0x1400)/* '>=' ���ȼ����� '&&' */
	if (ErrorCode == OD_SUCCESSFUL)/* '>=' ���ȼ����� '&&' */
    {
//	  /* ���RPDO���� */
//      offset += RPDOIndex - 0x1400;
//      if (offset <= lastIndex)
        {
          /* get the CobId */
//          pwCobId = d->objdict[offset].pSubindex[1].pObject;

          MSG_WAR (0x3930, "sendPDOrequest cobId is : ", *pwCobId);
          {
			/* ����PDO���� */
            Message pdo;
            pdo.cob_id = UNS16_LE(*pwCobId);
            pdo.rtr = REQUEST;
            pdo.len = 0;
			return canSend (d, &pdo);/* 0x00: �ɹ� ; 0xFF: ʧ��*/
          }
        }
    }
  MSG_ERR (0x1931, "sendPDOrequest : RPDO Index not found : ", RPDOIndex);
  return 0xFF;/* ���е���˵������ʧ�� */
}


/*!
**
**
** @param d
** @param m
**
** @return
**/
/* ����PDO���� */
UNS8 proceedPDO (CO_Data * d, Message * m)
{
  UNS8 numPdo;
  UNS8 numMap;                  /* Number of the mapped varable */
  UNS8 *pMappingCount = NULL;   /* count of mapped objects... */
  /* pointer to the var which is mapped to a pdo... */
  /*  void *     pMappedAppObject = NULL;   */
  /* pointer fo the var which holds the mapping parameter of an
     mapping entry */
  UNS32 *pMappingParameter = NULL;
  UNS32 errorCode;
  UNS8 *pTransmissionType = NULL;       /* pointer to the transmission
                                           type */
  UNS16 *pwCobId = NULL;
  UNS8 Size;
  UNS8 offset;
  UNS8 status;
  UNS32 objDict;

  UNS32 cob_id = 0;
////  UNS16 offsetObjdict;
//  UNS16 lastIndex;

  status = state2;
  cob_id = UNS16_LE(m->cob_id) & 0x7ff;
  MSG_WAR (0x3935, "proceedPDO, cobID : ", (UNS16_LE(m->cob_id) & 0x7ff));
  
	  
  
  //////////////////////
  offset = 0x00;
  numPdo = 0;
  numMap = 0;
  /* ��վ���յ�PDO���� */
  /* Flaxin ���յ�PDO���ݾ͸Ͻ������� */
  if ((*m).rtr == NOT_A_REQUEST)
    {
	  /* ��һ������PDOͨ�Ų������� */
//      offsetObjdict = d->firstIndex->PDO_RCV;
//	  /* ���һ������PDOͨ�Ų������� */
//      lastIndex = d->lastIndex->PDO_RCV;
//      /* offsetObjdict��Ϊ0 */
//      if (offsetObjdict)
		/* �������н���PDO */
//        while (offsetObjdict <= lastIndex)
	      while (d->Find_Object(NULL, 0x14000000 | numPdo<<16, &errorCode, d) != &NULL_OBJ)
          {
            switch (status)
              {
              case state2:
				/* cob_id */
			  
//                pwCobId = d->objdict[offsetObjdict].pSubindex[1].pObject;
			      pwCobId = d->Find_Object(NULL, 0x14000001 | numPdo<<16, &errorCode, d) -> p_content;
			    /* ��վ�ҳ��ñ������ĸ�����PDO���� */
                if (*pwCobId == UNS16_LE(m->cob_id))
                  {
                    /* The cobId is recognized */
					/* һ���ҵ��͸���״̬�����д��� */
                    status = state4;
                    MSG_WAR (0x3936, "cobId found at index ", 0x1400 + numPdo);
                    break;
                  }
				/* ����Ӧ�ͼ����ж���һ�� */
                else
                  {
                    /* received cobId does not match */
                    numPdo++;
//                    offsetObjdict++;
                    status = state2;
                    break;
                  }

              case state4:     /* Get Mapped Objects Number */
                /* The cobId of the message received has been found in the
                   dictionnary. */
//			    /* ָ���һ������PDOӳ��������� */
//                offsetObjdict = d->firstIndex->PDO_RCV_MAP;
//			    /* ָ�����һ������PDOӳ��������� */			  
//                lastIndex = d->lastIndex->PDO_RCV_MAP;
			    /* �������� */
//                pMappingCount = (UNS8 *) (d->objdict + offsetObjdict + numPdo)->pSubindex[0].pObject;
			    pMappingCount = (UNS8 *)d->Find_Object(NULL, 0x16000000 | numPdo<<16, &errorCode, d) -> p_content;
                numMap = 0;
			    /* �������������� */
                while (numMap < *pMappingCount)
                  {
                    UNS8 tmp[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
                    UNS32 ByteSize;
//                    pMappingParameter = (UNS32 *) (d->objdict + offsetObjdict + numPdo)->pSubindex[numMap + 1].pObject;
					  pMappingParameter = (UNS32 *)(d->Find_Object(NULL, (0x16000000 | (numPdo<<16) | (numMap + 1)), &errorCode, d) -> p_content);
                    if (pMappingParameter == NULL)
                      {
                        MSG_ERR (0x1937, "Couldn't get mapping parameter : ",numMap + 1);
                        return 0xFF;
                      }
                    /* Get the addresse of the mapped variable. */
                    /* detail of *pMappingParameter : */
                    /* The 16 hight bits contains the index, the medium 8 bits
                       contains the subindex, */
                    /* and the lower 8 bits contains the size of the mapped
                       variable. */
                    /* λ�� */
                    Size = (UNS8) (*pMappingParameter & (UNS32) 0x000000FF);

                    /* set variable only if Size != 0 and 
                     * Size is lower than remaining bits in the PDO */
					/* PDO������8���ֽ� */
                    if (Size && ((offset + Size) <= (m->len << 3)))
                      {
                        /* copy bit per bit in little endian */
						/* �����ݴӻ������п�������ʱ������ */
                        CopyBits (Size, (UNS8 *) & m->data[offset >> 3], offset % 8, 0, ((UNS8 *) tmp), 0, 0);
                        /*1->8 => 1 ; 9->16 =>2, ... */
						/* λ��ת��Ϊ�ֽڣ�1~8bitΪһ���ֽ� */
                        ByteSize = (UNS32)(1 + ((Size - 1) >> 3));
                        /* �����ݴ���ʱ��������д���ֵ��� */
                        objDict = setODentry (d, (UNS16)((*pMappingParameter) >> 16),(UNS8)(((*pMappingParameter) >> 8) & 0xFF), tmp, &ByteSize, 0);

                        if (objDict != OD_SUCCESSFUL)
                          {
                            MSG_ERR (0x1938,"error accessing to the mapped var : ",numMap + 1);
                            MSG_WAR (0x2939, "Mapped at index : ",(*pMappingParameter) >> 16);
                            MSG_WAR (0x2940, "subindex : ",((*pMappingParameter) >> 8) & 0xFF);
                            return 0xFF;
                          }

                        MSG_WAR (0x3942,"Variable updated by PDO cobid : ",UNS16_LE(m->cob_id));
                        MSG_WAR (0x3943, "Mapped at index : ",(*pMappingParameter) >> 16);
                        MSG_WAR (0x3944, "subindex : ",((*pMappingParameter) >> 8) & 0xFF);
                        offset += Size;
                      }
                    numMap++;
                  }             /* end loop while on mapped variables */
				/* ����PDO�¼���ʱ�����¼�ʱ������涨ʱ����û���յ����ģ��ͻص�_RxPDO_EventTimers_Handler���� */
                if (d->RxPDO_EventTimers)/* �������и�RxPDO_EventTimers����ֵ��0��������������������ǲ����� */
                {
//                    TIMEVAL EventTimerDuration = *(UNS16 *)d->objdict[offsetObjdict].pSubindex[5].pObject;
					  TIMEVAL EventTimerDuration = *(UNS16 *)(d->Find_Object(NULL, (0x14000005 | (numPdo<<16)), &errorCode, d) -> p_content);
					
                    if(EventTimerDuration)
					{
						/* ɾ����ʱ�� */
                        DelAlarm (d->RxPDO_EventTimers[numPdo], d->RxPDO_EventTimers_Handler, d);
                        CAN_ITConfig(d->canHandle, CAN_IT_FMP0, ENABLE);//�ú������ڿ������жϴ��
						/* �趨��ʱ�� */
                        d->RxPDO_EventTimers[numPdo] = SetAlarm (d, numPdo, d->RxPDO_EventTimers_Handler/* �պ��� */, MS_TO_TIMEVAL (EventTimerDuration), 0);
                        CAN_ITConfig(d->canHandle, CAN_IT_FMP0, ENABLE);//�ú������ڿ������жϴ��
                    }
                }
                return 0;
              }                 /* end switch status */
          }                     /* end while */
    }                           /* end if Donnees */
  /* ��վ���յ�PDOԶ������ */
  else if ((*m).rtr == REQUEST)
    {
      MSG_WAR (0x3946, "Receive a PDO request cobId : ", UNS16_LE(m->cob_id));
      status = state1;
//	  /* ָ���һ������PDO���� */
//      offsetObjdict = d->firstIndex->PDO_TRS;
//	  /* ָ�����һ������PDO���� */		
//      lastIndex = d->lastIndex->PDO_TRS;
//      if (offsetObjdict)
		/* �������з���PDO */
//        while (offsetObjdict <= lastIndex)
		  while (d->Find_Object(NULL, 0x18000000 | numPdo<<16, &errorCode, d)!=&NULL_OBJ)
          {
            /* study of all PDO stored in the objects dictionary */
            switch (status)
              {
              case state1:     /* check the CobId */
                /* get CobId of the dictionary which match to the received PDO
                 */
			    /* cob_id */
//                pwCobId =(d->objdict + offsetObjdict)->pSubindex[1].pObject;
			  	  pwCobId = d->Find_Object(NULL, 0x18000001 | numPdo<<16, &errorCode, d) -> p_content;
			    /* ��վ�ҳ��ñ������ĸ�PDO���� */
                if (*pwCobId == UNS16_LE(m->cob_id))
                  {
                    status = state4;
                    break;
                  }
				/* ��ƥ��ͼ����ж���һ�� */
                else
                  {
                    numPdo++;
//                    offsetObjdict++;
                  }
                status = state1;
                break;

              case state4:     /* check transmission type */
				/* �������� */
//                pTransmissionType = (UNS8 *) d->objdict[offsetObjdict].pSubindex[2].pObject;
			    pTransmissionType = (UNS8 *)(d->Find_Object(NULL, (0x18000002 | (numPdo<<16) ), &errorCode, d) -> p_content);
                /* If PDO is to be sampled and send on RTR, do it */
			    /* PDOԶ�����󣬷���Ӧ��PDO */
                if (*pTransmissionType == TRANS_RTR)/* #define TRANS_RTR  253   Transmission on request *//* ��RTR(�¼�����) */
                  {
                    status = state5;
                    break;
                  }
                /* RTR_SYNC means data prepared at SYNC, transmitted on RTR */
				/* Ϊͬ��Զ��PDO */
                else if (*pTransmissionType == TRANS_RTR_SYNC)/* #define TRANS_RTR_SYNC  252 *//* (��RTR(ͬ��)) */
                  {
					/* ���ͬ��Զ��PDO�Ѿ�׼�������ݣ�����Ӧ��PDO */
                    if (d->PDO_status[numPdo].transmit_type_parameter & PDO_RTR_SYNC_READY)/* Flaxin �����һ��׼���������ݣ��ͽ���һ��׼���õ����ݷ��ͳ�ȥ */
                      {
                        /*Data ready, just send */
                        canSend (d, &d->PDO_status[numPdo].last_message);
                        return 0;
                      }
					/* û׼���� *//* Flaxin �����һ��û׼�������ݣ��ͽ���һ�β�ѯ����TPDO���ͳ�ȥ������state5��ȥִ�з������� */
                    else
                      {
                        /* if SYNC did never occur, transmit current data */
                        /* DS301 do not tell what to do in such a case... */
						/* ���� */
                        MSG_ERR (0x1947,"Not ready RTR_SYNC TPDO send current data : ",UNS16_LE(m->cob_id));
                        status = state5;
                      }
                    break;
                  }
				/* �¼����� */
                else if ((*pTransmissionType == TRANS_EVENT_PROFILE) ||
                         (*pTransmissionType == TRANS_EVENT_SPECIFIC))
                  {
                    /* Zap all timers and inhibit flag */
					/* ɾ���¼���ʱ�� */
                    d->PDO_status[numPdo].event_timer = DelAlarm (d->PDO_status[numPdo].event_timer, &PDOEventTimerAlarm, d);
                    CAN_ITConfig(d->canHandle,CAN_IT_FMP0, ENABLE);//�ú������ڿ������жϴ��
					/* ɾ����ֹ��ʱ�� */					  
                    d->PDO_status[numPdo].inhibit_timer = DelAlarm(d->PDO_status[numPdo].inhibit_timer, &PDOInhibitTimerAlarm, d);
                    CAN_ITConfig(d->canHandle,CAN_IT_FMP0, ENABLE);//�ú������ڿ������жϴ��
                    d->PDO_status[numPdo].transmit_type_parameter &= ~PDO_INHIBITED;
                    /* Call  PDOEventTimerAlarm for this TPDO, 
                     * this will trigger emission et reset timers */
					/* ����PDO���ģ�������������ʱ�¼� */
                    PDOEventTimerAlarm (d, numPdo);
                    return 0;
                  }
                else
                  {
                    /* The requested PDO is not to send on request. So, does
                       nothing. */
                    MSG_WAR (0x2947, "PDO is not to send on request : ",UNS16_LE(m->cob_id));
                    return 0xFF;
                  }
              /* ����PDO���� */
              case state5:     /* build and send requested PDO */
                {
                  Message pdo;
                  if (buildPDO (d, numPdo, &pdo))
                    {
                      MSG_ERR (0x1948, " Couldn't build TPDO number : ", numPdo);
                      return 0xFF;
                    }
                  canSend (d, &pdo);
			      // ��������ռ�����������ͣ�ʵ�ַ�ʽ���ã���PDO�������ʱ������ȵ�����PDO����Ӳ��������ɺ���ܽ���whileִ�к����ָ������޸�Ϊ���л��塣
			      // ��CAN���߹���ʱ����ɳ���������жϷ�������У����������ͱȸ��жϷ���������ȼ��͵ĳ�������							
                  while(!((d->canHandle->TSR & 0x10000000)|(d->canHandle->TSR & 0x08000000)|(d->canHandle->TSR & 0x04000000)))
                  {
				  };/* ��������ֻ��3����������̫�� */					
					
                  return 0;
                }
              }                 /* end switch status */
          }                     /* end while */
    }                           /* end if Requete */

  return 0;
}

/*!
**
**
** @param NbBits
** @param SrcByteIndex
** @param SrcBitIndex
** @param SrcBigEndian
** @param DestByteIndex
** @param DestBitIndex
** @param DestBigEndian
**//* SrcBitIndex��DestBitIndex�����ݿ����ڵ�һ���ֽ��е���ʼλ�ã�0~7λ��������ȡֵֻ��Ϊ0~7������ֱ�Ӵ��� */
/* ��SrcByteIndex�Ĵ�SrcBitIndex��ʼ��NbBits��λ�����ݿ�����DestByteIndex��DestBitIndex��ʼ��λ���� */
void CopyBits (UNS8 NbBits,       UNS8 * SrcByteIndex,  UNS8 SrcBitIndex,
			   UNS8 SrcBigEndian, UNS8 * DestByteIndex, UNS8 DestBitIndex,
			   UNS8 DestBigEndian)
{
  /* This loop copy as many bits that it can each time, crossing */
  /* successively bytes */
  // boundaries from LSB to MSB.
  while (NbBits > 0)
    {
      /* Bit missalignement between src and dest */
      INTEGER8 Vect = DestBitIndex - SrcBitIndex;

      /* We can now get src and align it to dest */
      UNS8 Aligned = Vect > 0 ? *SrcByteIndex << Vect : *SrcByteIndex >> -Vect;

      /* Compute the nb of bit we will be able to copy */
      UNS8 BoudaryLimit = (Vect > 0 ? 8 - DestBitIndex : 8 - SrcBitIndex);
      UNS8 BitsToCopy = BoudaryLimit > NbBits ? NbBits : BoudaryLimit;

      /* Create a mask that will serve in: */
      UNS8 Mask =
        ((0xff << (DestBitIndex + BitsToCopy)) |
         (0xff >> (8 - DestBitIndex)));

      /* - Filtering src */
      UNS8 Filtered = Aligned & ~Mask;

      /* - and erase bits where we write, preserve where we don't */
      *DestByteIndex &= Mask;

      /* Then write. */
      *DestByteIndex |= Filtered;

      /*Compute next time cursors for src */
      if ((SrcBitIndex += BitsToCopy) > 7)      /* cross boundary ? */
        {
          SrcBitIndex = 0;      /* First bit */
          SrcByteIndex += (SrcBigEndian ? -1 : 1);      /* Next byte */
        }


      /*Compute next time cursors for dest */
      if ((DestBitIndex += BitsToCopy) > 7)
        {
          DestBitIndex = 0;     /* First bit */
          DestByteIndex += (DestBigEndian ? -1 : 1);    /* Next byte */
        }

      /*And decrement counter. */
      NbBits -= BitsToCopy;
    }
}

//int pdo_time=0;
/* ֻ�����������б�������
1. sendOnePDOevent
2. _sendPDOevent*/
static void sendPdo(CO_Data * d, UNS32 pdoNum, Message * pdo)
{
  /*store_as_last_message */
  d->PDO_status[pdoNum].last_message = *pdo;
  MSG_WAR (0x396D, "sendPDO cobId :", UNS16_LE(pdo->cob_id));
  MSG_WAR (0x396E, "     Nb octets :", pdo->len);

  canSend (d, pdo);
	d->pdo_time++;
	if(d->pdo_time == 10)
	{
		d->pdo_time = 0;
	}
	
}


/*!
**
**
** @param d
**
** @return
**/
/* �������л�û�е��ù��ú��� */
UNS8 sendPDOevent (CO_Data * d)
{
  /* Calls _sendPDOevent specifying it is not a sync event */
  return _sendPDOevent (d, 0);
}


int testbreakpoint=1;
/* ����һ��PDO�¼������ڴ�������Ϊ�첽�����Σ�301�ֲ���˵�����¼�������ʽ��Page131���������ͨ�Ų���������2 */
/* ���ַ�ʽ�������¼���ʱ�������ƶ�ʱ�����������¼���ʱ���ú��������룬�������ƶ�ʱ������ֹ���� */
/* ���Ƶ�ʱ��û���Ļ��ú����������루���ú����ĵ�һ��if�����ж������� */
/* ������ƶ�ʱ����ʱ�����õı��¼���ʱ���Ķ̵Ļ�����˵�������루����ͨ����һ��if�жϣ������ǵڶ��η��͵�PDO����֡�����һ����ͬ */
/* ������ͨ����������if��������������Ҳ����ֹ */
/* �¼���ʱ�������ƶ�ʱ���Ļص�����PDOEventTimerAlarm �� PDOInhibitTimerAlarm�����øú������룬������Ѿ����͹���PDO֡�Ļ� */
/* �ڶ��η��;ͻ���ֹ����Ϊ�ڶ���������͵Ļ������һ�η��͵�һ����û��Ҫ�ٷ��ͣ�Ҳ������ú����ķ������ݵ������� */
/* ���Ե��øú���һ��ֻ�ᷢ��һ��PDO���� */
UNS8 sendOnePDOevent (CO_Data * d, UNS8 pdoNum)
{

//  UNS16 offsetObjdict;
  UNS32 errorCode;
  Message pdo;
  /* ������PDO���ģ����߱���ֹ */
  if (!d->CurrentCommunicationState.csPDO || (d->PDO_status[pdoNum].transmit_type_parameter & PDO_INHIBITED))/* Flaxin �ڽ�ֹʱ��֮���ǽ�ֹͨ�ŵ� */
	{
	  return 0;
	}
  /* ��һ������PDOͨ�Ų��� */
//  offsetObjdict = (UNS16) (d->firstIndex->PDO_TRS + pdoNum);

  MSG_WAR (0x3968, "  PDO is on EVENT. Trans type : ", *((UNS8 *) d->objdict[offsetObjdict].pSubindex[2].pObject));
  
  memset(&pdo, 0, sizeof(pdo));
  /* �����ݿ�����PDO����ͨ�� */
  if (buildPDO (d, pdoNum, &pdo))
    {
      MSG_ERR (0x3907, " Couldn't build TPDO number : ", pdoNum);
      return 0;
    }
	
  /*Compare new and old PDO */
  /* �Ƚ�����PDO */
  if (d->PDO_status[pdoNum].last_message.cob_id == pdo.cob_id
      && d->PDO_status[pdoNum].last_message.len == pdo.len
      && memcmp(d->PDO_status[pdoNum].last_message.data, pdo.data, 8) == 0
     )
	/* int memcmp(const void * buf1,const void * buf2,unsigned int count);
	  �������ܣ��Ƚ��ڴ�����buf1��buf2��ǰcount���ֽ�
	  ��buf1<buf2ʱ������ֵС��0
	  ��buf1=buf2ʱ������ֵ����0
	  ��buf1>buf2ʱ������ֵ����0
	*/
    {
      /* No changes -> go to next pdo */
	  /* û�仯 */
      return 0;
    }
  else
    {
      TIMEVAL EventTimerDuration;
      TIMEVAL InhibitTimerDuration;

      MSG_WAR (0x306A, "Changes TPDO number : ", pdoNum);
      /* Changes detected -> transmit message */
//	  /* �¼���ʱ�� */
//      EventTimerDuration = *(UNS16 *) d->objdict[offsetObjdict].pSubindex[5].pObject;
//	  /* ���ƶ�ʱ�� */
//      InhibitTimerDuration = *(UNS16 *) d->objdict[offsetObjdict].pSubindex[3].pObject;
		EventTimerDuration = *(UNS16 *)(d->Find_Object(NULL, (0x18000005 | (pdoNum<<16) ), &errorCode, d) -> p_content);
		InhibitTimerDuration = *(UNS16 *)(d->Find_Object(NULL,(0x18000003 | (pdoNum<<16)), &errorCode, d) -> p_content);
      /* Start both event_timer and inhibit_timer */
	  /* ����PDO�¼���ʱ�� */
      if (EventTimerDuration)
        {
          DelAlarm (d->PDO_status[pdoNum].event_timer, &PDOEventTimerAlarm, d);
          CAN_ITConfig(d->canHandle,CAN_IT_FMP0, ENABLE);//�ú������ڿ������жϴ��
          d->PDO_status[pdoNum].event_timer = SetAlarm (d, pdoNum, &PDOEventTimerAlarm, MS_TO_TIMEVAL(EventTimerDuration), 0);
          CAN_ITConfig(d->canHandle,CAN_IT_FMP0, ENABLE);//�ú������ڿ������жϴ��
        }
      /* �������ƶ�ʱ�� */
      if (InhibitTimerDuration)
        {
          DelAlarm(d->PDO_status[pdoNum].inhibit_timer, &PDOInhibitTimerAlarm, d);
          CAN_ITConfig(d->canHandle,CAN_IT_FMP0, ENABLE);//�ú������ڿ������жϴ��
          d->PDO_status[pdoNum].inhibit_timer = SetAlarm(d, pdoNum, &PDOInhibitTimerAlarm, US_TO_TIMEVAL(InhibitTimerDuration*100), 0);
          CAN_ITConfig(d->canHandle,CAN_IT_FMP0, ENABLE);//�ú������ڿ������жϴ��
          /* and inhibit TPDO */
          d->PDO_status[pdoNum].transmit_type_parameter |= PDO_INHIBITED;
        }
      /* ����PDO���� */
	  
      if(1 == d->can_port_num )
	  {
	     testbreakpoint++;
	  }
		  
      sendPdo(d, pdoNum, &pdo);
    }
    return 1;
}

void PDOEventTimerAlarm (CO_Data * d, UNS32 pdoNum)
{
  /* This is needed to avoid deletion of re-attribuated timer */
  d->PDO_status[pdoNum].event_timer = TIMER_NONE;
  /* force emission of PDO by artificially changing last emitted */
  d->PDO_status[pdoNum].last_message.cob_id = 0;
  sendOnePDOevent(d, (UNS8) pdoNum);
}

void PDOInhibitTimerAlarm (CO_Data * d, UNS32 pdoNum)
{
  /* This is needed to avoid deletion of re-attribuated timer */
  d->PDO_status[pdoNum].inhibit_timer = TIMER_NONE;
  /* Remove inhibit flag */
  d->PDO_status[pdoNum].transmit_type_parameter &= ~PDO_INHIBITED;/* Flaxin ��ֹʱ�䵽�˲������ֹ��־λ */
  sendOnePDOevent(d, (UNS8) pdoNum);
}

void _RxPDO_EventTimers_Handler(CO_Data *d, UNS32 pdoNum)
{
}

/*!
**
**
** @param d
** @param isSyncEvent
**
** @return
**/
/* ����PDO�¼���������ͬ�������й� */
/* Flaxin �ú����ڽڵ㡰�յ���ͬ��֮֡�����*/
/* Flaxin �ú�����PDOInit�����б�����*/
/* Flaxin ���ܣ�����ͨ��������Ŀ��ͨ�Ų�����0x1800��������2�������ͣ��������������Ƿ���PDO���� */
/* isSyncEvent      1����Ҫͬ��֡��ϣ�ͬ����ʽ����0������Ҫͬ��֡��ϣ��첽��ʽ�� */
UNS8 _sendPDOevent (CO_Data * d, UNS8 isSyncEvent)
{
  UNS8 pdoNum = 0x00;           /* number of the actual processed pdo-nr. */
  UNS8 *pTransmissionType = NULL;
  UNS8 status = state3;
  UNS32 errorCode;
	
//  /* ָ��TPDO1ͨ�Ų������� */
//  UNS16 offsetObjdict = d->firstIndex->PDO_TRS;
//  /* ָ��TPDO1ӳ��������� */
//  UNS16 offsetObjdictMap = d->firstIndex->PDO_TRS_MAP;
//  /* ָ�����һ��TPDOͨ�Ų������� */
//  UNS16 lastIndex = d->lastIndex->PDO_TRS;
	
    /* ������״̬�²�֧��PDO���� */
    if (!d->CurrentCommunicationState.csPDO)
    {
      return 0;
    }
  /* study all PDO stored in the objects dictionary */
//  if (offsetObjdict)
    if (d->Find_Object(NULL, 0x18000000 | pdoNum<<16, &errorCode, d)!=&NULL_OBJ)
    {
      Message pdo;/* = Message_Initializer;*/
      memset(&pdo, 0, sizeof(pdo));
	  /* ��������TPDO */
//      while (offsetObjdict <= lastIndex)/* ��Ч��TPDOȫ����Ҫ���� */
	    while (d->Find_Object(NULL, 0x18000000 | pdoNum<<16, &errorCode, d)!=&NULL_OBJ)/* ��Ч��TPDOȫ����Ҫ���� */
		  
        {
          switch (status)
            {
            case state3:
			  /* COB-ID�����λΪ1���Ϸ� */
//              if ( /* bSubCount always 5 with objdictedit -> check disabled */
//                   /*d->objdict[offsetObjdict].bSubCount < 5 ||*/
//                   /* check if TPDO is not valid */
//                   *(UNS32 *) d->objdict[offsetObjdict].pSubindex[1].pObject & 0x80000000)/* Flaxin �μ�TPDOͨ�Ų���������λ31��validλ����valid����1ʱ��PDO�����ڻ���Ч */
              if ( (*(UNS32 *)((d->Find_Object(NULL, 0x18000001 | pdoNum<<16, &errorCode, d)) -> p_content)) & 0x80000000)
			    {
                  MSG_WAR (0x3960, "Not a valid PDO ", 0x1800 + pdoNum);
                  /*Go next TPDO */
                  status = state11;
                  break;/* ���break������if��仹������break�� *//* break��������switch����ѭ�� */
                }
              /* get the PDO transmission type */
			  /* ���ͷ�ʽ */
//              pTransmissionType = (UNS8 *) d->objdict[offsetObjdict].pSubindex[2].pObject;
			  pTransmissionType = (UNS8 *) (d->Find_Object(NULL, 0x18000002 | pdoNum<<16, &errorCode, d)) -> p_content;
              MSG_WAR (0x3962, "Reading PDO at index : ", 0x1800 + pdoNum);

              /* check if transmission type is SYNCRONOUS */
              /* message transmited every n SYNC with n=TransmissionType */
			  /* ��վ���յ�ͬ������֮�� */
			  /* ͬ�����ڴ������պõ�ͬ�������ͷ���PDO���ģ�����transmit_type_parameter++ */
              if (isSyncEvent &&
                 (*pTransmissionType >= TRANS_SYNC_MIN) &&
                 (*pTransmissionType <= TRANS_SYNC_MAX) && 
                 (++d->PDO_status[pdoNum].transmit_type_parameter == *pTransmissionType))
                {
                  /*Reset count of SYNC */
				  /* �Ȱ�ͬ����������Ϊ0 */
                  d->PDO_status[pdoNum].transmit_type_parameter = 0;
                  MSG_WAR (0x3964, "  PDO is on SYNCHRO. Trans type : ",*pTransmissionType);
                  memset(&pdo, 0, sizeof(pdo));

				  /* ��PDO���ݿ�����PDOͨ����������׼�����ͳ�ȥ */
                  if (buildPDO (d, pdoNum, &pdo))/* ����ʧ�� */
                    {
                      MSG_ERR (0x1906, " Couldn't build TPDO number : ",
                               pdoNum);
                      status = state11;
                      break;
                    }
                  status = state5;
                  /* If transmission RTR, with data sampled on SYNC */
                }
			  /* ��վ���յ�ͬ������֮�� */
			  /* TRANS_RTR_SYNC����˼�ǽ��յ�ͬ�����ľ�׼�������ݣ��ȴ�����ͷ������� */
				/* Flaxin ��һ�β��ᷢ��PDO���ݣ���һ��ֻ��׼�������ݣ������ٽ��յ�һ֡Զ������֡���ܷ���PDO���ݣ�����д��proceedPDO������*/
              else if (isSyncEvent && (*pTransmissionType == TRANS_RTR_SYNC))/* #define TRANS_RTR_SYNC        0xFC */
                {
				  /* ������׼���ã��ŵ�����ͨ�������� */
                  if (buildPDO(d, pdoNum, &d->PDO_status[pdoNum].last_message))
                    {
                      MSG_ERR (0x1966, " Couldn't build TPDO number : ", pdoNum);
                      d->PDO_status[pdoNum].transmit_type_parameter &= ~PDO_RTR_SYNC_READY;/* #define PDO_RTR_SYNC_READY 0x01 *//* transmit_type_parameter���һλ���� */
                    }
                  else
                    {
					  /* ������׼���ã���־λ��1 */
                      d->PDO_status[pdoNum].transmit_type_parameter |= PDO_RTR_SYNC_READY;/* transmit_type_parameter���һλ��1 */
                    }
                  status = state11;
                  break;
                  /* If transmission on Event and not inhibited, check for changes */
                }
			  /* ��վ���յ�ͬ������֮������Ƿ������Եľͷ���PDO���� */
				/* Flaxin ���������ģʽ������PDO���ݾͺ��¼���ʱ���ͽ�ֹ��ʱ���й��� */
              else if ( (isSyncEvent && (*pTransmissionType == TRANS_SYNC_ACYCLIC))||
                      (!isSyncEvent && /* �첽�¼� */
				      (*pTransmissionType == TRANS_EVENT_PROFILE || *pTransmissionType == TRANS_EVENT_SPECIFIC)&& /* PDOͨ�����䷽ʽҲ���첽��ʽ */
				      !(d->PDO_status[pdoNum].transmit_type_parameter & PDO_INHIBITED))) /* ���Ҵ�ʱδ����ֹ���� *//* #define PDO_INHIBITED 0x01 */
                {                                                                         /* transmit_type_parameter���һλΪ0 */ 
                  sendOnePDOevent(d, pdoNum);
                  status = state11;
                }
              else
                {
                  MSG_WAR (0x306C,"  PDO is not on EVENT or synchro or not at this SYNC. Trans type : ",*pTransmissionType);
                  status = state11;
                }
              break;
			/* ����PDO���� */
            case state5:       /*Send the pdo */
              sendPdo(d, pdoNum, &pdo);
			
// #ifdef	LOGIC_DEBUG_ON
//			    if(1 == d->can_port_num)
//				{	
////#endif					
//			        while(!((d->canHandle->TSR & 0x10000000)|(d->canHandle->TSR & 0x08000000)|(d->canHandle->TSR & 0x04000000)))
//			        {
//			        };/* ��������ֻ��3����������̫�� */
//					
//// #ifdef	LOGIC_DEBUG_ON					
//				}			
			
			
              status = state11;
              break;
            case state11:      /*Go to next TPDO */
              pdoNum++;
//              offsetObjdict++;
//              offsetObjdictMap++;
              MSG_WAR (0x3970, "next pdo index : ", pdoNum);
              status = state3;
              break;

            default:
              MSG_ERR (0x1972, "Unknown state has been reached :", status);
              return 0xFF;
            }                   /* end switch case */

        }                       /* end while */
    }
  return 0;
}

/*!
**
**
** @param d
** @param OD_entry
** @param bSubindex
** @return always 0
**/
/* ��setODentry�����ı���PDOͨ�Ų�����2��3��5������ʱ�ᱻ�����ص������������� */
UNS32 TPDO_Communication_Parameter_Callback (CO_Data * d,
                                       const OD_OI_TypeDef * OD_entry,
                                       UNS8 bSubindex)
{
	UNS32 ErrorCode;
  /* If PDO are actives */
  /* ������״̬������PDO���� */
  if (d->CurrentCommunicationState.csPDO)
    switch (bSubindex)
      {
      case 2:               /* ���ͷ�ʽ */   /* Changed transmition type */
      case 3:               /* ��ֹʱ�� */   /* Changed inhibit time */
      case 5:               /* �¼�ʱ�� */   /* Changed event time */
        {
		  /* ͨ�Ų������ֵ��е�ָ�� */
//          const OD_OI_TypeDef *TPDO_com = d->objdict + d->firstIndex->PDO_TRS;
			const OD_OI_TypeDef *TPDO_com = d->Find_Object(NULL, 0x18000000, &ErrorCode, d);

          UNS8 numPdo = (UNS8) (OD_entry - TPDO_com);   /* �ṹ�������еĲ�ͬ�ṹ����׵�ַ������õ���������֮��ṹ��ļ�� *//* ȷ���õ���PDO�� */ /* number of the actual processed pdo-nr. */

          /* Zap all timers and inhibit flag */
	      /* ɾ���¼���ʱ�� */
          d->PDO_status[numPdo].event_timer = DelAlarm(d->PDO_status[numPdo].event_timer, &PDOEventTimerAlarm, d);
          CAN_ITConfig(d->canHandle, CAN_IT_FMP0, ENABLE);//�ú������ڿ������жϴ��
		  /* ɾ����ֹ��ʱ�� */
          d->PDO_status[numPdo].inhibit_timer = DelAlarm(d->PDO_status[numPdo].inhibit_timer, &PDOInhibitTimerAlarm, d);
          CAN_ITConfig(d->canHandle, CAN_IT_FMP0, ENABLE);//�ú������ڿ������жϴ��
		  /* �������� */
          d->PDO_status[numPdo].transmit_type_parameter = 0;
          /* Call  PDOEventTimerAlarm for this TPDO, this will trigger emission et reset timers */
		  /* ������Щ��ʱ�� */
          PDOEventTimerAlarm(d, numPdo);
          return 0;
        }

      default:                 /* other subindex are ignored */
        break;
      }
  return 0;
}

/* PDO��ʼ�� */
void PDOInit (CO_Data * d)
{
  /* For each TPDO mapping parameters */
  /* ����PDOͨ�Ų������� */
  UNS16 pdoIndex = 0x1800;     /* OD index of TPDO */
  UNS32 ErrorCode ;
	OD_OI_TypeDef * Ptr;
//  /* ָ���һ��TPDOͨ�Ų������� */
//  UNS16 offsetObjdict = d->firstIndex->PDO_TRS;
//  /* ָ�����һ��TPDOͨ�Ų������� */	
//  UNS16 lastIndex = d->lastIndex->PDO_TRS;
	
//  if (offsetObjdict)
	if (d->Find_Object(NULL, 0x18000000, &ErrorCode, d)!=&NULL_OBJ)	  
	/* ��������TPDO */
//    while (offsetObjdict <= lastIndex)
	  while (d->Find_Object(NULL, 0x18000000 | pdoIndex<<16, &ErrorCode, d)!=&NULL_OBJ)
      {
        /* Assign callbacks to sensible TPDO mapping subindexes */
        UNS32 errorCode;
//        ODCallback_t *CallbackList;
        /* Find callback list */
		/* ɨ���ֵ��ҵ����� */
//        scanIndexOD (d, pdoIndex, &errorCode, &CallbackList);
		  Ptr = d->Find_Object(NULL, 0x18000000 | pdoIndex<<16, &ErrorCode, d);
		/* �ҵ��������������лص����� */
//		if (errorCode == OD_SUCCESSFUL && CallbackList)
        if (ErrorCode == OD_SUCCESSFUL && Ptr -> callback)
          {
            /*Assign callbacks to corresponding subindex */
            /* Transmission type */
//			/* ���ͷ�ʽ */
//            CallbackList[2] = &TPDO_Communication_Parameter_Callback;
//            /* Inhibit time */
//			/* ��ֹ��ʱ�� */
//            CallbackList[3] = &TPDO_Communication_Parameter_Callback;
//            /* Event timer */
//			/* �¼���ʱ�� */
//            CallbackList[5] = &TPDO_Communication_Parameter_Callback;
			  d->Find_Object(NULL, 0x18000002 | pdoIndex<<16, &ErrorCode, d) -> callback = (ODCallback_t)TPDO_Communication_Parameter_Callback;
			  d->Find_Object(NULL, 0x18000003 | pdoIndex<<16, &ErrorCode, d) -> callback = (ODCallback_t)TPDO_Communication_Parameter_Callback;
			  d->Find_Object(NULL, 0x18000005 | pdoIndex<<16, &ErrorCode, d) -> callback = (ODCallback_t)TPDO_Communication_Parameter_Callback;
          }
        pdoIndex++;
//        offsetObjdict++;
      }
  /* Trigger a non-sync event */
  /* ��ͬ�������¼� */
//  _sendPDOevent (d, 0);
}


/* ֹͣPDO */
void PDOStop (CO_Data * d)
{
  /* For each TPDO mapping parameters */
  UNS8 pdoNum = 0x00;           /* number of the actual processed pdo-nr. */
  UNS32 errorCode;
//  UNS16 offsetObjdict = d->firstIndex->PDO_TRS;
//  UNS16 lastIndex = d->lastIndex->PDO_TRS;
//  if (offsetObjdict)
	if (d->Find_Object(NULL, 0x18000000, &errorCode, d)!=&NULL_OBJ)
//    while (offsetObjdict <= lastIndex)
	  while (d->Find_Object(NULL, 0x18000000 | pdoNum<<16, &errorCode, d)!=&NULL_OBJ)
      {
        /* Delete TPDO timers */
		/* ɾ��PDO��ʱ�� */
        d->PDO_status[pdoNum].event_timer = DelAlarm(d->PDO_status[pdoNum].event_timer, &PDOEventTimerAlarm, d);
        CAN_ITConfig(d->canHandle, CAN_IT_FMP0, ENABLE);//�ú������ڿ������жϴ��
        d->PDO_status[pdoNum].inhibit_timer = DelAlarm(d->PDO_status[pdoNum].inhibit_timer, &PDOInhibitTimerAlarm, d);
        CAN_ITConfig(d->canHandle, CAN_IT_FMP0, ENABLE);//�ú������ڿ������жϴ��
        /* Reset transmit type parameter */
		/* ���ò��� */
        d->PDO_status[pdoNum].transmit_type_parameter = 0;
        d->PDO_status[pdoNum].last_message.cob_id = 0;
        pdoNum++;
//        offsetObjdict++;
      }
}
