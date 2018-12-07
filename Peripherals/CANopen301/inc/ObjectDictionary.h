/**
************************************************
* @Copyright (C),
* @File       : OD_Dictionary.h
* @Author     : 周帆
* @Version    : 
* @Date       : 
* @Description: 
*    1：如果通过如果是NULL,则直接返回源数据类型，如果

* @Others     : 其它内容的说明
* @Function List: 主要函数列表，每条记录应包括函数名及功能简要说明
*      
***********************************************/

#ifndef _OBJECT_DICTIONARY_H
#define _OBJECT_DICTIONARY_H

#include "OD_Config.h"
#include <stdint.h>
#include "applicfg.h"
#include "states.h"


typedef enum
{
 OD_SUCCESSFUL 	             = 0x00000000,
 OD_READ_NOT_ALLOWED         = 0x06010001,
 OD_WRITE_NOT_ALLOWED        = 0x06010002,
 OD_NO_SUCH_OBJECT           = 0x06020000,
 OD_NOT_MAPPABLE             = 0x06040041,
 OD_LENGTH_DATA_INVALID      = 0x06070010,
 OD_NO_SUCH_SUBINDEX 	     = 0x06090011,
 OD_VALUE_RANGE_EXCEEDED     = 0x06090030, /* Value range test result */
 OD_VALUE_TOO_LOW            = 0x06090031, /* Value range test result */
 OD_VALUE_TOO_HIGH           = 0x06090032,  /* Value range test result */
	
 SDOABT_TOGGLE_NOT_ALTERNED  = 0x05030000,
 SDOABT_TIMED_OUT            = 0x05040000,
 SDOABT_OUT_OF_MEMORY        = 0x05040005,
 SDOABT_GENERAL_ERROR        = 0x08000000,
 SDOABT_LOCAL_CTRL_ERROR     = 0x08000021,
 MEMCPY_ERROR                = 0x08000033,
	
}SDO_Abort_of_OD_ENUM;


/*******************************************
*
*   对象代码Object Code 枚举
*
*******************************************/
typedef enum
{
   OD_OBJCODE_NULL      = 0x00,    
   OD_OBJCODE_DOMAIN    = 0x02,
   OD_OBJCODE_DEFTYPE   = 0x05,
   OD_OBJCODE_DEFSTRUCT = 0x06,
   OD_OBJCODE_VAR       = 0x07,
   OD_OBJCODE_ARRAY     = 0x08,
   OD_OBJCODE_RECORD    = 0x09  

}OD_OBJCODE_ENUM;

/*******************************************
*
*   数据类型Data Type 枚举
*
*******************************************/
typedef enum
{
	OD_DATATYPE_VOID 			= 0x00, //自由
	OD_DATATYPE_BOOLEAN 		= 0x01,
	OD_DATATYPE_I8 				= 0x02,
	OD_DATATYPE_I16 			= 0x03,
	OD_DATATYPE_I32 			= 0x04,
	OD_DATATYPE_U8 				= 0x05, 
	OD_DATATYPE_U16 			= 0x06,
	OD_DATATYPE_U32 			= 0x07,
	OD_DATATYPE_F32 			= 0x08, //浮点32
	OD_DATATYPE_VS 				= 0x09, //visible string
	OD_DATATYPE_Octet_String    = 0x0A,	
	OD_DATATYPE_Unicode_String  = 0x0B,
	OD_DATATYPE_Time_of_Day     = 0x0C,		
	OD_DATATYPE_Time_Difference = 0x0D,	

	OD_DATATYPE_Domain 			= 0x0F,
	OD_DATATYPE_Int24 			= 0x10,
	OD_DATATYPE_Real64 			= 0x11,
	OD_DATATYPE_Int40 			= 0x12,
	OD_DATATYPE_Int48 			= 0x13,
	OD_DATATYPE_Int56 			= 0x14,
	OD_DATATYPE_Int64 			= 0x15,
	OD_DATATYPE_Uint24 			= 0x16,
	
	OD_DATATYPE_Uint40 			= 0x18,
	OD_DATATYPE_Uint48 			= 0x19,
	OD_DATATYPE_Uint56 			= 0x1A,
	OD_DATATYPE_Uint64 			= 0x1B,	
	
	
	OD_DATATYPE_PDOCP 			= 0x20, // PDO CommPar
	OD_DATATYPE_PDOMAP 			= 0x21, //PDO maping
	OD_DATATYPE_SDOP 			= 0x22, //SDO Parameter
	OD_DATATYPE_ID 				= 0x23,
	
	OD_DATATYPE_PVTDP 			= 0x40, //PVT DataPar
	OD_DATATYPE_PTD 			= 0x41, //PT DataPar
	OD_DATATYPE_BIQ 			= 0x42, //二进制编译器查询对象
	OD_DATATYPE_BIC 			= 0x43, //二进制命令
	OD_DATATYPE_PVDR 			= 0x44, //DSP402 PV data record
	OD_DATATYPE_IDCR 			= 0x45, //DSP402 interpolated data configuration record
	OD_DATATYPE_CMD 			= 0x46, //命令
	OD_DATATYPE_FUN 			= 0x47, //函数地址，其尺寸为0
	TIME=0x48,
	COST=0x49,
    PVT_DATA=50,
}OD_DATATYPE_ENUM;




