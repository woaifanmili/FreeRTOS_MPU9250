#include "TestSlave.h"
#include "CANopenMaster.h"
#include "ObjectDictionary.h"
//#include "global.h"
//#include "MC_PositionVelocityTime.h"


#define CAN_PORT2 CAN2
#define CAN_NUM2  2



/**************************************************************************/
/* Declaration of the value range types                                   */
/**************************************************************************/

#define VALUE_RANGE_EMC 0x9F /* Type for index 0x1003 subindex 0x00 (only set of value 0 is possible) */
UNS32 CANopenMaster_valueRangeTest (UNS8 typeValue, void * value)
{
  switch (typeValue) {
    case VALUE_RANGE_EMC:
      if (*(UNS8*)value != (UNS8)0) 
	  {
		  return OD_VALUE_RANGE_EXCEEDED;
	  }
      break;
  }
  return 0;
}


// data definition , data will be initialized when compiling 
/**************************************************************************/
/* The node id                                                            */
/**************************************************************************/
/* node_id default value.*/
UNS16 CANopenMaster_bDeviceNodeId = 0x00;
UNS16 CANopenMaster_iam_a_slave = 0;

TIMER_HANDLE CANopenMaster_heartBeatTimers[4]={TIMER_NONE,TIMER_NONE,TIMER_NONE,TIMER_NONE};

/*
$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

                               OBJECT DICTIONARY

$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
*/

UNS32 empty_obj_2F01 = 0;

OD_OI_TypeDef CANopenMaster_Index2F01[] = 
{
	{"empty obj for test", 0x2F010000, OD_OBJCODE_VAR, OD_DATATYPE_I32, OD_ACCESS_R, (void *)&empty_obj_2F01, 0, NULL, "" },
};


///////////////////////////////////////////////////////////////////////////
UNS32 CANopenMaster_obj1000 = 0x12D;	/* Canopen 402 协议 */
OD_OI_TypeDef CANopenMaster_Index1000[] = 
{
	{"Device Type", 0x10000000, OD_OBJCODE_VAR, OD_DATATYPE_U32, OD_ACCESS_R, (void *)&CANopenMaster_obj1000, 0, NULL, "16位的设备协议或应用协议 + 16位的逻辑设备的附加功能信息" },
};


///////////////////////////////////////////////////////////////////////////
UNS8 CANopenMaster_obj1001 = 0x00;	/* 0 */
OD_OI_TypeDef CANopenMaster_Index1001[] = 
{
	{"Error Register", 0x10010000, OD_OBJCODE_VAR, OD_DATATYPE_U8, OD_ACCESS_R, (void *)&CANopenMaster_obj1001, 0, NULL, "Canopen设备内部错误信息记录" },
};


////////////////////////////////////////////////////////////////////////////
extern OD_OI_TypeDef  CAN_master_error_field[];
//UNS8 CANopenMaster_highestSubIndex_obj1003 = 0x00; /* 初值为啥无所谓，emergencyInit的时候会把这赋值为0，即清零*/
UNS8 CANopenMaster_highestSubIndex_obj1003 = 8; /* 初值为啥无所谓，emergencyInit的时候会把这赋值为0，即清零*/
UNS32 CANopenMaster_obj1003[] = 
                    {
                      0x0,	/* TestSlave_obj1003[0]  */
                      0x0,	/* TestSlave_obj1003[1]  */
                      0x0,	/* TestSlave_obj1003[2]  */
                      0x0,	/* TestSlave_obj1003[3]  */
                      0x0,	/* TestSlave_obj1003[4]  */
                      0x0,	/* TestSlave_obj1003[5]  */
					  0x0,	/* TestSlave_obj1003[6]  */
                      0x0	/* TestSlave_obj1003[7]  */
                    };
					 /* 对象子索引回调函数列表 */
ODCallback_t CANopenMaster_Index1003_callbacks[] = 
										 {
										   Call_Null,
										   Call_Null,
										   Call_Null,
										   Call_Null,
										   Call_Null,
										   Call_Null,
										   Call_Null,
										   Call_Null,
										   Call_Null
										 };
				
OD_OI_TypeDef CANopenMaster_Index1003[] = 
{
	{"Pre-defined Error Field", 0x100300FF, OD_OBJCODE_ARRAY, OD_DATATYPE_U32, OD_ACCESS_RW, (void *)&CAN_master_error_field, 9, NULL, "Canopen设备内部错误信息记录" },
};

OD_OI_TypeDef  CAN_master_error_field[]=
{
  {"Number of errors", 0x10030000, OD_OBJCODE_NULL, OD_DATATYPE_U8,  OD_ACCESS_RW, (void *)&CANopenMaster_highestSubIndex_obj1003, 0, NULL, "错误数" }	,
  {"Error Field_1",    0x10030001, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_R,  (void *)&CANopenMaster_obj1003[0], 0, NULL, "最新的错误储存在这里，旧错误依次向下移" },
  {"Error Field_2",    0x10030002, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_R,  (void *)&CANopenMaster_obj1003[1], 0, NULL, "第2新的错误" },
  {"Error Field_3",    0x10030003, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_R,  (void *)&CANopenMaster_obj1003[2], 0, NULL, "第3新的错误" },
  {"Error Field_4",    0x10030004, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_R,  (void *)&CANopenMaster_obj1003[3], 0, NULL, "第4新的错误" },
  {"Error Field_5",    0x10030005, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_R,  (void *)&CANopenMaster_obj1003[4], 0, NULL, "第5新的错误" },
  {"Error Field_6",    0x10030006, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_R,  (void *)&CANopenMaster_obj1003[5], 0, NULL, "第6新的错误" },
  {"Error Field_7",    0x10030007, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_R,  (void *)&CANopenMaster_obj1003[6], 0, NULL, "第7新的错误" },
  {"Error Field_8",    0x10030008, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_R,  (void *)&CANopenMaster_obj1003[7], 0, NULL, "第8新的错误" },
};


///////////////////////////////////////////////////////////////////////////
UNS32 CANopenMaster_obj1005 = 0x00000080;	/* 0 */
ODCallback_t CANopenMaster_Index1005_callbacks[] = 
 {
   NULL,
 };
OD_OI_TypeDef CANopenMaster_Index1005[] = 
{
	{"SYNC COB ID", 0x10050000, OD_OBJCODE_VAR, OD_DATATYPE_U32, OD_ACCESS_RW, (void *)&CANopenMaster_obj1005, 0, NULL, "已配置的同步对象的COB-ID" },
};


///////////////////////////////////////////////////////////////////////////
UNS32 CANopenMaster_obj1006 = 0x0;	/* 0 */
ODCallback_t CANopenMaster_Index1006_callbacks[] = 
 {
   NULL,
 };
OD_OI_TypeDef CANopenMaster_Index1006[] = 
{
	{"Communication Cycle Period", 0x10060000, OD_OBJCODE_VAR, OD_DATATYPE_U32, OD_ACCESS_RW, (void *)&CANopenMaster_obj1006, 0, NULL, "SYNC时间间隔，单位是us" },
};

///////////////////////////////////////////////////////////////////////////
UNS16 CANopenMaster_obj100C = 0x0;	/* 不推荐使用节点监护，所以此处设置为0 */
ODCallback_t CANopenMaster_Index100C_callbacks[] = 
 {
   NULL,
 };
OD_OI_TypeDef CANopenMaster_Index100C[] = 
{
	{"GuardTime", 0x100C0000, OD_OBJCODE_VAR, OD_DATATYPE_U16, OD_ACCESS_RW, (void *)&CANopenMaster_obj100C, 0, NULL, "监护周期，单位为ms" },
};


///////////////////////////////////////////////////////////////////////////
UNS8 CANopenMaster_obj100D = 0x0;	/* 不推荐使用节点监护，所以此处设置为0 */
ODCallback_t CANopenMaster_Index100D_callbacks[] = 
 {
   NULL,
 };
OD_OI_TypeDef CANopenMaster_Index100D[] = 
{
	{"LifeTimeFactor", 0x100D0000, OD_OBJCODE_VAR, OD_DATATYPE_U8, OD_ACCESS_RW, (void *)&CANopenMaster_obj100D, 0, NULL, "生存周期因子" },
};


///////////////////////////////////////////////////////////////////////////
UNS32 CANopenMaster_obj1014 = 0;	/* 0x80 + Node-ID */
OD_OI_TypeDef CANopenMaster_Index1014[] = 
{
	{"Emergency COB ID", 0x10140000, OD_OBJCODE_VAR, OD_DATATYPE_U32, OD_ACCESS_RW, (void *)&CANopenMaster_obj1014, 0, NULL, "应急消息 COB_ID" },
};


