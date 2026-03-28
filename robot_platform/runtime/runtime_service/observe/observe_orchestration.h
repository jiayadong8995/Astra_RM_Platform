#ifndef BALANCE_CHASSIS_RUNTIME_SERVICE_OBSERVE_ORCHESTRATION_H
#define BALANCE_CHASSIS_RUNTIME_SERVICE_OBSERVE_ORCHESTRATION_H

#include "observe_runtime.h"

void observe_runtime_init(Observe_Runtime_t *runtime);

void observe_runtime_apply_inputs(Observe_Runtime_t *runtime,
                                  const Chassis_Cmd_t *cmd_msg,
                                  const Actuator_Feedback_t *feedback_msg,
                                  float dt_s);

Chassis_Observe_t observe_runtime_build_output(const Observe_Runtime_t *runtime);

#endif
