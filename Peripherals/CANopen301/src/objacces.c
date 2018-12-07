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
/* �ֵ������� */
/* codeΪ������룬sizeDataDictΪӦ�ø������ݳ��ȣ�sizeDataGivenΪ��ǰ���������ݳ��� */
UNS8 accessDictionaryError(UNS16 index, UNS8 subIndex,
                           UNS32 sizeDataDict, UNS32 sizeDataGiven, UNS32 code)
{
#ifdef DEBUG_WAR_CONSOLE_ON
  MSG_WAR(0x2B09,"Dictionary index : ", index);
  MSG_WAR(0X2B10,"           subindex : ", subIndex);
  switch (code) {
  /* �Ҳ��������� */
  case  OD_NO_SUCH_OBJECT:
    MSG_WAR(0x2B11,"Index not found ", index);
    break;
  /* �Ҳ����������� */
  case OD_NO_SUCH_SUBINDEX :
    MSG_WAR(0x2B12,"SubIndex not found ", subIndex);
    break;
  /* ������Ϊֻ����������д���� */
  case OD_WRITE_NOT_ALLOWED :
    MSG_WAR(0x2B13,"Write not allowed, data is read only ", index);
    break;
  /* Խ����� */
  case OD_LENGTH_DATA_INVALID :
    MSG_WAR(0x2B14,"Conflict size data. Should be (bytes)  : ", sizeDataDict);
    MSG_WAR(0x2B15,"But you have given the size  : ", sizeDataGiven);
    break;
  /* �������û��PDOӳ��ֵ */
  case OD_NOT_MAPPABLE :
    MSG_WAR(0x2B16,"Not mappable data in a PDO at index    : ", index);
    break;
  /* SDO��ֵ̫С */
  case OD_VALUE_TOO_LOW :
    MSG_WAR(0x2B17,"Value range error : value too low. SDOabort : ", code);
    break;
  /* SDO��ֵ̫�� */
  case OD_VALUE_TOO_HIGH :
    MSG_WAR(0x2B18,"Value range error : value too high. SDOabort : ", code);
    break;
  /* δ֪���� */
  default :
    MSG_WAR(0x2B20, "Unknown error code : ", code);
  }
  #endif

  return 0;
}
/* ��ȡ�����е����� */
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
  /* ɨ���ֵ� */
//  ptrTable = (*d->scanIndexOD)(wIndex, &errorCode, &Callback);
	ptrTable = d->Find_Object(NULL, Index, &errorCode, d);
  /* �Ҳ��������� */
  if (errorCode != OD_SUCCESSFUL)
    return errorCode;
  /* �Ҳ����������� */
//  if( ptrTable->bSubCount <= bSubindex ) {
//    if( ptrTable->size <= bSubindex ) {
//    /* Subindex not found */
//    accessDictionaryError(wIndex, bSubindex, 0, 0, OD_NO_SUCH_SUBINDEX);
//    return OD_NO_SUCH_SUBINDEX;
//  }
  /* �ö�������Ϊֻд���������ȡ���� */
//  if (checkAccess && (ptrTable->pSubindex[bSubindex].bAccessType & WO)) {
    if (checkAccess && (ptrTable->access == OD_ACCESS_W)) {
    MSG_WAR(0x2B30, "Access Type : ", ptrTable->access);
    accessDictionaryError(wIndex, bSubindex, 0, 0, OD_READ_NOT_ALLOWED);
    return OD_READ_NOT_ALLOWED;
  }
  /* ����������ΪNULL */
  if (pDestData == 0) {
    return SDOABT_GENERAL_ERROR;
  }
  /* �������ݱȻ�����Ҫ��Խ�� */
//  if (ptrTable->pSubindex[bSubindex].size > *pExpectedSize) {
    if (data_length(ptrTable->data_type) > *pExpectedSize) {
    /* Requested variable is too large to fit into a transfer line, inform    *
     * the caller about the real size of the requested variable.              */
//    *pExpectedSize = ptrTable->pSubindex[bSubindex].size;
	  *pExpectedSize = data_length(ptrTable->data_type);
    return SDOABT_OUT_OF_MEMORY;
  }
  /* ��ȡ������������ */
//  *pDataType = ptrTable->pSubindex[bSubindex].bDataType;
    *pDataType = ptrTable->data_type;
  /* �������ݴ�С */
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
  /* ��������������Ͳ���visible_string�ַ��� */
  if(*pDataType != OD_DATATYPE_VS)
	{
	  /* �������� */
//      memcpy(pDestData, ptrTable->pSubindex[bSubindex].pObject,szData);
		memcpy(pDestData, ptrTable->p_content,szData);
	  /* ��ȡ�������ݴ�С */
      *pExpectedSize = szData;
    }
	  /* �����visible_string�ַ����Ļ� */
  else
	{
      /* TODO : CONFORM TO DS-301 :
       *  - stop using NULL terminated strings
       *  - store string size in td_subindex
       * */
      /* Copy null terminated string to user, and return discovered size */
	  /* �ַ�����ʼλ�� */
//      UNS8 *ptr = (UNS8*)ptrTable->pSubindex[bSubindex].pObject;
		UNS8 *ptr = (UNS8*)ptrTable->p_content;
		
      UNS8 *ptr_start = ptr;
      /* *pExpectedSize IS < szData . if null, use szData */
	  /* �ַ�������λ�� */
      UNS8 *ptr_end = ptr + (*pExpectedSize ? *pExpectedSize : szData) ;/* ���ʣ�������pExpectedSizeһ����һ�����ڵ���szData��ֵ�ѣ���������жϾͲ����еڶ�������ˣ�ΪʲôҪ����ôһ������Ѿ���ȷ���˵��ж� */
      UNS8 *ptr_dest = (UNS8*)pDestData;
      while( *ptr && ptr < ptr_end ){/* flaxin ������ʱ������0���ǵ���Ԥ���ĸÿ��������ݳ��������ֹ���� */
          *(ptr_dest++) = *(ptr++);/* ��֪���������������ַ�����ʱ��szdata�᲻�Ὣ�����Ǹ�0�����������ȥ������������Ļ����ͺ������ */
      }/* ��Ϊwhile�����õ��� < �ţ����Ե�pExpectedSize��szdata���ʱ��������Ⱦ�ֻ�д�����������ˣ�����С�ڵģ���������ʵ�����ݳ��Ȼ��pExpectedSizeС1 */
    /* �ַ������� */
    *pExpectedSize = (UNS32) (ptr - ptr_start);
    /* terminate string if not maximum length */
	/* ��β��'\0' */
    if (*pExpectedSize < szData)/* Flaxin ���szdata�������Ǹ�������0Ҳ���ȥ�ˣ� ����һ����������������Ϊ����pExpectedSize��Ĵ��ǵ���szdata*/
            *(ptr) = 0;/* ʵ�ʿ��������ݴ�С����С��szData��С1����������һ������������ */
    }
  return OD_SUCCESSFUL;
}
/* ���ö����е����� */
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
  /* ɨ���ֵ� */
