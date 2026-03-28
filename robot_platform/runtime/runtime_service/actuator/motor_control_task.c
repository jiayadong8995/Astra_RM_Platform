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
#include "cmsis_os.h"
#include "../../app/balance_chassis/app_config/app_params.h"
#include "../../app/balance_chassis/app_config/robot_def.h"
#include "actuator_runtime.h"
#include "actuator_topics.h"

static uint32_t systick;
				
void motor_control_task(void)
{
    Actuator_Runtime_t runtime = {0};
    Actuator_Runtime_Bus_t runtime_bus = {0};
    INS_Data_t ins_msg = {0};
    Actuator_Cmd_t actuator_msg = {0};
    Actuator_Feedback_t feedback_msg = {0};
    actuator_runtime_bus_init(&runtime_bus);
    actuator_runtime_bus_wait_ready(&runtime_bus, &ins_msg);
    actuator_runtime_init(&runtime);
    osDelay(APP_CHASSIS_STARTUP_DELAY_MS);

	while(1)
	{	
        actuator_runtime_bus_pull_cmd(&runtime_bus, &actuator_msg);
        actuator_runtime_capture_feedback(&runtime, &feedback_msg);
        actuator_runtime_bus_publish_feedback(&runtime_bus, &feedback_msg);
		systick = osKernelSysTick();
        actuator_runtime_dispatch_command(&runtime, &actuator_msg, systick);
		osDelay(MOTOR_CONTROL_TASK_PERIOD_MS);
	}
}
