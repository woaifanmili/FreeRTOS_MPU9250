
#include "stdio.h"
#include <string.h>
#include "ObjectDictionary.h"
#include "TestSlave.h"


int xin=0;
//int totalobjnum=0;
const char * t;
UNS8 flaxin=0;
OD_OI_TypeDef NULL_OBJ={0};



//OD_TypeDef gOD = {0};


OD_OI_TypeDef * find_object(  char *name , UNS32 index ,  UNS32 * errorCode, CO_Data* d)
{


	int low, high, mid;
	low = 0;
	high = d->gOD.obj_num-1;

	//二分查找法

	
	//通过名字查找
	if( index == 0)/* 相当于没给index就通过name查找 */
	{
		while(low <= high)
		{
			mid = (low + high) / 2;
			if(strcmp(name, d->gOD.p_obj_name_array[mid]->p_name) < 0)
				high = mid - 1;
			else if(strcmp(name, d->gOD.p_obj_name_array[mid]->p_name) > 0)
				low = mid + 1;
			else
			{
				*errorCode = OD_SUCCESSFUL;
				return d->gOD.p_obj_name_array[mid];
			}
		}
		*errorCode = OD_NO_SUCH_OBJECT;
		return  &NULL_OBJ; 
	
	}
	else if(index)//通过index查找
	{
		while(low <= high)
		{
			mid=(low + high) / 2;
			if(index < d->gOD.p_obj_index_array[mid]->index)
				high = mid - 1;
			else if(index > d->gOD.p_obj_index_array[mid]->index)
				low = mid + 1;
			else
			{
				*errorCode = OD_SUCCESSFUL;
				return d->gOD.p_obj_index_array[mid];
			}
		}
		*errorCode = OD_NO_SUCH_OBJECT;
		return  &NULL_OBJ; 
	}
}




/*************************************************
* @Function   : OD_Object_Reg  对象接口注册函数，注册对象接口
*
* @Description: 1：往一个字典内注册对象接口ObjectInterface
*               2：注册本质是把对象接口的地址指针保存
*               3：注册中会对OI进行排序
*
* @Calls      : sort_by_name(OD_OI_TypeDef * begin , OD_OI_TypeDef * end) 根据名字排序
                sort_by_index(OD_OI_TypeDef * begin , OD_OI_TypeDef * end) 根据index排序

* @Called By  : 被应用程序调用
* @Input      : that  ：字典服务对象结构体指针
*               p_obj :待注册的静态对象结构体
*               obj_num;
* @Output     : 无
* @Return     : 无
* @Example    :
*************************************************/
/* 将整个p_obj_list指向的树状结构的对象读到p_obj_name_array和p_obj_name_index中 */
/*				   01                    */
/*	  	   02              09            */
/*	   03     06       10      13        */
/*   04  05 07  08   11  12  14  15      */

static int OI_Recursion_Add( OD_OI_TypeDef * p_obj_list  ,  UNS32 obj_num, CO_Data* d)
{
	UNS32 i;

	OD_OI_TypeDef *p_obj;

//	if( gOD.obj_num + obj_num > OD_MAX_OBJ_NUM )totalobjnum
	if( d->gOD.obj_num + obj_num > d->totalobjnum )
	{
		return 0;
		//ERR_PROCESS();
	}

	//哈希表初始化
	for(i=0; i<obj_num; i++ )
	{

		p_obj =  p_obj_list+i;

		d->gOD.p_obj_name_array [ d->gOD.obj_num ] =  p_obj;  //赋值

		d->gOD.p_obj_index_array[ d->gOD.obj_num ] =  p_obj; //赋值

		d->gOD.obj_num ++; //对象总数量累加
		

		if( ( p_obj->objcode ==  OD_OBJCODE_DEFSTRUCT ) || (p_obj->objcode ==  OD_OBJCODE_ARRAY) || (p_obj->objcode ==  OD_OBJCODE_RECORD) )/*  */
		{
			if( 0== OI_Recursion_Add(p_obj->p_content, p_obj->size, d))
				return 0;
		}

	}
	return 1;
}


