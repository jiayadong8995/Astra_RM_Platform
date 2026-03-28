 /**
   *********************************************************************
   * @file      motor_control_task.c/h
   * @brief     该任务轮腿底盘部分电机
   * @note       
   * @history
   *
   @verbatim
   ==============================================================================

   ==============================================================================
   @endverbatim
   *********************************************************************
   */

/*
菜鸟总结，开始做轮腿的时候看玺佬的视频，里面说一定要保证can通信的稳定，当时感觉接线和开发板，
电机等硬件没有问题，这个就能保证，但是由于基础不牢，can的接收频率和发送频率的原因，
一托六个电机，虽然每1ms延时发送一次，整个电机的发送频率为1000/6ms，满足发送需求，但是把接收频率给忽视了，
3508电机的接收是广播形式的接收，即接收1kHz接收一次，而达妙的电机是发送后才能收到接收，两类电机的接收冲突了。
*/

#include "motor_control_task.h"
#include "fdcan.h"
#include "cmsis_os.h"
#include "message_center.h"
#include "robot_def.h"

uint32_t systick;
				
void motor_control_task(void)
{
    Subscriber_t *ins_sub;
    Subscriber_t *actuator_cmd_sub;
    INS_Data_t ins_msg = {0};
    Actuator_Cmd_t actuator_msg = {0};
    uint8_t ins_ready = 0;
    Joint_Motor_t *joint_motor[4];

    ins_sub = SubRegister("ins_data", sizeof(INS_Data_t));
    actuator_cmd_sub = SubRegister("actuator_cmd", sizeof(Actuator_Cmd_t));

    while(ins_ready == 0)
	{
        if (SubGetMessage(ins_sub, &ins_msg))
        {
            ins_ready = ins_msg.ready;
        }
	    osDelay(1);
	}

    joint_motor[0] = chassis_joint_motor_state(0);
    joint_motor[1] = chassis_joint_motor_state(1);
    joint_motor[2] = chassis_joint_motor_state(2);
    joint_motor[3] = chassis_joint_motor_state(3);
	
	joint_motor_init(joint_motor[0],1,MIT_MODE);//发送id为1,控制模式 MIT
	joint_motor_init(joint_motor[1],2,MIT_MODE);//发送id为2,控制模式 MIT
	joint_motor_init(joint_motor[2],3,MIT_MODE);//发送id为3,控制模式 MIT
	joint_motor_init(joint_motor[3],4,MIT_MODE);//发送id为4,控制模式 MIT
  //需要对4个关节电机进行零点重置
	chassis_set_joint_enable_flag(0, enable_motor_mode(&hfdcan1, joint_motor[0]->para.id, joint_motor[0]->mode));
	//DM_motor_zeroset(&hfdcan1, joint_motor[0]->para.id);
	osDelay(1); 
	chassis_set_joint_enable_flag(1, enable_motor_mode(&hfdcan1, joint_motor[1]->para.id, joint_motor[1]->mode));
	//DM_motor_zeroset(&hfdcan1, joint_motor[1]->para.id);
	osDelay(1);
	chassis_set_joint_enable_flag(2, enable_motor_mode(&hfdcan1, joint_motor[2]->para.id, joint_motor[2]->mode));
	//DM_motor_zeroset(&hfdcan1, joint_motor[2]->para.id);
	osDelay(1);
	chassis_set_joint_enable_flag(3, enable_motor_mode(&hfdcan1, joint_motor[3]->para.id, joint_motor[3]->mode));
	//DM_motor_zeroset(&hfdcan1, joint_motor[3]->para.id);
	osDelay(1);
	osDelay(2);

	while(1)
	{	
        if (SubGetMessage(actuator_cmd_sub, &actuator_msg))
        {
            /* latest actuator command cached in topic subscriber */
        }

		systick = osKernelSysTick();
	   if(actuator_msg.start_flag==0){

			if(systick%2==0)//左腿
			{
				mit_ctrl(&hfdcan1,joint_motor[0]->para.id, 0.0f, 0.0f,0.0f, 0.0f,0.0f);//right.torque_set[0]
				mit_ctrl(&hfdcan1,joint_motor[1]->para.id, 0.0f, 0.0f,0.0f, 0.0f,0.0f);//right.torque_set[1]
				CAN_cmd_chassis(&hfdcan2,0,0,0,0);		
			}
			else
			{
				mit_ctrl(&hfdcan1,joint_motor[2]->para.id, 0.0f, 0.0f,0.0f, 0.0f,0.0f);//left.torque_set[0]
				mit_ctrl(&hfdcan1,joint_motor[3]->para.id, 0.0f, 0.0f,0.0f, 0.0f,0.0f);//left.torque_set[1]		
			}
		}
		else if(actuator_msg.start_flag==1)	
		{
			if(systick%2==0)
			{
				mit_ctrl(&hfdcan1,joint_motor[0]->para.id, 0.0f, 0.0f,0.0f, 0.0f,actuator_msg.joint_torque[0]);
				mit_ctrl(&hfdcan1,joint_motor[1]->para.id, 0.0f, 0.0f,0.0f, 0.0f,actuator_msg.joint_torque[1]);
				CAN_cmd_chassis(&hfdcan2,actuator_msg.wheel_current[0],actuator_msg.wheel_current[1],0,0);
			}
			else
			{
				mit_ctrl(&hfdcan1,joint_motor[2]->para.id, 0.0f, 0.0f,0.0f, 0.0f,actuator_msg.joint_torque[2]);
				mit_ctrl(&hfdcan1,joint_motor[3]->para.id, 0.0f, 0.0f,0.0f, 0.0f,actuator_msg.joint_torque[3]);
			}	
		}
		osDelay(1);
		
		
		
	}
}
