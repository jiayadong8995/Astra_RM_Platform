#include "device_layer_stubs.h"

#include <string.h>

static uint32_t g_init_call_count;
static uint32_t g_write_call_count;
static platform_device_result_t g_feedback_result = PLATFORM_DEVICE_RESULT_OK;
static platform_device_feedback_t g_seeded_feedback;
static platform_device_command_t g_last_command;

void platform_test_device_layer_stubs_reset(void)
{
    g_init_call_count = 0U;
    g_write_call_count = 0U;
    g_feedback_result = PLATFORM_DEVICE_RESULT_OK;
    memset(&g_seeded_feedback, 0, sizeof(g_seeded_feedback));
    memset(&g_last_command, 0, sizeof(g_last_command));
}

void platform_test_device_layer_seed_feedback(const platform_device_feedback_t *feedback,
                                              platform_device_result_t result)
{
    g_feedback_result = result;
    if (feedback == NULL) {
        memset(&g_seeded_feedback, 0, sizeof(g_seeded_feedback));
        return;
    }

    g_seeded_feedback = *feedback;
}

uint32_t platform_test_device_layer_get_init_call_count(void)
{
    return g_init_call_count;
}

uint32_t platform_test_device_layer_get_write_call_count(void)
{
    return g_write_call_count;
}

const platform_device_command_t *platform_test_device_layer_get_last_command(void)
{
    return &g_last_command;
}

platform_device_result_t platform_device_init_default_profile(void)
{
    g_init_call_count += 1U;
    return PLATFORM_DEVICE_RESULT_OK;
}

platform_device_result_t platform_device_read_default_feedback(platform_device_feedback_t *feedback)
{
    if (feedback == NULL) {
        return PLATFORM_DEVICE_RESULT_INVALID;
    }

    *feedback = g_seeded_feedback;
    return g_feedback_result;
}

platform_device_result_t platform_device_write_default_command(const platform_device_command_t *command)
{
    if (command == NULL) {
        return PLATFORM_DEVICE_RESULT_INVALID;
    }

    g_last_command = *command;
    g_write_call_count += 1U;
    return PLATFORM_DEVICE_RESULT_OK;
}
