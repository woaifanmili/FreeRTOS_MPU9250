#include "TestSlave.h"
#include "CANopenMaster.h"
#include "CANopenInit.h"
#include "timer.h"
void CANopenSlaveInit(void)
{
	/* 由拨码开关获取节点ID */
	UNS16 nodeId = 0x09;
//	g_node_id = nodeId;

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
	*TestSlave_Data.iam_a_slave = 0;    // test code

	/* 注册对象字典并进行排序 */
    OD_OI_Reg( TestSlave_Data.objdict, *TestSlave_Data.ObjdictSize, &TestSlave_Data);

	setState(&TestSlave_Data, Initialisation);/* _initialisation */ 	
}


void CanopenInit(void)
{
    CANopenSlaveInit();
}