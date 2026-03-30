# Feature Landscape

**Domain:** Robotmaster embedded robot platform for a wheeled-legged target
**Researched:** 2026-03-30

## Table Stakes

Features users expect. Missing = product feels incomplete.

| Feature | Why Expected | Complexity | Notes |
|---------|--------------|------------|-------|
| Reliable `build -> host test -> SITL smoke -> firmware generate` loop | This repo's current v1 promise is safe pre-hardware closure, so the default dev loop must be reproducible and trusted instead of ad hoc manual bring-up. | Med | **v1 requirement.** Depends on stabilizing CubeMX generation, keeping SITL and firmware targets buildable from one CLI, and making failures machine-readable. |
| Host-side control-path tests for C runtime logic | For an embedded control platform, "we can compile" is not enough; users expect controller, transport, and task logic to be testable before hardware. | High | **v1 requirement.** Directly blocked today by missing C test harness coverage in `message_center`, control, device, and task wiring. |
| Fake data-link validation with meaningful runtime observations | A simulated link is table stakes here because the project explicitly wants to separate comms faults from control faults before touching the robot. | High | **v1 requirement.** Needs SITL adapters bound to real runtime paths and output exporters that observe actual published topics rather than declared-only placeholders. |
| Safety gates for known dangerous failure modes | A wheeled-legged platform is not safe to iterate on without gates for inverted outputs, stale data, bad enable transitions, saturation breakage, and invalid sensor state. | High | **v1 requirement.** Should block promotion to hardware-facing runs when checks fail; depends on contract assertions and regression cases around the current failure list in `PROJECT.md`. |
| Deterministic startup, disable, and fault-handling behavior | Operators expect the robot to refuse control on bad data, recover to a safe disabled state on link loss, and avoid startup races. | High | **v1 requirement.** Depends on replacing readiness-by-busy-wait patterns with explicit state handling and validating stale-input behavior. |
| Basic runtime observability and artifact capture | Even early-stage robot teams expect logs, smoke summaries, and a small set of counters that explain why a run is unsafe or inconclusive. | Med | **v1 requirement.** Keep this machine-readable first: smoke JSON, dropped-packet counters, adapter binding status, topic/contract mismatch reporting. |
| Single authoritative bring-up path for `balance_chassis` | With one active target, users expect one canonical project profile and one authoritative control path, not split legacy/platformized behavior. | Med | **v1 requirement.** Depends on reducing the current legacy/new control-path fusion and documenting the one blessed path through runtime layers. |

## Differentiators

Features that set product apart. Not expected, but valued.

| Feature | Value Proposition | Complexity | Notes |
|---------|-------------------|------------|-------|
| Fault-injection matrix for link, sensor, and actuator degradation | Turns host-side validation from smoke checking into a real safety case by proving behavior under packet loss, stale IMU data, missing feedback, and partial actuator faults. | High | **Best near-term differentiator.** Build after table-stakes fake-link validation exists; this is more valuable now than chasing advanced control features. |
| Replayable regression corpus from SITL and later hardware traces | Lets the team lock down previously observed failures and compare controller changes against known bad scenarios. | Med | **Late v1 / early v2.** Depends on runtime observation export and stable report schemas. |
| Hardware-gated constrained closed-loop bring-up checklist | Distinguishes the platform from generic firmware repos by making real-robot bring-up a controlled phase gate instead of a manual ritual. | High | **v2-facing differentiator.** Requires v1 safety gates plus minimal trusted hardware telemetry. |
| Reusable platform profile proven beyond one robot configuration | Real platform value appears when the abstractions survive a second board or robot profile without collapsing into `balance_chassis`-specific code. | High | **Later-stage differentiator.** Do not force this into v1; first prove the abstraction on the current target. |
| Advanced wheeled-legged control layers such as hierarchical balance/posture control, terrain adaptation, or more robust disturbance rejection | These are what eventually make a wheeled-legged robot competitive and capable on real terrain. | High | **Later-stage only.** Current literature still treats hierarchical balance control and stronger anti-disturbance control as substantive research/implementation work, not baseline platform plumbing. Add only after the validation stack is trusted. |

