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

#ifndef __data_h__
#define __data_h__

#ifdef __cplusplus
extern "C" {
#endif

/* declaration of CO_Data type let us include all necessary headers
 struct struct_CO_Data can then be defined later
 */
typedef struct struct_CO_Data CO_Data;


	
//static inline int jdh(int aksdj)
//	{
//		int fjkasd=0;
//	}
#include "applicfg.h"
#include "def.h"
#include "can.h"
#include "timer.h"
#include "objdictdef.h"
#include "objacces.h"
#include "sdo.h"
#include "pdo.h"
#include "states.h"
#include "lifegrd.h"
#include "sync.h"
#include "nmtSlave.h"
#include "nmtMaster.h"
#include "emcy.h"
#include "ObjectDictionary.h"

#ifdef CO_ENABLE_LSS
#include "lss.h"
#endif


#define SDO_WRITE_ERROR_NUM 16

////////////////////////////////////////////////////////////////////////////////
// this part of typedef in timer.h before

typedef void (*TimerCallback_t)(CO_Data* d, UNS32 id);

struct struct_s_timer_entry {
	UNS8 state;                    /* TIMER_FREE表示该入口空闲，TIMER_ARMED表示该入口被事件占用 */
	CO_Data* d;                    /* 对象字典指针 */
	TimerCallback_t callback;      /* 该入口的事件回调函数 */
	UNS32 id;                      /* 该回调函数ID，结构体数组编号(从0开始编号) */
	TIMEVAL val;                   /* 第一次触发定时器时间的时间(从设置定时器时间开始计算) */
	TIMEVAL interval;              /* 周期触发时间，0表示非周期 */
};

typedef struct struct_s_timer_entry s_timer_entry;



/* ---------  prototypes --------- */
/*#define SetAlarm(d, id, callback, value, period) printf("%s, %d, SetAlarm(%s, %s, %s, %s, %s)\n",__FILE__, __LINE__, #d, #id, #callback, #value, #period); _SetAlarm(d, id, callback, value, period)*/
/**
 * @ingroup timer
 * @brief Set an alarm to execute a callback function when expired.
 * @param *d Pointer to a CAN object data structure
 * @param id The alarm Id
 * @param callback A callback function
 * @param value Call the callback function at current time + value
 * @param period Call periodically the callback function
 * @return handle The timer handle
 */
TIMER_HANDLE SetAlarm(CO_Data* d, UNS32 id, TimerCallback_t callback, TIMEVAL value, TIMEVAL period);

/**
 * @ingroup timer
 * @brief Delete an alarm before expiring.
 * @param handle A timer handle
 * @return The timer handle
 */
TIMER_HANDLE DelAlarm(TIMER_HANDLE handle,TimerCallback_t callback, CO_Data* d);


///////////////////////////////////////////////////////////////////////////////////



/**
 * @ingroup od
 * @brief This structure contains all necessary informations to define a CANOpen node 
 */
struct struct_CO_Data {
	/* Object dictionary */
	UNS16 *bDeviceNodeId;                                    /* &TestSlave_bDeviceNodeId */       /* 设备的节点ID，0x00~0xFF,即总共可以容纳127个节点设备，可通过setNodeId函数进行设置 */
    OD_OI_TypeDef *objdict;                                  /* TestSlave_objdict */
	s_PDO_status *PDO_status;                                /* TestSlave_PDO_status */
	TIMER_HANDLE *RxPDO_EventTimers;                         /* NULL */
	void (*RxPDO_EventTimers_Handler)(CO_Data*, UNS32);      /* _RxPDO_EventTimers_Handler */
	const UNS16 *ObjdictSize;                                /* TestSlave_ObjdictSize */
    UNS16 *iam_a_slave;                                 	 /* TestSlave_iam_a_slave */
	valueRangeTest_t valueRangeTest;                         /* TestSlave_valueRangeTest */
	OD_OI_TypeDef *(*Find_Object)(char*, UNS32, UNS32* , CO_Data* );/* find_object */
	
	/* SDO */
	s_transfer transfers[SDO_MAX_SIMULTANEOUS_TRANSFERS];    /*  REPEAT_SDO_MAX_SIMULTANEOUS_TRANSFERS_TIMES(s_transfer_Initializer) */
	/* s_sdo_parameter *sdo_parameters; */

	/* State machine */
	e_nodeState nodeState;                                       /* Unknown_state */
	s_state_communication CurrentCommunicationState;             /* {0,0,0,0,0,0,0} */
	initialisation_t initialisation;                             /* _initialisation */
	preOperational_t preOperational;                             /* _preOperational */
	operational_t operational;                                   /* _operational */
	stopped_t stopped;                                           /* _stopped */
    void (*NMT_Slave_Node_Reset_Callback)(CO_Data*);             /* NULL */
    void (*NMT_Slave_Communications_Reset_Callback)(CO_Data*);   /* NULL */
     
	/* NMT-heartbeat */
	UNS8 *ConsumerHeartbeatCount;            /* TestSlave_highestSubIndex_obj1016 */
	UNS32 *ConsumerHeartbeatEntries;         /* TestSlave_obj1016 */ /* 消费者心跳超时对象0x1016 */
	TIMER_HANDLE *ConsumerHeartBeatTimers;   /* TestSlave_heartBeatTimers */
	UNS16 *ProducerHeartBeatTime;            /* TestSlave_obj1017 */
	TIMER_HANDLE ProducerHeartBeatTimer;     /* TIMER_NONE */
	heartbeatError_t heartbeatError;         /* _heartbeatError */
	e_nodeState NMTable[NMT_MAX_NODE_ID];    /* {REPEAT_NMT_MAX_NODE_ID_TIMES(NMTable_Initializer)}, */ /* 用节点监护和心跳的方法收集其它节点的 d->nodeState */

	/* NMT-nodeguarding */
	TIMER_HANDLE GuardTimeTimer;            /* TIMER_NONE */
	TIMER_HANDLE LifeTimeTimer;             /* TIMER_NONE */
	nodeguardError_t nodeguardError;        /* _nodeguardError */
	UNS16 *GuardTime;                       /* TestSlave_obj100C */
	UNS8 *LifeTimeFactor;                   /* TestSlave_obj100D */
	UNS8 nodeGuardStatus[NMT_MAX_NODE_ID];  /* {REPEAT_NMT_MAX_NODE_ID_TIMES(nodeGuardStatus_Initializer)} *//* Flaxin 生命剩余值 */

	/* SYNC */
	TIMER_HANDLE syncTimer;                           /* TIMER_NONE */
	UNS32 *COB_ID_Sync;                               /* TestSlave_obj1005 */
	UNS32 *Sync_Cycle_Period;                         /* TestSlave_obj1006 */
	/*UNS32 *Sync_window_length;;*/
	post_sync_t post_sync;                            /* _post_sync */
	post_TPDO_t post_TPDO;                            /* _post_TPDO */
	post_SlaveBootup_t post_SlaveBootup;              /* _post_SlaveBootup */
    post_SlaveStateChange_t post_SlaveStateChange;    /* _post_SlaveStateChange */
	
	/* General */
	UNS8 toggle;                        /* 0 */
	CAN_PORT canHandle;	                /* NULL */  //CAN1, CAN2... hardware relative
//	scanIndexOD_t scanIndexOD;          /* TestSlave_scanIndexOD */
	storeODSubIndex_t storeODSubIndex;  /* _storeODSubIndex */
	
	/* DCF concise */
    const OD_OI_TypeDef* dcf_odentry;   /* NULL */
	UNS8* dcf_cursor;                /* NULL */
	UNS32 dcf_entries_count;         /* 1 */
	UNS8 dcf_status;                 /* 0 */
    UNS32 dcf_size;                  /* 0 */
    UNS8* dcf_data;                  /* NULL */
	
	/* EMCY */ /* Flaxin通过指针将这些变量和对象字典绑定在一起 */
	e_errorState error_state;               /* Error_free */ /* 指示节点是否存在错误 */
	UNS8 error_history_size;                /* sizeof(NODE_PREFIX ## _obj1003) / sizeof(NODE_PREFIX ## _obj1003[0]) */ /* 0x1003子索引的个数 */ /* Flaxin 在此工程中似乎是一个定值 */
	UNS8* error_number;                     /* TestSlave_highestSubIndex_obj1003 */ /* Flaxin 调用EMCY_setError函数的次数 */
	UNS32* error_first_element;             /* TestSlave_obj1003[0] */  /* Flaxin或许它就是对象0x1003,虽说它指示第一个元素，但第一个元素就是最新的错误（从EMCY_setError函数最后一个for循环那里就能看出来） *//* Flaxin 对象0x1003子索引的首地址,用的是指针列表方式，其实把子索引也表示出来了，参见0x1003值定义 */
	UNS8* error_register;                   /* TestSlave_obj1001 */ /* Flaxin 真正代表对象0x1001 */
    UNS32* error_cobid;                     /* TestSlave_obj1014 */
	s_errors error_data[EMCY_MAX_ERRORS];   /* REPEAT_EMCY_MAX_ERRORS_TIMES(ERROR_DATA_INITIALIZER) */ /* 记录发生过的错误的详细信息，最大8个，包括错误代码和错误寄存器，还有该错误是否发生（存在） */
	post_emcy_t post_emcy;                  /* _post_emcy */
	

//*********************************************************************
// new added data container to store can port and some global info,
// which make the canfestival protocal avilable to support more than one can station in one microcontroller
	UNS16 can_port_num;               //1,2,3,4...
	TIM_PORT can_timer_handle;          //TIM1, TIM2... hardware relative
    s_timer_entry timers[MAX_NB_TIMER];	
    TIMER_HANDLE last_timer_raw;	  // -1
	int totalobjnum;                  // 0
	OD_TypeDef gOD;
	TIMEVAL total_sleep_time;
	UNS32 send_num;
    TIMEVAL last_time_set;	
	int numsynctx;
	UNS16 pdo_time;
	int test_segmeng_download_request;
	int test_download_start_request;
	int sdoWriteNetworkDictCallBackAI_error[SDO_WRITE_ERROR_NUM];
	int getReadResultNetworkDict_num;
	int getElapsedTime_Error;
	int num_sync_rcv;
	int num_pdo_rcv;
	int line_error;
	int pdo_count;    // test TPDO transmit num
//*********************************************************************
#ifdef CO_ENABLE_LSS
	/* LSS */
	lss_transfer_t lss_transfer;
	lss_StoreConfiguration_t lss_StoreConfiguration;
#endif	

};




#define NMTable_Initializer Unknown_state,
#define nodeGuardStatus_Initializer 0x00,

#ifdef SDO_DYNAMIC_BUFFER_ALLOCATION
#define s_transfer_Initializer {\
		0,          /* CliServ{REPEAT_NMT_MAX_NODE_ID_TIMES(NMTable_Initializer)},Nbr */\
		0,          /* wohami */\
		SDO_RESET,  /* state */\
		0,          /* toggle */\
		0,          /* abortCode */\
		0,          /* index */\
		0,          /* subIndex */\
		0,          /* count */\
		0,          /* offset */\
		{0},        /* data (static use, so that all the table is initialize at 0)*/\
    NULL,       /* dynamicData */ \
    0,          /* dynamicDataSize */ \
		0,          /* peerCRCsupport */\
		0,          /* blksize */\
		0,          /* ackseq */\
		0,          /* objsize */\
		0,          /* lastblockoffset */\
		0,          /* seqno */\
		0,          /* endfield */\
		RXSTEP_INIT,/* rxstep */\
		{0},        /* tmpData */\
		0,          /* dataType */\
		-1,         /* timer */\
		NULL        /* Callback */\
	  },
