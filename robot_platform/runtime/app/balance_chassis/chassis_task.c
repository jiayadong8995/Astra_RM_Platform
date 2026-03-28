/**
  *********************************************************************
  * @file      chassis_task.c/h
  * @brief     该任务计算轮腿模型
  *			   得出输出电流和力矩后发送给电机控制线程
  * @note       
  * @history
  *
  @verbatim
  =====================================================================

  =====================================================================
  @endverbatim
  *********************************************************************
  */
	
	
#include "chassis_task.h"
#include "fdcan.h"
#include "cmsis_os.h"
#include "can_bsp.h"
#include "message_center.h"
#include "pid.h"
#include "VMC_calc.h"
#include "INS_task.h"
#include "robot_def.h"
#include "user_lib.h"
#include "control/chassis_runtime_helpers.h"
#include "io/chassis_topics.h"

static float LQR_K_R[12]= LQR_K_MATRIX;

static vmc_leg_t runtime_right_leg;
static vmc_leg_t runtime_left_leg;
static chassis_t chassis_runtime;
static PidTypeDef leg_r_pid;//右腿的腿长pd
static PidTypeDef leg_l_pid;//左腿的腿长pd
static PidTypeDef tp_pid;//防劈叉补偿pd
static PidTypeDef turn_pid;//转向pd
static PidTypeDef roll_pid;//横滚角补偿pd

uint32_t CHASSIS_TIME=CHASSIS_TASK_PERIOD;

static void Chassis_init(chassis_t *chassis,vmc_leg_t *vmcr_init,vmc_leg_t *vmcl_init);
static void Pensation_init(PidTypeDef *roll,PidTypeDef *Tp,PidTypeDef *turn,PidTypeDef *legr,PidTypeDef *legl);
static void chassis_feedback_update(chassis_t *chassis,vmc_leg_t *vmc_r,vmc_leg_t *vmc_l,INS_t *ins,const Actuator_Feedback_t *feedback);
static void chassis_control_loop(chassis_t * chassis,\
							      vmc_leg_t *    vmcr,\
							      vmc_leg_t *    vmcl,\
						              INS_t *     ins,\
						              float *  LQR_KR,\
						              float *  LQR_KL,\
							     PidTypeDef *    legr,\
							     PidTypeDef *    legl);

/* pub-sub handles */
static Chassis_Runtime_Bus_t runtime_bus;
static INS_t ins_runtime;

void Chassis_task(void)
{
    INS_Data_t ins_msg = {0};
    Chassis_Cmd_t cmd_msg = {0};
    Chassis_Observe_t observe_msg = {0};
    Actuator_Feedback_t feedback_msg = {0};

    chassis_runtime_bus_init(&runtime_bus);
    chassis_runtime_bus_wait_ready(&runtime_bus, &chassis_runtime, &ins_runtime, &ins_msg, &cmd_msg, &observe_msg, &feedback_msg);

    Chassis_init(&chassis_runtime,&runtime_right_leg,&runtime_left_leg);
    Pensation_init(&roll_pid,&tp_pid,&turn_pid,&leg_r_pid,&leg_l_pid);

    osDelay(6);

	while(1)
	{	
        chassis_runtime_bus_pull_inputs(&runtime_bus, &chassis_runtime, &ins_runtime, &ins_msg, &cmd_msg, &observe_msg, &feedback_msg);

		chassis_feedback_update(&chassis_runtime,&runtime_right_leg,&runtime_left_leg,&ins_runtime,&feedback_msg);
	    chassis_control_loop(&chassis_runtime,&runtime_right_leg,&runtime_left_leg,&ins_runtime,LQR_K_R,LQR_K_R,&leg_r_pid,&leg_l_pid);
        chassis_runtime_bus_publish_outputs(&runtime_bus, &chassis_runtime, &runtime_right_leg, &runtime_left_leg);

		osDelay(CHASSIS_TIME);
	}
}


static void Chassis_init(chassis_t *chassis,vmc_leg_t *vmcr_init,vmc_leg_t *vmcl_init)
{
	for(int i = 0;i < 2; i++)
	{
		chassis->wheel_motor[i].chassis_x = 0.0f;
	}  
	VMC_init(vmcr_init);//给杆长赋值
	VMC_init(vmcl_init);//给杆长赋值
}