//  ptrTable =(*d->scanIndexOD)(wIndex, &errorCode, &Callback);
//	ptrTable = d->Find_Object(NULL, Index|0xFF, &errorCode);
   ptrTable = d->Find_Object(NULL, Index, &errorCode, d);
  /* �Ҳ��������� */
  if (errorCode != OD_SUCCESSFUL)
    return errorCode;
  /* �Ҳ����������� */
//  if( ptrTable->bSubCount <= bSubindex ) {
//	if( ptrTable->size <= bSubindex ) {
//    /* Subindex not found */
//    accessDictionaryError(wIndex, bSubindex, 0, *pExpectedSize, OD_NO_SUCH_SUBINDEX);
//    return OD_NO_SUCH_SUBINDEX;
//  }

  /* �ö�������Ϊֻ������������������ */
//  if (checkAccess && (ptrTable->pSubindex[bSubindex].bAccessType == RO)) {
  if (checkAccess && (ptrTable->access == OD_ACCESS_R))
  {
    MSG_WAR(0x2B25, "Access Type : ", ptrTable->access);
    accessDictionaryError(wIndex, bSubindex, 0, *pExpectedSize, OD_WRITE_NOT_ALLOWED);
    return OD_WRITE_NOT_ALLOWED;
  }

  /* �������� */
  dataType = ptrTable->data_type;
  /* ���ݴ�С */
//  szData = ptrTable->pSubindex[bSubindex].size;
    szData = data_length(ptrTable->data_type);
  /* �����ַ���֮��ı�����С������ϣ��ַ������Աȹ涨��ҪС */
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
	  /* �Ϸ��Լ�� */
      errorCode = (*d->valueRangeTest)(dataType, pSourceData);
      if (errorCode) 
	  {
        accessDictionaryError(wIndex, bSubindex, szData, *pExpectedSize, errorCode);
        return errorCode;
      }
	  /* �����ݴӻ����������������� */
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
	  /* ������ַ����Ļ�����Ҫ�����һ���ֽ�����Ϊ'\0' */
      if(dataType == OD_DATATYPE_VS && *pExpectedSize < szData)
//        ((UNS8*)ptrTable->pSubindex[bSubindex].pObject)[*pExpectedSize] = 0;
	      ((UNS8*)ptrTable->p_content)[*pExpectedSize] = 0;
      
      *pExpectedSize = szData;

      /* Callbacks */
	  /* �ص����� */
	  if(ptrTable->callback != NULL)
//      if(Callback && Callback[bSubindex])
	   {
			errorCode = (*(ptrTable->callback))(d, (const OD_OI_typeDef *)ptrTable, bSubindex);
			if(errorCode != OD_SUCCESSFUL)//flaxin��ע�⣬�ڸ���Ŀע��ص�������ʱ�򣬻ص��������һ��Ҫreturn 0��return����ֵ�������Ϊ����
			{
				return errorCode;
			}
       }

      /* TODO : Store dans NVRAM */
	  /* �洢�ֵ������� */
//      if (ptrTable->pSubindex[bSubindex] & TO_BE_SAVE){
	    if (ptrTable->access & OD_ACCESS_C){
        (*d->storeODSubIndex)(d, wIndex, bSubindex);//�պ�������ʱδʵ��
      }
      return OD_SUCCESSFUL;
    }
	/* ��С����ҲҪ���� */
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
** @Param:wIndex����
** @Param:errorCode�������
** @Param:Callback�ص�����
** @Return:indextable *���͵�����
** @Brief:ɨ������ֵ�����
** @addtion:errorCode��Callback�����Ǹ������ṩ�����Ĳ����������������ױ��˵�����
**********************************************************/
//const indextable * scanIndexOD (CO_Data* d, UNS16 wIndex, UNS32 *errorCode, ODCallback_t **Callback)
//{
//  return (*d->scanIndexOD)(wIndex, errorCode, Callback);
////	scanIndexOD_t scanIndexOD;TestSlave_scanIndexOD
// /* ����ָ�����Ͷ��壺����Ϊ������������룬�ͻص�����*/
////typedef const indextable * (*scanIndexOD_t)(UNS16 wIndex, UNS32 * errorCode, ODCallback_t **Callback);
//}



/**********************************************************
** @FuncName:
** @Param:
** @Return:
** @Brief:
** @addtion:��Callback�����������ֵ�������ֵ��ж���wIndex�Ļص������б��TestSlave_Indexxxxx_callbacks[bSubindex]
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