///////////////////////////////////////////////////////////////////////////
extern OD_OI_TypeDef  master_consumer_heartbeat_time[];
UNS8 CANopenMaster_highestSubIndex_obj1016 = 4;
UNS32 CANopenMaster_obj1016[] = 
                    {
                      0x0,	/* 0x10160001 */
                      0x0,	/* 0x10160002 */
                      0x0,	/* 0x10160003 */
                      0x0,	/* 0x10160004 */
                    };
OD_OI_TypeDef CANopenMaster_Index1016[] = 
{
	{"master Consumer Heartbeat Time", 0x101600FF, OD_OBJCODE_ARRAY, OD_DATATYPE_U32, OD_ACCESS_RW, (void *)&master_consumer_heartbeat_time, 5, NULL, "消费者心跳超时" },
};
OD_OI_TypeDef  master_consumer_heartbeat_time[]=
{
  {"number of HeartBeat Consumer", 0x10160000, OD_OBJCODE_NULL, OD_DATATYPE_U8,  OD_ACCESS_C,   (void *)&CANopenMaster_highestSubIndex_obj1016, 0, NULL, "心跳消费者数目,单位为ms" },	
  {"master station Consumer1_Heartbeat_Time",     0x10160001, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1016[0], 0, NULL, "消费者心跳1" },
  {"master station Consumer2_Heartbeat_Time",     0x10160002, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1016[1], 0, NULL, "消费者心跳2" },
  {"master station Consumer3_Heartbeat_Time",     0x10160003, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1016[2], 0, NULL, "消费者心跳3" },
  {"master station Consumer4_Heartbeat_Time",     0x10160004, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1016[3], 0, NULL, "消费者心跳4" },
};


///////////////////////////////////////////////////////////////////////////
UNS16 CANopenMaster_obj1017 = 0;	/* 0 */
ODCallback_t CANopenMaster_Index1017_callbacks[] = 
 {
   NULL,
 };
OD_OI_TypeDef CANopenMaster_Index1017[] = 
{
	{"master station Producer Heartbeat Time", 0x10170000, OD_OBJCODE_VAR, OD_DATATYPE_U16, OD_ACCESS_RW, (void *)&CANopenMaster_obj1017, 0, NULL, "生产者心跳超时" },
};


///////////////////////////////////////////////////////////////////////////
extern OD_OI_TypeDef  CANopenMaster_Identity[];
UNS8 CANopenMaster_highestSubIndex_obj1018 = 4; /* number of subindex - 1*/
UNS32 CANopenMaster_obj1018[] = 
                    {
                      0x0,	/* 0x10180000 */
                      0x0,	/* 0x10180001 */
                      0x0,	/* 0x10180002 */
                      0x0,	/* 0x10180003 */
                    };
OD_OI_TypeDef CANopenMaster_Index1018[] = 
{
	{"Identity", 0x101800FF, OD_OBJCODE_RECORD, OD_DATATYPE_ID, OD_ACCESS_R, (void *)&CANopenMaster_Identity, 5, NULL, "对象身份" },
};
OD_OI_TypeDef  CANopenMaster_Identity[]=
{
  {"highest subindex supported", 0x10180000, OD_OBJCODE_NULL, OD_DATATYPE_U8,  OD_ACCESS_C,  (void *)&CANopenMaster_highestSubIndex_obj1018, 0, NULL, "最高子索引数" },	
  {"Vendor_ID",                  0x10180001, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_R,  (void *)&CANopenMaster_obj1018[0], 0, NULL, "Vendor_ID" },
  {"Product_Code",     			 0x10180002, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_R,  (void *)&CANopenMaster_obj1018[1], 0, NULL, "产品码" },
  {"Revision_Number",     	     0x10180003, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_R,  (void *)&CANopenMaster_obj1018[2], 0, NULL, "修订版本号" },
  {"Serial_Number",    			 0x10180004, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_R,  (void *)&CANopenMaster_obj1018[3], 0, NULL, "序列号" },
};


///////////////////////////////////////////////////////////////////////////
extern OD_OI_TypeDef  CAN_master_server_SDO_parameter[];
UNS8 CANopenMaster_highestSubIndex_obj1200 = 3; /* number of subindex - 1*/
UNS32 CANopenMaster_obj1200[] = 
                    {
                      0x600,	/* 600h + Node-ID */
                      0x580,	/* 580h + Node-ID */
                      0x000,
                    };
OD_OI_TypeDef CANopenMaster_Index1200[] = 
{
	{"SSDO Par", 0x120000FF, OD_OBJCODE_RECORD, OD_DATATYPE_SDOP, OD_ACCESS_R, (void *)&CAN_master_server_SDO_parameter, 4, NULL, "SDO服务器参数" },
};
OD_OI_TypeDef  CAN_master_server_SDO_parameter[]=
{
  {"highest subindex supported",           0x12000000, OD_OBJCODE_NULL, OD_DATATYPE_U8,  OD_ACCESS_C,  (void *)&CANopenMaster_highestSubIndex_obj1200, 0, NULL, "最高子索引数" },	
  {"COB_ID_Client_to_Server_Receive_SDO",  0x12000001, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1200[0], 0, NULL, "客户端 -> 服务器 COB_ID" },
  {"COB_ID_Server_to_Client_Transmit_SDO", 0x12000002, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1200[1], 0, NULL, "服务器 -> 客户端 COB_ID" },
  {"Node-ID of the SDO client",            0x12000003, OD_OBJCODE_NULL, OD_DATATYPE_U8,  OD_ACCESS_RW,  (void *)&CANopenMaster_obj1200[2], 0, NULL, "客户端 COB_ID" },
};


///////////////////////////////////////////////////////////////////////////
extern OD_OI_TypeDef  CAN_master_client_SDO_parameter[];
UNS8 CANopenMaster_highestSubIndex_obj1280 = 3; /* number of subindex - 1*/
UNS32 CANopenMaster_obj1280[] = 
                    {
                      0x580,	/* 600h + Node-ID */
                      0x600,	/* 600h + Node-ID */
					  0,
                    };
OD_OI_TypeDef CANopenMaster_Index1280[] = 
{
	{"CSDO Par", 0x128000FF, OD_OBJCODE_RECORD, OD_DATATYPE_SDOP, OD_ACCESS_R, (void *)&CAN_master_client_SDO_parameter, 4, NULL, "SDO服务器参数" },
};
OD_OI_TypeDef  CAN_master_client_SDO_parameter[]=
{
  {"highest subindex supported",            0x12800000, OD_OBJCODE_NULL, OD_DATATYPE_U8,  OD_ACCESS_C,   (void *)&CANopenMaster_highestSubIndex_obj1280, 0, NULL, "最高子索引数" },	
  {"COB_ID_Client_to_Server_Transmit_SDO",  0x12800001, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1280[0], 0, NULL, "客户端 -> 服务器 COB_ID" },
  {"COB_ID_Server_to_Client_Receive_SDO",   0x12800002, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1280[1], 0, NULL, "服务器 -> 客户端 COB_ID" },
  {"Node-ID of the SDO server",             0x12800003, OD_OBJCODE_NULL, OD_DATATYPE_U8,  OD_ACCESS_RW,  (void *)&CANopenMaster_obj1280[2], 0, NULL, "服务器COB_ID" },
};


///////////////////////////////////////////////////////////////////////////
extern OD_OI_TypeDef  can_master_receive_PDO_1_parameter[];
UNS8 CANopenMaster_highestSubIndex_obj1400 = 5; /* number of subindex - 1*/
UNS32 CANopenMaster_obj1400[] = 
                    {
                      0x200,	/* 0x18000001 */
                      0,	/* 0x18000002 */
					  0,	/* 0x18000003 */
                      0,	/* 0x18000004 */
                      0,	/* 0x18000005 */
                    };
ODCallback_t CANopenMaster_Index1400_callbacks[] = 
 {
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
 };
