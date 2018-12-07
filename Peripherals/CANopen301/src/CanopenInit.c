#include "TestSlave.h"
#include "CANopenMaster.h"
//#include "device_control.h"
#include "ST_GetId.h"
#include "timer.h"
#include "OD_CallbackProcess.h"
#include "global.h"


#define MASTER_NODE_ID 3

void CANopenSlaveInit(void)
{
	/* 由拨码开关获取节点ID */
	UNS16 nodeId = getId();
	g_node_id = nodeId;

	/* 将获取到的节点ID分配给该设备，作为该设备的节点号 */
	*TestSlave_Data.bDeviceNodeId = nodeId;

	/* 为301协议分配STM32的CAN外设 */
	TestSlave_Data.canHandle = CAN1;

	for(int i=0;i<16;i++)
	{
		TestSlave_Data.transfers[i].timer = -1;
	}

	/* 初始化STM32的CAN功能，使用CAN1，波特率设置为500Kbps */
	//CAN_BAUD_1M, CAN_BAUD_500K, CAN_BAUD_250K
	initCan(TestSlave_Data.can_port_num, CAN_BAUD_500K);

	/* DS301协议时间调度定时器初始化 */
	TIM_CANopenSlaveStart();

	/* 将该设备设置为从机 */
	*TestSlave_Data.iam_a_slave = 1;    // test code

	/* 注册对象字典并进行排序 */
    OD_OI_Reg( TestSlave_Data.objdict, *TestSlave_Data.ObjdictSize, &TestSlave_Data);

	CANopenSlaveCallbackRegister();
//	/*Define Producer HeartBeat time*/
//	{
//		UNS16 ProducerHeartBeatTime = 50;/* UNS16型数据，给值不能超65535，单位为毫秒：ms */

//		setODentry(&TestSlave_Data, 0x1017, 0x00, &ProducerHeartBeatTime, &sizeUNS16, OD_ACCESS_RW);
//	}
	/* 从机状态直接设置为状态机状态Stopped，该状态操作完后自动切换到Pre_operational，操作完后再自动切换到Operational状态上运行 */
	//setState(&TestSlave_Data, Stopped);/* _stopped */
	setState(&TestSlave_Data, Initialisation);/* _initialisation */ 	
}


void CANopenMasterInit(void)
{
	/* setting a constant node id */
	UNS16 nodeId = MASTER_NODE_ID;

	/* 将获取到的节点ID分配给该设备，作为该设备的节点号 */
	*CANopenMaster_Data.bDeviceNodeId = nodeId;

	/* 为301协议分配STM32的CAN外设 */
	CANopenMaster_Data.canHandle = CAN2;

	for(int i=0;i<16;i++)
	{
		CANopenMaster_Data.transfers[i].timer = -1;
	}

	/* init CAN2 of STM32, setting baudrate to 500Kbps */
	//CAN_BAUD_1M, CAN_BAUD_500K, CAN_BAUD_250K
	initCan(CANopenMaster_Data.can_port_num, CAN_BAUD_500K);
	
	/* DS301协议时间调度定时器初始化 */
	TIM_CANopenMasterStart();
	
	/* 将该设备设置为主机 */
	*CANopenMaster_Data.iam_a_slave = 0;
	
	/* 注册对象字典并进行排序 */
    OD_OI_Reg( CANopenMaster_Data.objdict, *CANopenMaster_Data.ObjdictSize, &CANopenMaster_Data);
	
	CANopenMasterCallbackRegister();
    
//		/*Define Producer HeartBeat time*/
//	{
//		UNS16 ProducerHeartBeatTime = 50;/* UNS16型数据，给值不能超65535，单位为毫秒：ms */

//		setODentry(&TestSlave_Data, 0x1017, 0x00, &ProducerHeartBeatTime, &sizeUNS16, OD_ACCESS_RW);
//	}
	/* 主机状态直接设置为状态机状态Initialisation，该状态操作完后自动切换到Pre_operational，操作完后再自动切换到Operational状态上运行 */
	setState(&CANopenMaster_Data, Initialisation);/* _initialisation */ 
}



void CanopenInit(void)
{
    CANopenSlaveInit();
	CANopenMasterInit();
}
