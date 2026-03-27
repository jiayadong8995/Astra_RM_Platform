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

//-15.2927   -1.2844   -7.4831   -4.5426   13.4664    1.9690
//   22.4682    2.6679   22.1886   12.1150   34.9348    2.9753
//-14.5354   -1.1578   -7.1281   -4.3071   14.0014    2.0200
//   22.7326    2.6070   23.0702   12.5308   33.2775    2.7684（0.16）
//-17.7188   -1.7148   -8.4302   -5.1792   11.7744    1.7642
//   21.5827    2.8661   19.4008   10.7767   39.3566    3.5794（0.22）
//-18.0188,   -2.0148,   -9.9302,   -6.5892,   11.7744,    1.7642,
//  21.5827,    2.8661,   20.2008,   11.2767,   40.3566,    4.0794（0.22较完善）
static float LQR_K_R[12]= LQR_K_MATRIX;
//{-13.1027, -1.0044, -11.8831, -8.8426, 14.0664, 2.0690,
//20.0682, 2.0679, 26.7886, 15.9150, 33.4348, 3.2053};
////{
//    -17.4503,   -1.9640,   -4.8365,   -4.4086,    6.2068,    1.4190,
//     14.0360,    2.1759,    7.1015,    5.9832,   16.9087,    2.6989
//};//0.20
vmc_leg_t right;
vmc_leg_t left;

extern INS_t INS;

int enable_flag[4]= {1,1,1,1};
int Begin_flag = 0;

float leg_G = LEG_GRAVITY_COMP;
float Joint_T_Max = JOINT_TORQUE_MAX;
																
chassis_t chassis_move;
															
PidTypeDef LegR_Pid;//右腿的腿长pd
PidTypeDef LegL_Pid;//左腿的腿长pd
PidTypeDef Tp_Pid;//防劈叉补偿pd
PidTypeDef Turn_Pid;//转向pd
PidTypeDef Roll_Pid;//横滚角补偿pd

uint32_t CHASSIS_TIME=CHASSIS_TASK_PERIOD;

static void Chassis_init(chassis_t *chassis,vmc_leg_t *vmcr_init,vmc_leg_t *vmcl_init);
static void Pensation_init(PidTypeDef *roll,PidTypeDef *Tp,PidTypeDef *turn,PidTypeDef *legr,PidTypeDef *legl);
static void chassis_feedback_update(chassis_t *chassis,vmc_leg_t *vmc_r,vmc_leg_t *vmc_l,INS_t *ins);
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
static void Limit_Int(int16_t *in,int16_t min,int16_t max);
static void chassis_ground_detection(chassis_t * chassis,\
									 vmc_leg_t * vmcr   ,\
									 vmc_leg_t * vmcl   ,\
						                 INS_t * ins     );

/* pub-sub handles */
static Publisher_t  *chassis_state_pub;
static Publisher_t  *leg_right_pub;
static Publisher_t  *leg_left_pub;
static Subscriber_t *ins_sub;
static Subscriber_t *cmd_sub;