OD_OI_TypeDef CANopenMaster_Index1400[] = 
{
	{"RPDO1 Com", 0x140000FF, OD_OBJCODE_RECORD, OD_DATATYPE_PDOCP, OD_ACCESS_RW, (void *)&can_master_receive_PDO_1_parameter, 6, NULL, "RPDO通信参数1" },
};
OD_OI_TypeDef  can_master_receive_PDO_1_parameter[]=
{
  {"RPDO1 SubCount",            0x14000000, OD_OBJCODE_NULL, OD_DATATYPE_U8,  OD_ACCESS_C,   (void *)&CANopenMaster_highestSubIndex_obj1400, 0, NULL, "最高子索引数" },	
  {"RPDO1 COB_ID_used_by_PDO",  0x14000001, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1400[0], 0, NULL, "RPDO使用的COB_ID" },
  {"RPDO1 Transmission_Type", 	0x14000002, OD_OBJCODE_NULL, OD_DATATYPE_U8,  OD_ACCESS_RW,  (void *)&CANopenMaster_obj1400[1], 0, NULL, "传输类型" },
  {"RPDO1 Inhibit_Time",  		0x14000003, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1400[2], 0, NULL, "抑制时间，单位为100us" },
  {"RPDO1 Compatibility_Entry",	0x14000004, OD_OBJCODE_NULL, OD_DATATYPE_U8,  OD_ACCESS_RW,  (void *)&CANopenMaster_obj1400[3], 0, NULL, "保留" },
  {"RPDO1 Event_Timer", 		0x14000005, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1400[4], 0, NULL, "事件定时器,单位为ms" }
};



///////////////////////////////////////////////////////////////////////////
extern OD_OI_TypeDef  can_master_receive_PDO_2_parameter[];
UNS8 CANopenMaster_highestSubIndex_obj1401 = 5; /* number of subindex - 1*/
UNS32 CANopenMaster_obj1401[] = 
                    {
                      0x300,	/* 0x18010001 */
                      0x0,	/* 0x18010002 */
                      0x0,	/* 0x18010003 */
                      0x0,	/* 0x18010004 */
                      0x0,	/* 0x18010005 */
                    };
ODCallback_t CANopenMaster_Index1401_callbacks[] = 
 {
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
 };
OD_OI_TypeDef CANopenMaster_Index1401[] = 
{
	{"RPDO2 Com", 0x140100FF, OD_OBJCODE_RECORD, OD_DATATYPE_PDOCP, OD_ACCESS_RW, (void *)&can_master_receive_PDO_2_parameter, 6, NULL, "RPDO通信参数2" },
};
OD_OI_TypeDef  can_master_receive_PDO_2_parameter[]=
{
  {"RPDO2 SubCount",              0x14010000, OD_OBJCODE_NULL, OD_DATATYPE_U8,  OD_ACCESS_C,   (void *)&CANopenMaster_highestSubIndex_obj1401, 0, NULL, "最高子索引数" },	
  {"RPDO2 COB_ID_used_by_PDO",    0x14010001, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1401[0], 0, NULL, "RPDO使用的COB_ID" },
  {"RPDO2 Transmission_Type",     0x14010002, OD_OBJCODE_NULL, OD_DATATYPE_U8,  OD_ACCESS_RW,  (void *)&CANopenMaster_obj1401[1], 0, NULL, "传输类型" },
  {"RPDO2 Inhibit_Time",          0x14010003, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1401[2], 0, NULL, "抑制时间，单位为100us" },
  {"RPDO2 Compatibility_Entry",	  0x14010004, OD_OBJCODE_NULL, OD_DATATYPE_U8,  OD_ACCESS_RW,  (void *)&CANopenMaster_obj1401[3], 0, NULL, "保留" },
  {"RPDO2 Event_Timer",           0x14010005, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1401[4], 0, NULL, "事件定时器,单位为ms" },
};


///////////////////////////////////////////////////////////////////////////
extern OD_OI_TypeDef  can_master_receive_PDO_3_parameter[];
UNS8 CANopenMaster_highestSubIndex_obj1402 = 5; /* number of subindex - 1*/
UNS32 CANopenMaster_obj1402[] = 
                    {
                      0x400,	/* 0x18020001 */
                      0x0,	/* 0x18020002 */
					  0x0,	/* 0x18020003 */
                      0x0,	/* 0x18020004 */
                      0x0,	/* 0x18020005 */
                    };
ODCallback_t CANopenMaster_Index1402_callbacks[] = 
 {
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
 };
OD_OI_TypeDef CANopenMaster_Index1402[] = 
{
	{"RPDO3 Com", 0x140200FF, OD_OBJCODE_RECORD, OD_DATATYPE_PDOCP, OD_ACCESS_RW, (void *)&can_master_receive_PDO_3_parameter, 6, NULL, "RPDO通信参数3" },
};
OD_OI_TypeDef  can_master_receive_PDO_3_parameter[]=
{
  {"RPDO3 SubCount",              0x14020000, OD_OBJCODE_NULL, OD_DATATYPE_U8,  OD_ACCESS_C,   (void *)&CANopenMaster_highestSubIndex_obj1402, 0, NULL, "最高子索引数" },	
  {"RPDO3 COB_ID_used_by_PDO",    0x14020001, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1402[0], 0, NULL, "RPDO使用的COB_ID" },
  {"RPDO3 Transmission_Type",     0x14020002, OD_OBJCODE_NULL, OD_DATATYPE_U8,  OD_ACCESS_RW,  (void *)&CANopenMaster_obj1402[1], 0, NULL, "传输类型" },
  {"RPDO3 Inhibit_Time",          0x14020003, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1402[2], 0, NULL, "抑制时间，单位为100us" },
  {"RPDO3 Compatibility_Entry",   0x14020004, OD_OBJCODE_NULL, OD_DATATYPE_U8,  OD_ACCESS_RW,  (void *)&CANopenMaster_obj1402[3], 0, NULL, "保留" },
  {"RPDO3 Event_Timer",           0x14020005, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1402[4], 0, NULL, "事件定时器,单位为ms" },
};


///////////////////////////////////////////////////////////////////////////
extern OD_OI_TypeDef  can_master_receive_PDO_4_parameter[];
UNS8 CANopenMaster_highestSubIndex_obj1403 = 5; /* number of subindex - 1*/
UNS32 CANopenMaster_obj1403[] = 
                    {
                      0x500,	/* 0x18030001 */
                      0x0,	/* 0x18030002 */
                      0x0,	/* 0x18030003 */
                      0x0,	/* 0x18030004 */
                      0x0,	/* 0x18030005 */
                    };
ODCallback_t CANopenMaster_Index1403_callbacks[] = 
                     {
                       NULL,
                       NULL,
                       NULL,
                       NULL,
                       NULL,
                       NULL,
                     };
OD_OI_TypeDef CANopenMaster_Index1403[] = 
{
	{"RPDO4 Com", 0x140300FF, OD_OBJCODE_RECORD, OD_DATATYPE_PDOCP, OD_ACCESS_RW, (void *)&can_master_receive_PDO_4_parameter, 6, NULL, "RPDO通信参数4" },
};
OD_OI_TypeDef  can_master_receive_PDO_4_parameter[]=
{
  {"RPDO4 SubCount",               0x14030000, OD_OBJCODE_NULL, OD_DATATYPE_U8,  OD_ACCESS_C,   (void *)&CANopenMaster_highestSubIndex_obj1403, 0, NULL, "最高子索引数" },	
  {"RPDO4 COB_ID_used_by_PDO",     0x14030001, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1403[0], 0, NULL, "RPDO使用的COB_ID" },
  {"RPDO4 Transmission_Type",      0x14030002, OD_OBJCODE_NULL, OD_DATATYPE_U8,  OD_ACCESS_RW,  (void *)&CANopenMaster_obj1403[1], 0, NULL, "传输类型" },
  {"RPDO4 Inhibit_Time",           0x14030003, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1403[2], 0, NULL, "抑制时间，单位为100us" },
  {"RPDO4 Compatibility_Entry",    0x14030004, OD_OBJCODE_NULL, OD_DATATYPE_U8,  OD_ACCESS_RW,  (void *)&CANopenMaster_obj1403[3], 0, NULL, "保留" },
  {"RPDO4 Event_Timer",            0x14030005, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1403[4], 0, NULL, "事件定时器,单位为ms" },
};


///////////////////////////////////////////////////////////////////////////
extern OD_OI_TypeDef  can_master_receive_PDO_5_parameter[];
UNS8 CANopenMaster_highestSubIndex_obj1404 = 5; /* number of subindex - 1*/
UNS32 CANopenMaster_obj1404[] = 
                    {
                      0x501,	/* 0x18040001 */
                      0xFD,	/* 0x18040002 */
                      0x0,	/* 0x18040003 */
                      0x0,	/* 0x18040004 */
                      0x0,	/* 0x18040005 */
                    };
ODCallback_t CANopenMaster_Index1404_callbacks[] = 
                     {
                       NULL,
                       NULL,
                       NULL,
                       NULL,
                       NULL,
                       NULL,
                     };
