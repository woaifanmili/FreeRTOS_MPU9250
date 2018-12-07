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

/** @defgroup statemachine State Machine
 *  @ingroup userapi
 */
 
#ifndef __states_h__
#define __states_h__

#include <applicfg.h>
//#include "ST_GetId.h"
//#include "data.h"



extern UNS32 sizeUNS8;
extern UNS32 sizeUNS16;
extern UNS32 sizeUNS32;



/* 对象0x1005 */
#define SYNC_COBID_gen_0 	0x00000000
#define SYNC_COBID_gen_1 	0x40000000
#define SYNC_COBID_frame_0 	0x00000000
#define SYNC_COBID_frame_1 	0x20000000
/* 对象0x1014 */
#define EMCY_COBID_valid_0 	0x00000000
#define EMCY_COBID_valid_1 	0x80000000
#define EMCY_COBID_frame_0 	0x00000000
#define EMCY_COBID_frame_1 	0x20000000

/* 对象0x1200和0x1280,CSDO 和SSDO 相同 */
#define SDO_COBID_valid_0 	0x00000000
#define SDO_COBID_valid_1 	0x80000000
#define SDO_COBID_dyn_0 	0x00000000
#define SDO_COBID_dyn_1 	0x40000000
#define SDO_COBID_frame_0 	0x00000000
#define SDO_COBID_frame_1 	0x20000000
/* 对象0x1400 */
#define RPDO_COBID_valid_0 	0x00000000
#define RPDO_COBID_valid_1 	0x80000000
#define RPDO_COBID_frame_0 	0x00000000
#define RPDO_COBID_frame_1 	0x20000000
/* 对象0x1800 */
#define TPDO_COBID_valid_0 	0x00000000
#define TPDO_COBID_valid_1 	0x80000000
#define TPDO_COBID_RTR_0 	0x00000000
#define TPDO_COBID_RTR_1 	0x40000000
#define TPDO_COBID_frame_0 	0x00000000
#define TPDO_COBID_frame_1 	0x20000000
/* 对方的节点号 */
#define NodeID_OF_Opposite 0x01   /* 对方的节点号，本节点作为心跳消费者 */


/* The nodes states 
 * -----------------
 * values are choosen so, that they can be sent directly
 * for heartbeat messages...
 * Must be coded on 7 bits only
 * */
/* Should not be modified */
/* 节点运行状态枚举体 */
 /* Flaxin DS301 节点控制协议那小节 */
 /* NMT状态机状态和节点间连接关系状态两种写在一起了，只有一个状态是共用的 */
enum enum_nodeState {
  Initialisation  = 0x00, 
  Disconnected    = 0x01,
  Connecting      = 0x02,
  Preparing       = 0x02,
  Stopped         = 0x04,
  Operational     = 0x05,
  Pre_operational = 0x7F,
  Unknown_state   = 0x0F
};

typedef enum enum_nodeState e_nodeState;
typedef struct struct_CO_Data CO_Data;


/* 通信对象使能/使能标志位 */
typedef struct
{
	INTEGER8 csBoot_Up;     /* 引导报文 */
	INTEGER8 csSDO;         /* SDO报文 */
	INTEGER8 csEmergency;   /* 紧急报文 */
	INTEGER8 csSYNC;        /* 同步报文 */
	INTEGER8 csLifeGuard;   /* 心跳/节点保护报文 */
	INTEGER8 csPDO;         /* PDO报文 */
	INTEGER8 csLSS;         /* LSS报文 */
} s_state_communication;    



/** 
 * @brief Function that user app can overload
 * @ingroup statemachine
 */
typedef void (*initialisation_t)(CO_Data*);
typedef void (*preOperational_t)(CO_Data*);
typedef void (*operational_t)(CO_Data*);
typedef void (*stopped_t)(CO_Data*);


/** 
 * @ingroup statemachine
 * @brief Function that user app can overload
 * @param *d Pointer on a CAN object data structure
 */
void TestSlave_initialisation(CO_Data* d);
void CANopenMaster_initialisation(CO_Data* d);


/** 
 * @ingroup statemachine
 * @brief Function that user app can overload
 * @param *d Pointer on a CAN object data structure
 */
void _preOperational(CO_Data* d);

/**
 * @ingroup statemachine 
 * @brief Function that user app can overload
 * @param *d Pointer on a CAN object data structure
 */
void _operational(CO_Data* d);

/** 
 * @ingroup statemachine
 * @brief Function that user app can overload
 * @param *d Pointer on a CAN object data structure
 */
void _stopped(CO_Data* d);



/************************* prototypes ******************************/

/** 
 * @brief Called by driver/app when receiving messages
 * @param *d Pointer on a CAN object data structure
 * @param *m Pointer on a CAN message structure
 */
void canDispatch(CO_Data* d, Message *m);

/** 
 * @ingroup statemachine
 * @brief Returns the state of the node
 * @param *d Pointer on a CAN object data structure
 * @return The node state
 */
e_nodeState getState (CO_Data* d);

/** 
 * @ingroup statemachine
 * @brief Change the state of the node 
 * @param *d Pointer on a CAN object data structure
 * @param newState The state to assign
 * @return 
 */
UNS8 setState (CO_Data* d, e_nodeState newState);

/**
 * @ingroup statemachine 
 * @brief Returns the nodId 
 * @param *d Pointer on a CAN object data structure
 * @return
 */
UNS8 getNodeId (CO_Data* d);

/** 
 * @ingroup statemachine
 * @brief Define the node ID. Initialize the object dictionary
 * @param *d Pointer on a CAN object data structure
 * @param nodeId The node ID to assign
 */
void setNodeId (CO_Data* d, UNS8 nodeId);

/** 
 * @brief Some stuff to do when the node enter in pre-operational mode
 * @param *d Pointer on a CAN object data structure
 */
void initPreOperationalMode (CO_Data* d);




#endif