void Chassis_task(void)
{
	while(INS.ins_flag==0)
	{// wait for IMU convergence
	  osDelay(1);	
	}

    Chassis_init(&chassis_move,&right,&left);
    Pensation_init(&Roll_Pid,&Tp_Pid,&Turn_Pid,&LegR_Pid,&LegL_Pid);

    /* register pub-sub */
    chassis_state_pub = PubRegister("chassis_state", sizeof(Chassis_State_t));
    leg_right_pub     = PubRegister("leg_right",     sizeof(Leg_Output_t));
    leg_left_pub      = PubRegister("leg_left",      sizeof(Leg_Output_t));
    ins_sub           = SubRegister("ins_data",      sizeof(INS_Data_t));
    cmd_sub           = SubRegister("chassis_cmd",   sizeof(Chassis_Cmd_t));

    osDelay(6);

	while(1)
	{	
		chassis_feedback_update(&chassis_move,&right,&left,&INS);
	    chassis_control_loop(&chassis_move,&right,&left,&INS,LQR_K_R,LQR_K_R,&LegR_Pid,&LegL_Pid);

		/* publish chassis state */
		{
			Chassis_State_t state_msg = {
				.v_filter  = chassis_move.v_filter,
				.x_filter  = chassis_move.x_filter,
				.x_set     = chassis_move.x_set,
				.total_yaw = chassis_move.total_yaw,
				.roll      = chassis_move.roll,
				.turn_set  = chassis_move.turn_set,
			};
			PubPushMessage(chassis_state_pub, &state_msg);
		}
		/* publish leg outputs for motor_control_task */
		{
			Leg_Output_t r_msg = {
				.joint_torque = {right.torque_set[0], right.torque_set[1]},
				.wheel_torque = chassis_move.wheel_motor[1].torque_set,
				.wheel_current = chassis_move.wheel_motor[1].give_current,
			};
			Leg_Output_t l_msg = {
				.joint_torque = {left.torque_set[0], left.torque_set[1]},
				.wheel_torque = chassis_move.wheel_motor[0].torque_set,
				.wheel_current = chassis_move.wheel_motor[0].give_current,
			};
			PubPushMessage(leg_right_pub, &r_msg);
			PubPushMessage(leg_left_pub,  &l_msg);
		}

		osDelay(CHASSIS_TIME);
	}
}


static void Chassis_init(chassis_t *chassis,vmc_leg_t *vmcr_init,vmc_leg_t *vmcl_init)
{
	for(int i = 0;i < 2; i++)
	{
		chassis_move.wheel_motor[i].chassis_x = 0.0f;
		chassis_move.wheel_motor[i].chassis_motor_measure = get_chassis_motor_measure_point(i);
	}  
	VMC_init(vmcr_init);//给杆长赋值
	VMC_init(vmcl_init);//给杆长赋值
	
	chassis_feedback_update(&chassis_move,&right,&left,&INS);//更新数据

	Begin_flag =1;
}

static void Pensation_init(PidTypeDef *roll,PidTypeDef *Tp,PidTypeDef *turn,PidTypeDef *legr,PidTypeDef *legl)
{
	//腿长pid初始化
	const static float roll_pid[3] = {ROLL_PID_KP,ROLL_PID_KI,ROLL_PID_KD};
    const static float legr_pid[3] = {LEG_PID_KP, LEG_PID_KI,LEG_PID_KD};
    const static float legl_pid[3] = {LEG_PID_KP, LEG_PID_KI,LEG_PID_KD};
    //补偿pid初始化：防劈叉补偿、偏航角补偿
	const static float tp_pid[3] = {TP_PID_KP, TP_PID_KI, TP_PID_KD};
	const static float turn_pid[3] = {TURN_PID_KP, TURN_PID_KI, TURN_PID_KD};
	
    PID_init(roll, PID_POSITION, roll_pid, ROLL_PID_MAX_OUT, ROLL_PID_MAX_IOUT);
	PID_init(Tp,   PID_POSITION, tp_pid,   TP_PID_MAX_OUT,   TP_PID_MAX_IOUT);
	PID_init(turn, PID_POSITION, turn_pid, TURN_PID_MAX_OUT, TURN_PID_MAX_IOUT);

	PID_init(legr, PID_POSITION,legr_pid, LEG_PID_MAX_OUT, LEG_PID_MAX_IOUT);//腿长pid
	PID_init(legl, PID_POSITION,legl_pid, LEG_PID_MAX_OUT, LEG_PID_MAX_IOUT);//腿长pid
}