OD_OI_TypeDef CANopenMaster_Index1404[] = 
{
	{"RPDO5 Com", 0x140400FF, OD_OBJCODE_RECORD, OD_DATATYPE_PDOCP, OD_ACCESS_RW, (void *)&can_master_receive_PDO_5_parameter, 6, NULL, "RPDO通信参数5" },
};
OD_OI_TypeDef  can_master_receive_PDO_5_parameter[]=
{
  {"RPDO5 SubCount",              0x14040000, OD_OBJCODE_NULL, OD_DATATYPE_U8,  OD_ACCESS_C,   (void *)&CANopenMaster_highestSubIndex_obj1404, 0, NULL, "最高子索引数" },	
  {"RPDO5 COB_ID_used_by_PDO",    0x14040001, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1404[0], 0, NULL, "RPDO使用的COB_ID" },
  {"RPDO5 Transmission_Type",     0x14040002, OD_OBJCODE_NULL, OD_DATATYPE_U8,  OD_ACCESS_RW,  (void *)&CANopenMaster_obj1404[1], 0, NULL, "传输类型" },
  {"RPDO5 Inhibit_Time",          0x14040003, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1404[2], 0, NULL, "抑制时间，单位为100us" },
  {"RPDO5 Compatibility_Entry",   0x14040004, OD_OBJCODE_NULL, OD_DATATYPE_U8,  OD_ACCESS_RW,  (void *)&CANopenMaster_obj1404[3], 0, NULL, "保留" },
  {"RPDO5 Event_Timer",           0x14040005, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1404[4], 0, NULL, "事件定时器,单位为ms" },
};


///////////////////////////////////////////////////////////////////////////
extern OD_OI_TypeDef  can_master_receive_PDO_1_mapping[];
UNS8 CANopenMaster_highestSubIndex_obj1600 = 2; /* number of subindex - 1*/
UNS32 CANopenMaster_obj1600[] =
                    {
                      0x60640000,	
                      0x60780000,	
                      0x0,	/* 0x18040003 */						
                    };
OD_OI_TypeDef CANopenMaster_Index1600[] = 
{
	{"RPDO1 Mapping", 0x160000FF, OD_OBJCODE_RECORD, OD_DATATYPE_PDOMAP, OD_ACCESS_RW, (void *)&can_master_receive_PDO_1_mapping, 3, NULL, "RPDO1映射参数" },
};
OD_OI_TypeDef  can_master_receive_PDO_1_mapping[]=
{
  {"RPDO1 MappingCount", 0x16000000, OD_OBJCODE_NULL, OD_DATATYPE_U8,  OD_ACCESS_RW,  (void *)&CANopenMaster_highestSubIndex_obj1600, 0, NULL, "最高子索引数" },	
  {"RPDO1 Mapping1",  	 0x16000001, OD_OBJCODE_NULL, OD_DATATYPE_I32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1600[0], 0, NULL, "映射参数值为0x60640000" },
  {"RPDO1 Mapping2", 	 0x16000002, OD_OBJCODE_NULL, OD_DATATYPE_I32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1600[1], 0, NULL, "映射参数值为0x60780000" },
};


///////////////////////////////////////////////////////////////////////////
extern OD_OI_TypeDef  can_master_receive_PDO_2_mapping[];
UNS8 CANopenMaster_highestSubIndex_obj1601 = 2; /* number of subindex - 1*/
UNS32 CANopenMaster_obj1601[] = 
                    {
						0x606C0000,	
						0x60770000,	
                        0x0,	/* 0x18040003 */						
                    };
OD_OI_TypeDef CANopenMaster_Index1601[] = 
{
	{"RPDO2 Mapping", 0x160100FF, OD_OBJCODE_RECORD, OD_DATATYPE_PDOMAP, OD_ACCESS_RW, (void *)&can_master_receive_PDO_2_mapping, 3, NULL, "RPDO2映射参数" },
};
OD_OI_TypeDef  can_master_receive_PDO_2_mapping[]=
{
	{"RPDO2 MappingCount", 0x16010000, OD_OBJCODE_NULL, OD_DATATYPE_U8,  OD_ACCESS_RW,  (void *)&CANopenMaster_highestSubIndex_obj1601, 0, NULL, "最高子索引数" },	
	{"RPDO2 Mapping1",     0x16010001, OD_OBJCODE_NULL, OD_DATATYPE_I32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1601[0], 0, NULL, "映射参数值为 0x606C0000" },
	{"RPDO2 Mapping2",     0x16010002, OD_OBJCODE_NULL, OD_DATATYPE_I32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1601[1], 0, NULL, "映射参数值为 0x60770000" },
};


///////////////////////////////////////////////////////////////////////////
extern OD_OI_TypeDef  can_master_receive_PDO_3_mapping[];
UNS8 CANopenMaster_highestSubIndex_obj1602 = 2; /* number of subindex - 1*/
UNS32 CANopenMaster_obj1602[] = 
                    {
                      0x2F040000,	
					  0x2F020000,
				      0x0,	/* 0x18040003 */
                    };
OD_OI_TypeDef CANopenMaster_Index1602[] = 
{
	{"RPDO3 Mapping", 0x160200FF, OD_OBJCODE_RECORD, OD_DATATYPE_PDOMAP, OD_ACCESS_RW, (void *)&can_master_receive_PDO_3_mapping, 3, NULL, "RPDO3映射参数" },
};
OD_OI_TypeDef  can_master_receive_PDO_3_mapping[]=
{
  {"RPDO3 MappingCount", 0x16020000, OD_OBJCODE_NULL, OD_DATATYPE_U8,  OD_ACCESS_RW,  (void *)&CANopenMaster_highestSubIndex_obj1602, 0, NULL, "最高子索引数" },	
  {"RPDO3 Mapping1",     0x16020001, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1602[0], 0, NULL, "映射参数值为 0x2F040000" },
  {"RPDO3 Mapping2",     0x16020002, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1602[1], 0, NULL, "映射参数值为 0x2F020000" },
};


///////////////////////////////////////////////////////////////////////////
extern OD_OI_TypeDef  can_master_receive_PDO_4_mapping[];
UNS8 CANopenMaster_highestSubIndex_obj1603 = 0; /* number of subindex - 1*/
UNS32 CANopenMaster_obj1603[] = 
                    {
                      0x2F010000	
                    };
OD_OI_TypeDef CANopenMaster_Index1603[] = 
{
	{"RPDO4 Mapping", 0x160300FF, OD_OBJCODE_RECORD, OD_DATATYPE_PDOMAP, OD_ACCESS_RW, (void *)&can_master_receive_PDO_4_mapping, 2, NULL, "RPDO4映射参数" },
};
OD_OI_TypeDef  can_master_receive_PDO_4_mapping[]=
{
  {"RPDO4 MappingCount", 0x16030000, OD_OBJCODE_NULL, OD_DATATYPE_U8,  OD_ACCESS_RW,  (void *)&CANopenMaster_highestSubIndex_obj1603, 0, NULL, "最高子索引数" },	
  {"RPDO4 Mapping1",  	 0x16030001, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1603[0],              0, NULL, "映射参数值为 0x2F010000" },
};


///////////////////////////////////////////////////////////////////////////
extern OD_OI_TypeDef  can_master_receive_PDO_5_mapping[];
UNS8 CANopenMaster_highestSubIndex_obj1604 = 0; /* number of subindex - 1*/
UNS32 CANopenMaster_obj1604[] = 
                    {
                      0x2F010000	
                    };
OD_OI_TypeDef CANopenMaster_Index1604[] = 
{
	{"RPDO5 Mapping", 0x160400FF, OD_OBJCODE_RECORD, OD_DATATYPE_PDOMAP, OD_ACCESS_RW, (void *)&can_master_receive_PDO_5_mapping, 2, NULL, "RPDO5映射参数" },
};
OD_OI_TypeDef  can_master_receive_PDO_5_mapping[]=
{
  {"RPDO5 MappingCount", 0x16040000, OD_OBJCODE_NULL, OD_DATATYPE_U8,  OD_ACCESS_RW,  (void *)&CANopenMaster_highestSubIndex_obj1604, 0, NULL, "最高子索引数" },	
  {"RPDO5 Mapping1",  	 0x16040001, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1604[0], 0, NULL, "映射参数值为 0x2F010000" },
};


/*******************************************************************************************************************************************/
/*******************************************************************************************************************************************/
extern OD_OI_TypeDef  can_master_transmit_PDO_1_parameter[];
UNS8 CANopenMaster_highestSubIndex_obj1800 = 4; /* number of subindex - 1*/
UNS32 CANopenMaster_obj1800[] = 
                    {
                      0x180,	/* 0x18000001 */
                      0x0,	/* 0x18000002 */
					  0x0,	/* 0x18000003 */
                      0x0,	/* 0x18000004 */
                      0x0,	/* 0x18000005 */
                    };
ODCallback_t CANopenMaster_Index1800_callbacks[] = 
 {
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
 };
