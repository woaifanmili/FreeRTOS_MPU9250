/*
  This file is part of CanFestival, a library implementing CanOpen
  Stack.

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
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
  USA
*/
/*!
** @file   objacces.c
** @author Edouard TISSERANT and Francis DUPIN
** @date   Tue Jun  5 08:55:23 2007
**
** @brief
**
**
*/


/* #define DEBUG_WAR_CONSOLE_ON */
/* #define DEBUG_ERR_CONSOLE_ON */


#include "data.h"
#include "ObjectDictionary.h"
#define getMainIndex(Index) ((Index >> 16) & 0x0000FFFF)
#define getSubIndex(Index) (Index & 0x000000FF)


//We need the function implementation for linking
//Only a placeholder with a define isnt enough!
/* 字典错误入口 */
/* code为错误代码，sizeDataDict为应该给的数据长度，sizeDataGiven为当前所给的数据长度 */
UNS8 accessDictionaryError(UNS16 index, UNS8 subIndex,
                           UNS32 sizeDataDict, UNS32 sizeDataGiven, UNS32 code)
{
#ifdef DEBUG_WAR_CONSOLE_ON
  MSG_WAR(0x2B09,"Dictionary index : ", index);
  MSG_WAR(0X2B10,"           subindex : ", subIndex);
  switch (code) {
  /* 找不到该索引 */
  case  OD_NO_SUCH_OBJECT:
    MSG_WAR(0x2B11,"Index not found ", index);
    break;
  /* 找不到该子索引 */
  case OD_NO_SUCH_SUBINDEX :
    MSG_WAR(0x2B12,"SubIndex not found ", subIndex);
    break;
  /* 该数据为只读，不允许写操作 */
  case OD_WRITE_NOT_ALLOWED :
    MSG_WAR(0x2B13,"Write not allowed, data is read only ", index);
    break;
  /* 越界访问 */
  case OD_LENGTH_DATA_INVALID :
    MSG_WAR(0x2B14,"Conflict size data. Should be (bytes)  : ", sizeDataDict);
    MSG_WAR(0x2B15,"But you have given the size  : ", sizeDataGiven);
    break;
  /* 这个索引没有PDO映射值 */
  case OD_NOT_MAPPABLE :
    MSG_WAR(0x2B16,"Not mappable data in a PDO at index    : ", index);
    break;
  /* SDO数值太小 */
  case OD_VALUE_TOO_LOW :
    MSG_WAR(0x2B17,"Value range error : value too low. SDOabort : ", code);
    break;
  /* SDO数值太大 */
  case OD_VALUE_TOO_HIGH :
    MSG_WAR(0x2B18,"Value range error : value too high. SDOabort : ", code);
    break;
  /* 未知错误 */
  default :
    MSG_WAR(0x2B20, "Unknown error code : ", code);
  }
  #endif

  return 0;
}
/* 获取对象中的数据 */
UNS32 _getODentry( CO_Data* d,
                   UNS16 wIndex,
                   UNS8 bSubindex,
                   void * pDestData,
                   UNS32 * pExpectedSize,
                   UNS8 * pDataType,
                   UNS8 checkAccess,
                   UNS8 endianize)
{ /* DO NOT USE MSG_ERR because the macro may send a PDO -> infinite
    loop if it fails. */

  UNS32 Index = (wIndex<<16) | bSubindex;
	
  UNS32 errorCode;
  UNS32 szData;
  const OD_OI_TypeDef *ptrTable;
//  ODCallback_t *Callback;
  /* 扫描字典 */
//  ptrTable = (*d->scanIndexOD)(wIndex, &errorCode, &Callback);
	ptrTable = d->Find_Object(NULL, Index, &errorCode, d);
  /* 找不到该索引 */
  if (errorCode != OD_SUCCESSFUL)
    return errorCode;
  /* 找不到该子索引 */
//  if( ptrTable->bSubCount <= bSubindex ) {
//    if( ptrTable->size <= bSubindex ) {
//    /* Subindex not found */
//    accessDictionaryError(wIndex, bSubindex, 0, 0, OD_NO_SUCH_SUBINDEX);
//    return OD_NO_SUCH_SUBINDEX;
//  }
  /* 该对象属性为只写即不允许读取数据 */
//  if (checkAccess && (ptrTable->pSubindex[bSubindex].bAccessType & WO)) {
    if (checkAccess && (ptrTable->access == OD_ACCESS_W)) {
    MSG_WAR(0x2B30, "Access Type : ", ptrTable->access);
    accessDictionaryError(wIndex, bSubindex, 0, 0, OD_READ_NOT_ALLOWED);
    return OD_READ_NOT_ALLOWED;
  }
  /* 缓冲区不能为NULL */
  if (pDestData == 0) {
    return SDOABT_GENERAL_ERROR;
  }
  /* 对象数据比缓冲区要大，越界 */
//  if (ptrTable->pSubindex[bSubindex].size > *pExpectedSize) {
    if (data_length(ptrTable->data_type) > *pExpectedSize) {
    /* Requested variable is too large to fit into a transfer line, inform    *
     * the caller about the real size of the requested variable.              */
//    *pExpectedSize = ptrTable->pSubindex[bSubindex].size;
	  *pExpectedSize = data_length(ptrTable->data_type);
    return SDOABT_OUT_OF_MEMORY;
  }
  /* 获取对象数据类型 */
//  *pDataType = ptrTable->pSubindex[bSubindex].bDataType;
    *pDataType = ptrTable->data_type;
  /* 对象数据大小 */
//  szData = ptrTable->pSubindex[bSubindex].size;
    szData = data_length(ptrTable->data_type);
#  ifdef CANOPEN_BIG_ENDIAN
  if(endianize && *pDataType > boolean && !(
         *pDataType >= visible_string &&
         *pDataType <= domain)) {
    /* data must be transmited with low byte first */
    UNS8 i, j = 0;
    MSG_WAR(boolean, "data type ", *pDataType);
    MSG_WAR(visible_string, "data type ", *pDataType);
    for ( i = szData ; i > 0 ; i--) {
      MSG_WAR(i," ", j);
      ((UNS8*)pDestData)[j++] =
        ((UNS8*)ptrTable->pSubindex[bSubindex].pObject)[i-1];
    }
    *pExpectedSize = szData;
  }
  else /* no endianisation change */
#  endif
  /* 如果对象数据类型不是visible_string字符串 */
  if(*pDataType != OD_DATATYPE_VS)
	{
	  /* 拷贝数据 */
//      memcpy(pDestData, ptrTable->pSubindex[bSubindex].pObject,szData);
		memcpy(pDestData, ptrTable->p_content,szData);
	  /* 获取对象数据大小 */
      *pExpectedSize = szData;
    }
	  /* 如果是visible_string字符串的话 */
  else
	{
      /* TODO : CONFORM TO DS-301 :
       *  - stop using NULL terminated strings
       *  - store string size in td_subindex
       * */
      /* Copy null terminated string to user, and return discovered size */
	  /* 字符串起始位置 */
//      UNS8 *ptr = (UNS8*)ptrTable->pSubindex[bSubindex].pObject;
		UNS8 *ptr = (UNS8*)ptrTable->p_content;
		
      UNS8 *ptr_start = ptr;
      /* *pExpectedSize IS < szData . if null, use szData */
	  /* 字符串结束位置 */
      UNS8 *ptr_end = ptr + (*pExpectedSize ? *pExpectedSize : szData) ;/* 疑问：到这里pExpectedSize一定是一个大于等于szData的值把，那这里的判断就不会有第二种情况了，为什么要做这么一个结果已经是确定了的判断 */
      UNS8 *ptr_dest = (UNS8*)pDestData;
      while( *ptr && ptr < ptr_end ){/* flaxin 当拷贝时遇到了0或是到了预定的该拷贝的数据长度则会中止拷贝 */
          *(ptr_dest++) = *(ptr++);/* 不知道当数据类型是字符串的时候szdata会不会将最后的那个0结束符给算进去，如果是这样的话，就好理解了 */
      }/* 因为while那里用的是 < 号，所以当pExpectedSize和szdata相等时（除了相等就只有大于这种情况了，不会小于的），拷贝的实际数据长度会比pExpectedSize小1 */
    /* 字符串长度 */
    *pExpectedSize = (UNS32) (ptr - ptr_start);
    /* terminate string if not maximum length */
	/* 结尾加'\0' */
    if (*pExpectedSize < szData)/* Flaxin 如果szdata将最后的那个结束符0也算进去了， 这里一定会满足条件，因为不管pExpectedSize设的大还是等于szdata*/
            *(ptr) = 0;/* 实际拷贝的数据大小都会小于szData，小1，所以这里一定会满足条件 */
    }
  return OD_SUCCESSFUL;
}
/* 设置对象中的数据 */
UNS32 _setODentry( CO_Data* d,
                   UNS16 wIndex,
                   UNS8 bSubindex,
                   void * pSourceData,
                   UNS32 * pExpectedSize,
                   UNS8 checkAccess,
                   UNS8 endianize)
{
  UNS32 Index = (wIndex<<16) | bSubindex;
	
  UNS32 szData;
  UNS8 dataType;
  UNS32 errorCode;
  const OD_OI_TypeDef *ptrTable;
  ODCallback_t *Callback;
  /* 扫描字典 */
//  ptrTable =(*d->scanIndexOD)(wIndex, &errorCode, &Callback);
//	ptrTable = d->Find_Object(NULL, Index|0xFF, &errorCode);
   ptrTable = d->Find_Object(NULL, Index, &errorCode, d);
  /* 找不到该索引 */
  if (errorCode != OD_SUCCESSFUL)
    return errorCode;
  /* 找不到该子索引 */
//  if( ptrTable->bSubCount <= bSubindex ) {
//	if( ptrTable->size <= bSubindex ) {
//    /* Subindex not found */
//    accessDictionaryError(wIndex, bSubindex, 0, *pExpectedSize, OD_NO_SUCH_SUBINDEX);
//    return OD_NO_SUCH_SUBINDEX;
//  }

  /* 该对象属性为只读即不允许设置数据 */
//  if (checkAccess && (ptrTable->pSubindex[bSubindex].bAccessType == RO)) {
  if (checkAccess && (ptrTable->access == OD_ACCESS_R))
  {
    MSG_WAR(0x2B25, "Access Type : ", ptrTable->access);
    accessDictionaryError(wIndex, bSubindex, 0, *pExpectedSize, OD_WRITE_NOT_ALLOWED);
    return OD_WRITE_NOT_ALLOWED;
  }

  /* 数据类型 */
  dataType = ptrTable->data_type;
  /* 数据大小 */
//  szData = ptrTable->pSubindex[bSubindex].size;
    szData = data_length(ptrTable->data_type);
  /* 除了字符串之外的变量大小必须符合，字符串可以比规定的要小 */
  if( *pExpectedSize == 0 ||
      *pExpectedSize == szData ||
      /* allow to store a shorter string than entry size */
//      (dataType == visible_string && *pExpectedSize < szData))
        (dataType == OD_DATATYPE_VS && *pExpectedSize < szData))
    {
	  #ifdef CANOPEN_BIG_ENDIAN
      /* re-endianize do not occur for bool, strings time and domains */
      if(endianize && dataType > boolean && 
		  !(dataType >= visible_string && dataType <= domain))
        {
          /* we invert the data source directly. This let us do range
            testing without */
          /* additional temp variable */
          UNS8 i;
          for ( i = 0 ; i < ( ptrTable->pSubindex[bSubindex].size >> 1)  ; i++)
            {
              UNS8 tmp =((UNS8 *)pSourceData) [(ptrTable->pSubindex[bSubindex].size - 1) - i];
              ((UNS8 *)pSourceData) [(ptrTable->pSubindex[bSubindex].size - 1) - i] = ((UNS8 *)pSourceData)[i];
              ((UNS8 *)pSourceData)[i] = tmp;
            }
        }
	  #endif
	  /* 合法性检查 */
      errorCode = (*d->valueRangeTest)(dataType, pSourceData);
      if (errorCode) 
	  {
        accessDictionaryError(wIndex, bSubindex, szData, *pExpectedSize, errorCode);
        return errorCode;
      }
	  /* 把数据从缓冲区拷贝到对象中 */
//      memcpy(ptrTable->pSubindex[bSubindex].pObject,pSourceData, *pExpectedSize);
	  int * memcpypointer;
		memcpypointer =(int *) memcpy(ptrTable->p_content,pSourceData, *pExpectedSize);
	  if(memcpypointer == NULL)
	  {
		  return MEMCPY_ERROR;
	  }
//	switch (dataType)
//	{
//		case OD_DATATYPE_U8:  	  memcpy((UNS8*)ptrTable->p_content,(UNS8*)pSourceData, *pExpectedSize); break;
//		case OD_DATATYPE_U16: 	  memcpy((UNS8*)ptrTable->p_content,(UNS8*)pSourceData, *pExpectedSize); break;
//		case OD_DATATYPE_U32: 	  memcpy((UNS8*)ptrTable->p_content,(UNS8*)pSourceData, *pExpectedSize); break;
//		case OD_DATATYPE_I8:  	  memcpy((UNS8*)ptrTable->p_content,(UNS8*)pSourceData, *pExpectedSize); break;
//		case OD_DATATYPE_I16: 	  memcpy((UNS8*)ptrTable->p_content,(UNS8*)pSourceData, *pExpectedSize); break;
//		case OD_DATATYPE_I32: 	  memcpy((UNS8*)ptrTable->p_content,(UNS8*)pSourceData, *pExpectedSize); break;
//		case OD_DATATYPE_F32: 	  memcpy((UNS8*)ptrTable->p_content,(UNS8*)pSourceData, *pExpectedSize); break;
//		case OD_DATATYPE_BOOLEAN: memcpy((UNS8*)ptrTable->p_content,(UNS8*)pSourceData, *pExpectedSize); break;
//		default:break;
//	}
     /* TODO : CONFORM TO DS-301 : 
      *  - stop using NULL terminated strings
      *  - store string size in td_subindex
      * */
      /* terminate visible_string with '\0' */
	  /* 如果是字符串的话，还要把最后一个字节设置为'\0' */
      if(dataType == OD_DATATYPE_VS && *pExpectedSize < szData)
//        ((UNS8*)ptrTable->pSubindex[bSubindex].pObject)[*pExpectedSize] = 0;
	      ((UNS8*)ptrTable->p_content)[*pExpectedSize] = 0;
      
      *pExpectedSize = szData;

      /* Callbacks */
	  /* 回调函数 */
	  if(ptrTable->callback != NULL)
//      if(Callback && Callback[bSubindex])
	   {
			errorCode = (*(ptrTable->callback))(d, (const OD_OI_typeDef *)ptrTable, bSubindex);
			if(errorCode != OD_SUCCESSFUL)//flaxin，注意，在给条目注册回调函数的时候，回调函数最后一定要return 0，return其他值这里会认为出错
			{
				return errorCode;
			}
       }

      /* TODO : Store dans NVRAM */
	  /* 存储字典子索引 */
//      if (ptrTable->pSubindex[bSubindex] & TO_BE_SAVE){
	    if (ptrTable->access & OD_ACCESS_C){
        (*d->storeODSubIndex)(d, wIndex, bSubindex);//空函数，暂时未实现
      }
      return OD_SUCCESSFUL;
    }
	/* 大小不对也要报错 */
	else
	{
      *pExpectedSize = szData;
      accessDictionaryError(wIndex, bSubindex, szData, *pExpectedSize, OD_LENGTH_DATA_INVALID);
      return OD_LENGTH_DATA_INVALID;
    }
}


