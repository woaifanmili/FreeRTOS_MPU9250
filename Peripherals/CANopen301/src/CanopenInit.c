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
	/* �ɲ��뿪�ػ�ȡ�ڵ�ID */
	UNS16 nodeId = getId();
	g_node_id = nodeId;

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
	*TestSlave_Data.iam_a_slave = 1;    // test code

	/* ע������ֵ䲢�������� */
    OD_OI_Reg( TestSlave_Data.objdict, *TestSlave_Data.ObjdictSize, &TestSlave_Data);

	CANopenSlaveCallbackRegister();
//	/*Define Producer HeartBeat time*/
//	{
//		UNS16 ProducerHeartBeatTime = 50;/* UNS16�����ݣ���ֵ���ܳ�65535����λΪ���룺ms */

//		setODentry(&TestSlave_Data, 0x1017, 0x00, &ProducerHeartBeatTime, &sizeUNS16, OD_ACCESS_RW);
//	}
	/* �ӻ�״ֱ̬������Ϊ״̬��״̬Stopped����״̬��������Զ��л���Pre_operational������������Զ��л���Operational״̬������ */
	//setState(&TestSlave_Data, Stopped);/* _stopped */
	setState(&TestSlave_Data, Initialisation);/* _initialisation */ 	
}


void CANopenMasterInit(void)
{
	/* setting a constant node id */
	UNS16 nodeId = MASTER_NODE_ID;

	/* ����ȡ���Ľڵ�ID��������豸����Ϊ���豸�Ľڵ�� */
	*CANopenMaster_Data.bDeviceNodeId = nodeId;

	/* Ϊ301Э�����STM32��CAN���� */
	CANopenMaster_Data.canHandle = CAN2;

	for(int i=0;i<16;i++)
	{
		CANopenMaster_Data.transfers[i].timer = -1;
	}

	/* init CAN2 of STM32, setting baudrate to 500Kbps */
	//CAN_BAUD_1M, CAN_BAUD_500K, CAN_BAUD_250K
	initCan(CANopenMaster_Data.can_port_num, CAN_BAUD_500K);
	
	/* DS301Э��ʱ����ȶ�ʱ����ʼ�� */
	TIM_CANopenMasterStart();
	
	/* �����豸����Ϊ���� */
	*CANopenMaster_Data.iam_a_slave = 0;
	
	/* ע������ֵ䲢�������� */
    OD_OI_Reg( CANopenMaster_Data.objdict, *CANopenMaster_Data.ObjdictSize, &CANopenMaster_Data);
	
	CANopenMasterCallbackRegister();
    
//		/*Define Producer HeartBeat time*/
//	{
//		UNS16 ProducerHeartBeatTime = 50;/* UNS16�����ݣ���ֵ���ܳ�65535����λΪ���룺ms */

//		setODentry(&TestSlave_Data, 0x1017, 0x00, &ProducerHeartBeatTime, &sizeUNS16, OD_ACCESS_RW);
//	}
	/* ����״ֱ̬������Ϊ״̬��״̬Initialisation����״̬��������Զ��л���Pre_operational������������Զ��л���Operational״̬������ */
	setState(&CANopenMaster_Data, Initialisation);/* _initialisation */ 
}



void CanopenInit(void)
{
    CANopenSlaveInit();
	CANopenMasterInit();
}
