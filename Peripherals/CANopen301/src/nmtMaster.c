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
** @file   nmtMaster.c
** @author Edouard TISSERANT and Francis DUPIN
** @date   Tue Jun  5 08:47:18 2007
**
** @brief
**
**
*/
#include "nmtMaster.h"
#include "canfestival.h"
#include "sysdep.h"

/*!
**
**功能：主站改变从站状态
** @param d：提供CAN通信端口
** @param Node_ID：从节点ID
** @param cs：状态码
**
** @return：0表示成功，1表示失败
**/
/* canopen的主站通过NMT报文来控制从站状态的切换 */
UNS8 masterSendNMTstateChange(CO_Data* d, UNS8 Node_ID, UNS8 cs)
{
  Message m;

  MSG_WAR(0x3501, "Send_NMT cs : ", cs);
  MSG_WAR(0x3502, "    to node : ", Node_ID);
  /* message configuration */
  m.cob_id = 0x0000; /*(NMT) << 7*/
  m.rtr = NOT_A_REQUEST;
  m.len = 2;
  m.data[0] = cs;
  m.data[1] = Node_ID;

  return canSend(d, &m);
}


/*!
**
**功能：节点监护--主站询问从站状态
** @param d：提供CAN通信端口
** @param nodeId：被监护的从节点的节点ID
**
** @return：0表示成功，1表示失败
**/
UNS8 masterSendNMTnodeguard(CO_Data* d, UNS8 nodeId)
{
  Message m;

  /* message configuration */
  /* 错误控制协议功能码1110 */
  UNS16 tmp = nodeId | (NODE_GUARD << 7); 
  m.cob_id = UNS16_LE(tmp);
  m.rtr = REQUEST;
  m.len = 0;

  MSG_WAR(0x3503, "Send_NODE_GUARD to node : ", nodeId);

  return canSend(d, &m);
}

/*!
**
**功能：主站询问节点号为nodeID的从节点的状态
** @param d：提供CAN通信端口
** @param nodeId：从节点ID
**
** @return：0表示成功，1表示失败
**/
/* 没被调用过 */
UNS8 masterRequestNodeState(CO_Data* d, UNS8 nodeId)
{
  /* FIXME: should warn for bad toggle bit. */

  /* NMTable configuration to indicate that the master is waiting
    for a Node_Guard frame from the slave whose node_id is ID
  */
  /* 设置节点状态为未知 */
  d->NMTable[nodeId] = Unknown_state; /* A state that does not exist
                                       */
  /* 设置所有节点状态为未知 */
  if (nodeId == 0) { /* NMT broadcast */
    UNS8 i = 0;
    for (i = 0 ; i < NMT_MAX_NODE_ID ; i++) {
      d->NMTable[i] = Unknown_state;
    }
  }
  /* 这个函数此位置之前的所有代码都是询问之前先清空该节点的状态 */
  return masterSendNMTnodeguard(d,nodeId);
}

