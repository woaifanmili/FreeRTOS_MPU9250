/*
  This file is part of CanFestival, a library implementing CanOpen
  Stack.

  Copyright (C): Edouard TISSERANT and Francis DUPIN
  Modified by: Jaroslav Fojtik

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
** @file   emcy.c
** @author Luis Jimenez
** @date   Wed Sep 26 2007
**
** @brief Definitions of the functions that manage EMCY (emergency) messages
**
**
*/

#include <data.h>
#include "emcy.h"
#include "canfestival.h"
#include "sysdep.h"

UNS32 OnNumberOfErrorsUpdate(CO_Data* d, const OD_OI_typeDef * unsused_indextable, UNS8 unsused_bSubindex);

#define Data data  /* temporary fix */

/*! This is called when Index 0x1003 is updated.
**
**
** @param d
** @param unsused_indextable
** @param unsused_bSubindex
**
** @return
**/
/* 0x1003（预定义错误域）对象发生改变时的回调函数 */
/* Flaxin error_first_element代表的就是对象0x1003的0x01号子索引,所以当error_first_element变化了就要调用该函数 */
UNS32 OnNumberOfErrorsUpdate(CO_Data* d, const OD_OI_typeDef * unsused_indextable, UNS8 unsused_bSubindex)
{
	UNS8 index;
  // if 0, reset Pre-defined Error Field
  // else, don't change and give an abort message (eeror code: 0609 0030h)
	/* 如果此时所有错误都被解决，清空所有错误记录 */
	if (*d->error_number == 0)
        for (index = 0; index < d->error_history_size; ++index)
            *(d->error_first_element + index) = 0;		/* clear all the fields in Pre-defined Error Field (1003h) */
	else
		;// abort message /* Flaxin 需要什么功能需要自己添加 */
  return 0;
}

/*! start the EMCY mangagement.
**
**
** @param d
**/
 /* 和SYNC报文一样，EMCY报文也会随着状态的切换开启和关闭 */
/* 设置0x1003（预定义错误域）的回调函数 */
void emergencyInit(CO_Data* d)
{
  /* 设置0x1003对象发生改变时的回调函数 */
  /* Flaxin 如果没有理解错的话，是将OnNumberOfErrorsUpdate函数的地址给写到对象0x1003的TestSlave_Index1003_callbacks[0x00] */
  RegisterSetODentryCallBack(d, 0x10030000, &OnNumberOfErrorsUpdate); /* Flaxin 问题： 注册了回调函数什么时候会调用它呢？ */
	/* 只给0x1003,0x00这一个子索引设置了回调函数，_setODentry函数写0x1003的1-8子索引并不会触发回调函数，因为并未给他们设置回调函数 */
  /* 错误个数清0 */
  *d->error_number = 0;/* 对象0x1003,0x00子索引，代表对象0x1003最高子索引号 */
}

/*!
**
**
** @param d
**/
/* 停止紧急报文 */
void emergencyStop(CO_Data* d)
{

}


/*!
 **
 ** @param d
 ** @param cob_id
 **
 ** @return
 **/
/* 发送紧急报文 */
/* 参见301中文文档P64EMCY写协议 */
UNS8 sendEMCY(CO_Data* d, UNS16 errCode, UNS8 errRegister, const void *Specific, UNS8 SpecificLength)
{
	Message m;

	MSG_WAR(0x3051, "sendEMCY", 0);

	m.cob_id = (UNS16)(*(UNS32*)d->error_cobid);
	m.rtr = NOT_A_REQUEST;
	m.Data[0] = errCode & 0xFF;        /* LSB */
	m.Data[1] = (errCode >> 8) & 0xFF; /* MSB */
	m.Data[2] = errRegister;
    /* 制造商特殊错误省略 */
	if(Specific==NULL)
	{
	  m.Data[3] = 0;		/* Manufacturer specific Error Field omitted */
	  m.Data[4] = 0;
	  m.Data[5] = 0;
	  m.Data[6] = 0;
	  m.Data[7] = 0;
	  SpecificLength = 5;
	}
	else
	{
      if(SpecificLength > 5) 
	  {
		  SpecificLength = 5;
	  }
	  memcpy(&m.Data[3],Specific,SpecificLength);
	}
	m.len = SpecificLength + 3;

	return canSend(d, &m);
}