OD_OI_TypeDef CANopenMaster_Index1800[] = 
{
	{"TPDO1 Com", 0x180000FF, OD_OBJCODE_RECORD, OD_DATATYPE_PDOCP, OD_ACCESS_RW, (void *)&can_master_transmit_PDO_1_parameter, 5, NULL, "TPDO1通信参数" },
};
OD_OI_TypeDef  can_master_transmit_PDO_1_parameter[]=
{
  {"TPDO1 SubCount",               0x18000000, OD_OBJCODE_NULL, OD_DATATYPE_U8,  OD_ACCESS_C,   (void *)&CANopenMaster_highestSubIndex_obj1800, 0, NULL, "最高子索引数" },	
  {"TPDO1 COB_ID_used_by_PDO",     0x18000001, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1800[0], 0, NULL, "TPDO使用的COB_ID" },
  {"TPDO1 Transmission_Type",      0x18000002, OD_OBJCODE_NULL, OD_DATATYPE_U8,  OD_ACCESS_RW,  (void *)&CANopenMaster_obj1800[1], 0, NULL, "传输类型" },
  {"TPDO1 Inhibit_Time",           0x18000003, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1800[2], 0, NULL, "抑制时间，单位为100us" },
  {"TPDO1 Compatibility_Entry",    0x18000004, OD_OBJCODE_NULL, OD_DATATYPE_U8,  OD_ACCESS_RW,  (void *)&CANopenMaster_obj1800[3], 0, NULL, "保留" },
  {"TPDO1 Event_Timer",            0x18000005, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1800[4], 0, NULL, "事件定时器,单位为ms" }
};


///////////////////////////////////////////////////////////////////////////
extern OD_OI_TypeDef  can_master_transmit_PDO_2_parameter[];
UNS8 CANopenMaster_highestSubIndex_obj1801 = 4; /* number of subindex - 1*/
UNS32 CANopenMaster_obj1801[] = 
                    {
                      0x280,	/* 0x18010001 */
                      0x0,	/* 0x18010002 */
                      0x0,	/* 0x18010003 */
                      0x0,	/* 0x18010004 */
                      0x0,	/* 0x18010005 */
                    };
ODCallback_t CANopenMaster_Index1801_callbacks[] = 
 {
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
 };
OD_OI_TypeDef CANopenMaster_Index1801[] = 
{
	{"TPDO2 Com", 0x180100FF, OD_OBJCODE_RECORD, OD_DATATYPE_PDOCP, OD_ACCESS_RW, (void *)&can_master_transmit_PDO_2_parameter, 5, NULL, "TPDO2通信参数" },
};
OD_OI_TypeDef  can_master_transmit_PDO_2_parameter[]=
{
  {"TPDO2 SubCount",                 0x18010000, OD_OBJCODE_NULL, OD_DATATYPE_U8,  OD_ACCESS_C,   (void *)&CANopenMaster_highestSubIndex_obj1801, 0, NULL, "最高子索引数" },	
  {"TPDO2 COB_ID_used_by_PDO",       0x18010001, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1801[0], 0, NULL, "TPDO使用的COB_ID" },
  {"TPDO2 Transmission_Type",        0x18010002, OD_OBJCODE_NULL, OD_DATATYPE_U8,  OD_ACCESS_RW,  (void *)&CANopenMaster_obj1801[1], 0, NULL, "传输类型" },
  {"TPDO2 Inhibit_Time",             0x18010003, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1801[2], 0, NULL, "抑制时间，单位为100us" },
  {"TPDO2 Compatibility_Entry",      0x18010004, OD_OBJCODE_NULL, OD_DATATYPE_U8,  OD_ACCESS_RW,  (void *)&CANopenMaster_obj1801[3], 0, NULL, "保留" },
  {"TPDO2 Event_Timer",              0x18010005, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1801[4], 0, NULL, "事件定时器,单位为ms" },
};

///////////////////////////////////////////////////////////////////////////
extern OD_OI_TypeDef  can_master_transmit_PDO_3_parameter[];
UNS8 CANopenMaster_highestSubIndex_obj1802 = 4; /* number of subindex - 1*/
UNS32 CANopenMaster_obj1802[] = 
                    {
                      0x380,	/* 0x18020001 */
                      0x0,	/* 0x18020002 */
                      0x0,	/* 0x18020003 */
                      0x0,	/* 0x18020004 */
                      0x0,	/* 0x18020005 */
                    };
ODCallback_t CANopenMaster_Index1802_callbacks[] = 
 {
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
 };
OD_OI_TypeDef CANopenMaster_Index1802[] = 
{
	{"TPDO3 Com", 0x180200FF, OD_OBJCODE_RECORD, OD_DATATYPE_PDOCP, OD_ACCESS_RW, (void *)&can_master_transmit_PDO_3_parameter, 5, NULL, "TPDO3通信参数" },
};
OD_OI_TypeDef  can_master_transmit_PDO_3_parameter[]=
{
  {"TPDO3 SubCount",               0x18020000, OD_OBJCODE_NULL, OD_DATATYPE_U8,  OD_ACCESS_C,   (void *)&CANopenMaster_highestSubIndex_obj1802, 0, NULL, "最高子索引数" },	
  {"TPDO3 COB_ID_used_by_PDO",     0x18020001, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1802[0], 0, NULL, "TPDO使用的COB_ID" },
  {"TPDO3 Transmission_Type",      0x18020002, OD_OBJCODE_NULL, OD_DATATYPE_U8,  OD_ACCESS_RW,  (void *)&CANopenMaster_obj1802[1], 0, NULL, "传输类型" },
  {"TPDO3 Inhibit_Time",           0x18020003, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1802[2], 0, NULL, "抑制时间，单位为100us" },
  {"TPDO3 Compatibility_Entry",    0x18020004, OD_OBJCODE_NULL, OD_DATATYPE_U8,  OD_ACCESS_RW,  (void *)&CANopenMaster_obj1802[3], 0, NULL, "保留" },
  {"TPDO3 Event_Timer",            0x18020005, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1802[4], 0, NULL, "事件定时器,单位为ms" },
};


///////////////////////////////////////////////////////////////////////////
extern OD_OI_TypeDef  can_master_transmit_PDO_4_parameter[];
UNS8 CANopenMaster_highestSubIndex_obj1803 = 4; /* number of subindex - 1*/
UNS32 CANopenMaster_obj1803[] = 
                    {
                      0x480,	/* 0x18030001 */
                      0x0,	/* 0x18030002 */
                      0x0,	/* 0x18030003 */
                      0x0,	/* 0x18030004 */
                      0x0,	/* 0x18030005 */
                    };
ODCallback_t CANopenMaster_Index1803_callbacks[] = 
 {
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
 };
OD_OI_TypeDef CANopenMaster_Index1803[] = 
{
	{"TPDO4 Com", 0x180300FF, OD_OBJCODE_RECORD, OD_DATATYPE_PDOCP, OD_ACCESS_RW, (void *)&can_master_transmit_PDO_4_parameter, 5, NULL, "TPDO4通信参数" },
};
OD_OI_TypeDef  can_master_transmit_PDO_4_parameter[]=
{
  {"TPDO4 SubCount",                0x18030000, OD_OBJCODE_NULL, OD_DATATYPE_U8,  OD_ACCESS_C,   (void *)&CANopenMaster_highestSubIndex_obj1803, 0, NULL, "最高子索引数" },	
  {"TPDO4 COB_ID_used_by_PDO",      0x18030001, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1803[0], 0, NULL, "TPDO使用的COB_ID" },
  {"TPDO4 Transmission_Type",       0x18030002, OD_OBJCODE_NULL, OD_DATATYPE_U8,  OD_ACCESS_RW,  (void *)&CANopenMaster_obj1803[1], 0, NULL, "传输类型" },
  {"TPDO4 Inhibit_Time",            0x18030003, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1803[2], 0, NULL, "抑制时间，单位为100us" },
  {"TPDO4 Compatibility_Entry",     0x18030004, OD_OBJCODE_NULL, OD_DATATYPE_U8,  OD_ACCESS_RW,  (void *)&CANopenMaster_obj1803[3], 0, NULL, "保留" },
  {"TPDO4 Event_Timer",             0x18030005, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1803[4], 0, NULL, "事件定时器,单位为ms" },
};


///////////////////////////////////////////////////////////////////////////
extern OD_OI_TypeDef  can_master_transmit_PDO_5_parameter[];
UNS8 CANopenMaster_highestSubIndex_obj1804 = 4; /* number of subindex - 1*/
UNS32 CANopenMaster_obj1804[] = 
                    {
                      0x401,	/* 0x18040001 */
                      0xFD,	/* 0x18040002 */
					  0x0,	/* 0x18040003 */
                      0x0,	/* 0x18040004 */
                      0x0,	/* 0x18040005 */
                    };
 ODCallback_t CANopenMaster_Index1804_callbacks[] = 
 {
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
 };
