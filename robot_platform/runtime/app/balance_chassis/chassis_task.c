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
static void chassis_jump_loop(chassis_t * chassis,\
							  vmc_leg_t * vmcr   ,\
							  vmc_leg_t * vmcl   ,\
							 PidTypeDef * legr   ,\
							 PidTypeDef * legl	  );
static void chassis_ground_detection(chassis_t * chassis,\
									 vmc_leg_t * vmcr   ,\
									 vmc_leg_t * vmcl   ,\
						                 INS_t * ins     );

/* pub-sub handles */
static Publisher_t  *chassis_state_pub;
static Publisher_t  *leg_right_pub;
static Publisher_t  *leg_left_pub;
static Publisher_t  *actuator_cmd_pub;
static Subscriber_t *ins_sub;
static Subscriber_t *cmd_sub;
static Subscriber_t *observe_sub;
static Subscriber_t *actuator_feedback_sub;
static INS_t ins_runtime;

static void update_ins_runtime(INS_t *ins, const INS_Data_t *msg);
static void apply_chassis_cmd(chassis_t *chassis, const Chassis_Cmd_t *cmd);
static void apply_observe_state(chassis_t *chassis, const Chassis_Observe_t *observe);
static void pull_runtime_inputs(chassis_t *chassis,
                                INS_t *ins,
                                INS_Data_t *ins_msg,
                                Chassis_Cmd_t *cmd_msg,
                                Chassis_Observe_t *observe_msg,
                                Actuator_Feedback_t *feedback_msg);
static void publish_runtime_outputs(const chassis_t *chassis,
                                    const vmc_leg_t *right_leg,
                                    const vmc_leg_t *left_leg);

void Chassis_task(void)
{
    INS_Data_t ins_msg = {0};
    Chassis_Cmd_t cmd_msg = {0};
    Chassis_Observe_t observe_msg = {0};
    Actuator_Feedback_t feedback_msg = {0};

    ins_sub           = SubRegister("ins_data",      sizeof(INS_Data_t));
    cmd_sub           = SubRegister("chassis_cmd",   sizeof(Chassis_Cmd_t));
    observe_sub       = SubRegister("chassis_observe", sizeof(Chassis_Observe_t));
    actuator_feedback_sub = SubRegister("actuator_feedback", sizeof(Actuator_Feedback_t));

	while(ins_runtime.ins_flag == 0 || feedback_msg.ready == 0U)
	{
        pull_runtime_inputs(&chassis_runtime, &ins_runtime, &ins_msg, &cmd_msg, &observe_msg, &feedback_msg);
	    osDelay(1);
	}

    Chassis_init(&chassis_runtime,&runtime_right_leg,&runtime_left_leg);
    Pensation_init(&roll_pid,&tp_pid,&turn_pid,&leg_r_pid,&leg_l_pid);

    /* register pub-sub */
    chassis_state_pub = PubRegister("chassis_state", sizeof(Chassis_State_t));
    leg_right_pub     = PubRegister("leg_right",     sizeof(Leg_Output_t));
    leg_left_pub      = PubRegister("leg_left",      sizeof(Leg_Output_t));
    actuator_cmd_pub  = PubRegister("actuator_cmd",  sizeof(Actuator_Cmd_t));

    osDelay(6);

	while(1)
	{	
        pull_runtime_inputs(&chassis_runtime, &ins_runtime, &ins_msg, &cmd_msg, &observe_msg, &feedback_msg);

		chassis_feedback_update(&chassis_runtime,&runtime_right_leg,&runtime_left_leg,&ins_runtime,&feedback_msg);
	    chassis_control_loop(&chassis_runtime,&runtime_right_leg,&runtime_left_leg,&ins_runtime,LQR_K_R,LQR_K_R,&leg_r_pid,&leg_l_pid);
        publish_runtime_outputs(&chassis_runtime, &runtime_right_leg, &runtime_left_leg);

		osDelay(CHASSIS_TIME);
	}
}

static void update_ins_runtime(INS_t *ins, const INS_Data_t *msg)
{
    ins->Pitch = msg->pitch;
    ins->Roll = msg->roll;
    ins->YawTotalAngle = msg->yaw_total;
    ins->Gyro[0] = msg->gyro[0];
    ins->Gyro[1] = msg->gyro[1];
    ins->Gyro[2] = msg->gyro[2];
    ins->MotionAccel_b[0] = msg->accel_b[0];
    ins->MotionAccel_b[1] = msg->accel_b[1];
    ins->MotionAccel_b[2] = msg->accel_b[2];
    ins->ins_flag = msg->ready;
}

