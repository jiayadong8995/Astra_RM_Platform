#include "remote_node.h"

#include "remote_control.h"

static const platform_dbus_remote_device_config_t g_platform_remote_node = {
    .acquire_fn = (const void *(*)(void))get_remote_control_point,
    .is_error_fn = RC_data_is_error,
};

const platform_dbus_remote_device_config_t *platform_remote_node_default(void)
{
    return &g_platform_remote_node;
}