OD_OI_TypeDef CANopenMaster_Index1804[] = 
{
	{"TPDO5 Com", 0x180400FF, OD_OBJCODE_RECORD, OD_DATATYPE_PDOCP, OD_ACCESS_RW, (void *)&can_master_transmit_PDO_5_parameter, 5, NULL, "TPDO5通信参数" },
};
OD_OI_TypeDef  can_master_transmit_PDO_5_parameter[]=
{
  {"TPDO5 SubCount",                 0x18040000, OD_OBJCODE_NULL, OD_DATATYPE_U8,  OD_ACCESS_C,   (void *)&CANopenMaster_highestSubIndex_obj1804, 0, NULL, "最高子索引数" },	
  {"TPDO5 COB_ID_used_by_PDO",       0x18040001, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1804[0], 0, NULL, "TPDO使用的COB_ID" },
  {"TPDO5 Transmission_Type",        0x18040002, OD_OBJCODE_NULL, OD_DATATYPE_U8,  OD_ACCESS_RW,  (void *)&CANopenMaster_obj1804[1], 0, NULL, "传输类型" },
  {"TPDO5 Inhibit_Time",             0x18040003, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1804[2], 0, NULL, "抑制时间，单位为100us" },
  {"TPDO5 Compatibility_Entry",      0x18040004, OD_OBJCODE_NULL, OD_DATATYPE_U8,  OD_ACCESS_RW,  (void *)&CANopenMaster_obj1804[3], 0, NULL, "保留" },
  {"TPDO5 Event_Timer",              0x18040005, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1804[4], 0, NULL, "事件定时器,单位为ms" },
};


///////////////////////////////////////////////////////////////////////////
extern OD_OI_TypeDef  can_master_transmit_PDO_1_mapping[];
//extern int pvt_o_pos_ref;
UNS8 CANopenMaster_highestSubIndex_obj1A00 = 7; /* number of subindex - 1*/
UNS32 CANopenMaster_obj1A00[] = 
                    {
                      0x2F120000,	/* CANopenMaster_obj1A00[0] */
                      0,	/* CANopenMaster_obj1A00[1] */
                      0,	/* CANopenMaster_obj1A00[2] */
                      0,	/* CANopenMaster_obj1A00[3] */
                      0,	/* CANopenMaster_obj1A00[4] */
                      0,	/* CANopenMaster_obj1A00[5] */
                      0,	/* CANopenMaster_obj1A00[6] */
                      0,	/* CANopenMaster_obj1A00[7] */
                    };
OD_OI_TypeDef CANopenMaster_Index1A00[] = 
{
	{"TPDO1 Mapping", 0x1A0000FF, OD_OBJCODE_RECORD, OD_DATATYPE_PDOMAP, OD_ACCESS_RW, (void *)&can_master_transmit_PDO_1_mapping, 8, NULL, "TPDO1映射参数" },
};
OD_OI_TypeDef  can_master_transmit_PDO_1_mapping[]=
{
	{"TPDO1 MappingCount", 0x1A000000, OD_OBJCODE_NULL, OD_DATATYPE_U8,  OD_ACCESS_RW,  (void *)&CANopenMaster_highestSubIndex_obj1A00, 0, NULL, "最高子索引数" },	
	{"TPDO1 Mapping1",     0x1A000001, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1A00[0], 0, NULL, "映射参数值为0x2F120000" },									
	{"TPDO1 Mapping2", 	   0x1A000002, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1A00[1], 0, NULL, "映射参数值为" },					//pvt_o_vel_ref
	{"TPDO1 Mapping3",     0x1A000003, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1A00[2], 0, NULL, "映射参数值为" },
	{"TPDO1 Mapping4",	   0x1A000004, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1A00[3], 0, NULL, "映射参数值为" },
	{"TPDO1 Mapping5", 	   0x1A000005, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1A00[4], 0, NULL, "映射参数值为" },
	{"TPDO1 Mapping6",     0x1A000006, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1A00[5], 0, NULL, "映射参数值为" },
	{"TPDO1 Mapping7", 	   0x1A000007, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1A00[6], 0, NULL, "映射参数值为" },
	{"TPDO1 Mapping8",     0x1A000008, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1A00[7], 0, NULL, "映射参数值为" },
};


///////////////////////////////////////////////////////////////////////////
extern OD_OI_TypeDef  can_master_transmit_PDO_2_mapping[];
UNS8 CANopenMaster_highestSubIndex_obj1A01 = 7; /* number of subindex - 1*/
UNS32 CANopenMaster_obj1A01[] = 
                    {
						0,	
						0,	
						0,	
						0,	
						0,	
						0,	
						0,	
						0	
                    };
OD_OI_TypeDef CANopenMaster_Index1A01[] = 
{
	{"TPDO2 Mapping", 0x1A0100FF, OD_OBJCODE_RECORD, OD_DATATYPE_PDOMAP, OD_ACCESS_RW, (void *)&can_master_transmit_PDO_2_mapping, 8, NULL, "TPDO2映射参数" },
};
OD_OI_TypeDef  can_master_transmit_PDO_2_mapping[]=
{
	{"TPDO2 MappingCount",  0x1A010000, OD_OBJCODE_NULL, OD_DATATYPE_U8,  OD_ACCESS_RW,  (void *)&CANopenMaster_highestSubIndex_obj1A01, 0, NULL, "最高子索引数" },	
	{"TPDO2 Mapping1",      0x1A010001, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1A01[0], 0, NULL, "映射参数值为" },
	{"TPDO2 Mapping2",  	0x1A010002, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1A01[1], 0, NULL, "映射参数值为 0" },
	{"TPDO2 Mapping3",  	0x1A010003, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1A01[2], 0, NULL, "映射参数值为 0" },
	{"TPDO2 Mapping4",  	0x1A010004, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1A01[3], 0, NULL, "映射参数值为 0" },
	{"TPDO2 Mapping5",  	0x1A010005, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1A01[4], 0, NULL, "映射参数值为 0" },
	{"TPDO2 Mapping6",  	0x1A010006, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1A01[5], 0, NULL, "映射参数值为 0" },
	{"TPDO2 Mapping7",  	0x1A010007, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1A01[6], 0, NULL, "映射参数值为 0" },
	{"TPDO2 Mapping8",  	0x1A010008, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1A01[7], 0, NULL, "映射参数值为 0" },
};


///////////////////////////////////////////////////////////////////////////
extern OD_OI_TypeDef  can_master_transmit_PDO_3_mapping[];
UNS8 CANopenMaster_highestSubIndex_obj1A02 = 0; /* number of subindex - 1*/
UNS32 CANopenMaster_obj1A02[] = 
                    {
                      0x2F010000
                    };
OD_OI_TypeDef CANopenMaster_Index1A02[] = 
{
	{"TPDO3 Mapping", 0x1A0200FF, OD_OBJCODE_RECORD, OD_DATATYPE_PDOMAP, OD_ACCESS_RW, (void *)&can_master_transmit_PDO_3_mapping, 2, NULL, "TPDO3映射参数" },
};
OD_OI_TypeDef  can_master_transmit_PDO_3_mapping[]=
{
  {"TPDO3 MappingCount",    0x1A020000, OD_OBJCODE_NULL, OD_DATATYPE_U8,  OD_ACCESS_RW,  (void *)&CANopenMaster_highestSubIndex_obj1A02, 0, NULL, "最高子索引数" },	
  {"TPDO3 Mapping1",  	    0x1A020001, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1A02[0], 0, NULL, "映射参数值为 200B0010" },
};


///////////////////////////////////////////////////////////////////////////
extern OD_OI_TypeDef  can_master_transmit_PDO_4_mapping[];
UNS8 CANopenMaster_highestSubIndex_obj1A03 = 0; /* number of subindex - 1*/
UNS32 CANopenMaster_obj1A03[] = 
                    {
                      0x2F010000
                    };
OD_OI_TypeDef CANopenMaster_Index1A03[] = 
{
	{"TPDO4 Mapping", 0x1A0300FF, OD_OBJCODE_RECORD, OD_DATATYPE_PDOMAP, OD_ACCESS_RW, (void *)&can_master_transmit_PDO_4_mapping, 2, NULL, "TPDO4服务器参数" },
};
OD_OI_TypeDef  can_master_transmit_PDO_4_mapping[]=
{
  {"TPDO4 MappingCount", 0x1A030000, OD_OBJCODE_NULL, OD_DATATYPE_U8,  OD_ACCESS_RW,  (void *)&CANopenMaster_highestSubIndex_obj1A03, 0, NULL, "最高子索引数" },	
  {"TPDO4 Mapping1",  	 0x1A030001, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1A03[0], 0, NULL, "映射参数值为 200C0010" },
};