static void apply_chassis_cmd(chassis_t *chassis, const Chassis_Cmd_t *cmd)
{
    chassis->v_set = cmd->vx_cmd;
    chassis->turn_set = cmd->turn_cmd;
    chassis->leg_set = cmd->leg_set;
    chassis->start_flag = cmd->start_flag;
    chassis->jump_flag = cmd->jump_flag;
    chassis->recover_flag = cmd->recover_flag;
}

static void apply_observe_state(chassis_t *chassis, const Chassis_Observe_t *observe)
{
    chassis->v_filter = observe->v_filter;
    chassis->x_filter = observe->x_filter;
}

static void pull_runtime_inputs(chassis_t *chassis,
                                INS_t *ins,
                                INS_Data_t *ins_msg,
                                Chassis_Cmd_t *cmd_msg,
                                Chassis_Observe_t *observe_msg,
                                Actuator_Feedback_t *feedback_msg)
{
    if (SubGetMessage(ins_sub, ins_msg))
    {
        update_ins_runtime(ins, ins_msg);
    }
    if (SubGetMessage(cmd_sub, cmd_msg))
    {
        apply_chassis_cmd(chassis, cmd_msg);
    }
    if (SubGetMessage(observe_sub, observe_msg))
    {
        apply_observe_state(chassis, observe_msg);
    }
    SubGetMessage(actuator_feedback_sub, feedback_msg);
}

static void publish_runtime_outputs(const chassis_t *chassis,
                                    const vmc_leg_t *right_leg,
                                    const vmc_leg_t *left_leg)
{
    Chassis_State_t state_msg = {
        .v_filter  = chassis->v_filter,
        .x_filter  = chassis->x_filter,
        .x_set     = chassis->x_set,
        .total_yaw = chassis->total_yaw,
        .roll      = chassis->roll,
        .turn_set  = chassis->turn_set,
    };
    Leg_Output_t right_msg = {
        .joint_torque = {right_leg->torque_set[0], right_leg->torque_set[1]},
        .wheel_torque = chassis->wheel_motor[1].torque_set,
        .wheel_current = chassis->wheel_motor[1].give_current,
        .leg_length = right_leg->L0,
    };
    Leg_Output_t left_msg = {
        .joint_torque = {left_leg->torque_set[0], left_leg->torque_set[1]},
        .wheel_torque = chassis->wheel_motor[0].torque_set,
        .wheel_current = chassis->wheel_motor[0].give_current,
        .leg_length = left_leg->L0,
    };
    Actuator_Cmd_t actuator_msg = {
        .joint_torque = {
            right_leg->torque_set[0],
            right_leg->torque_set[1],
            left_leg->torque_set[0],
            left_leg->torque_set[1],
        },
        .wheel_current = {
            chassis->wheel_motor[0].give_current,
            chassis->wheel_motor[1].give_current,
        },
        .start_flag = chassis->start_flag,
    };

    PubPushMessage(chassis_state_pub, &state_msg);
    PubPushMessage(leg_right_pub, &right_msg);
    PubPushMessage(leg_left_pub,  &left_msg);
    PubPushMessage(actuator_cmd_pub, &actuator_msg);
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

	chassis_jump_loop(chassis,vmcr,vmcl,legr,legl);//有F0的计算过程
   
	chassis_ground_detection(chassis,vmcr,vmcl,ins);//离地检测
	
	VMC_calc_2(vmcr);//计算期望的关节输出力矩
	VMC_calc_2(vmcl);
	 
	chassis->wheel_motor[0].give_current = chassis->wheel_motor[0].torque_set * M3508_TORQUE_TO_CURRENT;
	chassis->wheel_motor[1].give_current = chassis->wheel_motor[1].torque_set * M3508_TORQUE_TO_CURRENT;

    chassis_saturate_outputs(chassis, vmcr, vmcl);
}