static void chassis_feedback_update(chassis_t *chassis,vmc_leg_t *vmc_r,vmc_leg_t *vmc_l,INS_t *ins)
{
    vmc_r->phi1=pi/2.0f+chassis->joint_motor[0].para.pos+JOINT0_OFFSET;
	vmc_r->phi4=pi/2.0f+chassis->joint_motor[1].para.pos+JOINT1_OFFSET;
	vmc_l->phi1=pi/2.0f+chassis->joint_motor[2].para.pos+JOINT2_OFFSET;
	vmc_l->phi4=pi/2.0f+chassis->joint_motor[3].para.pos+JOINT3_OFFSET;

	chassis_move.wheel_motor[0].chassis_x = chassis_move.wheel_motor[0].chassis_motor_measure->total_angle / WHEEL_GEAR_RATIO * WHEEL_RADIUS;		
	chassis_move.wheel_motor[1].chassis_x = chassis_move.wheel_motor[1].chassis_motor_measure->total_angle / WHEEL_GEAR_RATIO * WHEEL_RADIUS;

	chassis_move.wheel_motor[0].w_speed = chassis_move.wheel_motor[0].chassis_motor_measure->speed_rpm * M3508_RPM_TO_RADS; 		
	chassis_move.wheel_motor[1].w_speed = chassis_move.wheel_motor[1].chassis_motor_measure->speed_rpm * M3508_RPM_TO_RADS; 
	
	chassis_move.wheel_motor[0].speed = chassis_move.wheel_motor[0].w_speed * WHEEL_RADIUS;		
	chassis_move.wheel_motor[1].speed = chassis_move.wheel_motor[1].w_speed * WHEEL_RADIUS;		

  chassis->myPithGyroL = - ins->Gyro[0];
	chassis->myPithL = - ins->Pitch;//-0.05 //+0.05
	chassis->myPithR = ins->Pitch;//0.05
	chassis->myPithGyroR = ins->Gyro[0];
	
	chassis->total_yaw = ins->YawTotalAngle;
	chassis->roll = ins->Roll;
	chassis->theta_err = 0.0f-(vmc_r->theta+vmc_l->theta);
	
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
	
  chassis->turn_T=Turn_Pid.Kp*(chassis->turn_set-chassis->total_yaw)-Turn_Pid.Kd*ins->Gyro[2];//这样计算更稳一点
	
	chassis->roll_f0=Roll_Pid.Kp*(chassis->roll_set-chassis->roll)-Roll_Pid.Kd*ins->Gyro[1];
	
    chassis->leg_tp = PID_Calc(&Tp_Pid, chassis->theta_err,0.0f);//防劈叉pid计算

	chassis->wheel_motor[1].torque_set=(LQR_KR[0]*(vmcr->theta) 
									   +LQR_KR[1]*(vmcr->d_theta) 
									   +LQR_KR[2]*(chassis->x_filter - chassis->x_set)
									   +LQR_KR[3]*(chassis->v_filter - 0)
									   +LQR_KR[4]*(chassis->myPithR-0.0f)
									   +LQR_KR[5]*(chassis->myPithGyroR-0.0f));					
	//右边髋关节输出力矩				
	vmcr->Tp=(LQR_KR[6]*(vmcr->theta)
			 +LQR_KR[7]*(vmcr->d_theta)
			 +LQR_KR[8]*( chassis->x_filter - chassis->x_set) 
			 +LQR_KR[9]*( chassis->v_filter - 0) 
			 +LQR_KR[10]*(chassis->myPithR)
			 +LQR_KR[11]*(chassis->myPithGyroR));
					
					
					
					
				
	//左边髋轮毂关节输出力矩
    chassis->wheel_motor[0].torque_set=(LQR_KL[0]*(vmcl->theta ) 
									   +LQR_KL[1]*(vmcl->d_theta)
									   +LQR_KL[2]*(chassis->x_set - chassis->x_filter)
									   +LQR_KL[3]*(0 - chassis->v_filter)
									   +LQR_KL[4]*(chassis->myPithL-0.0f)
									   +LQR_KL[5]*(chassis->myPithGyroL-0.0f));
	//左边髋关节输出力矩				
	vmcl->Tp=(LQR_KL[6] *(vmcl->theta)
			 +LQR_KL[7] *(vmcl->d_theta)
			 +LQR_KL[8] *(chassis->x_set - chassis->x_filter)
			 +LQR_KL[9] *(0 - chassis->v_filter)
			 +LQR_KL[10]*(chassis->myPithL)
			 +LQR_KL[11]*(chassis->myPithGyroL));

					
   //右轮毂电机扭矩设定
   for(int i=0;i<2;i++)
   {
		chassis->wheel_motor[i].torque_set= WHEEL_TORQUE_RATIO * chassis->wheel_motor[i].torque_set+TURN_TORQUE_RATIO * chassis->turn_T;	//轮毂电机输出力矩
		mySaturate(&chassis->wheel_motor[i].torque_set,-WHEEL_TORQUE_MAX,WHEEL_TORQUE_MAX);	
		Limit_Int(&chassis->wheel_motor[i].give_current,-8000,8000);
   }

    vmcr->Tp = vmcr->Tp+chassis->leg_tp;//右髋关节输出力矩
	vmcl->Tp = vmcl->Tp+chassis->leg_tp;//左髋关节输出力矩


	chassis->roll_f0 = 0;

	chassis_jump_loop(chassis,vmcr,vmcl,legr,legl);//有F0的计算过程
   
	chassis_ground_detection(chassis,vmcr,vmcl,ins);//离地检测
	
	VMC_calc_2(vmcr);//计算期望的关节输出力矩
	VMC_calc_2(vmcl);
	 
	chassis->wheel_motor[0].give_current = chassis->wheel_motor[0].torque_set * M3508_TORQUE_TO_CURRENT;
	chassis->wheel_motor[1].give_current = chassis->wheel_motor[1].torque_set * M3508_TORQUE_TO_CURRENT;
	 
	//额定扭矩
	mySaturate(&vmcr->F0,-100.0f,100.0f);//限幅 
    mySaturate(&vmcr->torque_set[1],-Joint_T_Max,Joint_T_Max);//3	
	mySaturate(&vmcr->torque_set[0],-Joint_T_Max,Joint_T_Max);
	
	mySaturate(&vmcl->F0,-100.0f,100.0f);//限幅
	mySaturate(&vmcl->torque_set[1],-Joint_T_Max,Joint_T_Max);//3
	mySaturate(&vmcl->torque_set[0],-Joint_T_Max,Joint_T_Max);
	
}

