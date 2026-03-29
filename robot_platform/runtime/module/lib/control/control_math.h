#ifndef PLATFORM_MODULE_LIB_CONTROL_MATH_H
#define PLATFORM_MODULE_LIB_CONTROL_MATH_H

#include <stdint.h>

void platform_float_clamp(float *value, float min, float max);
void platform_slew_to_target(float target, float *state, float step);
void platform_int16_clamp(int16_t *value, int16_t min, int16_t max);

#endif
