#include "control_math.h"

void platform_float_clamp(float *value, float min, float max)
{
    if (*value < min)
    {
        *value = min;
    }
    else if (*value > max)
    {
        *value = max;
    }
}

void platform_slew_to_target(float target, float *state, float step)
{
    if (target > *state)
    {
        *state += step;
        if (*state >= target)
        {
            *state = target;
        }
    }
    else if (target < *state)
    {
        *state -= step;
        if (*state <= target)
        {
            *state = target;
        }
    }
}

void platform_int16_clamp(int16_t *value, int16_t min, int16_t max)
{
    if (*value < min)
    {
        *value = min;
    }
    else if (*value > max)
    {
        *value = max;
    }
}
