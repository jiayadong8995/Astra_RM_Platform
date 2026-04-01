#ifndef BALANCE_CHASSIS_APP_IO_CHASSIS_OBSERVATION_H
#define BALANCE_CHASSIS_APP_IO_CHASSIS_OBSERVATION_H

#include "../../../control/contracts/actuator_command.h"

#include <stdbool.h>
#include <stdint.h>

/**
 * Observation instrumentation for actuator commands.
 *
 * These functions capture actuator_command publishes for test
 * verification and SITL runtime-output logging.  They are called
 * from chassis_control_task after each publish.
 */

void chassis_observation_on_publish(const platform_actuator_command_t *actuator_command);
void chassis_runtime_bus_reset_observation(void);
uint32_t chassis_runtime_bus_observation_count(void);
bool chassis_runtime_bus_get_first_observation(platform_actuator_command_t *actuator_command);
bool chassis_runtime_bus_get_latest_observation(platform_actuator_command_t *actuator_command);

#endif
