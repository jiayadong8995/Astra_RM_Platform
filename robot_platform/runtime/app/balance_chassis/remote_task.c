/**
  *********************************************************************
  * @file      remote_task.c/h
  * @brief     该任务是读取并处理遥控数据，
	*            将遥控数据转化为期望的速度、期望的转角、期望的腿长等
  * @note       
  * @history
  *
  @verbatim
  ==============================================================================

  ==============================================================================
  @endverbatim
  *********************************************************************
  */
	
#include "remote_task.h"
#include "cmsis_os.h"
#include "control/remote_runtime.h"
#include "control/remote_runtime_helpers.h"
#include "io/remote_topics.h"

void remote_task(void)
{	
    Remote_Runtime_t cmd_state = {0};
    Remote_Runtime_Bus_t runtime_bus = {0};
    RC_Data_t rc_msg = {0};
    INS_Data_t ins_msg = {0};
    Chassis_State_t state_msg = {0};
    Leg_Output_t right_msg = {0};
    Leg_Output_t left_msg = {0};
    Chassis_Cmd_t cmd_msg = {0};

    remote_runtime_init(&cmd_state);
    remote_runtime_bus_init(&runtime_bus);
	while(1)
	{	
        remote_runtime_bus_pull_inputs(&runtime_bus, &rc_msg, &ins_msg, &state_msg, &right_msg, &left_msg);
        remote_runtime_apply_inputs(&cmd_state, &rc_msg, &ins_msg, &state_msg);
        remote_runtime_limit_leg_set(&cmd_state, &right_msg, &left_msg);
        cmd_msg = remote_runtime_build_cmd(&cmd_state);
        remote_runtime_bus_publish_cmd(&runtime_bus, &cmd_msg);

		osDelay(REMOTE_TASK_PERIOD);
		osDelay(50);
	}
}
