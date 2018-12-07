#include "OD_CallbackProcess.h"
#include "read_write_SDO.h"
#include "objacces.h"
#include "TestSlave.h"
#include "CANopenMaster.h"
#include "servo_driver_control.h"
#include "global.h"


#define MAX_PVT_BUFFER 64


// CANopenMaster OD callback register
void CANopenMasterCallbackRegister(void)
{
	// receive change of the servo driver's statusword
	RegisterSetODentryCallBack(&CANopenMaster_Data, 0x2F030000, &servoControlwordCallback); 	
	RegisterSetODentryCallBack(&CANopenMaster_Data, 0x2F040000, &servoStatuswordCallback);    
}


// CANopenSlave OD callback register
void CANopenSlaveCallbackRegister(void)
{
	RegisterSetODentryCallBack(&TestSlave_Data, 0x60400000, &controlwordCallback);    /* controlWord主要是被别人写，被写了用来触发电机驱动该响应主控的命令了 */
	RegisterSetODentryCallBack(&TestSlave_Data, 0x60410000, &statuswordCallback);     /* statusword主要是被自己写，被写了用来触发驱动通知主控该干点啥了 */
	RegisterSetODentryCallBack(&TestSlave_Data, 0x60600000, &modesOfOperationCallback); /* when mode of operation changed by main control board, the joint board need to tell the driver */
  
    RegisterSetODentryCallBack(&TestSlave_Data, 0x607A0000, &ppmProfileTargetPositionUpdatedCallback);
    RegisterSetODentryCallBack(&TestSlave_Data, 0x60810000, &ppmProfileVelocityUpdatedCallback);
    RegisterSetODentryCallBack(&TestSlave_Data, 0x60830000, &ppmProfileAccelUpdatedCallback);
    RegisterSetODentryCallBack(&TestSlave_Data, 0x60840000, &ppmProfileDecelUpdatedCallback);
    RegisterSetODentryCallBack(&TestSlave_Data, 0x60C40002, &actualBufferSizeCallback);
    RegisterSetODentryCallBack(&TestSlave_Data, 0x60C10001, &pvtEnableServoCallback);	
//    RegisterSetODentryCallBack(&TestSlave_Data, 0x60FF0000, &pv_data_target_velocity_updated_callback);  
//    RegisterSetODentryCallBack(&TestSlave_Data, 0x60710000, &profileTorqueDataTargetTorqueUpdatedCallback); 
//    RegisterSetODentryCallBack(&TestSlave_Data, 0x60870000, &profileTorqueDataTorqueSlopeUpdatedCallback); 
//    RegisterSetODentryCallBack(&TestSlave_Data, 0x200A0000, &unprofilePositionDataTargetPositionUpdatedCallback);
    
    for(int i = 0; i <= MAX_PVT_BUFFER; i++)
    {
        RegisterSetODentryCallBack(&TestSlave_Data, 0x60C10000+i, &interpolation_data_record_callback);
    }
}

uint16_t number_of_pvt_data = 0;
UNS32 interpolation_data_record_callback(CO_Data* d, const OD_OI_typeDef * unsused_indextable, UNS8 unsused_bSubindex)
{
//	if(0 == number_of_pvt_data)
//	{
//        servoShutdownToReady();	
//	}
    number_of_pvt_data++;
	return 0;
}


// main controlboard gives the new controlword to the jointboard via the CANopen network
// controlword_callback
UNS32 controlwordCallback(CO_Data* d, const OD_OI_typeDef * unsused_indextable, UNS8 unsused_bSubindex)//主控传来了数据并且置位了驱动控制字的new_set_point
{
    switch (controlword)
    {
	    case ABS_POS:
            controlword_to_driver = controlword;
		    break;
		
	    case ABS_POS_IMM:
			controlword_to_driver = controlword;
		    break;		
		
	    case REL_POS:
		    controlword_to_driver = controlword;
		    break;
		
	    case REL_POS_IMM:
			controlword_to_driver = controlword;
		    break;		
		
		case ENABLE_IP_MODE:
		    controlword_to_driver = 0x0F;  // refer as maxon driver controlword bits definition
			break;
		
	    default:
		    break;
    }
	return 0;
}


// jointboard use the setODentry function to trigger the callback to inform the main controlboard 
// after jointboard received the set_new_point_acknowledge bit
// statusword_callback
UNS32 statuswordCallback(CO_Data* d, const OD_OI_typeDef * unsused_indextable, UNS8 unsused_bSubindex)
{
	switch (gMC.modes_of_operation)
	{
		case POS_PTP_MODE:
//			if(get_set_new_point_acknowledge_of_statusword())
//			{
//				UNS8 pdoNum=0;
//				sendOnePDOevent (&TestSlave_Data, pdoNum);
//			}
//			if(!get_set_new_point_acknowledge_of_statusword())
//			{
//			}
		break;
		
		case POS_PVT_MODE:
		break;
		
		case VEL_PV_MODE:
		break;	
		
		case TRQ_PDPQ:
		break;
		
		default:
		break;
	}
	return 0;	
}

