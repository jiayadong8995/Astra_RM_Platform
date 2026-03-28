#ifndef BALANCE_CHASSIS_APP_IO_CHASSIS_TOPICS_H
#define BALANCE_CHASSIS_APP_IO_CHASSIS_TOPICS_H

#include "topic_contract.h"

void chassis_runtime_bus_init(Chassis_Runtime_Bus_t *bus);

void chassis_runtime_bus_wait_ready(Chassis_Runtime_Bus_t *bus, Chassis_Bus_Input_t *inputs);

void chassis_runtime_bus_pull_inputs(Chassis_Runtime_Bus_t *bus, Chassis_Bus_Input_t *inputs);

void chassis_runtime_bus_publish_outputs(Chassis_Runtime_Bus_t *bus, const Chassis_Bus_Output_t *outputs);

#endif