///////////////////////////////////////////////////////////////////////////

extern OD_OI_TypeDef  can_master_transmit_PDO_5_mapping[];
UNS8 CANopenMaster_highestSubIndex_obj1A04 = 0; /* number of subindex - 1*/
UNS32 CANopenMaster_obj1A04[] = 
                    {
                      0x2F010000	
                    };
OD_OI_TypeDef CANopenMaster_Index1A04[] = 
{
	{"TPDO5 Mapping", 0x1A0400FF, OD_OBJCODE_RECORD, OD_DATATYPE_PDOMAP, OD_ACCESS_RW, (void *)&can_master_transmit_PDO_5_mapping, 2, NULL, "SDO服务器参数" },
};
OD_OI_TypeDef  can_master_transmit_PDO_5_mapping[]=
{
  {"TPDO5 MappingCount", 0x1A040000, OD_OBJCODE_NULL, OD_DATATYPE_U8,  OD_ACCESS_RW,  (void *)&CANopenMaster_highestSubIndex_obj1A04, 0, NULL, "最高子索引数" },	
  {"TPDO5 Mapping1",  	 0x1A040001, OD_OBJCODE_NULL, OD_DATATYPE_U32, OD_ACCESS_RW,  (void *)&CANopenMaster_obj1A04[0], 0, NULL, "映射参数值为 200C0010" },
};


///////////////////////////////////////////////////////////////////////////
ODCallback_t CANopenMaster_Index2F12_callbacks[] = 
 {
   NULL,
 };
