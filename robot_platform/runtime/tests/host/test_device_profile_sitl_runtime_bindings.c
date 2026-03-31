#include <assert.h>
#include <string.h>

#include "device_layer.h"

int main(void)
{
    platform_device_layer_t layer = {0};
    platform_device_result_t result =
        platform_device_layer_init_profile(&layer, PLATFORM_DEVICE_BACKEND_PROFILE_SITL);

    /* Keep this guard anchored to the authoritative chain:
     * remote input + state observation -> intent parsing / mode constraints -> chassis control -> execution output
     */
    assert(result == PLATFORM_DEVICE_RESULT_OK);

    assert(strcmp(layer.imu.name, "bmi088_sitl_udp") == 0);
    assert(strcmp(layer.remote.name, "dbus_remote_sitl_udp") == 0);

    assert(layer.imu.context != 0);
    assert(layer.remote.context != 0);

    assert(layer.imu.ops.init != 0);
    assert(layer.imu.ops.read_sample != 0);
    assert(layer.remote.ops.init != 0);
    assert(layer.remote.ops.read_input != 0);

    return 0;
}
