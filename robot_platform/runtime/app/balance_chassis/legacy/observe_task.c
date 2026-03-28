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
#include "cmsis_os.h"
#include "../app_config/robot_def.h"
#include "../app_io/observe_topics.h"
#include "../app_flow/observe_runtime.h"
#include "../app_flow/observe_runtime_helpers.h"

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
uint32_t OBSERVE_TIME = OBSERVE_TASK_PERIOD;

void Observe_task(void)
{
    Observe_Runtime_t runtime = {0};
    Observe_Runtime_Bus_t runtime_bus = {0};
    INS_Data_t ins_msg = {0};
    Chassis_Cmd_t cmd_msg = {0};
    Actuator_Feedback_t feedback_msg = {0};
    Chassis_Observe_t observe_msg = {0};

    observe_runtime_init(&runtime);
    observe_runtime_bus_init(&runtime_bus);
    observe_runtime_bus_wait_ready(&runtime_bus, &ins_msg);

	xvEstimateKF_Init(&vaEstimateKF);

	while(1)
	{
        observe_runtime_bus_pull_inputs(&runtime_bus, &cmd_msg, &feedback_msg);
        observe_runtime_apply_inputs(&runtime, &cmd_msg, &feedback_msg, (float)OBSERVE_TIME / 1000.0f);
        observe_msg = observe_runtime_build_output(&runtime);
        observe_runtime_bus_publish(&runtime_bus, &observe_msg);

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