/*! Sets a new error with code errCode. Also sets corresponding bits in Error register (1001h)
 **
 **
 ** @param d
 ** @param errCode Code of the error
 ** @param errRegister Bits of Error register (1001h) to be set.
 ** @return 1 if error, 0 if successful
 */
/* 设置一个新的错误集，在错误寄存器0x1001中设置相应位 */
 /* 产生新的错误之后，该错误未被解决之前会被存放在error_data中，并置为未被解决的状态 */
 /* 0x1003中，从新到旧记录着最近发生的一些错误，最多记录 error_history_size 条 */
 /* Flaxin注意，error_data中存在相应的错误记录并不说明节点就存在该错误，它只能说明曾今发生过该错误，该错误是不是仍然有效得看active是否等于1 */
/* 该函数在得知应用程序的某个地方发生了错误时调用，比如电机超电压了，电机超电流了，温度过高等等 */
UNS8 EMCY_setError(CO_Data* d, UNS16 errCode, UNS8 errRegMask, UNS16 addInfo)
{
	UNS8 index;
	UNS8 errRegister_tmp;
	/* 遍历错误记录 */
	for (index = 0; index < EMCY_MAX_ERRORS; ++index)
	{
		/* 该错误被记录过 */
		if (d->error_data[index].errCode == errCode)		/* error already registered */
		{
			/* 错误已经上报过，并依然存在，不做任何动作 */
			if (d->error_data[index].active)
			{
				MSG_WAR(0x3052, "EMCY message already sent", 0);
				return 0;
			}
			/* Flaxin该错误发生过，也被解决了，并且它的错误记录还没有被新来的错误给覆盖掉，该错误先前已经被解决过，这里则需要重新上报，就是该错误以前发生过，也解决过 */
			else
				d->error_data[index].active = 1;		/* set as active error */
			break;
		}
	}
	/* 如果该错误没有被记录过，则需要记录该错误 */
	if (index == EMCY_MAX_ERRORS)		/* if errCode not already registered */
		for (index = 0; index < EMCY_MAX_ERRORS; ++index)
			if (d->error_data[index].active == 0) /* Flaxin暂时没起作用的错误可能会被后来的错误覆盖掉，因为这里找的是第一个active为0的错误结构体 */
			  break;	/* find first inactive error */
	/* 记录错误的数组满了，则做出提示 */
	if (index == EMCY_MAX_ERRORS)		/* error_data full */
	{
		MSG_ERR(0x3053, "error_data full", 0);
		return 1;
	}
	/* 记录该错误的一些信息，并置该错误为有效即未被解决状态 */
	d->error_data[index].errCode = errCode;
	d->error_data[index].errRegMask = errRegMask;
	d->error_data[index].active = 1;

	/* set the new state in the error state machine */
	/* 在错误状态机设置为存在错误 */
	d->error_state = Error_occurred;

	/* set Error Register (1001h) */
	/* 重新更新一下错误寄存器0x1001 */
	for (index = 0, errRegister_tmp = 0; index < EMCY_MAX_ERRORS; ++index)
		if (d->error_data[index].active == 1) errRegister_tmp |= d->error_data[index].errRegMask;
	*d->error_register = errRegister_tmp;

	/* set Pre-defined Error Field (1003h) */
	/* 设定预定义错误域0x1003，这个位置存放最近发生的错误，最大不超过error_history_size个 */
	/* 把新错误加进来 */
	for (index = d->error_history_size - 1; index > 0; --index)
		*(d->error_first_element + index) = *(d->error_first_element + index - 1);
	*(d->error_first_element) = errCode | ((UNS32)addInfo << 16); /* 参考对象0x1003值定义 */
	/* 更新错误个数 */
	if(*d->error_number < d->error_history_size)
		++(*d->error_number); /* error_number初始化的时候是被赋值为0了，见对象字典中的TestSlave_highestSubIndex_obj1003
	                          而且error_number只在这里递增，emergencyInit函数中才会对他清零， 所以EMCY_setError和EMCY_errorRecovered
	                              函数设置和清除错误的做法是只要设置了错误，error_number就会加1，清除错误error_number并不会减1，而是将
                             	相应的active位清0了，所以说error_number并不能指示起作用的错误的个数，而只能通过错误的active位是否置一来判断相应的错误是否有效 */
	/* send EMCY message */
	/* 发送紧急报文 */
	 /* Flaxin 如果允许EMCY通信，则发送紧急报文，csEmergency置1了才是允许 */
	if (d->CurrentCommunicationState.csEmergency)
		return sendEMCY(d, errCode, *d->error_register, NULL, 0);
	else return 1;
}

