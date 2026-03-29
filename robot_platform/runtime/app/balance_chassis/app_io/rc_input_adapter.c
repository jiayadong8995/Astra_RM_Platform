#include "rc_input_adapter.h"

#include <string.h>

#include "cmsis_os.h"
#include "message_center.h"
#include "../app_config/robot_def.h"
#include "../app_config/app_params.h"
#include "../../../device/device_layer.h"

void rc_input_adapter_task(void)
{
    Publisher_t *rc_pub = PubRegister("rc_data", sizeof(RC_Data_t));

    while (1)
    {
        platform_rc_input_t input = {0};
        RC_Data_t rc_msg = {0};

        if (platform_device_read_default_remote(&input) == PLATFORM_DEVICE_RESULT_OK && input.valid)
        {
            memcpy(rc_msg.ch, input.channels, sizeof(rc_msg.ch));
            rc_msg.sw[0] = input.switches[0];
            rc_msg.sw[1] = input.switches[1];
            rc_msg.online = 1U;
        }

        PubPushMessage(rc_pub, &rc_msg);
        osDelay(REMOTE_TASK_PERIOD_MS);
    }
}
