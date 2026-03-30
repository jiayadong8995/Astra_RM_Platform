# Architecture Patterns

**Domain:** Reusable Robotmaster embedded control platform
**Researched:** 2026-03-30

## Recommended Architecture

Keep the platform, but make it thinner and more directional. The repo already has the right top-level split in `runtime/device`, `runtime/control`, `runtime/app`, `runtime/bsp`, `runtime/generated`, and `sim/`; the problem is that ownership is still duplicated across those layers, especially between the legacy app/task tree and the newer device/control abstractions. The correct move is not a bigger framework. It is to make each layer own exactly one transformation and forbid cross-layer backedges.

Recommended structure:

```text
projects/<robot_profile>/
  project.yaml                 # robot selection, runtime mode, feature flags
  robot_contract/              # robot-specific contract shapes, ids, limits
  robot_composition/           # task graph, topic graph, safety policy, wiring

runtime/
  contracts/                   # typed messages and port definitions, size-checked
  ports/                       # narrow interfaces: sensor port, actuator port, link port, clock port
  device/                      # backend-specific adapters implementing ports
  control/
    state/                     # DeviceInput -> RobotState
    intent/                    # RC/link/test input -> RobotIntent
    controllers/               # RobotState + RobotIntent -> ActuatorCommand
    safety/                    # gating, arming, stale-data, saturation, mode guards
    execution/                 # ActuatorCommand -> DeviceCommand
  orchestration/               # task startup, scheduling, lifecycle, no control math
  platform_bus/                # deterministic typed transport, or direct ports in host tests
  bsp/                         # board-only code
  generated/                   # CubeMX output only

sim/
  adapters/                    # fake-link and runtime observation adapters
  backends/                    # SITL transport/process backend
  projects/<robot_profile>/    # project-specific smoke contracts and validation
```

The main boundary choice is this: `control` must not know whether it is running on STM32, Linux SITL, fake-link, or replay. `device` must not contain robot behavior. `app/orchestration` must not contain control logic. `projects/balance_chassis` should become composition and limits, not the place where platform semantics leak.

### Component Boundaries

| Component | Responsibility | Communicates With |
|-----------|---------------|-------------------|
| `projects/<robot_profile>` | Select robot profile, enabled features, motor/joint ids, safety limits, runtime wiring | `runtime/orchestration`, `runtime/contracts` |
| `runtime/contracts` | Define typed messages, topic ids, port contracts, compile-time size constraints | All runtime layers |
| `runtime/device` | Bind backend-specific sensor, remote, and actuator adapters into stable device ports | `runtime/control/state`, `runtime/control/intent`, `runtime/control/execution` |
| `runtime/control/state` | Convert sensor/device feedback into a coherent `RobotState` | `runtime/device`, `runtime/contracts`, `runtime/control/controllers`, `runtime/control/safety` |
| `runtime/control/intent` | Convert operator input, scripted input, or fake-link input into `RobotIntent` | `runtime/device`, `runtime/contracts`, `runtime/control/controllers`, `runtime/control/safety` |
| `runtime/control/controllers` | Compute desired actuator outputs from state and intent | `runtime/control/state`, `runtime/control/intent`, `runtime/control/safety`, `runtime/control/execution` |
| `runtime/control/safety` | Apply arming, stale-link, limit, mode, saturation, and fault gating before hardware dispatch | `runtime/control/state`, `runtime/control/intent`, `runtime/control/controllers`, `runtime/control/execution` |
| `runtime/control/execution` | Map abstract actuator commands to concrete device commands and collect feedback | `runtime/device`, `runtime/contracts`, `runtime/platform_bus` |
| `runtime/orchestration` | Start tasks, own scheduling, initialize ports, wire buses, expose lifecycle states | All runtime layers except `bsp/generated` internals |
| `runtime/platform_bus` | Deliver typed snapshots/events between tasks without hidden payload assumptions | `runtime/orchestration`, all runtime publishers/subscribers |
| `runtime/bsp` | Board-specific clocks, HAL wrappers, peripheral glue | `runtime/device`, `runtime/generated` |
| `runtime/generated` | CubeMX-generated low-level sources, no handwritten platform semantics | `runtime/bsp` |
| `sim/backends` + `sim/projects/*` | Run SITL process, feed fake inputs, observe runtime outputs, write smoke/validation reports | `runtime` through backend transport contracts |

### Data Flow

Direction should be strictly left to right:

```text
Board/SITL/Fake Link
  -> device adapters
  -> DeviceInput / DeviceFeedback
  -> state estimation + intent decoding
  -> RobotState / RobotIntent
  -> controllers
  -> SafetyGate
  -> ActuatorCommand
  -> execution mapper
  -> DeviceCommand
  -> actuator adapters
  -> board/SITL transport
```

Supporting feedback loop:

```text
Actuator feedback
  -> device adapters
  -> DeviceFeedback
  -> state / safety / reporting
```

Validation flow for v1:

```text
Test vector / fake-link script
  -> sim backend adapter
  -> runtime input boundary
  -> control pipeline
  -> runtime output boundary
  -> observation exporter
  -> smoke report / assertions
```