extern unsigned int startaddress;
extern unsigned int endaddress;
int pvt_data_lost = 0;
//actual_buffer_size_callback
UNS32 actualBufferSizeCallback(CO_Data* d, const OD_OI_typeDef * unsused_indextable, UNS8 unsused_bSubindex)
{
	
	servoControlwordPresetDebug2();
    if(number_of_pvt_data == actual_buffer_size)
	{
        number_of_pvt_data = 0;
		//the situation of 0 point or 1 point or more than 64 points is illegal input	
        if(actual_buffer_size == 0 || actual_buffer_size == 1 || actual_buffer_size > MAX_PVT_BUFFER)
		{
          for(int i = 0; i <= MAX_PVT_BUFFER; i++)
          {
              gMC.PVT_struct.pvt_table_struct[i].position = 0;
              gMC.PVT_struct.pvt_table_struct[i].velocity = 0;
              gMC.PVT_struct.pvt_table_struct[i].time = 0;
          }
		}
		
        if(actual_buffer_size >= 2 && actual_buffer_size <= MAX_PVT_BUFFER)
        {
            for(int i = 0; i <= actual_buffer_size; i++)
            {
                gMC.PVT_struct.pvt_table_struct[i] = pvttable[i];
            }
            gMC.PVT_struct.p_start = &gMC.PVT_struct.pvt_table_struct[1];
            gMC.PVT_struct.p_end   = &gMC.PVT_struct.pvt_table_struct[actual_buffer_size];
								startaddress = (unsigned int)gMC.PVT_struct.p_start;
					endaddress = (unsigned int)gMC.PVT_struct.p_end;
			// 由于PVT模式跑完之后会自动切到NP模式，而再次开始PVT模式时会更新目标位置，放这里容易引起误动作 			
            // gMC.i_pos_target = gMC.PVT_struct.pvt_table_struct[actual_buffer_size].position;   
        }

	}	
    else
    {
        pvt_data_lost++;
        number_of_pvt_data = 0;
        for(int i = 0; i <= MAX_PVT_BUFFER; i++)
        {
            gMC.PVT_struct.pvt_table_struct[i].position = 0;
            gMC.PVT_struct.pvt_table_struct[i].velocity = 0;
            gMC.PVT_struct.pvt_table_struct[i].time = 0;
        }
    }	
	
	return 0;	
}	


// modes_of_operation_callback
// drive operating mode: PPM:1; PVM:3; HMM:6; CSP:8; CSV:9; CST:10;
UNS32 modesOfOperationCallback(CO_Data* d, const OD_OI_typeDef * unsused_indextable, UNS8 unsused_bSubindex)
{
	switch (gMC.modes_of_operation)
	{
		case POS_PTP_MODE:
			mode_to_driver = 1;
		break;
		
		case POS_PVT_MODE:
			mode_to_driver = 8;
		break;
		
		case VEL_PV_MODE:
		break;	
		
		case TRQ_PDPQ:			
		break;
		
		default:
		break;
	}
	return 0;
}


// 
UNS32 ppmProfileTargetPositionUpdatedCallback(CO_Data* d, const OD_OI_typeDef * unsused_indextable, UNS8 unsused_bSubindex)
{
    target_position_to_driver = profile_target_position;
	return 0;
}


// 
UNS32 ppmProfileVelocityUpdatedCallback(CO_Data* d, const OD_OI_typeDef * unsused_indextable, UNS8 unsused_bSubindex)
{
    profile_vel_to_driver = profile_velocity;
	return 0;
}


// 
UNS32 ppmProfileAccelUpdatedCallback(CO_Data* d, const OD_OI_typeDef * unsused_indextable, UNS8 unsused_bSubindex)
{
    profile_accel_to_driver = profile_acceleration;
	return 0;
}


// 
UNS32 ppmProfileDecelUpdatedCallback(CO_Data* d, const OD_OI_typeDef * unsused_indextable, UNS8 unsused_bSubindex)
{
    profile_decel_to_driver = profile_deceleration;
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CANopenMaster OD callback
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// when servo driver's statusword is changed, the servo driver sends statusword to jointboard via TPDO, 
// the action jonitboard's receives the RPDO and mapping to OD(setODentry) will call the callback function,
// and then change the jointboard statusword according with the statusword from servo driver and the jointboard status.
UNS32 servoStatuswordCallback(CO_Data* d, const OD_OI_typeDef * unsused_indextable, UNS8 unsused_bSubindex)
{
//	int is_bit_10_set = (statusword_from_servo_driver &= (1<<10));
//	int is_bit_12_set = (statusword_from_servo_driver &= (1<<12));	
//	int is_bit_13_set = (statusword_from_servo_driver &= (1<<13));	
	
//	statusword = statusword_from_servo_driver;
	
	// after servo driver ppm move is done
//	if((ppm_before_pvt_flag) && is_bit_10_set)
//	{
//		ppm_before_pvt_flag = 0;
//	}
		
    return 0;
}


UNS32 servoControlwordCallback(CO_Data* d, const OD_OI_typeDef * unsused_indextable, UNS8 unsused_bSubindex)
{
	//servo_driver_controlword_display;
    return 0;
}


UNS32 pvtEnableServoCallback(CO_Data* d, const OD_OI_typeDef * unsused_indextable, UNS8 unsused_bSubindex)  
{
//    servoShutdownToReady();
//	servoControlwordPresetDebug2();
	
    return 0;
}