/******************************************
*
*   访问属性Access枚举
*
******************************************/
typedef enum
{
    OD_ACCESS_NULL=0,
    OD_ACCESS_R =1,  //Read
    OD_ACCESS_W =2,  //Write
    OD_ACCESS_C =4,  //save

    OD_ACCESS_RW = ( OD_ACCESS_R | OD_ACCESS_W ),
    OD_ACCESS_RC = ( OD_ACCESS_R | OD_ACCESS_C ),

}OD_ACCESS_ENUM;

/*****************************************
*
*   实时策略Category枚举
*
******************************************/
typedef enum
{
    OD_STORE_NULL,
    OD_STORE_RAM,
    OD_STORE_FLASH,
    OD_STORE_EEPROM

}OD_STORE_ENUM;

/*****************************************
*
*   响应代码枚举
*
******************************************/
typedef enum
{
    OD_RESPON_NULL,
	OD_RESPON_NF,   //没找到对象
    OD_RESPON_R_OK,
	OD_RESPON_R_FAIL,
    OD_RESPON_W_OK,
	OD_RESPON_W_FAIL,
	OD_RESPON_E_OK,
	OD_RESPON_E_FAIL,
    OD_RESPON_ERR

}OD_RESPON_ENUM;

typedef struct OD_OI_TypeDef OD_OI_typeDef;
typedef UNS32 (*ODCallback_t)(CO_Data* d, const OD_OI_typeDef *a, UNS8 bSubindex);
typedef const OD_OI_typeDef * (*scanIndexOD_t)(UNS16 wIndex, UNS32 * errorCode, ODCallback_t **Callback);

/******************
*
*  本地对象object_Interface定义,
*  每一个可以被对象字典所访问的对象，都需要按照以下接口暴露和注册
*
*******************/
typedef struct
{
    char* p_name;    //无论是否是子对象，全局不可重名
	  UNS32 index; //在canopen，无论是否是子对象，该index全局不可重复
    OD_OBJCODE_ENUM objcode; //object code
    OD_DATATYPE_ENUM data_type;   //
    OD_ACCESS_ENUM access;   //数据读写属性
	  //OD_STORE_ENUM  store;
    void *p_content;         //变量实际地址
	  UNS32 size;      //子索引（对象）数目或者变量尺寸
	  ODCallback_t callback;
    char *  p_description;
}OD_OI_TypeDef;





/* 临时缓存 */
typedef struct
{
	UNS8        *  origin;  //起始地址
	UNS32          size;    //缓冲区大小
	OD_DATATYPE_ENUM  type;
	UNS32          used;    //已使用
	OD_ACCESS_ENUM    access;  //访问

} OD_BUF_POINT_TypeDef;

/******************
 *
 * 对象字典的定义
 *
*******************/
typedef struct
{
	//同步输入变量，调用 OD_Input 函数写入数据到以下六个 i_xxxx 变量中，如果外界是要请求数据则不用给最后两个数据，写如数据的话那肯定需要最后两个数据，否则都不知道要写什么数据啊
	OD_ACCESS_ENUM    i_access[ OD_CHN_NUM ]; //表示没有动作=0，读=1，写=2
	char              i_name[ OD_CHN_NUM ][ OD_MAX_OBJ_NAME_NUM ];// 输入对象名字，如果为空，则通过ObjectIndex查询
	UNS32          i_index[ OD_CHN_NUM ];
	OD_DATATYPE_ENUM  i_data_type[ OD_CHN_NUM ]; //如果access是写动作，则data_type等于写的数据类型，如果access是读，则data_type代表要读会的数据类型
	UNS32          i_data_num[ OD_CHN_NUM ]; //输入数据大小
	UNS8           i_data_flow[ OD_CHN_NUM ][ OD_DATA_FLOW_MAX_LEN ] ; //:输入数据流

	//同步输出变量，调用 OD_Output 函数将以下四个 o_xxxx 变量值反馈给 OD_Output 函数的参数
	OD_RESPON_ENUM    o_respon[ OD_CHN_NUM ]; //动作
	OD_DATATYPE_ENUM  o_data_type[ OD_CHN_NUM ] ;  //输出数据类型
	UNS32          o_data_num[ OD_CHN_NUM];   //输出数据数量
	UNS8           o_data_flow[ OD_CHN_NUM ][OD_DATA_FLOW_MAX_LEN]; // 输出数据流

	//内部变量，临时缓存
	OD_BUF_POINT_TypeDef   buf_point[ OD_CHN_NUM ];/* 由 OD_Period 函数更新 */
	UNS16          obj_num;     //字典总数量

	OD_OI_TypeDef *  p_obj_index_array [ OD_MAX_OBJ_NUM ]    ; /* OD_OI_Reg 函数 先借助 OI_Recursion_Add 将对象加载到这里，然后对这里进行排序 *///指向对象的指针数组，一部对象字典也是一个obj
	OD_OI_TypeDef *  p_obj_name_array  [ OD_MAX_OBJ_NUM ]    ;

} OD_TypeDef;



extern OD_OI_TypeDef NULL_OBJ;

//void OD_Page_Format( uint8_t page_num , uint8_t page_type , uint8_t page_size );

//p_obj_list对象列表，顶级对象会被排序
void OD_OI_Reg(OD_OI_TypeDef * p_obj_list , UNS32 obj_num, CO_Data* d);

//uint32_t OD_Get_Object_Num(void);

//OD_OI_TypeDef * * OD_Get_ObjectList_By_NameOrder( void );


OD_OI_TypeDef * find_object(  char *name , UNS32 index ,  UNS32 * errorCode, CO_Data* d);


int data_length(UNS8 datatype);


#endif // OD_H