The current repo is close to this flow, but not cleanly enough. `INS_task`, `Observe_task`, `Chassis_task`, and `motor_control_task` already approximate a pipeline, yet readiness, topic naming, and device binding remain implicit. The architecture should preserve that staged flow while replacing implicit coupling with explicit contracts.

## Patterns to Follow

### Pattern 1: Contract-First Runtime Boundary
**What:** Treat every cross-task and cross-process handoff as a formal contract with an owned type, explicit topic/port id, and compile-time size validation.
**When:** Apply to message bus topics, SITL transport packets, and project-level output declarations.
**Example:**
```c
typedef struct
{
    uint32_t sequence;
    bool valid;
    platform_robot_pose_t pose;
    platform_body_rates_t rates;
} platform_robot_state_t;

PLATFORM_STATIC_ASSERT_TOPIC_FITS(robot_state_topic, platform_robot_state_t);
```

This is the highest-value architectural correction because the current `message_center` fixed 64-byte payload model directly undermines safe reuse and TDD.

### Pattern 2: Ports and Adapters, Not Shared Concrete Drivers
**What:** Keep `device_layer` as the stable semantic boundary, but make all HW/SITL/fake implementations explicit adapters behind narrow ports.
**When:** Any sensor, remote, actuator, or transport path that must run in both host tests and embedded builds.
**Example:**
```c
typedef struct
{
    platform_device_result_t (*read)(void *ctx, platform_imu_sample_t *out);
    void *ctx;
} platform_imu_port_t;
```

The repo already intends this in `device_profile_hw.c` and `device_profile_sitl.c`. Finish it by preventing control code from depending on BSP symbols or on adapter-specific behavior.

### Pattern 3: Orchestration Owns Lifecycle, Control Owns Math
**What:** Tasks, startup order, retries, and health state live in orchestration; estimation and control math live in control modules.
**When:** All FreeRTOS task startup and host-side runtime boot paths.
**Example:**
```c
void platform_runtime_start(const platform_runtime_config_t *config)
{
    platform_bus_init(&config->bus);
    platform_device_bootstrap(&config->device_profile);
    platform_start_state_tasks();
    platform_start_control_tasks();
}
```

This addresses the current leakage where app bring-up files still carry too much authority over the runtime chain.

### Pattern 4: Safety Gate as a First-Class Stage
**What:** Put stale data handling, arming, saturation, invalid mode transitions, and output clamping in one explicit stage between controller output and actuator dispatch.
**When:** Always, especially before any hardware bring-up.
**Example:**
```c
platform_safety_decision_t decision =
    platform_safety_evaluate(&state, &intent, &controller_output, &feedback_health);

platform_apply_safety_decision(&decision, &actuator_command);
```

This is critical for v1 TDD because safe bring-up is not just a controller problem.

### Pattern 5: Observation Exporter Separate from Control Logic
**What:** Runtime-output observation for SITL and fake-link validation should be a dedicated exporter layer, not embedded in controller code.
**When:** Smoke tests, report generation, fake-link runs, replay runs.
**Example:**
```python
observations = runtime_output_exporter.collect(
    topics=("robot_state", "actuator_command", "device_feedback"),
)
```

The current `collect_runtime_output_observations()` stub is the missing seam that prevents v1 validation from proving anything meaningful.

## Anti-Patterns to Avoid

### Anti-Pattern 1: Robot Profile Logic Inside Platform Runtime
**What:** Hard-coding `balance_chassis` assumptions into generic actuator mapping, task graphs, or device semantics.
**Why bad:** The platform stops being reusable, but still keeps all the weight of a platform.
**Instead:** Keep robot-specific joint/wheel topology, gains, and mode rules inside `projects/<robot_profile>/robot_composition` and `robot_contract`.

### Anti-Pattern 2: Legacy App Layer Owning Runtime Semantics
**What:** Letting `runtime/app/balance_chassis` remain the real authority for control chain wiring, topic names, and readiness semantics.
**Why bad:** It preserves the current partial fusion between old app code and new platformized layers.
**Instead:** Shrink app into orchestration/composition only. Control, safety, and device semantics should live in runtime-owned modules.

### Anti-Pattern 3: Hidden Readiness Through Polling Message Contents
**What:** Using busy-wait loops and cached message values as implicit lifecycle coordination.
**Why bad:** Startup order becomes nondeterministic and difficult to test.
**Instead:** Introduce explicit component states such as `UNINITIALIZED`, `READY`, `DEGRADED`, `FAULTED`, and publish lifecycle events or synchronous boot checks.

### Anti-Pattern 4: Simulation That Proves Nothing
**What:** Declaring runtime boundaries in the Python profile while the runtime publishes different names or no observable outputs.
**Why bad:** CI stays green while the control path is disconnected.
**Instead:** Make the same contract package define runtime topic ids and sim observation bindings.

### Anti-Pattern 5: Premature Generalization Across Robot Classes
**What:** Designing for every future Robotmaster mechanism before the first profile is proven.
**Why bad:** The platform bloats faster than validation coverage.
**Instead:** Generalize only at the boundaries already exercised by `balance_chassis`: state, intent, actuator command, safety, device ports, and project composition.

