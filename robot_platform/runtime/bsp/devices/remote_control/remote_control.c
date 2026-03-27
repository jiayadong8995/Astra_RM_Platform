/**
 ******************************************************************************
 * @file    remote_control.c
 ******************************************************************************
 */
#include "remote_control.h"

#include <string.h>

#include "bsp_uart.h"
#include "main.h"

#define RC_CHANNAL_ERROR_VALUE 700

static int16_t RC_abs(int16_t value);

RC_ctrl_t rc_ctrl;
uint8_t sbus_rx_buf[RC_FRAME_LENGTH];

const RC_ctrl_t *get_remote_control_point(void)
{
    return &rc_ctrl;
}

uint8_t RC_data_is_error(void)
{
    if (RC_abs(rc_ctrl.rc.ch[0]) > RC_CHANNAL_ERROR_VALUE)
    {
        goto error;
    }
    if (RC_abs(rc_ctrl.rc.ch[1]) > RC_CHANNAL_ERROR_VALUE)
    {
        goto error;
    }
    if (RC_abs(rc_ctrl.rc.ch[2]) > RC_CHANNAL_ERROR_VALUE)
    {
        goto error;
    }
    if (RC_abs(rc_ctrl.rc.ch[3]) > RC_CHANNAL_ERROR_VALUE)
    {
        goto error;
    }
    if (rc_ctrl.rc.s[0] == 0)
    {
        goto error;
    }
    if (rc_ctrl.rc.s[1] == 0)
    {
        goto error;
    }
    return 0;

error:
    rc_ctrl.rc.ch[0] = 0;
    rc_ctrl.rc.ch[1] = 0;
    rc_ctrl.rc.ch[2] = 0;
    rc_ctrl.rc.ch[3] = 0;
    rc_ctrl.rc.ch[4] = 0;
    rc_ctrl.rc.s[0] = RC_SW_DOWN;
    rc_ctrl.rc.s[1] = RC_SW_DOWN;
    rc_ctrl.mouse.x = 0;
    rc_ctrl.mouse.y = 0;
    rc_ctrl.mouse.z = 0;
    rc_ctrl.mouse.press_l = 0;
    rc_ctrl.mouse.press_r = 0;
    rc_ctrl.key.v = 0;
    return 1;
}

static int16_t RC_abs(int16_t value)
{
    return (value > 0) ? value : -value;
}

void sbus_to_rc(volatile const uint8_t *sbus_buf_local, RC_ctrl_t *rc_ctrl_local)
{
    if (sbus_buf_local == NULL || rc_ctrl_local == NULL)
    {
        return;
    }

    rc_ctrl_local->rc.ch[0] = (sbus_buf_local[0] | (sbus_buf_local[1] << 8)) & 0x07ff;
    rc_ctrl_local->rc.ch[1] = ((sbus_buf_local[1] >> 3) | (sbus_buf_local[2] << 5)) & 0x07ff;
    rc_ctrl_local->rc.ch[2] = ((sbus_buf_local[2] >> 6) | (sbus_buf_local[3] << 2) |
                              (sbus_buf_local[4] << 10)) & 0x07ff;
    rc_ctrl_local->rc.ch[3] = ((sbus_buf_local[4] >> 1) | (sbus_buf_local[5] << 7)) & 0x07ff;
    rc_ctrl_local->rc.s[0] = ((sbus_buf_local[5] >> 4) & 0x0003);
    rc_ctrl_local->rc.s[1] = ((sbus_buf_local[5] >> 4) & 0x000C) >> 2;
    rc_ctrl_local->mouse.x = sbus_buf_local[6] | (sbus_buf_local[7] << 8);
    rc_ctrl_local->mouse.y = sbus_buf_local[8] | (sbus_buf_local[9] << 8);
    rc_ctrl_local->mouse.z = sbus_buf_local[10] | (sbus_buf_local[11] << 8);
    rc_ctrl_local->mouse.press_l = sbus_buf_local[12];
    rc_ctrl_local->mouse.press_r = sbus_buf_local[13];
    rc_ctrl_local->key.v = sbus_buf_local[14] | (sbus_buf_local[15] << 8);
    rc_ctrl_local->rc.ch[4] = sbus_buf_local[16] | (sbus_buf_local[17] << 8);

    if (rc_ctrl_local->rc.ch[0] <= 5 && rc_ctrl_local->rc.ch[0] >= -5)
    {
        rc_ctrl_local->rc.ch[0] = 0;
    }
    if (rc_ctrl_local->rc.ch[1] <= 5 && rc_ctrl_local->rc.ch[1] >= -5)
    {
        rc_ctrl_local->rc.ch[1] = 0;
    }
    if (rc_ctrl_local->rc.ch[2] <= 5 && rc_ctrl_local->rc.ch[2] >= -5)
    {
        rc_ctrl_local->rc.ch[2] = 0;
    }
    if (rc_ctrl_local->rc.ch[3] <= 5 && rc_ctrl_local->rc.ch[3] >= -5)
    {
        rc_ctrl_local->rc.ch[3] = 0;
    }

    rc_ctrl_local->rc.ch[0] -= RC_CH_VALUE_OFFSET;
    rc_ctrl_local->rc.ch[1] -= RC_CH_VALUE_OFFSET;
    rc_ctrl_local->rc.ch[2] -= RC_CH_VALUE_OFFSET;
    rc_ctrl_local->rc.ch[3] -= RC_CH_VALUE_OFFSET;
    rc_ctrl_local->rc.ch[4] -= RC_CH_VALUE_OFFSET;

    RC_data_is_error();
}
