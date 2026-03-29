#ifndef PLATFORM_DEVICE_TYPES_H
#define PLATFORM_DEVICE_TYPES_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
  PLATFORM_DEVICE_RESULT_OK = 0,
  PLATFORM_DEVICE_RESULT_INVALID = 1,
  PLATFORM_DEVICE_RESULT_TIMEOUT = 2,
  PLATFORM_DEVICE_RESULT_UNAVAILABLE = 3,
  PLATFORM_DEVICE_RESULT_UNSUPPORTED = 4,
} platform_device_result_t;

typedef struct {
  uint32_t timestamp_us;
  uint32_t sequence;
  bool valid;
} platform_device_stamp_t;

#endif
