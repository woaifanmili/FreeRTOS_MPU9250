#include "states.h"
#include "TestSlave.h"
#include "CANopenMaster.h"
#include "can.h"
#include "canfestival.h"
#include "HAL_CAN.h"


// process the CAN1 Received msg
void canReceive(void)
{
  int i;
  CAN_RxHeaderTypeDef RxMessageHeader; 
	CAN_RxHeaderTypeDef *prx_msghead = &RxMessageHeader;
  uint8_t Data[8];    
  Message m;
  
  HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, prx_msghead, Data);
  m.cob_id=RxMessageHeader.StdId;
    
  if(RxMessageHeader.RTR == CAN_RTR_REMOTE)
     m.rtr=1;
  else if(RxMessageHeader.RTR == CAN_RTR_DATA)
     m.rtr=0;
     m.len=RxMessageHeader.DLC;
  for(i = 0; i < RxMessageHeader.DLC; i++)
     m.data[i]=Data[i];
  canDispatch(&TestSlave_Data, &m);
}

/***************************************************************************************/
unsigned int  send_num = 0;
int master_send_faild = 0;
int slave_send_faild = 0;
unsigned char canSend(CO_Data* d, Message *m)
{
	uint32_t  i;
  uint32_t  TxMailbox;
  
  CAN_TxHeaderTypeDef TxMessageHeader; 
	CAN_TxHeaderTypeDef *ptx_msghead=&TxMessageHeader;
  uint8_t Data[8];
  
	ptx_msghead->StdId = m->cob_id;
    
    d->send_num=1;

		if(m->rtr)
		{	 
			ptx_msghead->RTR = CAN_RTR_REMOTE;
		}
		else
		{
			 ptx_msghead->RTR = CAN_RTR_DATA;
		}
		
		ptx_msghead->IDE = CAN_ID_STD;
		ptx_msghead->DLC = m->len;
		
		for(i = 0; i < m->len; i++)
		{
			 Data[i] = m->data[i];
		}
		
    if( HAL_CAN_AddTxMessage(&hcan1, ptx_msghead, Data,&TxMailbox)==HAL_ERROR ) /* 如果没有空闲的发送邮箱可以使用，则返回0xff,发送成功则返回0x00 */
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