static void Pensation_init(PidTypeDef *roll,PidTypeDef *Tp,PidTypeDef *turn,PidTypeDef *legr,PidTypeDef *legl)
{
	//腿长pid初始化
	static const float roll_pid[3] = {ROLL_PID_KP,ROLL_PID_KI,ROLL_PID_KD};
    static const float legr_pid[3] = {LEG_PID_KP, LEG_PID_KI,LEG_PID_KD};
    static const float legl_pid[3] = {LEG_PID_KP, LEG_PID_KI,LEG_PID_KD};
    //补偿pid初始化：防劈叉补偿、偏航角补偿
	static const float tp_pid[3] = {TP_PID_KP, TP_PID_KI, TP_PID_KD};
	static const float turn_pid[3] = {TURN_PID_KP, TURN_PID_KI, TURN_PID_KD};
	
    PID_init(roll, PID_POSITION, roll_pid, ROLL_PID_MAX_OUT, ROLL_PID_MAX_IOUT);
	PID_init(Tp,   PID_POSITION, tp_pid,   TP_PID_MAX_OUT,   TP_PID_MAX_IOUT);
	PID_init(turn, PID_POSITION, turn_pid, TURN_PID_MAX_OUT, TURN_PID_MAX_IOUT);

	PID_init(legr, PID_POSITION,legr_pid, LEG_PID_MAX_OUT, LEG_PID_MAX_IOUT);//腿长pid
	PID_init(legl, PID_POSITION,legl_pid, LEG_PID_MAX_OUT, LEG_PID_MAX_IOUT);//腿长pid
}

static void chassis_feedback_update(chassis_t *chassis,vmc_leg_t *vmc_r,vmc_leg_t *vmc_l,INS_t *ins,const Actuator_Feedback_t *feedback)
{
    chassis_apply_feedback_snapshot(chassis, vmc_r, vmc_l, ins, feedback);
	
	if(ins->Pitch<PITCH_FALL_THRESHOLD&&ins->Pitch>-PITCH_FALL_THRESHOLD)
	{//根据pitch角度判断倒地自起是否完成
		chassis->recover_flag=0;
	}

}

static void chassis_control_loop( chassis_t * chassis,\
							      vmc_leg_t *    vmcr,\
							      vmc_leg_t *    vmcl,\
						              INS_t *     ins,\
						              float *  LQR_KR,\
						              float *  LQR_KL,\
							     PidTypeDef *    legr,\
							     PidTypeDef *    legl)
{
	
	VMC_calc_1_right(vmcr,ins,2.0f/1000.0f);//计算theta和d_theta给lqr用，同时也计算右腿长L0,该任务控制周期是4*0.001秒
	VMC_calc_1_left(vmcl,ins,2.0f/1000.0f);//计算theta和d_theta给lqr用，同时也计算左腿长L0,该任务控制周期是4*0.001秒
	chassis_compute_turn_and_leg_compensation(chassis, ins, &turn_pid, &roll_pid, &tp_pid);
    chassis_compute_lqr_outputs(chassis, vmcr, vmcl, LQR_KR, LQR_KL);
    chassis_mix_wheel_torque(chassis);

    vmcr->Tp = vmcr->Tp+chassis->leg_tp;//右髋关节输出力矩
	vmcl->Tp = vmcl->Tp+chassis->leg_tp;//左髋关节输出力矩


	chassis->roll_f0 = 0;

	chassis_apply_jump_logic(chassis, vmcr, vmcl, legr, legl);
   
	chassis_apply_ground_detection(chassis, vmcr, vmcl, ins);
	
	VMC_calc_2(vmcr);//计算期望的关节输出力矩
	VMC_calc_2(vmcl);
	 
	chassis->wheel_motor[0].give_current = chassis->wheel_motor[0].torque_set * M3508_TORQUE_TO_CURRENT;
	chassis->wheel_motor[1].give_current = chassis->wheel_motor[1].torque_set * M3508_TORQUE_TO_CURRENT;

    chassis_saturate_outputs(chassis, vmcr, vmcl);
}