## Scalability Considerations

| Concern | At 100 users | At 10K users | At 1M users |
|---------|--------------|--------------|-------------|
| Robot profile count | One or two robot profiles can share the same runtime contracts and device ports | More profiles need generated contract tables and profile-specific composition manifests | Full product-line scaling requires strict schema/versioning for contracts and composition metadata |
| Test matrix size | Host TDD and fake-link tests run per commit | CI must shard by backend and robot profile, keeping hardware tests gated | Long-term requires artifacted replay corpora and contract compatibility checks |
| Runtime transport | Current message bus can work only if payload sizing is fixed safely | Multiple tasks/topics require indexed topics, capacity checks, and diagnostics | Larger platforms eventually need generated bus metadata or direct port wiring for critical paths |
| Backend diversity | HW and SITL can share device semantics with separate adapters | Replay and fault-injection backends become necessary for confidence | Additional robots/boards require adapter factories, not controller rewrites |
| Bring-up risk | Safety gates plus fake-link validation can block obvious faults | More robots increase configuration error risk, so lifecycle/state introspection becomes mandatory | Hardware fleet scaling needs persistent telemetry and release gating beyond local smoke runs |

## Build Order

Build the platform in dependency order, not by trying to abstract everything at once:

1. **Fix the contract surface first**
   Replace unsafe topic payload assumptions, unify runtime topic names with sim declarations, and make every boundary size-checked.

2. **Stabilize the device port layer**
   Keep `device_layer` and backend profiles, but make SITL use real SITL adapters instead of stubs so host-side tests exercise meaningful paths.

3. **Insert an explicit safety stage**
   Separate controller output from actuator dispatch with arming, stale-input, saturation, and fault gating.

4. **Move orchestration out of legacy app authority**
   Keep the current task graph if needed, but relocate ownership of startup, readiness, and lifecycle to a platform orchestration layer.

5. **Make runtime outputs observable in v1**
   Implement runtime observation export so fake-link and smoke tests can assert on `robot_state`, `actuator_command`, and safety outcomes.

6. **Only then broaden profile reuse**
   After `balance_chassis` passes host-side TDD and fake-link validation reliably, extract reusable profile composition helpers for the next robot.

This order matters because v1 needs trust, not maximum abstraction. If the repo generalizes profile/plugin/config surfaces before contracts, observability, and safety are proven, it will become heavier without becoming safer.

## What To Defer

Defer anything that expands platform surface area without helping v1 TDD or v2 safe bring-up:

- A generic physics simulator inside the main platform
- Cross-robot behavior frameworks beyond current wheel-leg needs
- Dynamic plugin loading or runtime reflection systems
- Generalized multi-board orchestration before one board path is reliable
- Rich service buses, RPC layers, or ROS-style graph complexity in the embedded runtime
- Broad mechanism abstractions that are not exercised by `balance_chassis`

The reusable platform should generalize only the seams already needed now: contracts, device ports, safety gates, orchestration, and profile composition.

## Sources

- `/home/xbd/worspcae/code/Astra_RM2025_Balance/.planning/PROJECT.md`
- `/home/xbd/worspcae/code/Astra_RM2025_Balance/.planning/codebase/CONCERNS.md`
- `/home/xbd/worspcae/code/Astra_RM2025_Balance/robot_platform/CMakeLists.txt`
- `/home/xbd/worspcae/code/Astra_RM2025_Balance/robot_platform/runtime/device/README.md`
- `/home/xbd/worspcae/code/Astra_RM2025_Balance/robot_platform/runtime/control/README.md`
- `/home/xbd/worspcae/code/Astra_RM2025_Balance/robot_platform/runtime/device/device_layer.c`
- `/home/xbd/worspcae/code/Astra_RM2025_Balance/robot_platform/runtime/device/device_profile_sitl.c`
- `/home/xbd/worspcae/code/Astra_RM2025_Balance/robot_platform/runtime/app/balance_chassis/app_bringup/task_registry.c`
- `/home/xbd/worspcae/code/Astra_RM2025_Balance/robot_platform/runtime/app/balance_chassis/app_io/chassis_topics.c`
- `/home/xbd/worspcae/code/Astra_RM2025_Balance/robot_platform/runtime/control/state/ins_task.c`
- `/home/xbd/worspcae/code/Astra_RM2025_Balance/robot_platform/runtime/control/state/observe_task.c`
- `/home/xbd/worspcae/code/Astra_RM2025_Balance/robot_platform/runtime/control/execution/motor_control_task.c`
- `/home/xbd/worspcae/code/Astra_RM2025_Balance/robot_platform/runtime/control/execution/actuator_gateway.c`
- `/home/xbd/worspcae/code/Astra_RM2025_Balance/robot_platform/sim/README.md`
- `/home/xbd/worspcae/code/Astra_RM2025_Balance/robot_platform/sim/projects/balance_chassis/profile.py`
- `/home/xbd/worspcae/code/Astra_RM2025_Balance/robot_platform/sim/projects/balance_chassis/bridge_adapter.py`
