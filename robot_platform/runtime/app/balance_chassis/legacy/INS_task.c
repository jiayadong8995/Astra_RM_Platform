/**
  *********************************************************************
  * @file      ins_task.c/h
  * @brief     该任务是用mahony方法获取机体姿态，同时获取机体在绝对坐标系下的运动加速度
  * @note       
  * @history
  *
  @verbatim
  ==============================================================================

  ==============================================================================
  @endverbatim
  *********************************************************************
  */
	
#include "INS_task.h"
#include "bsp_PWM.h"
#include "bsp_dwt.h"
#include "cmsis_os.h"
#include "../app_config/app_params.h"
#include "../app_config/robot_def.h"
#include "../app_flow/ins_runtime.h"
#include "../app_io/ins_topics.h"

static INS_Runtime_State_t runtime_state;
static INS_Runtime_Bus_t runtime_bus;

void INS_Init(void)
{ 
   ins_runtime_state_init(&runtime_state);
   ins_runtime_bus_init(&runtime_bus);
}

void INS_task(void)
{
     INS_Data_t msg = {0};
     float ins_dt = 0.0f;
	 INS_Init();
	 
	 while(1)
	 {  
		ins_dt = DWT_GetDeltaT(&runtime_state.dwt_count);
        BMI088_Read(&BMI088);
        ins_runtime_apply_sample(&runtime_state, ins_dt, BMI088.Accel, BMI088.Gyro);
        ins_runtime_build_msg(&runtime_state, &msg);
        ins_runtime_bus_publish(&runtime_bus, &msg);

		osDelay(INS_TASK_PERIOD_MS);
	}
} 
