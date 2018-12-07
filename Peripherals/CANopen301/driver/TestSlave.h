
/* File generated by gen_cfile.py. Should not be modified. */

#ifndef TESTSLAVE_H
#define TESTSLAVE_H

#include "data.h"

typedef struct
{
	UNS32 year;
	UNS32 month;
	UNS16 day;
	UNS16 week;
	UNS8 hour;
	UNS32 minute;
	UNS8 second;
	UNS16 ms;
	UNS16 us;
}Time;
typedef struct
{
	UNS8 apple;
	UNS32 orange;
	UNS8 pear;
	UNS8 water;
	UNS32 milk;
	UNS16 coffee;
	UNS32 rice;
	UNS8 mango;
	UNS8 nut;
	UNS8 peanut;
	UNS32 sea;	
	UNS8 cookie;
	UNS16 cabbage;
}Cost;
extern int Timelength;
extern int Costlength;
/* Prototypes of function provided by object dictionnary */
UNS32 TestSlave_valueRangeTest (UNS8 typeValue, void * value);
//const OD_OI_TypeDef * TestSlave_scanIndexOD (UNS16 wIndex, UNS32 * errorCode, ODCallback_t **callbacks);
UNS32 Call_Null(CO_Data* d, const OD_OI_typeDef * d12, UNS8 bSubindex);
/* Master node data struct */
extern CO_Data TestSlave_Data;
//extern ODCallback_t Store_parameters_callbacks[];		/* Callbacks of index0x1010 */
//extern ODCallback_t Restore_Default_Parameters_callbacks[];		/* Callbacks of index0x1011 */
//extern ODCallback_t Transmit_PDO_1_Parameter_callbacks[];		/* Callbacks of index0x1800 */
//extern ODCallback_t Transmit_PDO_2_Parameter_callbacks[];		/* Callbacks of index0x1801 */
//extern ODCallback_t Transmit_PDO_3_Parameter_callbacks[];		/* Callbacks of index0x1802 */
//extern ODCallback_t Transmit_PDO_4_Parameter_callbacks[];		/* Callbacks of index0x1803 */
//extern ODCallback_t Transmit_PDO_5_Parameter_callbacks[];		/* Callbacks of index0x1804 */
extern UNS32 SlaveMap1;		/* Mapped at index 0x2000, subindex 0x00*/
extern UNS32 SlaveMap2;		/* Mapped at index 0x2001, subindex 0x00*/
extern UNS8 SlaveMap3;		/* Mapped at index 0x2002, subindex 0x00*/
extern UNS8 SlaveMap4;		/* Mapped at index 0x2003, subindex 0x00*/
extern UNS8 SlaveMap5;		/* Mapped at index 0x2004, subindex 0x00*/
extern UNS8 SlaveMap6;		/* Mapped at index 0x2005, subindex 0x00*/
extern UNS8 SlaveMap7;		/* Mapped at index 0x2006, subindex 0x00*/
extern UNS8 SlaveMap8;		/* Mapped at index 0x2007, subindex 0x00*/
extern UNS8 SlaveMap9;		/* Mapped at index 0x2008, subindex 0x00*/
extern UNS8 SlaveMap10;		/* Mapped at index 0x2009, subindex 0x00*/
extern UNS16 SlaveMap11;		/* Mapped at index 0x200A, subindex 0x00*/
extern INTEGER16 SlaveMap12;		/* Mapped at index 0x200B, subindex 0x00*/
extern INTEGER16 SlaveMap13;		/* Mapped at index 0x200C, subindex 0x00*/

extern UNS32 RPDO1DATA1;
extern UNS32 RPDO1DATA2;

extern UNS32 RPDO2DATA1;
extern UNS32 RPDO2DATA2;
extern UNS32 RPDO2DATA3;
extern UNS32 RPDO2DATA4;
extern UNS32 RPDO2DATA5;
extern UNS32 RPDO2DATA6;
extern UNS32 RPDO2DATA7;
extern UNS32 RPDO2DATA8;

extern UNS32 TPDO1DATA1;
extern UNS32 TPDO1DATA2;

extern UNS32 TPDO2DATA1;
extern UNS32 TPDO2DATA2;
extern UNS32 TPDO2DATA3;
extern UNS32 TPDO2DATA4;
extern UNS32 TPDO2DATA5;
extern UNS32 TPDO2DATA6;
extern UNS32 TPDO2DATA7;
extern UNS32 TPDO2DATA8;

extern int32_t unprofile_position;//200A

extern OD_OI_TypeDef TestSlave_objdict[];
//extern int objnum;


//////////////////////////////////////////
/* added new data*/
extern int PIT,ROL,YAW;


#endif // TESTSLAVE_H