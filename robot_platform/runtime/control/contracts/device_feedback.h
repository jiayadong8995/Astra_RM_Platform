#ifndef DEVICE_FEEDBACK_H
#define DEVICE_FEEDBACK_H

#include "device_input.h"

typedef struct {
  uint32_t timestamp_us;
  uint32_t sequence;
  platform_actuator_feedback_snapshot_t actuator_feedback;
  uint32_t backend_flags;
} platform_device_feedback_t;

#endif
