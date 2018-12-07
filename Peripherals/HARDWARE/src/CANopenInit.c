#include "TestSlave.h"
#include "CANopenMaster.h"
#include "CANopenInit.h"
#include "timer.h"
void CANopenSlaveInit(void)
{
	/* �ɲ��뿪�ػ�ȡ�ڵ�ID */
	UNS16 nodeId = 0x09;
//	g_node_id = nodeId;

	/* ����ȡ���Ľڵ�ID��������豸����Ϊ���豸�Ľڵ�� */
	*TestSlave_Data.bDeviceNodeId = nodeId;

	/* Ϊ301Э�����STM32��CAN���� */
	TestSlave_Data.canHandle = CAN1;

	for(int i=0;i<16;i++)
	{
		TestSlave_Data.transfers[i].timer = -1;
	}

	/* ��ʼ��STM32��CAN���ܣ�ʹ��CAN1������������Ϊ500Kbps */
	//CAN_BAUD_1M, CAN_BAUD_500K, CAN_BAUD_250K
	initCan(TestSlave_Data.can_port_num, CAN_BAUD_500K);

	/* DS301Э��ʱ����ȶ�ʱ����ʼ�� */
	TIM_CANopenSlaveStart();

	/* �����豸����Ϊ�ӻ� */
	*TestSlave_Data.iam_a_slave = 0;    // test code

	/* ע������ֵ䲢�������� */
    OD_OI_Reg( TestSlave_Data.objdict, *TestSlave_Data.ObjdictSize, &TestSlave_Data);

	setState(&TestSlave_Data, Initialisation);/* _initialisation */ 	
}


void CanopenInit(void)
{
    CANopenSlaveInit();
}