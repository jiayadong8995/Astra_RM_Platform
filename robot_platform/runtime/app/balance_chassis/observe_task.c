/**
  *********************************************************************
  * @file      observe_task.c/h
  * @brief     该任务是对机体运动速度估计，用于抑制打滑
  * @note       
  * @history
  *
  @verbatim
  ==============================================================================

  ==============================================================================
  @endverbatim
  *********************************************************************
  */
	
#include "observe_task.h"
#include "kalman_filter.h"
#include "cmsis_os.h"
#include "app_config/robot_def.h"
#include "message_center.h"

KalmanFilter_t vaEstimateKF;	   // 卡尔曼滤波器结构体

float vaEstimateKF_F[4] = {1.0f, 0.003f, 
                           0.0f, 1.0f};	   // 状态转移矩阵，控制周期为0.001s

float vaEstimateKF_P[4] = {1.0f, 0.0f,
                           0.0f, 1.0f};    // 后验估计协方差初始值

float vaEstimateKF_Q[4] = {0.1f, 0.0f, 
                           0.0f, 0.1f};    // Q矩阵初始值

float vaEstimateKF_R[4] = {100.0f, 0.0f, 
                            0.0f,  100.0f}; 	
														
float vaEstimateKF_K[4];
													 
const float vaEstimateKF_H[4] = {1.0f, 0.0f,
                                 0.0f, 1.0f};	// 设置矩阵H为常量
														 															 
float vel_acc[2]; 
uint32_t OBSERVE_TIME=OBSERVE_TASK_PERIOD;															 
void Observe_task(void)
{
    Publisher_t *observe_pub;
    Subscriber_t *cmd_sub;
    Subscriber_t *feedback_sub;
    Chassis_Cmd_t cmd_msg = {0};
    Actuator_Feedback_t feedback_msg = {0};
    float observe_v_filter = 0.0f;
    float observe_x_filter = 0.0f;

    Subscriber_t *ins_sub;
    INS_Data_t ins_msg = {0};
    uint8_t ins_ready = 0U;

    ins_sub = SubRegister("ins_data", sizeof(INS_Data_t));

	while(ins_ready == 0U)
	{//等待加速度收敛
      if (SubGetMessage(ins_sub, &ins_msg))
      {
          ins_ready = ins_msg.ready;
      }
	  osDelay(1);	
	}
	
	xvEstimateKF_Init(&vaEstimateKF);
    observe_pub = PubRegister("chassis_observe", sizeof(Chassis_Observe_t));
    cmd_sub = SubRegister("chassis_cmd", sizeof(Chassis_Cmd_t));
    feedback_sub = SubRegister("actuator_feedback", sizeof(Actuator_Feedback_t));
	
	while(1)
	{  
        SubGetMessage(cmd_sub, &cmd_msg);
        SubGetMessage(feedback_sub, &feedback_msg);
		// 当前先保留最简单的轮速估计边界，不再依赖 legacy 全局状态和腿模型中间量。
		observe_v_filter = -(feedback_msg.wheel_speed[1] - feedback_msg.wheel_speed[0])/2.0f;
		observe_x_filter = observe_x_filter + observe_v_filter * ((float)OBSERVE_TIME/1000.0f);
		// 历史逻辑中的 last_leg_set 当前未被维护，这里只保留 recover 模式下不清零的位置估计。
		if(cmd_msg.recover_flag != 0)
		{
			observe_x_filter = 0.0f;
		}

        {
            Chassis_Observe_t observe_msg = {
                .v_filter = observe_v_filter,
                .x_filter = observe_x_filter,
            };
            PubPushMessage(observe_pub, &observe_msg);
        }
		
		
		osDelay(OBSERVE_TIME);
	}
}

void xvEstimateKF_Init(KalmanFilter_t *EstimateKF)
{
    Kalman_Filter_Init(EstimateKF, 2, 0, 2);	// 状态向量2维 没有控制量 测量向量2维
	
	memcpy(EstimateKF->F_data, vaEstimateKF_F, sizeof(vaEstimateKF_F));
    memcpy(EstimateKF->P_data, vaEstimateKF_P, sizeof(vaEstimateKF_P));
    memcpy(EstimateKF->Q_data, vaEstimateKF_Q, sizeof(vaEstimateKF_Q));
    memcpy(EstimateKF->R_data, vaEstimateKF_R, sizeof(vaEstimateKF_R));
    memcpy(EstimateKF->H_data, vaEstimateKF_H, sizeof(vaEstimateKF_H));

}

void xvEstimateKF_Update(KalmanFilter_t *EstimateKF ,float acc,float vel)
{   	
    //卡尔曼滤波器测量值更新
    EstimateKF->MeasuredVector[0] =	vel;//测量速度
    EstimateKF->MeasuredVector[1] = acc;//测量加速度
    		
    //卡尔曼滤波器更新函数
    Kalman_Filter_Update(EstimateKF);

    // 提取估计值
    for (uint8_t i = 0; i < 2; i++)
    {
      vel_acc[i] = EstimateKF->FilteredValue[i];
    }
}