/**********************************************************
** @FuncName:scanIndexOD
** @Param:d
** @Param:wIndex索引
** @Param:errorCode错误代码
** @Param:Callback回调函数
** @Return:indextable *类型的数据
** @Brief:扫描对象字典索引
** @addtion:errorCode和Callback并不是给函数提供操作的参数，而是用它来套被人的数据
**********************************************************/
//const indextable * scanIndexOD (CO_Data* d, UNS16 wIndex, UNS32 *errorCode, ODCallback_t **Callback)
//{
//  return (*d->scanIndexOD)(wIndex, errorCode, Callback);
////	scanIndexOD_t scanIndexOD;TestSlave_scanIndexOD
// /* 函数指针类型定义：参数为索引，错误代码，和回调函数*/
////typedef const indextable * (*scanIndexOD_t)(UNS16 wIndex, UNS32 * errorCode, ODCallback_t **Callback);
//}



/**********************************************************
** @FuncName:
** @Param:
** @Return:
** @Brief:
** @addtion:将Callback这个函数名赋值给对象字典中对象wIndex的回调函数列表的TestSlave_Indexxxxx_callbacks[bSubindex]
**********************************************************/
UNS32 RegisterSetODentryCallBack(CO_Data* d, UNS32 Index, ODCallback_t Callback)
{	
	UNS32 errorCode;

	OD_OI_TypeDef *odentry;

	odentry = d->Find_Object (NULL, Index, &errorCode, d);
	if(errorCode == OD_SUCCESSFUL) 
	  odentry->callback = Callback; 
	return errorCode;
}

void _storeODSubIndex (CO_Data* d, UNS16 wIndex, UNS8 bSubindex){}