/*! Deletes error errCode. Also clears corresponding bits in Error register (1001h)
 **
 **
 ** @param d
 ** @param errCode Code of the error
 ** @param errRegister Bits of Error register (1001h) to be set.
 ** @return 1 if error, 0 if successful
 */
 /* 消除错误调用EMCY_errorRecovered函数，当所有错误都被消除之后，发出报文 */
/* 删除一个错误集，在错误寄存器0x1001中删除相应的位 */
 /* 只是将错误的active位复位为0，再在错误寄存器中删除相应的位 */
/* 该函数在得知应用程序的某个错误被修复时调用，比如电机超电压问题解决了，电机超电流问题解决了等等 */
void EMCY_errorRecovered(CO_Data* d, UNS16 errCode)
{
	UNS8 index;
	UNS8 errRegister_tmp;
	UNS8 anyActiveError = 0;
	/* 找到错误记录 */
	for (index = 0; index < EMCY_MAX_ERRORS; ++index)
		if (d->error_data[index].errCode == errCode)
			break;		/* find the position of the error */

	/* 如果找到该记录，并且该错误依然存在 */
	if ((index != EMCY_MAX_ERRORS) && (d->error_data[index].active == 1))
	{
		/* 设置该错误为不活跃即该错误被处理 */
		d->error_data[index].active = 0;

		/* set Error Register (1001h) and check error state machine */
		/* 设置错误寄存器0x1001，并检查状态机 */
		/* 更新活跃标志位和掩码位 */
		for (index = 0, errRegister_tmp = 0; index < EMCY_MAX_ERRORS; ++index)
		{
			if (d->error_data[index].active == 1)
			{
				anyActiveError = 1;
				errRegister_tmp |= d->error_data[index].errRegMask; /* Flaxin 注意，这里只统计了仍然活跃的错误的错误寄存器，前面刚被清除的错误并没统计进来 */
			}                                                        /* Flaxin 所以结合后面的语句*d->error_register = errRegister_tmp;对错误寄存器0x1003还是清除了相应的位的 */
		}
        /* 如果没有任何错误未被处理，错误状态机设置为没有错误 */
		if(anyActiveError == 0)
		{
			d->error_state = Error_free;
			/* send a EMCY message with code "Error Reset or No Error" */
			/* 发送一个紧急消息，并通知已经没有错误了 */
			if (d->CurrentCommunicationState.csEmergency)
				sendEMCY(d, 0x0000, 0x00, NULL, 0);
		}
		/* 把掩码位存进错误寄存器 */
		*d->error_register = errRegister_tmp; /* 真正的错误寄存器的值是在这里更新的 */
	}
	else
		MSG_WAR(0x3054, "recovered error was not active", 0);
}

/*! This function is responsible to process an EMCY canopen-message.
 **
 **
 ** @param d
 ** @param m The CAN-message which has to be analysed.
 **
 **/
/* 这个函数负责处理canopen的紧急报文 */
/* 该文件中只有这个函数在canDispatch中被调用了，其他的暂时还没有用到 */
void proceedEMCY(CO_Data* d, Message* m)
{
	UNS8 nodeID;
	UNS16 errCode;
	UNS8 errReg;

	MSG_WAR(0x3055, "EMCY received. Proceed. ", 0);

	/* Test if the size of the EMCY is ok */
	if ( m->len != 8) {
		MSG_ERR(0x1056, "Error size EMCY. CobId  : ", m->cob_id);
		return;
	}
	/* post the received EMCY */
	/* 上报系统接收到紧急报文 */
	nodeID = m->cob_id & 0x7F;/* 从这里就可以看出来是谁发了这个emcy帧，即nodeID并不是该节点的节点号，而是因出现错误而发出EMCY帧的节点的节点号 */
	errCode = m->Data[0] | ((UNS16)m->Data[1] << 8);
	errReg = m->Data[2];
	(*d->post_emcy)(d, nodeID, errCode, errReg); /* Flaxin 函数的具体功能还没有实现，空函数就在下面，因为初始化的时候给d->post_emcy赋的值是下面这个函数 */
}

void _post_emcy(CO_Data* d, UNS8 nodeID, UNS16 errCode, UNS8 errReg){}/* nodeID 为出错的节点的节点号 */