static void Limit_Int(int16_t *in,int16_t min,int16_t max)
{
  if(*in < min)
  {
    *in = min;
  }
  else if(*in > max)
  {
    *in = max;
  }
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
				vmcr->F0= leg_G + PID_Calc(legr,vmcr->L0,0.18f);
				vmcl->F0= leg_G + PID_Calc(legl,vmcl->L0,0.18f);
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
				vmcr->F0= leg_G + PID_Calc(legr,vmcr->L0,0.3f);//暂时定为0.30，测试跳跃
				vmcl->F0= leg_G + PID_Calc(legl,vmcl->L0,0.3f);
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
				vmcr->F0= leg_G + PID_Calc(legr,vmcr->L0,0.18f);//右前馈+pd
				vmcl->F0= leg_G + PID_Calc(legl,vmcl->L0,0.18f);//左前馈+pd
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
				vmcr->F0= leg_G + PID_Calc(legr,vmcr->L0,chassis->leg_set);//右前馈+pd
				vmcl->F0= leg_G + PID_Calc(legl,vmcl->L0,chassis->leg_set);//左前馈+pd
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
			vmcr->F0= leg_G + PID_Calc(legr,vmcr->L0,chassis->leg_set)+ chassis->roll_f0;//右前馈+pd
			vmcl->F0= leg_G + PID_Calc(legl,vmcl->L0,chassis->leg_set)- chassis->roll_f0;//左前馈+pd
			
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
