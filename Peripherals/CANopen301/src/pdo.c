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
** @return  0：成功；非0：失败
**/
/* 把TPDO映射参数映射的值拷贝到传输通道的缓冲区中 */
UNS8 buildPDO (CO_Data * d, UNS8 numPdo, Message * pdo)
{
	 UNS32 ErrorCode;
  /* PDO通信参数 */
  /* PDO映射参数 */
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
  /* 子索引数 */
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
	  /* 映射表中的值 */
//      UNS32 *pMappingParameter = (UNS32 *) TPDO_map->pSubindex[prp_j + 1].pObject;
	    UNS32 *pMappingParameter = (UNS32 *) ((d->Find_Object(NULL, 0x1A000000|(numPdo<<16)|(prp_j + 1), &ErrorCode, d))->p_content);
	  /* 索引 */
      UNS16 index = (UNS16) ((*pMappingParameter) >> 16);
	  /* 位数 */
      UNS32 Size = (UNS32) (*pMappingParameter & (UNS32) 0x000000FF);     /* Size in bits */

      /* get variable only if Size != 0 and Size is lower than remaining bits in the PDO */
	  /* 最多传8个字节，多了就会丢弃，所以一个PDO映射的的变量总位数不要超过64，即8字节 */
      if (Size && ((offset + Size) <= 64))
        {
		  /* 位数转换为字节，1-8bit为一个字节 */
          UNS32 ByteSize = 1 + ((Size - 1) >> 3);        /*1->8 => 1 ; 9->16 => 2, ... */
		  /* 子索引 */
          UNS8 subIndex = (UNS8) (((*pMappingParameter) >> (UNS8) 8) & (UNS32) 0x000000FF);

          MSG_WAR (0x300F, "  got mapping parameter : ", *pMappingParameter);
          MSG_WAR (0x3050, "    at index : ", TPDO_map->index);
          MSG_WAR (0x3051, "    sub-index : ", prp_j + 1);
          /* 从字典里面取出数据和数据类型放到临时缓存 */
//          if (getODentry (d, index, subIndex, tmp, &ByteSize, &dataType, 0) != OD_SUCCESSFUL)
			if (getODentry (d, index, subIndex, tmp, &ByteSize, &dataType, 0) != OD_SUCCESSFUL)
            {
              MSG_ERR (0x1013, " Couldn't find mapped variable at index-subindex-size : ", (UNS32) (*pMappingParameter));
              return 0xFF;
            }
          /* copy bit per bit in little endian */
		  /* 从临时缓存里把数据拷贝到PDO缓冲区 */
          CopyBits ((UNS8) Size, ((UNS8 *) tmp), 0, 0, (UNS8 *) & pdo->data[offset >> 3], (UNS8)(offset % 8), 0);
          /* 更新偏移量 */
          offset += Size;
        }
      prp_j++;
    }
  while (prp_j < *pMappingCount);
  /* 一共要传输的字节数 */
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
** @return  0x00: 请求成功 ; 0xFF: 请求失败
**/
/* 主站发送PDO请求 */
/* 本工程中并未使用 */
UNS8 sendPDOrequest (CO_Data * d, UNS16 RPDOIndex)
{
  UNS16 *pwCobId;
  UNS32 ErrorCode;
//  /* 指向第一个PDO通信参数索引0x1400 */
//  UNS16 offset = d->firstIndex->PDO_RCV;
//  /* 指向最后一个PDO通信参数索引 */
//  UNS16 lastIndex = d->lastIndex->PDO_RCV;
    pwCobId = (UNS16*)(d->Find_Object(NULL, 0x00000001|RPDOIndex<<16, &ErrorCode, d)->p_content);	
  /* 当前运行状态允许PDO传输 */
  if (!d->CurrentCommunicationState.csPDO)
    {
      return 0;
    }
  /* Sending the request only if the cobid have been found on the PDO
     receive */
  /* part dictionary */
  MSG_WAR (0x3930, "sendPDOrequest RPDO Index : ", RPDOIndex);
  /* RPDO：0x1400-0x15FF */
//  if (offset && RPDOIndex >= 0x1400)/* '>=' 优先级高于 '&&' */
	if (ErrorCode == OD_SUCCESSFUL)/* '>=' 优先级高于 '&&' */
    {
//	  /* 算出RPDO索引 */
//      offset += RPDOIndex - 0x1400;
//      if (offset <= lastIndex)
        {
          /* get the CobId */
//          pwCobId = d->objdict[offset].pSubindex[1].pObject;

          MSG_WAR (0x3930, "sendPDOrequest cobId is : ", *pwCobId);
          {
			/* 发送PDO请求 */
            Message pdo;
            pdo.cob_id = UNS16_LE(*pwCobId);
            pdo.rtr = REQUEST;
            pdo.len = 0;
			return canSend (d, &pdo);/* 0x00: 成功 ; 0xFF: 失败*/
          }
        }
    }
  MSG_ERR (0x1931, "sendPDOrequest : RPDO Index not found : ", RPDOIndex);
  return 0xFF;/* 运行到这说明请求失败 */
}