static void chassis_jump_loop(chassis_t * chassis,\
							  vmc_leg_t * vmcr   ,\
							  vmc_leg_t * vmcl   ,\
							 PidTypeDef * legr   ,\
							 PidTypeDef * legl	  )
{
	if(chassis->start_flag == 1)
	{
		if(chassis->jump_flag == 1)
		{
			if(chassis->jump_status == 0)
			{
				vmcr->F0= LEG_GRAVITY_COMP + PID_Calc(legr,vmcr->L0,0.18f);
				vmcl->F0= LEG_GRAVITY_COMP + PID_Calc(legl,vmcl->L0,0.18f);
				if(vmcr->L0<0.185f&&vmcl->L0<0.185f)//0.18
				{
					chassis->jump_time++;
				}
				if(chassis->jump_time>10)
				{
					chassis->jump_time = 0;
					chassis->jump_status = 1;
				}
			}else if(chassis->jump_status == 1)
			{
				vmcr->F0= LEG_GRAVITY_COMP + PID_Calc(legr,vmcr->L0,0.3f);//暂时定为0.30，测试跳跃
				vmcl->F0= LEG_GRAVITY_COMP + PID_Calc(legl,vmcl->L0,0.3f);
				if(vmcr->L0>0.22f&&vmcl->L0>0.22f)
				{
					chassis->jump_time++;					
				}
				if(chassis->jump_time>10)
				{
					chassis->jump_time = 0;
					chassis->jump_status = 2;
				}
			}else if(chassis->jump_status == 2)
			{
				vmcr->F0= LEG_GRAVITY_COMP + PID_Calc(legr,vmcr->L0,0.18f);//右前馈+pd
				vmcl->F0= LEG_GRAVITY_COMP + PID_Calc(legl,vmcl->L0,0.18f);//左前馈+pd
				if(vmcr->L0<0.250f&&vmcl->L0<0.250f)
				{
					chassis->jump_time++;			
				}
				if(chassis->jump_time>10)
				{
					chassis->jump_time = 0;
					chassis->jump_status = 3;
				}
//收腿的两个逻辑，在状态2最后加一个时间判断回到设定值，或者在3状态加一个腿长判断，或者判断支持力				
			}else if(chassis->jump_status == 3)
			{
				vmcr->F0= LEG_GRAVITY_COMP + PID_Calc(legr,vmcr->L0,chassis->leg_set);//右前馈+pd
				vmcl->F0= LEG_GRAVITY_COMP + PID_Calc(legl,vmcl->L0,chassis->leg_set);//左前馈+pd
//				vmcr->F0= leg_G + PID_Calc(legr,vmcr->L0,0.13);
//				vmcl->F0= leg_G + PID_Calc(legl,vmcl->L0,0.13);
//				if(vmcr->L0 < 0.17 && vmcl->L0 < 0.17)
//				{
//				vmcr->F0= leg_G + PID_Calc(legr,vmcr->L0,chassis->leg_set);//右前馈+pd
//				vmcl->F0= leg_G + PID_Calc(legl,vmcl->L0,chassis->leg_set);//左前馈+pd
//				}
			}
		}else if(chassis->jump_flag == 0)
		{
			vmcr->F0= LEG_GRAVITY_COMP + PID_Calc(legr,vmcr->L0,chassis->leg_set)+ chassis->roll_f0;//右前馈+pd
			vmcl->F0= LEG_GRAVITY_COMP + PID_Calc(legl,vmcl->L0,chassis->leg_set)- chassis->roll_f0;//左前馈+pd
			
			chassis->jump_time = 0;
			chassis->jump_status = 0;			
		}
	}
}

static void chassis_ground_detection(chassis_t * chassis,\
									 vmc_leg_t * vmcr   ,\
									 vmc_leg_t * vmcl   ,\
						                 INS_t * ins     )
{
	static uint8_t left_flag  = 0;
	static uint8_t right_flag = 0;
	
	right_flag = ground_detectionR(vmcr,ins);//右腿离地检测
	left_flag  = ground_detectionL(vmcl,ins);
   
	if(chassis->recover_flag==0)		
	{//倒地自起不需要检测是否离地	 
		if(right_flag==1&&left_flag==1)
		{//当两腿同时离地并且遥控器没有在控制腿的伸缩时，才认为离地
			chassis->wheel_motor[0].torque_set=0.0f;
			chassis->wheel_motor[1].torque_set=0.0f;
			//vmcr->Tp=(LQR_K_R[6]+1)*(vmcr->theta-0.0f)+ (LQR_K_R[7]+0.2)*(vmcr->d_theta-0.0f); //保证机体与腿部垂直(测试是否可以)
			chassis->x_filter=0.0f;
			chassis->x_set=chassis->x_filter;
			chassis->turn_set=chassis->total_yaw;
			vmcr->Tp=vmcr->Tp+chassis->leg_tp;
      chassis->text_jump_true = 1;			
		}else
		{
		chassis->text_jump_true = 0;
		}
	}else if(chassis->recover_flag==1)
	{
		 vmcr->Tp=0.0f;
		 vmcl->Tp=0.0f;
	}
	
}
