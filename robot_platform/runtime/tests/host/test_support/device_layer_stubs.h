#ifndef PLATFORM_TEST_DEVICE_LAYER_STUBS_H
#define PLATFORM_TEST_DEVICE_LAYER_STUBS_H

#include "device_layer.h"

void platform_test_device_layer_stubs_reset(void);
void platform_test_device_layer_seed_feedback(const platform_device_feedback_t *feedback,
                                              platform_device_result_t result);
uint32_t platform_test_device_layer_get_init_call_count(void);
uint32_t platform_test_device_layer_get_write_call_count(void);
const platform_device_command_t *platform_test_device_layer_get_last_command(void);

#endif
