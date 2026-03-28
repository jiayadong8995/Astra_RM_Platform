#include "rc_input_bridge.h"

#include <string.h>

#include "cmsis_os.h"
#include "message_center.h"
#include "remote_control.h"
#include "robot_def.h"

void rc_input_task(void)
{
    Publisher_t *rc_pub = PubRegister("rc_data", sizeof(RC_Data_t));

    while (1)
    {
        const RC_ctrl_t *rc = get_remote_control_point();
        RC_Data_t rc_msg = {0};

        if (rc != NULL && !RC_data_is_error())
        {
            memcpy(rc_msg.ch, rc->rc.ch, sizeof(rc_msg.ch));
            rc_msg.sw[0] = (uint8_t)rc->rc.s[0];
            rc_msg.sw[1] = (uint8_t)rc->rc.s[1];
            rc_msg.online = 1U;
        }

        PubPushMessage(rc_pub, &rc_msg);
        osDelay(REMOTE_TASK_PERIOD);
    }
}
