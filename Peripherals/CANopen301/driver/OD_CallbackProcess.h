#ifndef OD_CALLBACK_PROCESS_H__
#define OD_CALLBACK_PROCESS_H__


#include "data.h"
void CANopenMasterCallbackRegister(void);
void CANopenSlaveCallbackRegister(void);

// Slave
UNS32 controlwordCallback(CO_Data* d, const OD_OI_typeDef * unsused_indextable, UNS8 unsused_bSubindex);
UNS32 statuswordCallback(CO_Data* d, const OD_OI_typeDef * unsused_indextable, UNS8 unsused_bSubindex);
UNS32 actualBufferSizeCallback(CO_Data* d, const OD_OI_typeDef * unsused_indextable, UNS8 unsused_bSubindex);
UNS32 interpolation_data_record_callback(CO_Data* d, const OD_OI_typeDef * unsused_indextable, UNS8 unsused_bSubindex);

UNS32 modesOfOperationCallback(CO_Data* d, const OD_OI_typeDef * unsused_indextable, UNS8 unsused_bSubindex);
UNS32 ppmProfileTargetPositionUpdatedCallback(CO_Data* d, const OD_OI_typeDef * unsused_indextable, UNS8 unsused_bSubindex);
UNS32 ppmProfileVelocityUpdatedCallback(CO_Data* d, const OD_OI_typeDef * unsused_indextable, UNS8 unsused_bSubindex);
UNS32 ppmProfileAccelUpdatedCallback(CO_Data* d, const OD_OI_typeDef * unsused_indextable, UNS8 unsused_bSubindex);
UNS32 ppmProfileDecelUpdatedCallback(CO_Data* d, const OD_OI_typeDef * unsused_indextable, UNS8 unsused_bSubindex);
UNS32 pvtEnableServoCallback(CO_Data* d, const OD_OI_typeDef * unsused_indextable, UNS8 unsused_bSubindex);

// Master 
UNS32 servoStatuswordCallback(CO_Data* d, const OD_OI_typeDef * unsused_indextable, UNS8 unsused_bSubindex);
UNS32 servoControlwordCallback(CO_Data* d, const OD_OI_typeDef * unsused_indextable, UNS8 unsused_bSubindex);


#endif
