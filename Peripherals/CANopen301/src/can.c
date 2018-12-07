#include "states.h"
#include "TestSlave.h"
#include "CANopenMaster.h"
#include "can.h"
#include "canfestival.h"
#include "HAL_CAN.h"

unsigned char initCan(int can_port_num,unsigned int bitrate)
{
	if(1 == can_port_num)
	{
        CAN1_Config(bitrate);	
	}
	else if(2 == can_port_num)
	{
        CAN2_Config(bitrate);	
	}

	return 0;
}


// process the CAN1 Received msg
//unsigned int count_receive=0;
//int cobID_error = 0;
void CAN1_RX0_IRQHandler(void)
{
    CAN_ITConfig(CAN1,CAN_IT_FMP0, DISABLE);//防止中断服务函数还没执行完又有数据传过来了

	CanRxMsg RxMessage;
	Message m;
	int i;
	CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);
	m.cob_id=RxMessage.StdId;
    
	if(RxMessage.RTR == CAN_RTR_REMOTE)
		 m.rtr=1;
	else if(RxMessage.RTR == CAN_RTR_DATA)
		 m.rtr=0;
		 m.len=RxMessage.DLC;
	for(i = 0; i < RxMessage.DLC; i++)
		 m.data[i]=RxMessage.Data[i];
	canDispatch(&TestSlave_Data, &m);
	
    
    CAN_ITConfig(CAN1,CAN_IT_FMP0, ENABLE); 
}

/***************************************************************************************/
unsigned int  send_num = 0;
int master_send_faild = 0;
int slave_send_faild = 0;
unsigned char canSend(CO_Data* d, Message *m)
{
	uint32_t  i;
    
	CanTxMsg TxMessage;
	CanTxMsg *ptx_msg=&TxMessage;
	ptx_msg->StdId = m->cob_id;
    
    d->send_num=1;

		if(m->rtr)
		{	 
			ptx_msg->RTR = CAN_RTR_REMOTE;
		}
		else
		{
			 ptx_msg->RTR = CAN_RTR_DATA;
		}
		
		ptx_msg->IDE = CAN_ID_STD;
		ptx_msg->DLC = m->len;
		
		for(i = 0; i < m->len; i++)
		{
			 ptx_msg->Data[i] = m->data[i];
		}
		
    if( CAN_Transmit( d->canHandle, ptx_msg )==CAN_NO_MB ) /* 如果没有空闲的发送邮箱可以使用，则返回0xff,发送成功则返回0x00 */
    {
		if(d->can_port_num == 1)
		{
		    slave_send_faild++;
		}
		
		if(d->can_port_num == 2)
		{
		    master_send_faild++;
		}
        return 0xff;/* 失败 */
    }
    else
    {
        d->send_num=0;
        return 0x00;/* 成功 */
    }
}