void swap(OD_OI_TypeDef ** L, int i, int j)
{
	OD_OI_TypeDef* temp=L[i];
	L[i]=L[j];
	L[j]=temp;
}
int index_Partition(OD_OI_TypeDef ** L, int low, int high)
{
	UNS32 pivotkey;
	pivotkey=L[low]->index;/* 取数组的第一个元素作为 pivotkey */
	while(low<high)
	{
		while(low<high&&L[high]->index>=pivotkey)
			high--;/* 从右往左扫，找出比 pivotkey 小的，将其和 pivotkey 进行交换 */
		swap(L,low,high);
		while(low<high&&L[low]->index<=pivotkey)
			low++; /* 再从左往右扫，找出比 pivotkey 大的，将其和 pivotkey 进行交换 */ 
		swap(L,low,high);
		
	}
	return low;
}
void sort_by_index(OD_OI_TypeDef ** L, int low, int high)
{
int pivot;
	if(low<high)
	{
		pivot = index_Partition(L,low,high);
		
		sort_by_index(L,low,pivot-1);
		sort_by_index(L,pivot+1,high);
	}
}




int name_Partition(OD_OI_TypeDef ** L, int low, int high)
{
	char* pivotkey;
	pivotkey=L[low]->p_name;
	while(low<high)
	{
		while(low<high&&strcmp(L[high]->p_name,pivotkey)>=0)
//		while(low<high&&Str_Cmp(L[high]->p_name,pivotkey)>=0)
			high--;
//	  flaxin = strcmp(L[high]->p_name,pivotkey);
		swap(L,low,high);
		while(low<high&&strcmp(L[low]->p_name,pivotkey)<=0)
//		while(low<high&&Str_Cmp(L[low]->p_name,pivotkey)<=0)
			low++;
		swap(L,low,high);
	}
	return low;
}
void sort_by_name(OD_OI_TypeDef ** L, int low, int high)
{
	int pivot;
	if(low<high)
	{
		pivot = name_Partition(L,low,high);
		
		sort_by_name(L,low,pivot-1);
		sort_by_name(L,pivot+1,high);
	}
}


void OD_OI_Reg(OD_OI_TypeDef * p_obj_list , UNS32 obj_num, CO_Data* d)
{
	OD_OI_TypeDef * p_obj = NULL;
	p_obj = p_obj_list;
	for(xin=0; xin < obj_num; xin++)
	{
		//d->totalobjnum += TestSlave_objdict[xin].size;		// 统计所有的子索引总数
		d->totalobjnum += (p_obj++)->size;;		// 统计所有的子索引总数
	}
	
	d->totalobjnum = obj_num + d->totalobjnum;					// 加上主索引总数
	
	OI_Recursion_Add(p_obj_list , obj_num, d);  //递归增加
	
	//排序
	sort_by_name( d->gOD.p_obj_name_array, 0, d->gOD.obj_num-1   );	

	sort_by_index( d->gOD.p_obj_index_array, 0, d->gOD.obj_num-1   );
}


int data_length(UNS8 datatype)
{
	int length = 0;
	switch (datatype)
	{
		case OD_DATATYPE_U8:  	  length = sizeof(UNS8); break;
		case OD_DATATYPE_U16: 	  length = sizeof(UNS16); break;
		case OD_DATATYPE_U32: 	  length = sizeof(UNS32); break;
		case OD_DATATYPE_I8:  	  length = sizeof(INTEGER8); break;
		case OD_DATATYPE_I16: 	  length = sizeof(INTEGER16); break;
		case OD_DATATYPE_I32: 	  length = sizeof(INTEGER32); break;
		case OD_DATATYPE_F32: 	  length = sizeof(REAL32); break;
		case OD_DATATYPE_BOOLEAN: length = sizeof(UNS8); break;
		case TIME: 			      length = Timelength; break;
		case COST: 				  length = Costlength; break;
        case PVT_DATA:length=12;break;
		default:length = 0;break;
	}
	return length;
}




