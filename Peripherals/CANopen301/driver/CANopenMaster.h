#ifndef CANOPEN_MASTER_H
#define CANOPEN_MASTER_H

#include "data.h"


//extern CO_Data CANopenMaster_Data;
extern OD_OI_TypeDef CANopenMaster_objdict[];

extern UNS32 empty_obj_2F01;
//extern UNS16 err_code_from_servo;
extern UNS16 statusword_from_servo_driver;
extern UNS16 servo_driver_controlword_display;


#endif
