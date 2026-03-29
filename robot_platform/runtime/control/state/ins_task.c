#include "ins_task.h"

#include "bsp_PWM.h"
#include "bsp_dwt.h"
#include "cmsis_os.h"
#include "../../app/balance_chassis/app_config/app_params.h"
#include "../../app/balance_chassis/app_config/robot_def.h"
#include "../../device/device_layer.h"
#include "ins_state_estimator.h"
#include "ins_topics.h"

static platform_ins_state_estimator_t runtime_state;
static platform_ins_bus_t runtime_bus;

void INS_Init(void)
{
   platform_ins_state_estimator_init(&runtime_state);
   platform_ins_bus_init(&runtime_bus);
}

void INS_task(void)
{
     INS_Data_t msg = {0};
     platform_imu_sample_t sample = {0};
     float ins_dt = 0.0f;
     INS_Init();

     while(1)
     {
        ins_dt = DWT_GetDeltaT(&runtime_state.dwt_count);
        if (platform_device_read_default_imu(&sample) == PLATFORM_DEVICE_RESULT_OK && sample.valid)
        {
            platform_ins_state_estimator_apply_sample(&runtime_state, ins_dt, sample.accel, sample.gyro);
        }
        platform_ins_state_estimator_build_msg(&runtime_state, &msg);
        platform_ins_bus_publish(&runtime_bus, &msg);

        osDelay(INS_TASK_PERIOD_MS);
     }
}