#else
#define s_transfer_Initializer {\
		0,          /* nodeId */\
		0,          /* wohami */\
		SDO_RESET,  /* state */\
		0,          /* toggle */\
		0,          /* abortCode */\
		0,          /* index */\
		0,          /* subIndex */\
		0,          /* count */\
		0,          /* offset */\
		{0},        /* data (static use, so that all the table is initialize at 0)*/\
		0,          /* peerCRCsupport */\
		0,          /* blksize */\
		0,          /* ackseq */\
		0,          /* objsize */\
		0,          /* lastblockoffset */\
		0,          /* seqno */\
		0,          /* endfield */\
		RXSTEP_INIT,/* rxstep */\
		{0},        /* tmpData */\
		0,          /*  */\
		-1,         /*  */\
		NULL        /*  */\
	  },
#endif //SDO_DYNAMIC_BUFFER_ALLOCATION

#define ERROR_DATA_INITIALIZER \
	{\
	0, /* errCode */\
	0, /* errRegMask */\
	0 /* active */\
	},
	
#ifdef CO_ENABLE_LSS

#ifdef CO_ENABLE_LSS_FS	
#define lss_fs_Initializer \
		,0,						/* IDNumber */\
  		128, 					/* BitChecked */\
  		0,						/* LSSSub */\
  		0, 						/* LSSNext */\
  		0, 						/* LSSPos */\
  		LSS_FS_RESET,			/* FastScan_SM */\
  		-1,						/* timerFS */\
  		{{0,0,0,0},{0,0,0,0}}   /* lss_fs_transfer */