//OD_OI_TypeDef CANopenMaster_Index2F12[] = 
//{
//	{"PVT write pointer", 0x2F120000, OD_OBJCODE_VAR, OD_DATATYPE_I32, OD_ACCESS_RW, (void *)&pvt_o_pos_ref, 0, NULL, "PVT写指针" },
//};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
UNS16 err_code_from_servo = 0;
OD_OI_TypeDef CANopenMaster_Index2F02[] = 
{
	{"Error Code from servo", 0x2F020000, OD_OBJCODE_VAR, OD_DATATYPE_U16, OD_ACCESS_RW, (void *)&err_code_from_servo, 0, NULL, "对象1003h的低16位，现行故障代码" },
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
UNS16 servo_driver_controlword_display = 0;
OD_OI_TypeDef CANopenMaster_Index2F03[] = 
{
	{"controlword from servo driver", 0x2F030000, OD_OBJCODE_VAR, OD_DATATYPE_U16, OD_ACCESS_R, (void *)&servo_driver_controlword_display, 0, NULL, "伺服驱动器的控制字" },
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
UNS16 statusword_from_servo_driver = 0;
OD_OI_TypeDef CANopenMaster_Index2F04[] = 
{
	{"statusword from servo driver", 0x2F040000, OD_OBJCODE_VAR, OD_DATATYPE_U16, OD_ACCESS_RW, (void *)&statusword_from_servo_driver, 0, NULL, "伺服驱动器的状态字" },
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//OD_OI_TypeDef CANopenMaster_Index6064[] = 
//{
//	{"position actual value", 0x60640000, OD_OBJCODE_VAR, OD_DATATYPE_I32, OD_ACCESS_R, (void *)&gMC.i_pos_fdbk, 0, NULL, "实际位置" },
//};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//OD_OI_TypeDef CANopenMaster_Index6065[] = 
//{
//	{"following error", 0x60650000, OD_OBJCODE_VAR, OD_DATATYPE_U32, OD_ACCESS_RW, (void *)&gMC.following_error_window, 0, NULL, "跟踪误差" },
//};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern INTEGER16  torque_actual_value;
OD_OI_TypeDef CANopenMaster_Index6077[] = 
{
	{"torque actual value", 0x60770000, OD_OBJCODE_VAR, OD_DATATYPE_I16, OD_ACCESS_R, (void *)&torque_actual_value, 0, NULL, "电机实际扭矩 千分比" },
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern INTEGER32  current_actual_value;
OD_OI_TypeDef CANopenMaster_Index6078[] = 
{
	{"current actual value", 0x60780000, OD_OBJCODE_VAR, OD_DATATYPE_I32, OD_ACCESS_R, (void *)&current_actual_value, 0, NULL, "实际电流 mA" },
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern INTEGER32  velocity_actual_value;
OD_OI_TypeDef CANopenMaster_Index606C[] = 
{
	{"velocity actual value", 0x606C0000, OD_OBJCODE_VAR, OD_DATATYPE_I32, OD_ACCESS_R, (void *)&velocity_actual_value, 0, NULL, "实际速度" },
};




///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
OD_OI_TypeDef  CANopenMaster_objdict[]=
{
	{"Device Type",                         0x10000000, OD_OBJCODE_VAR,    OD_DATATYPE_U32,    OD_ACCESS_R,   (void *)&CANopenMaster_obj1000,                0, NULL, "16位的设备协议或应用协议 + 16位的逻辑设备的附加功能信息" },
	{"Error Register",                      0x10010000, OD_OBJCODE_VAR,    OD_DATATYPE_U8,     OD_ACCESS_R,   (void *)&CANopenMaster_obj1001,                0, NULL, "Canopen设备内部错误信息记录" },
	{"Pre-defined Error Field",             0x100300FF, OD_OBJCODE_ARRAY,  OD_DATATYPE_U32,    OD_ACCESS_RW,  (void *)&CAN_master_error_field,               9, NULL, "Canopen设备内部错误信息记录" },
	{"SYNC COB ID",                         0x10050000, OD_OBJCODE_VAR,    OD_DATATYPE_U32,    OD_ACCESS_RW,  (void *)&CANopenMaster_obj1005,                0, NULL, "已配置的同步对象的COB-ID" },
	{"Communication Cycle Period",          0x10060000, OD_OBJCODE_VAR,    OD_DATATYPE_U32,    OD_ACCESS_RW,  (void *)&CANopenMaster_obj1006,                0, NULL, "SYNC时间间隔，单位是us" },
	{"GuardTime",                           0x100C0000, OD_OBJCODE_VAR,    OD_DATATYPE_U16,    OD_ACCESS_RW,  (void *)&CANopenMaster_obj100C,                0, NULL, "监护周期，单位为ms" },
	{"LifeTimeFactor",                      0x100D0000, OD_OBJCODE_VAR,    OD_DATATYPE_U8,     OD_ACCESS_RW,  (void *)&CANopenMaster_obj100D,                0, NULL, "生存周期因子" },
//	{"Store parameters", 					0x101000FF, OD_OBJCODE_ARRAY, 	OD_DATATYPE_U32, 	OD_ACCESS_RW, 	(void *)&Store_parameters, 					6, NULL, "此对象控制保存参数到非易失性存储器中" },
//	{"Restore Default Parameters", 			0x101100FF, OD_OBJCODE_ARRAY, 	OD_DATATYPE_U32, 	OD_ACCESS_RW, 	(void *)&Restore_Default_Parameters, 		6, NULL, "此对象控制保存参数到非易失性存储器中" },
	{"Emergency COB ID",                    0x10140000, OD_OBJCODE_VAR,    OD_DATATYPE_U32,    OD_ACCESS_RW,  (void *)&CANopenMaster_obj1014,                0, NULL, "应急消息 COB_ID" },
	{"Consumer Heartbeat Time",             0x101600FF, OD_OBJCODE_ARRAY,  OD_DATATYPE_U32,    OD_ACCESS_RW,  (void *)&master_consumer_heartbeat_time,       5, NULL, "消费者心跳超时" },
	{"Producer Heartbeat Time",             0x10170000, OD_OBJCODE_VAR,    OD_DATATYPE_U16,    OD_ACCESS_RW,  (void *)&CANopenMaster_obj1017,                0, NULL, "生产者心跳超时" },
	{"Identity",                            0x101800FF, OD_OBJCODE_RECORD, OD_DATATYPE_ID,     OD_ACCESS_R,   (void *)&CANopenMaster_Identity,               5, NULL, "对象身份" },
	{"SSDO Par",                            0x120000FF, OD_OBJCODE_RECORD, OD_DATATYPE_SDOP,   OD_ACCESS_R,   (void *)&CAN_master_server_SDO_parameter,      4, NULL, "SDO服务器参数" },	
	{"CSDO Par",                            0x128000FF, OD_OBJCODE_RECORD, OD_DATATYPE_SDOP,   OD_ACCESS_R,   (void *)&CAN_master_client_SDO_parameter,      4, NULL, "SDO客户端参数" },
	
	{"RPDO1 Com",                           0x140000FF, OD_OBJCODE_RECORD, OD_DATATYPE_PDOCP,  OD_ACCESS_RW,  (void *)&can_master_receive_PDO_1_parameter,   6, NULL, "RPDO1通信参数" },
	{"RPDO2 Com",                           0x140100FF, OD_OBJCODE_RECORD, OD_DATATYPE_PDOCP,  OD_ACCESS_RW,  (void *)&can_master_receive_PDO_2_parameter,   6, NULL, "RPDO2通信参数" },
	{"RPDO3 Com",                           0x140200FF, OD_OBJCODE_RECORD, OD_DATATYPE_PDOCP,  OD_ACCESS_RW,  (void *)&can_master_receive_PDO_3_parameter,   6, NULL, "RPDO3通信参数" },
//	{"RPDO4 Com",                           0x140301FF, OD_OBJCODE_RECORD, OD_DATATYPE_PDOCP,  OD_ACCESS_RW,  (void *)&can_master_receive_PDO_4_parameter,   6, NULL, "RPDO4通信参数" },
//	{"RPDO5 Com",                           0x140401FF, OD_OBJCODE_RECORD, OD_DATATYPE_PDOCP,  OD_ACCESS_RW,  (void *)&can_master_receive_PDO_5_parameter,   6, NULL, "RPDO5通信参数" },
	
	{"RPDO1 Mapping", 						0x160000FF, OD_OBJCODE_RECORD, OD_DATATYPE_PDOMAP, OD_ACCESS_RW,  (void *)&can_master_receive_PDO_1_mapping,     3, NULL, "RPDO1映射参数" },
	{"RPDO2 Mapping",                       0x160100FF, OD_OBJCODE_RECORD, OD_DATATYPE_PDOMAP, OD_ACCESS_RW,  (void *)&can_master_receive_PDO_2_mapping,     3, NULL, "RPDO2映射参数" },
	{"RPDO3 Mapping",                       0x160200FF, OD_OBJCODE_RECORD, OD_DATATYPE_PDOMAP,  OD_ACCESS_RW,  (void *)&can_master_receive_PDO_3_mapping,     3, NULL, "RPDO3映射参数" },	
//	{"RPDO4 Mapping",                       0x160300FF, OD_OBJCODE_RECORD, OD_DATATYPE_PDOCP,  OD_ACCESS_RW,  (void *)&can_master_receive_PDO_4_mapping,     2, NULL, "RPDO4映射参数" },	
//	{"RPDO5 Mapping",                       0x160400FF, OD_OBJCODE_RECORD, OD_DATATYPE_PDOCP,  OD_ACCESS_RW,  (void *)&can_master_receive_PDO_5_mapping,     2, NULL, "RPDO5映射参数" },
	
	{"TPDO1 Com",                           0x180000FF, OD_OBJCODE_RECORD, OD_DATATYPE_PDOCP,  OD_ACCESS_RW,  (void *)&can_master_transmit_PDO_1_parameter,  5, NULL, "TPDO1通信参数" },	
//	{"TPDO2 Com", 							0x180100FF, OD_OBJCODE_RECORD, OD_DATATYPE_PDOCP,  OD_ACCESS_RW,  (void *)&can_master_transmit_PDO_2_parameter,  5, NULL, "TPDO2通信参数" },
//	{"TPDO3 Com", 							0x180201FF, OD_OBJCODE_RECORD, OD_DATATYPE_PDOCP,  OD_ACCESS_RW,  (void *)&can_master_transmit_PDO_3_parameter,  5, NULL, "TPDO3通信参数" },
//	{"TPDO4 Com", 							0x180301FF, OD_OBJCODE_RECORD, OD_DATATYPE_PDOCP,  OD_ACCESS_RW,  (void *)&can_master_transmit_PDO_4_parameter,  5, NULL, "TPDO4通信参数" },
//	{"TPDO5 Com", 							0x180401FF, OD_OBJCODE_RECORD, OD_DATATYPE_PDOCP,  OD_ACCESS_RW,  (void *)&can_master_transmit_PDO_5_parameter,  5, NULL, "TPDO5通信参数" },
	
	{"TPDO1 Mapping", 						0x1A0000FF, OD_OBJCODE_RECORD, OD_DATATYPE_PDOMAP, OD_ACCESS_RW,  (void *)&can_master_transmit_PDO_1_mapping,    8, NULL, "TPDO1映射参数" },
//	{"TPDO2 Mapping", 						0x1A0100FF, OD_OBJCODE_RECORD, OD_DATATYPE_PDOMAP, OD_ACCESS_RW,  (void *)&can_master_transmit_PDO_2_mapping,    8, NULL, "TPDO2映射参数" },
//	{"TPDO3 Mapping", 						0x1A0200FF, OD_OBJCODE_RECORD, OD_DATATYPE_PDOCP,  OD_ACCESS_RW,  (void *)&can_master_transmit_PDO_3_mapping,    2, NULL, "TPDO3映射参数" },
//	{"TPDO4 Mapping", 						0x1A0300FF, OD_OBJCODE_RECORD, OD_DATATYPE_PDOCP,  OD_ACCESS_RW,  (void *)&can_master_transmit_PDO_4_mapping,    2, NULL, "TPDO4映射参数" },
//	{"TPDO5 Mapping", 						0x1A0400FF, OD_OBJCODE_RECORD, OD_DATATYPE_PDOCP,  OD_ACCESS_RW,  (void *)&can_master_transmit_PDO_5_mapping,    2, NULL, "TPDO5映射参数" },
	{"empty obj for test",                  0x2F010000, OD_OBJCODE_VAR,    OD_DATATYPE_I32,    OD_ACCESS_R,    (void *)&empty_obj_2F01,                      0, NULL, "" },
    {"Error Code from servo",               0x2F020000, OD_OBJCODE_VAR,    OD_DATATYPE_U16,    OD_ACCESS_RW,    (void *)&err_code_from_servo,                 0, NULL, "对象1003h的低16位，现行故障代码" },	
	{"controlword from servo driver",       0x2F030000, OD_OBJCODE_VAR,    OD_DATATYPE_U16,    OD_ACCESS_R,    (void *)&servo_driver_controlword_display,       0, NULL, "伺服驱动器的控制字" },
    {"statusword from servo driver",        0x2F040000, OD_OBJCODE_VAR,    OD_DATATYPE_U16,    OD_ACCESS_RW,    (void *)&statusword_from_servo_driver,        0, NULL, "伺服驱动器的状态字" },
//	{"PVT demand position",                 0x2F120000, OD_OBJCODE_VAR,    OD_DATATYPE_I32,    OD_ACCESS_RW,   (void *)&pvt_o_pos_ref,                       0, NULL, "PVT插补点" },
//	
//    {"position actual value",               0x60640000, OD_OBJCODE_VAR,    OD_DATATYPE_I32,    OD_ACCESS_R,    (void *)&gMC.i_pos_fdbk,                      0, NULL, "电机实际位置" },
//	{"following error window",              0x60650000, OD_OBJCODE_VAR,    OD_DATATYPE_U32,    OD_ACCESS_RW,   (void *)&gMC.following_error_window,          0, NULL, "跟踪误差窗口值" },	
	{"velocity actual value",               0x606C0000, OD_OBJCODE_VAR,    OD_DATATYPE_I32,    OD_ACCESS_R,    (void *)&velocity_actual_value,               0, NULL, "实际速度" },	
	{"torque actual value",                 0x60770000, OD_OBJCODE_VAR,    OD_DATATYPE_I16,    OD_ACCESS_R,    (void *)&torque_actual_value,                 0, NULL, "电机实际扭矩 千分比" },	
	{"current actual value",                0x60780000, OD_OBJCODE_VAR,    OD_DATATYPE_I32,    OD_ACCESS_R,    (void *)&current_actual_value,                0, NULL, "实际电流 mA" },
};


s_PDO_status CANopenMaster_PDO_status[5] = {s_PDO_status_Initializer,s_PDO_status_Initializer,s_PDO_status_Initializer,s_PDO_status_Initializer,s_PDO_status_Initializer};

UNS16 CANopenMaster_ObjdictSize = sizeof(CANopenMaster_objdict)/sizeof(CANopenMaster_objdict[0]);

//CO_Data CANopenMaster_Data = CANOPEN_NODE_DATA_INITIALIZER( CANopenMaster, CAN_NUM2, CAN_PORT2, TIM4 );