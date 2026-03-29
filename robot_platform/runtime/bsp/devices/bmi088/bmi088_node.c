#include "bmi088_node.h"

#include "BMI088driver.h"
#include "spi.h"

static const platform_bmi088_device_config_t g_platform_bmi088_node = {
    .spi_handle = &hspi2,
    .sample_state = &BMI088,
    .calibrate = 1U,
    .init_fn = (void (*)(void *, uint8_t))BMI088_Init,
    .read_fn = (void (*)(void *))BMI088_Read,
};

const platform_bmi088_device_config_t *platform_bmi088_node_default(void)
{
    return &g_platform_bmi088_node;
}