#else
#define lss_fs_Initializer
#endif		

#define lss_Initializer {\
		LSS_RESET,  			/* state */\
		0,						/* command */\
		LSS_WAITING_MODE, 		/* mode */\
		0,						/* dat1 */\
		0,						/* dat2 */\
		0,          			/* NodeID */\
		0,          			/* addr_sel_match */\
		0,          			/* addr_ident_match */\
		"none",         		/* BaudRate */\
		0,          			/* SwitchDelay */\
		SDELAY_OFF,   			/* SwitchDelayState */\
		NULL,					/* canHandle_t */\
		-1,						/* TimerMSG */\
		-1,          			/* TimerSDELAY */\
		NULL,        			/* Callback */\
		0						/* LSSanswer */\
		lss_fs_Initializer		/*FastScan service initialization */\
	  },\
	  NULL 	/* _lss_StoreConfiguration*/
#else
#define lss_Initializer
#endif


/* A macro to initialize the data in client app.*/
/* CO_Data structure */
#define CANOPEN_NODE_DATA_INITIALIZER(NODE_PREFIX, NUM, CANPORT, TIM_HANDLE) {\
/* Object dictionary*/\
	& NODE_PREFIX ## _bDeviceNodeId,     /* bDeviceNodeId */\
	NODE_PREFIX ## _objdict,             /* TestSlave_objdict  */\
	NODE_PREFIX ## _PDO_status,          /* PDO_status */\
	NULL,                                /* RxPDO_EventTimers */\
	_RxPDO_EventTimers_Handler,          /* RxPDO_EventTimers_Handler */\
	& NODE_PREFIX ## _ObjdictSize,       /* ObjdictSize */\
	& NODE_PREFIX ## _iam_a_slave,       /* iam_a_slave */\
	NODE_PREFIX ## _valueRangeTest,      /* valueRangeTest */\
	find_object,\
	/* SDO, structure s_transfer */\
	{\
          REPEAT_SDO_MAX_SIMULTANEOUS_TRANSFERS_TIMES(s_transfer_Initializer)\
	},\
	\
	/* State machine*/\
	Unknown_state,      /* nodeState */\
	/* structure s_state_communication */\
	{\
		0,          /* csBoot_Up */\
		0,          /* csSDO */\
		0,          /* csEmergency */\
		0,          /* csSYNC */\
		0,          /* csHeartbeat */\
		0,           /* csPDO */\
		0           /* csLSS */\
	},\
	NODE_PREFIX ## _initialisation,     /* initialisation */\
	_preOperational,     /* preOperational */\
	_operational,        /* operational */\
	_stopped,            /* stopped */\
	NULL,                /* NMT node reset callback */\
	NULL,                /* NMT communications reset callback */\
	\
	/* NMT-heartbeat */\
	& NODE_PREFIX ## _highestSubIndex_obj1016, /* ConsumerHeartbeatCount */\
	NODE_PREFIX ## _obj1016,                   /* ConsumerHeartbeatEntries */\
	NODE_PREFIX ## _heartBeatTimers,           /* ConsumerHeartBeatTimers  */\
	& NODE_PREFIX ## _obj1017,                 /* ProducerHeartBeatTime */\
	TIMER_NONE,                                /* ProducerHeartBeatTimer */\
	_heartbeatError,           /* heartbeatError */\
	\
	{REPEAT_NMT_MAX_NODE_ID_TIMES(NMTable_Initializer)},\
                                                   /* is  well initialized at "Unknown_state". Is it ok ? (FD)*/\
	\
	/* NMT-nodeguarding */\
	TIMER_NONE,                                /* GuardTimeTimer */\
	TIMER_NONE,                                /* LifeTimeTimer */\
	_nodeguardError,           /* nodeguardError */\
	& NODE_PREFIX ## _obj100C,                 /* GuardTime */\
	& NODE_PREFIX ## _obj100D,                 /* LifeTimeFactor */\
	{REPEAT_NMT_MAX_NODE_ID_TIMES(nodeGuardStatus_Initializer)},\
	\
	/* SYNC */\
	TIMER_NONE,                                /* syncTimer */\
	& NODE_PREFIX ## _obj1005,                 /* COB_ID_Sync */\
	& NODE_PREFIX ## _obj1006,                 /* Sync_Cycle_Period */\
	/*& NODE_PREFIX ## _obj1007, */            /* Sync_window_length */\
	_post_sync,                 /* post_sync */\
	_post_TPDO,                 /* post_TPDO */\
	_post_SlaveBootup,			/* post_SlaveBootup */\
    _post_SlaveStateChange,			/* post_SlaveStateChange */\
	\
	/* General */\
	0,                                         /* toggle */\
	CANPORT,                   /* canSend */\
	_storeODSubIndex,                /* storeODSubIndex */\
    /* DCF concise */\
    NULL,       /*dcf_odentry*/\
	NULL,		/*dcf_cursor*/\
	1,		/*dcf_entries_count*/\
	0,		/* dcf_status*/\
    0,      /* dcf_size */\
    NULL,   /* dcf_data */\
	\
	/* EMCY */\
	Error_free,                      /* error_state */\
	sizeof(NODE_PREFIX ## _obj1003) / sizeof(NODE_PREFIX ## _obj1003[0]),      /* error_history_size */\
	& NODE_PREFIX ## _highestSubIndex_obj1003,    /* error_number */\
	& NODE_PREFIX ## _obj1003[0],    /* error_first_element */\
	& NODE_PREFIX ## _obj1001,       /* error_register */\
    & NODE_PREFIX ## _obj1014,       /* error_cobid */\
	/* error_data: structure s_errors */\
	{\
	REPEAT_EMCY_MAX_ERRORS_TIMES(ERROR_DATA_INITIALIZER)\
	},\
	_post_emcy,              /* post_emcy */\
	/****************************************************************************************/\
	/****************************************************************************************/\
	/**********************************NEW DATA**********************************************/\
	NUM,        /* CAN_PORT_NUM */\
	TIM_HANDLE,    /* CAN_PORT_ADDR */\
	/* structure s_timer_entry */\
	{\
		{TIMER_FREE, NULL, NULL, 0, 0, 0},\
		{TIMER_FREE, NULL, NULL, 0, 0, 0},\
		{TIMER_FREE, NULL, NULL, 0, 0, 0},\
		{TIMER_FREE, NULL, NULL, 0, 0, 0},\
		{TIMER_FREE, NULL, NULL, 0, 0, 0},\
		{TIMER_FREE, NULL, NULL, 0, 0, 0},\
		{TIMER_FREE, NULL, NULL, 0, 0, 0},\
		{TIMER_FREE, NULL, NULL, 0, 0, 0}\
	},\
	-1,         /*last_timer_raw*/\
	0,          /*totalobjnum*/\
	{0},        /*gOD*/\
	TIMEVAL_MAX,/*total_sleep_time*/\
	0,          /*send_num*/\
	TIMEVAL_MAX,/*last_time_set*/\
	0,          /*numsynctx*/\
	0,          /*pdo_time*/\
	0,          /*test_segmeng_download_request*/\
	0,          /*test_download_start_request*/\
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},       /*sdoWriteNetworkDictCallBackAI_error*/\
	0,          /*getReadResultNetworkDict_num*/\
	0,          /*getElapsedTime_Error*/\
	0,          /*num_sync_rcv*/\
	0,          /*num_pdo_rcv*/\
	0,          /*line_error*/\
	0,          /*pdo_count*/\
	/* LSS */\
	lss_Initializer\
}

#ifdef __cplusplus
};
#endif

#endif /* __data_h__ */