/*!
**
**
** @param d
** @param m
**
** @return
**/
/* 处理PDO报文 */
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
  /* 主站接收到PDO数据 */
  /* Flaxin 接收到PDO数据就赶紧存起来 */
  if ((*m).rtr == NOT_A_REQUEST)
    {
	  /* 第一个接收PDO通信参数索引 */
//      offsetObjdict = d->firstIndex->PDO_RCV;
//	  /* 最后一个接收PDO通信参数索引 */
//      lastIndex = d->lastIndex->PDO_RCV;
//      /* offsetObjdict不为0 */
//      if (offsetObjdict)
		/* 遍历所有接收PDO */
//        while (offsetObjdict <= lastIndex)
	      while (d->Find_Object(NULL, 0x14000000 | numPdo<<16, &errorCode, d) != &NULL_OBJ)
          {
            switch (status)
              {
              case state2:
				/* cob_id */
			  
//                pwCobId = d->objdict[offsetObjdict].pSubindex[1].pObject;
			      pwCobId = d->Find_Object(NULL, 0x14000001 | numPdo<<16, &errorCode, d) -> p_content;
			    /* 主站找出该报文由哪个接收PDO接收 */
                if (*pwCobId == UNS16_LE(m->cob_id))
                  {
                    /* The cobId is recognized */
					/* 一旦找到就更新状态并进行处理 */
                    status = state4;
                    MSG_WAR (0x3936, "cobId found at index ", 0x1400 + numPdo);
                    break;
                  }
				/* 不对应就继续判断下一个 */
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
//			    /* 指向第一个接收PDO映射参数索引 */
//                offsetObjdict = d->firstIndex->PDO_RCV_MAP;
//			    /* 指向最后一个接收PDO映射参数索引 */			  
//                lastIndex = d->lastIndex->PDO_RCV_MAP;
			    /* 子索引数 */
//                pMappingCount = (UNS8 *) (d->objdict + offsetObjdict + numPdo)->pSubindex[0].pObject;
			    pMappingCount = (UNS8 *)d->Find_Object(NULL, 0x16000000 | numPdo<<16, &errorCode, d) -> p_content;
                numMap = 0;
			    /* 遍历所有子索引 */
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
                    /* 位数 */
                    Size = (UNS8) (*pMappingParameter & (UNS32) 0x000000FF);

                    /* set variable only if Size != 0 and 
                     * Size is lower than remaining bits in the PDO */
					/* PDO不超过8个字节 */
                    if (Size && ((offset + Size) <= (m->len << 3)))
                      {
                        /* copy bit per bit in little endian */
						/* 将数据从缓冲区中拷贝到临时缓冲区 */
                        CopyBits (Size, (UNS8 *) & m->data[offset >> 3], offset % 8, 0, ((UNS8 *) tmp), 0, 0);
                        /*1->8 => 1 ; 9->16 =>2, ... */
						/* 位数转换为字节：1~8bit为一个字节 */
                        ByteSize = (UNS32)(1 + ((Size - 1) >> 3));
                        /* 将数据从临时缓冲区中写到字典里 */
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
				/* 接收PDO事件定时器重新计时，如果规定时间内没接收到报文，就回调_RxPDO_EventTimers_Handler函数 */
                if (d->RxPDO_EventTimers)/* 本程序中给RxPDO_EventTimers赋的值是0，所以这里这个条件总是不满足 */
                {
//                    TIMEVAL EventTimerDuration = *(UNS16 *)d->objdict[offsetObjdict].pSubindex[5].pObject;
					  TIMEVAL EventTimerDuration = *(UNS16 *)(d->Find_Object(NULL, (0x14000005 | (numPdo<<16)), &errorCode, d) -> p_content);
					
                    if(EventTimerDuration)
					{
						/* 删除定时器 */
                        DelAlarm (d->RxPDO_EventTimers[numPdo], d->RxPDO_EventTimers_Handler, d);
                        CAN_ITConfig(d->canHandle, CAN_IT_FMP0, ENABLE);//该函数现在可允许被中断打断
						/* 设定定时器 */
                        d->RxPDO_EventTimers[numPdo] = SetAlarm (d, numPdo, d->RxPDO_EventTimers_Handler/* 空函数 */, MS_TO_TIMEVAL (EventTimerDuration), 0);
                        CAN_ITConfig(d->canHandle, CAN_IT_FMP0, ENABLE);//该函数现在可允许被中断打断
                    }
                }
                return 0;
              }                 /* end switch status */
          }                     /* end while */
    }                           /* end if Donnees */
  /* 从站接收到PDO远程请求 */
  else if ((*m).rtr == REQUEST)
    {
      MSG_WAR (0x3946, "Receive a PDO request cobId : ", UNS16_LE(m->cob_id));
      status = state1;
//	  /* 指向第一个发送PDO索引 */
//      offsetObjdict = d->firstIndex->PDO_TRS;
//	  /* 指向最后一个发送PDO索引 */		
//      lastIndex = d->lastIndex->PDO_TRS;
//      if (offsetObjdict)
		/* 遍历所有发送PDO */
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
			    /* 从站找出该报文由哪个PDO发送 */
                if (*pwCobId == UNS16_LE(m->cob_id))
                  {
                    status = state4;
                    break;
                  }
				/* 不匹配就继续判断下一个 */
                else
                  {
                    numPdo++;
//                    offsetObjdict++;
                  }
                status = state1;
                break;

              case state4:     /* check transmission type */
				/* 发送类型 */
//                pTransmissionType = (UNS8 *) d->objdict[offsetObjdict].pSubindex[2].pObject;
			    pTransmissionType = (UNS8 *)(d->Find_Object(NULL, (0x18000002 | (numPdo<<16) ), &errorCode, d) -> p_content);
                /* If PDO is to be sampled and send on RTR, do it */
			    /* PDO远程请求，发送应答PDO */
                if (*pTransmissionType == TRANS_RTR)/* #define TRANS_RTR  253   Transmission on request *//* 仅RTR(事件驱动) */
                  {
                    status = state5;
                    break;
                  }
                /* RTR_SYNC means data prepared at SYNC, transmitted on RTR */
				/* 为同步远程PDO */
                else if (*pTransmissionType == TRANS_RTR_SYNC)/* #define TRANS_RTR_SYNC  252 *//* (仅RTR(同步)) */
                  {
					/* 如果同步远程PDO已经准备好数据，发送应答PDO */
                    if (d->PDO_status[numPdo].transmit_type_parameter & PDO_RTR_SYNC_READY)/* Flaxin 如果上一次准备好了数据，就将上一次准备好的数据发送出去 */
                      {
                        /*Data ready, just send */
                        canSend (d, &d->PDO_status[numPdo].last_message);
                        return 0;
                      }
					/* 没准备好 *//* Flaxin 如果上一次没准备好数据，就将这一次查询到的TPDO发送出去（跳到state5上去执行发送任务） */
                    else
                      {
                        /* if SYNC did never occur, transmit current data */
                        /* DS301 do not tell what to do in such a case... */
						/* 报错 */
                        MSG_ERR (0x1947,"Not ready RTR_SYNC TPDO send current data : ",UNS16_LE(m->cob_id));
                        status = state5;
                      }
                    break;
                  }
				/* 事件触发 */
                else if ((*pTransmissionType == TRANS_EVENT_PROFILE) ||
                         (*pTransmissionType == TRANS_EVENT_SPECIFIC))
                  {
                    /* Zap all timers and inhibit flag */
					/* 删除事件定时器 */
                    d->PDO_status[numPdo].event_timer = DelAlarm (d->PDO_status[numPdo].event_timer, &PDOEventTimerAlarm, d);
                    CAN_ITConfig(d->canHandle,CAN_IT_FMP0, ENABLE);//该函数现在可允许被中断打断
					/* 删除禁止定时器 */					  
                    d->PDO_status[numPdo].inhibit_timer = DelAlarm(d->PDO_status[numPdo].inhibit_timer, &PDOInhibitTimerAlarm, d);
                    CAN_ITConfig(d->canHandle,CAN_IT_FMP0, ENABLE);//该函数现在可允许被中断打断
                    d->PDO_status[numPdo].transmit_type_parameter &= ~PDO_INHIBITED;
                    /* Call  PDOEventTimerAlarm for this TPDO, 
                     * this will trigger emission et reset timers */
					/* 发送PDO报文，并重新启动定时事件 */
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
              /* 发送PDO数据 */
              case state5:     /* build and send requested PDO */
                {
                  Message pdo;
                  if (buildPDO (d, numPdo, &pdo))
                    {
                      MSG_ERR (0x1948, " Couldn't build TPDO number : ", numPdo);
                      return 0xFF;
                    }
                  canSend (d, &pdo);
			      // 避免邮箱占满，阻塞发送，实现方式不好，当PDO数量大的时候程序会等到所有PDO都从硬件发送完成后才能结束while执行后面的指令。后续修改为队列缓冲。
			      // 当CAN总线故障时会造成程序堵塞在中断服务程序中，造成主程序和比该中断服务程序优先级低的程序卡死。							
                  while(!((d->canHandle->TSR & 0x10000000)|(d->canHandle->TSR & 0x08000000)|(d->canHandle->TSR & 0x04000000)))
                  {
				  };/* 发送邮箱只有3个，不能塞太快 */					
					
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
**//* SrcBitIndex和DestBitIndex是数据拷贝在第一个字节中的起始位置（0~7位），它们取值只能为0~7，否则直接错误 */
/* 将SrcByteIndex的从SrcBitIndex开始的NbBits个位的数据拷贝到DestByteIndex从DestBitIndex开始的位置上 */
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
/* 只在两个函数中被调用了
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
/* 本工程中还没有调用过该函数 */
UNS8 sendPDOevent (CO_Data * d)
{
  /* Calls _sendPDOevent specifying it is not a sync event */
  return _sendPDOevent (d, 0);
}


int testbreakpoint=1;
/* 发送一次PDO事件，用于传输类型为异步的情形，301手册上说的是事件驱动方式（Page131），具体见通信参数子索引2 */
/* 此种方式会启用事件定时器和抑制定时器，开启了事件定时器该函数会重入，但是抑制定时器会阻止重入 */
/* 抑制的时间没到的话该函数不会重入（见该函数的第一个if语句的判断条件） */
/* 如果抑制定时器的时间设置的比事件定时器的短的话，虽说可以重入（可以通过第一个if判断），但是第二次发送的PDO数据帧会和上一次相同 */
/* 这样会通不过第三个if语句的条件，函数也会中止 */
/* 事件定时器和抑制定时器的回调函数PDOEventTimerAlarm 和 PDOInhibitTimerAlarm都会让该函数重入，但如果已经发送过该PDO帧的话 */
/* 第二次发送就会中止（因为第二次如果发送的话会和上一次发送的一样，没必要再发送，也不满足该函数的发送数据的条件） */
/* 所以调用该函数一般只会发送一次PDO数据 */
UNS8 sendOnePDOevent (CO_Data * d, UNS8 pdoNum)
{

//  UNS16 offsetObjdict;
  UNS32 errorCode;
  Message pdo;
  /* 不允许PDO报文，或者被禁止 */
  if (!d->CurrentCommunicationState.csPDO || (d->PDO_status[pdoNum].transmit_type_parameter & PDO_INHIBITED))/* Flaxin 在禁止时间之内是禁止通信的 */
	{
	  return 0;
	}
  /* 第一个发送PDO通信参数 */
//  offsetObjdict = (UNS16) (d->firstIndex->PDO_TRS + pdoNum);

  MSG_WAR (0x3968, "  PDO is on EVENT. Trans type : ", *((UNS8 *) d->objdict[offsetObjdict].pSubindex[2].pObject));
  
  memset(&pdo, 0, sizeof(pdo));
  /* 将数据拷贝到PDO传输通道 */
  if (buildPDO (d, pdoNum, &pdo))
    {
      MSG_ERR (0x3907, " Couldn't build TPDO number : ", pdoNum);
      return 0;
    }
	
  /*Compare new and old PDO */
  /* 比较新老PDO */
  if (d->PDO_status[pdoNum].last_message.cob_id == pdo.cob_id
      && d->PDO_status[pdoNum].last_message.len == pdo.len
      && memcmp(d->PDO_status[pdoNum].last_message.data, pdo.data, 8) == 0
     )
	/* int memcmp(const void * buf1,const void * buf2,unsigned int count);
	  函数功能：比较内存区域buf1和buf2的前count个字节
	  当buf1<buf2时，返回值小于0
	  当buf1=buf2时，返回值等于0
	  当buf1>buf2时，返回值大于0
	*/
    {
      /* No changes -> go to next pdo */
	  /* 没变化 */
      return 0;
    }
  else
    {
      TIMEVAL EventTimerDuration;
      TIMEVAL InhibitTimerDuration;

      MSG_WAR (0x306A, "Changes TPDO number : ", pdoNum);
      /* Changes detected -> transmit message */
//	  /* 事件定时器 */
//      EventTimerDuration = *(UNS16 *) d->objdict[offsetObjdict].pSubindex[5].pObject;
//	  /* 抑制定时器 */
//      InhibitTimerDuration = *(UNS16 *) d->objdict[offsetObjdict].pSubindex[3].pObject;
		EventTimerDuration = *(UNS16 *)(d->Find_Object(NULL, (0x18000005 | (pdoNum<<16) ), &errorCode, d) -> p_content);
		InhibitTimerDuration = *(UNS16 *)(d->Find_Object(NULL,(0x18000003 | (pdoNum<<16)), &errorCode, d) -> p_content);
      /* Start both event_timer and inhibit_timer */
	  /* 启动PDO事件定时器 */
      if (EventTimerDuration)
        {
          DelAlarm (d->PDO_status[pdoNum].event_timer, &PDOEventTimerAlarm, d);
          CAN_ITConfig(d->canHandle,CAN_IT_FMP0, ENABLE);//该函数现在可允许被中断打断
          d->PDO_status[pdoNum].event_timer = SetAlarm (d, pdoNum, &PDOEventTimerAlarm, MS_TO_TIMEVAL(EventTimerDuration), 0);
          CAN_ITConfig(d->canHandle,CAN_IT_FMP0, ENABLE);//该函数现在可允许被中断打断
        }
      /* 启动抑制定时器 */
      if (InhibitTimerDuration)
        {
          DelAlarm(d->PDO_status[pdoNum].inhibit_timer, &PDOInhibitTimerAlarm, d);
          CAN_ITConfig(d->canHandle,CAN_IT_FMP0, ENABLE);//该函数现在可允许被中断打断
          d->PDO_status[pdoNum].inhibit_timer = SetAlarm(d, pdoNum, &PDOInhibitTimerAlarm, US_TO_TIMEVAL(InhibitTimerDuration*100), 0);
          CAN_ITConfig(d->canHandle,CAN_IT_FMP0, ENABLE);//该函数现在可允许被中断打断
          /* and inhibit TPDO */
          d->PDO_status[pdoNum].transmit_type_parameter |= PDO_INHIBITED;
        }
      /* 发送PDO数据 */
	  
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
  d->PDO_status[pdoNum].transmit_type_parameter &= ~PDO_INHIBITED;/* Flaxin 禁止时间到了才清除禁止标志位 */
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
/* 发送PDO事件函数，和同步对象有关 */
/* Flaxin 该函数在节点“收到”同步帧之后调用*/
/* Flaxin 该函数在PDOInit函数中被调用*/
/* Flaxin 功能：根据通信类型条目（通信参数如0x1800的子索引2传输类型）的设置来决定是否发送PDO数据 */
/* isSyncEvent      1：需要同步帧配合（同步方式）；0：不需要同步帧配合（异步方式） */
UNS8 _sendPDOevent (CO_Data * d, UNS8 isSyncEvent)
{
  UNS8 pdoNum = 0x00;           /* number of the actual processed pdo-nr. */
  UNS8 *pTransmissionType = NULL;
  UNS8 status = state3;
  UNS32 errorCode;
	
//  /* 指向TPDO1通信参数索引 */
//  UNS16 offsetObjdict = d->firstIndex->PDO_TRS;
//  /* 指向TPDO1映射参数索引 */
//  UNS16 offsetObjdictMap = d->firstIndex->PDO_TRS_MAP;
//  /* 指向最后一个TPDO通信参数索引 */
//  UNS16 lastIndex = d->lastIndex->PDO_TRS;
	
    /* 该运行状态下不支持PDO报文 */
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
	  /* 遍历所有TPDO */
//      while (offsetObjdict <= lastIndex)/* 有效的TPDO全部都要处理 */
	    while (d->Find_Object(NULL, 0x18000000 | pdoNum<<16, &errorCode, d)!=&NULL_OBJ)/* 有效的TPDO全部都要处理 */
		  
        {
          switch (status)
            {
            case state3:
			  /* COB-ID，最高位为1不合法 */
//              if ( /* bSubCount always 5 with objdictedit -> check disabled */
//                   /*d->objdict[offsetObjdict].bSubCount < 5 ||*/
//                   /* check if TPDO is not valid */
//                   *(UNS32 *) d->objdict[offsetObjdict].pSubindex[1].pObject & 0x80000000)/* Flaxin 参见TPDO通信参数索引，位31是valid位，当valid等于1时，PDO不存在或无效 */
              if ( (*(UNS32 *)((d->Find_Object(NULL, 0x18000001 | pdoNum<<16, &errorCode, d)) -> p_content)) & 0x80000000)
			    {
                  MSG_WAR (0x3960, "Not a valid PDO ", 0x1800 + pdoNum);
                  /*Go next TPDO */
                  status = state11;
                  break;/* 这个break是跳出if语句还是跳出break？ *//* break用来跳出switch语句和循环 */
                }
              /* get the PDO transmission type */
			  /* 发送方式 */
//              pTransmissionType = (UNS8 *) d->objdict[offsetObjdict].pSubindex[2].pObject;
			  pTransmissionType = (UNS8 *) (d->Find_Object(NULL, 0x18000002 | pdoNum<<16, &errorCode, d)) -> p_content;
              MSG_WAR (0x3962, "Reading PDO at index : ", 0x1800 + pdoNum);

              /* check if transmission type is SYNCRONOUS */
              /* message transmited every n SYNC with n=TransmissionType */
			  /* 从站接收到同步报文之后 */
			  /* 同步周期触发，刚好到同步次数就发送PDO报文，否则transmit_type_parameter++ */
              if (isSyncEvent &&
                 (*pTransmissionType >= TRANS_SYNC_MIN) &&
                 (*pTransmissionType <= TRANS_SYNC_MAX) && 
                 (++d->PDO_status[pdoNum].transmit_type_parameter == *pTransmissionType))
                {
                  /*Reset count of SYNC */
				  /* 先把同步次数重置为0 */
                  d->PDO_status[pdoNum].transmit_type_parameter = 0;
                  MSG_WAR (0x3964, "  PDO is on SYNCHRO. Trans type : ",*pTransmissionType);
                  memset(&pdo, 0, sizeof(pdo));

				  /* 把PDO数据拷贝到PDO通道缓存区，准备发送出去 */
                  if (buildPDO (d, pdoNum, &pdo))/* 缓存失败 */
                    {
                      MSG_ERR (0x1906, " Couldn't build TPDO number : ",
                               pdoNum);
                      status = state11;
                      break;
                    }
                  status = state5;
                  /* If transmission RTR, with data sampled on SYNC */
                }
			  /* 从站接收到同步报文之后 */
			  /* TRANS_RTR_SYNC的意思是接收到同步报文就准备好数据，等待请求就发送数据 */
				/* Flaxin 这一次不会发送PDO数据，这一次只是准备好数据，后面再接收到一帧远程请求帧才能发用PDO数据，具体写在proceedPDO函数中*/
              else if (isSyncEvent && (*pTransmissionType == TRANS_RTR_SYNC))/* #define TRANS_RTR_SYNC        0xFC */
                {
				  /* 把数据准备好，放到传输通道缓冲区 */
                  if (buildPDO(d, pdoNum, &d->PDO_status[pdoNum].last_message))
                    {
                      MSG_ERR (0x1966, " Couldn't build TPDO number : ", pdoNum);
                      d->PDO_status[pdoNum].transmit_type_parameter &= ~PDO_RTR_SYNC_READY;/* #define PDO_RTR_SYNC_READY 0x01 *//* transmit_type_parameter最后一位清零 */
                    }
                  else
                    {
					  /* 将数据准备好，标志位置1 */
                      d->PDO_status[pdoNum].transmit_type_parameter |= PDO_RTR_SYNC_READY;/* transmit_type_parameter最后一位置1 */
                    }
                  status = state11;
                  break;
                  /* If transmission on Event and not inhibited, check for changes */
                }
			  /* 从站接收到同步报文之后，如果是非周期性的就发送PDO数据 */
				/* Flaxin 如果是这种模式，发送PDO数据就和事件定时器和禁止定时器有关了 */
              else if ( (isSyncEvent && (*pTransmissionType == TRANS_SYNC_ACYCLIC))||
                      (!isSyncEvent && /* 异步事件 */
				      (*pTransmissionType == TRANS_EVENT_PROFILE || *pTransmissionType == TRANS_EVENT_SPECIFIC)&& /* PDO通道传输方式也是异步方式 */
				      !(d->PDO_status[pdoNum].transmit_type_parameter & PDO_INHIBITED))) /* 并且此时未被禁止传输 *//* #define PDO_INHIBITED 0x01 */
                {                                                                         /* transmit_type_parameter最后一位为0 */ 
                  sendOnePDOevent(d, pdoNum);
                  status = state11;
                }
              else
                {
                  MSG_WAR (0x306C,"  PDO is not on EVENT or synchro or not at this SYNC. Trans type : ",*pTransmissionType);
                  status = state11;
                }
              break;
			/* 发送PDO报文 */
            case state5:       /*Send the pdo */
              sendPdo(d, pdoNum, &pdo);
			
// #ifdef	LOGIC_DEBUG_ON
//			    if(1 == d->can_port_num)
//				{	
////#endif					
//			        while(!((d->canHandle->TSR & 0x10000000)|(d->canHandle->TSR & 0x08000000)|(d->canHandle->TSR & 0x04000000)))
//			        {
//			        };/* 发送邮箱只有3个，不能塞太快 */
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
/* 在setODentry函数改变了PDO通信参数的2、3、5子索引时会被当做回调函数触发调用 */
UNS32 TPDO_Communication_Parameter_Callback (CO_Data * d,
                                       const OD_OI_TypeDef * OD_entry,
                                       UNS8 bSubindex)
{
	UNS32 ErrorCode;
  /* If PDO are actives */
  /* 该运行状态下允许PDO报文 */
  if (d->CurrentCommunicationState.csPDO)
    switch (bSubindex)
      {
      case 2:               /* 发送方式 */   /* Changed transmition type */
      case 3:               /* 禁止时间 */   /* Changed inhibit time */
      case 5:               /* 事件时间 */   /* Changed event time */
        {
		  /* 通信参数在字典中的指针 */
//          const OD_OI_TypeDef *TPDO_com = d->objdict + d->firstIndex->PDO_TRS;
			const OD_OI_TypeDef *TPDO_com = d->Find_Object(NULL, 0x18000000, &ErrorCode, d);

          UNS8 numPdo = (UNS8) (OD_entry - TPDO_com);   /* 结构体数组中的不同结构体的首地址相减，得到的是他们之间结构体的间隔 *//* 确定用的是PDO几 */ /* number of the actual processed pdo-nr. */

          /* Zap all timers and inhibit flag */
	      /* 删除事件定时器 */
          d->PDO_status[numPdo].event_timer = DelAlarm(d->PDO_status[numPdo].event_timer, &PDOEventTimerAlarm, d);
          CAN_ITConfig(d->canHandle, CAN_IT_FMP0, ENABLE);//该函数现在可允许被中断打断
		  /* 删除禁止定时器 */
          d->PDO_status[numPdo].inhibit_timer = DelAlarm(d->PDO_status[numPdo].inhibit_timer, &PDOInhibitTimerAlarm, d);
          CAN_ITConfig(d->canHandle, CAN_IT_FMP0, ENABLE);//该函数现在可允许被中断打断
		  /* 发送类型 */
          d->PDO_status[numPdo].transmit_type_parameter = 0;
          /* Call  PDOEventTimerAlarm for this TPDO, this will trigger emission et reset timers */
		  /* 重置这些定时器 */
          PDOEventTimerAlarm(d, numPdo);
          return 0;
        }

      default:                 /* other subindex are ignored */
        break;
      }
  return 0;
}

/* PDO初始化 */
void PDOInit (CO_Data * d)
{
  /* For each TPDO mapping parameters */
  /* 发送PDO通信参数索引 */
  UNS16 pdoIndex = 0x1800;     /* OD index of TPDO */
  UNS32 ErrorCode ;
	OD_OI_TypeDef * Ptr;
//  /* 指向第一个TPDO通信参数索引 */
//  UNS16 offsetObjdict = d->firstIndex->PDO_TRS;
//  /* 指向最后一个TPDO通信参数索引 */	
//  UNS16 lastIndex = d->lastIndex->PDO_TRS;
	
//  if (offsetObjdict)
	if (d->Find_Object(NULL, 0x18000000, &ErrorCode, d)!=&NULL_OBJ)	  
	/* 遍历所有TPDO */
//    while (offsetObjdict <= lastIndex)
	  while (d->Find_Object(NULL, 0x18000000 | pdoIndex<<16, &ErrorCode, d)!=&NULL_OBJ)
      {
        /* Assign callbacks to sensible TPDO mapping subindexes */
        UNS32 errorCode;
//        ODCallback_t *CallbackList;
        /* Find callback list */
		/* 扫描字典找到索引 */
//        scanIndexOD (d, pdoIndex, &errorCode, &CallbackList);
		  Ptr = d->Find_Object(NULL, 0x18000000 | pdoIndex<<16, &ErrorCode, d);
		/* 找到该索引，并且有回调函数 */
//		if (errorCode == OD_SUCCESSFUL && CallbackList)
        if (ErrorCode == OD_SUCCESSFUL && Ptr -> callback)
          {
            /*Assign callbacks to corresponding subindex */
            /* Transmission type */
//			/* 发送方式 */
//            CallbackList[2] = &TPDO_Communication_Parameter_Callback;
//            /* Inhibit time */
//			/* 禁止定时器 */
//            CallbackList[3] = &TPDO_Communication_Parameter_Callback;
//            /* Event timer */
//			/* 事件定时器 */
//            CallbackList[5] = &TPDO_Communication_Parameter_Callback;
			  d->Find_Object(NULL, 0x18000002 | pdoIndex<<16, &ErrorCode, d) -> callback = (ODCallback_t)TPDO_Communication_Parameter_Callback;
			  d->Find_Object(NULL, 0x18000003 | pdoIndex<<16, &ErrorCode, d) -> callback = (ODCallback_t)TPDO_Communication_Parameter_Callback;
			  d->Find_Object(NULL, 0x18000005 | pdoIndex<<16, &ErrorCode, d) -> callback = (ODCallback_t)TPDO_Communication_Parameter_Callback;
          }
        pdoIndex++;
//        offsetObjdict++;
      }
  /* Trigger a non-sync event */
  /* 非同步触发事件 */
//  _sendPDOevent (d, 0);
}


/* 停止PDO */
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
		/* 删除PDO定时器 */
        d->PDO_status[pdoNum].event_timer = DelAlarm(d->PDO_status[pdoNum].event_timer, &PDOEventTimerAlarm, d);
        CAN_ITConfig(d->canHandle, CAN_IT_FMP0, ENABLE);//该函数现在可允许被中断打断
        d->PDO_status[pdoNum].inhibit_timer = DelAlarm(d->PDO_status[pdoNum].inhibit_timer, &PDOInhibitTimerAlarm, d);
        CAN_ITConfig(d->canHandle, CAN_IT_FMP0, ENABLE);//该函数现在可允许被中断打断
        /* Reset transmit type parameter */
		/* 重置参数 */
        d->PDO_status[pdoNum].transmit_type_parameter = 0;
        d->PDO_status[pdoNum].last_message.cob_id = 0;
        pdoNum++;
//        offsetObjdict++;
      }
}