## Anti-Features

Features to explicitly NOT build.

| Anti-Feature | Why Avoid | What to Do Instead |
|--------------|-----------|-------------------|
| Full competition feature breadth now: perception, autonomy, strategy, multi-subsystem integration | It dilutes the current goal. The repo's stated problem is pre-hardware trust, not end-state Robotmaster capability breadth. | Finish the host-side safety and fake-link validation loop first. |
| High-fidelity digital twin or physics perfection before contract correctness | The project already recognizes simulated data is fake. Spending effort on realism before fixing adapter binding and output observation will create false confidence. | Use simple SITL/fake-link paths to catch logic and contract failures, then graduate to constrained hardware validation. |
| Sophisticated control research first: MPC, RL, dynamic obstacle avoidance, aggressive terrain features | These add tuning and debugging surface before the baseline message, state, and safety contracts are even trustworthy. | Keep v1 on deterministic balance-control validation, safe disable semantics, and fault containment. |
| Broad platform generalization before `balance_chassis` is proven | Premature generality will increase coupling and make TDD harder in a repo that already feels over-platformized. | Keep reusable interfaces, but validate them through one authoritative `balance_chassis` path first. |
| Rich GUI/operator dashboards as a v1 deliverable | UI polish will not solve the current engineering closure problem and can hide missing machine-readable validation underneath screenshots. | Ship CLI-first reports, explicit failure reasons, and artifact files that can later feed a UI if needed. |
| Always-on direct hardware iteration as the main debug loop | This is the failure mode the project is trying to escape; it increases risk of unstable robot behavior and slows diagnosis. | Treat on-robot runs as gated outputs of the host-side validation pipeline, not the default development surface. |

## Feature Dependencies

```text
Reliable build/test/generate loop -> host-side C control tests
Reliable build/test/generate loop -> fake data-link validation
Host-side C control tests -> safety gates for dangerous failure modes
Fake data-link validation -> runtime observation export -> replayable regression corpus
Safety gates + deterministic fault handling -> constrained real-robot bring-up
Single authoritative balance_chassis path -> reusable second-target platform proof
```

## MVP Recommendation

Prioritize:
1. Reliable `build -> host test -> SITL smoke -> generate` loop
2. Host-side C control-path tests plus fake data-link validation with real runtime observations
3. Safety gates for stale data, invalid enable transitions, saturation, and inverted/unsafe outputs

Defer: advanced wheeled-legged control sophistication, second-target platform proof, and rich operator surfaces. They are valuable, but they are not the current blocker and will produce false progress if the v1 trust stack stays weak.

## Sources

- Internal project brief: `/home/xbd/worspcae/code/Astra_RM2025_Balance/.planning/PROJECT.md` and `/home/xbd/worspcae/code/Astra_RM2025_Balance/.planning/codebase/CONCERNS.md` and `/home/xbd/worspcae/code/Astra_RM2025_Balance/.planning/codebase/TESTING.md` and `/home/xbd/worspcae/code/Astra_RM2025_Balance/.planning/codebase/INTEGRATIONS.md` (HIGH confidence for current repo maturity and blockers)
- ROS 2 Control hardware component docs, especially compile-and-test guidance for mock/testable hardware paths: https://control.ros.org/rolling/doc/ros2_control/hardware_interface/doc/writing_new_hardware_component.html (MEDIUM confidence; external confirmation that offline hardware-interface testing is standard practice)
- Cao et al., "Design and Control of a Wheeled Bipedal Robot Based on Hybrid Linear Quadratic Regulator and Proportional-Derivative Control," *Sensors*, 2025: https://www.mdpi.com/1424-8220/25/17/5398 (MEDIUM confidence; supports treating advanced hierarchical/disturbance-robust control as later-stage capability, not v1 table stakes)
