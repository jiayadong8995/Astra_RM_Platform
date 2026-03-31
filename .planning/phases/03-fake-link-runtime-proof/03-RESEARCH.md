# Phase 3: Fake-Link Runtime Proof - Research

**Researched:** 2026-04-01
**Domain:** SITL/fake-link runtime proof, runtime observability, and verification artifact classification for `balance_chassis`
**Confidence:** HIGH

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions
### Application-Layer Requirement Frame
- **D-01:** Phase 3 planning should treat the robot requirements first as a thin application-layer responsibility framework, not as a hardware-first structure.
- **D-02:** The current minimum application-layer responsibility set is: remote input, intent parsing and mode switching, state observation, chassis control, execution output, safety protection, and runtime observability/diagnostics.
- **D-03:** These are application responsibilities, not commitments to the current file/module layout; downstream work should avoid confusing current implementation seams with the desired requirement frame.

### Authoritative Application Chain
- **D-04:** The primary application chain should be understood as `remote input + state observation -> intent parsing / mode constraints -> chassis control -> execution output`.
- **D-05:** `state observation` is not downstream of intent parsing in the conceptual model; intent/mode decisions must be made with current observed state available as an input.
- **D-06:** Phase 3 should continue to validate the current real runtime implementation path rather than introducing a parallel sim-only or test-only control flow.

### Sim and Fake-Link Role
- **D-07:** Sim, fake-link, and test cases should be treated as application-side runtime drivers and verification tools, not as hardware-owning systems.
- **D-08:** Fake adapters are allowed as injection and observation mechanisms, but they must not become a second implementation of the robot’s core control logic.
- **D-09:** Phase 3 should not pursue a higher-fidelity simulator as its main objective; proving runtime-path truth and observability matters more than realism in this phase.

### Safety as a Cross-Cutting Capability
- **D-10:** Safety protection should be treated as a cross-cutting adjudication layer across the chain, not only as a final gate immediately before actuator output.
- **D-11:** Verification evidence in this phase should preserve whether a blocked, degraded, or enabled result came from safety protection versus communication failure versus control behavior.

### Runtime Observability and Diagnostic Priority
- **D-12:** Phase 3 observability must serve two goals together: prove that the application/runtime chain really ran, and help localize failures by layer.
- **D-13:** When those two goals conflict, Phase 3 should prioritize proving that the real runtime path was actually driven, while still capturing enough evidence to distinguish communication-path failures from control-path failures.
- **D-14:** The authoritative first-wave evidence should include runtime output observation, adapter-binding/runtime-boundary status, and bridge/transport diagnostics rather than only declared expectations.

### Contract and Boundary Strictness
- **D-15:** Topic, port, protocol, or runtime-boundary declaration drift should be treated as explicit validation failure in this phase rather than as soft warning-only behavior.
- **D-16:** Phase 3 should keep machine-readable artifacts as the primary proof surface, with human-readable summaries layered on top.

### the agent's Discretion
- The exact artifact schema for Phase 3 may be refined by research/planning as long as it clearly records runtime-path evidence, adapter/binding status, boundary mismatches, and failure-layer classification.
- The precise observation hooks may be chosen by the agent as long as they stay anchored to the current real runtime path and do not create a separate shadow control implementation.
- The exact naming of the application-layer responsibilities may be tightened slightly during planning if the meaning stays consistent with the decisions above.

### Deferred Ideas (OUT OF SCOPE)
- A broader architecture rewrite that fully reorganizes the platform around the new application-layer responsibility frame belongs mainly to Phase 4.
- High-fidelity simulation or digital-twin ambitions remain out of scope for this phase.
- Broad multi-environment claims such as "all hosts share one unified application chain" are not locked decisions yet and should not be assumed beyond the narrower Phase 3 decisions above.
</user_constraints>

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|------------------|
| LINK-01 | Sim/fake-link adapters drive the real runtime control path rather than stub-only placeholder behavior | Rebind SITL IMU/remote ingress to real transport-backed adapters or equivalent runtime-fed seams; keep `remote_task -> Observe_task -> Chassis_task -> motor_control_task` authoritative |
| LINK-02 | Validation captures observable runtime outputs for the fake-link path, not only declared expectations | Extend existing `actuator_command` runtime-output observation path and persist observations into smoke/verification artifacts |
| LINK-03 | Verification can distinguish communication-path failures from control-path failures in its output artifacts | Add explicit layer classification derived from adapter binding, boundary/port matching, bridge counters, stale/missing observations, and runtime output status |
| LINK-04 | Topic, port, or contract mismatches between runtime and sim declarations fail validation explicitly | Treat protocol, boundary, and transport-port drift as required checks, not warnings; fail the verification stage machine-readably |
| OBS-01 | Smoke and verification runs emit machine-readable artifacts with adapter-binding status, validation outcomes, and failure reasons | Reuse `sim/core/runner.py` summary model and extend JSON schema instead of creating a second report path |
| OBS-02 | Verification artifacts expose counters or diagnostics for dropped packets, stale inputs, or missing runtime observations where applicable | Promote bridge counters and add adapter/runtime freshness diagnostics into per-run artifacts |
</phase_requirements>

## Summary

Phase 3 should extend the current SITL smoke and verification surfaces, not replace them. The repository already has the right orchestration seam in `robot_platform/tools/platform_cli/main.py`, the right report aggregator in `robot_platform/sim/core/runner.py`, the right declared proof target in `robot_platform/sim/projects/balance_chassis/profile.py`, and the right first runtime-output observation seam in `robot_platform/runtime/app/balance_chassis/app_io/chassis_topics.c`.

The main trust gap is not report formatting. It is that the current SITL input side is still partly fake in the wrong way: `device_profile_sitl.c` binds IMU and remote to stub devices, while only motor output goes over UDP. That means the bridge can observe command traffic without proving that fake-link inputs actually drove the real runtime path. Planning should therefore prioritize honest adapter binding and artifact classification over simulator fidelity.

The existing JSON-first model is already strong enough to carry Phase 3. `runner.py` records declared versus observed protocol, boundary, ports, stats, and runtime outputs; current tests already validate that behavior. The next phase should harden that model into an authoritative Phase 3 verification artifact that records adapter binding truth, observed runtime evidence, contract drift failures, and a machine-readable failure layer.

**Primary recommendation:** Keep `verify` and `sim` as the public entrypoints, rebind fake-link ingress to the real runtime path, and make Phase 3 artifacts fail explicitly on adapter/boundary drift while classifying failures as `communication`, `control`, or `observation`.

## Project Constraints (from CLAUDE.md)

- Work must stay inside the GSD workflow; planning should assume Phase 3 implementation happens via the normal phase workflow, not ad hoc repo edits.
- Preserve the reusable platform direction; do not collapse Phase 3 into a one-off `balance_chassis` bypass.
- Pre-hardware verification remains the gate for any later on-robot work.
- Fake-link validation catches logic and contract faults early but is not physical proof.
- Preserve existing runtime layering where it provides leverage, but allow review of implementation coupling when it blocks trustworthy verification.
- Treat `balance_chassis` as the proving path without turning it into the entire architecture.

## Standard Stack

### Core
| Library / Tool | Version | Purpose | Why Standard |
|---------|---------|---------|--------------|
| Python stdlib `json` + `subprocess` + `unittest` | Python 3.12.3 | CLI orchestration, report writing, report-level tests | Already used by `platform_cli`, `runner`, and existing verification tests |
| CMake | 3.28.3 | Build SITL and host verification targets | Already the authoritative build surface for host and SITL |
| CTest | 3.28.3 | Host verification execution | Already the current host test runner for Phase 2 |
| GCC | 13.3.0 | Linux host/SITL compilation | Present and aligned with current Linux toolchain flow |
| Runtime topic path (`message_center` + task chain) | repo-local | Authoritative runtime path under proof | Phase 3 must validate this path, not a shadow harness |

### Supporting
| Library / Tool | Version | Purpose | When to Use |
|---------|---------|---------|-------------|
| `robot_platform.sim.core.runner` | repo-local | Collect declared vs observed evidence and emit smoke JSON | Use for all SITL/fake-link evidence aggregation |
| `robot_platform.sim.backends.sitl_bridge` | repo-local | Bridge transport, counters, and runtime-output event emission | Use as the single fake-link bridge surface |
| `robot_platform.tools.platform_cli.main` | repo-local | Authoritative `sim` and `verify` entrypoint | Use for Phase 3 orchestration and artifact writing |
| `arm-none-eabi-gcc` | 13.2.1 | Not required for Phase 3 execution, but present for broader platform builds | Use only if a plan step touches hardware-side build completeness |

### Alternatives Considered
| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| Extend `runner.py` and `platform_cli.main` | Create a separate Phase 3 report generator | Worse: duplicates artifact logic and splits the proof surface |
| Observe runtime only from the bridge | Add a second control-path mirror inside Python | Worse: creates a shadow control implementation and weakens LINK-01 |
| Use warning-only drift detection | Soft validation summaries | Conflicts with locked decision D-15 and requirement LINK-04 |

**Installation:** None beyond the repo’s existing Python/CMake toolchain.

**Version verification:** Verified locally on 2026-04-01 via `python3 --version`, `cmake --version`, `ctest --version`, `gcc --version`, `ninja --version`, and `arm-none-eabi-gcc --version`.

## Architecture Patterns

### Recommended Project Structure
```text
robot_platform/
├── runtime/                    # Authoritative runtime tasks, topics, contracts, and device bindings
├── sim/backends/               # Fake-link transport bridge and counters
├── sim/projects/balance_chassis/ # Per-project declarations, adapter hooks, validation status builder
└── tools/platform_cli/         # Public sim/verify entrypoints and verification artifact writers
```

### Pattern 1: Prove the Existing Runtime Chain, Don’t Reimplement It
**What:** Fake-link inputs must feed the same runtime chain already validated in Phase 2: `remote_task -> Observe_task -> Chassis_task -> motor_control_task`.
**When to use:** For every Phase 3 proof case.
**Example:**
```c
runtime->rc_result = platform_device_read_default_remote(&runtime->rc_input);
platform_remote_intent_bus_pull_inputs(&runtime->intent_bus, &runtime->robot_state);
platform_remote_intent_state_apply_inputs(&runtime->intent_state, &runtime->rc_input, &runtime->robot_state);
runtime->intent = platform_remote_intent_build(&runtime->intent_state);
platform_remote_intent_bus_publish(&runtime->intent_bus, &runtime->intent);
```
Source: `robot_platform/runtime/app/balance_chassis/app_bringup/remote_task.c`

### Pattern 2: Keep Proof Logic in Artifacts, Not Console-Only Output
**What:** The console can summarize; the artifact must carry the authoritative evidence and failure reason.
**When to use:** For every `sim` and `verify phase3` result.
**Example:**
```python
summary.update(_extract_bridge_metadata(metadata_lines))
_summarize_runtime_boundary(summary)
_summarize_bridge_stats(summary)
_summarize_validation_targets(summary, profile)
_summarize_smoke_health(summary, profile)
_build_smoke_result(summary)
```
Source: `robot_platform/sim/core/runner.py`

### Pattern 3: Declare Boundaries Once, Then Fail on Drift
**What:** The profile’s declared protocol, ports, and topic boundaries are the contract; observed drift must fail the run.
**When to use:** For LINK-04 and all classification logic.
**Example:**
```python
if isinstance(declared, dict) and isinstance(observed, dict):
    summary["runtime_boundary_match"] = declared == observed

if isinstance(declared_ports, dict) and isinstance(observed_ports, dict):
    summary["transport_ports_match"] = declared_ports == observed_ports
```
Source: `robot_platform/sim/core/runner.py`

### Pattern 4: Observe Runtime Outputs at the Runtime Boundary
**What:** Runtime-output proof should be emitted where the real task publishes `actuator_command`, not inferred only from bridge activity.
**When to use:** For LINK-02 and OBS-01.
**Example:**
```c
PubPushMessage(bus->actuator_command_pub, (void *)actuator_command);
g_latest_actuator_command = *actuator_command;
g_actuator_command_observation_count += 1U;
if (g_actuator_command_observed == 0U)
{
    printf("[RuntimeOutput] topic=actuator_command start=%u control_enable=%u actuator_enable=%u\n",
           actuator_command->start ? 1U : 0U,
           actuator_command->control_enable ? 1U : 0U,
           actuator_command->actuator_enable ? 1U : 0U);
}
```
Source: `robot_platform/runtime/app/balance_chassis/app_io/chassis_topics.c`

### Anti-Patterns to Avoid
- **Shadow control harness:** Do not recreate control behavior in Python to make the artifact look richer.
- **Bridge-only proof:** Seeing UDP motor commands alone is insufficient if IMU/remote ingress still comes from stubs.
- **Warning-only boundary mismatches:** This weakens LINK-04 and hides bad declarations.
- **Declared-only validation targets:** A target marked `declared_only` is evidence of missing proof, not a passing state.

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Report generation | A new standalone Phase 3 JSON writer | Extend `robot_platform.sim.core.runner` and CLI verification payloads | Existing schema already carries boundary, port, stats, and runtime-output evidence |
| Failure classification | Free-text summary logic | A small machine-readable enum/field set in artifacts | Planner needs deterministic outputs for LINK-03 and OBS-01 |
| Runtime-output proof | A fake adapter-only output mirror | Runtime-boundary observation from `chassis_topics.c` plus bridge metadata | Keeps evidence anchored to the real runtime path |
| Contract mismatch detection | Loose string checks scattered across files | Profile-declared vs observed comparison in one summary path | One place to enforce protocol/topic/port drift uniformly |

**Key insight:** Phase 3 should add the missing honest bindings and classifications around the current proof surface, not replace the proof surface itself.

## Common Pitfalls

### Pitfall 1: Counting Motor UDP Traffic as Proof of Full Runtime Truth
**What goes wrong:** The bridge sees motor commands and emits `runtime_output_observation`, but IMU and remote ingress may still be stub-fed.
**Why it happens:** `device_profile_sitl.c` binds IMU and remote to `*_sitl_stub` devices, while motor output already uses UDP.
**How to avoid:** Make Phase 3 planning treat input-side adapter truth as a first-class artifact field and a hard gate for LINK-01.
**Warning signs:** `bridge_stats_last.mit_seen` rises, but adapter binding still reports stub devices or remote/input freshness never changes.

### Pitfall 2: Confusing Declared Boundaries with Observed Boundaries
**What goes wrong:** Validation passes because declarations exist, even when actual observed boundaries or topics drift.
**Why it happens:** The current report model stores both declared and observed data, but planners could stop at declaration checks.
**How to avoid:** Require `bridge_protocol_match`, `runtime_boundary_match`, and `transport_ports_match` as explicit pass/fail checks for Phase 3.
**Warning signs:** Validation target status stays `declared_only` or summary shows observed counts below required counts.

### Pitfall 3: Losing Failure Layer Information in a Single “smoke failed” Status
**What goes wrong:** Developers cannot tell whether the issue is transport startup, stale input, missing observation, or wrong control behavior.
**Why it happens:** Current `smoke_result.primary_failure` identifies the first failed check but not a normalized failure layer.
**How to avoid:** Add a separate artifact field such as `failure_layer` and `failure_kind` driven by adapter binding, bridge stats, boundary drift, and runtime outputs.
**Warning signs:** Different failure modes collapse into the same top-level reason string.

### Pitfall 4: Adding a Better Simulator Instead of Better Proof
**What goes wrong:** The phase expands into toy plant modeling or richer synthetic feedback while the core trust gap remains unresolved.
**Why it happens:** Bridge code already contains simple motor integration, which can tempt scope creep.
**How to avoid:** Keep the only required proof target narrow: `actuator_command` and adapter/boundary diagnostics on the real runtime path.
**Warning signs:** New work starts discussing realism, physics fidelity, or digital-twin behavior without improving artifact truthfulness.

### Pitfall 5: Leaving Diagnostics as Optional Warnings
**What goes wrong:** Missing observations, port drift, or stale-link evidence appear in summaries but do not fail verification.
**Why it happens:** The current smoke flow still treats missing motor activity as warnings.
**How to avoid:** Promote the checks tied to LINK-01 through LINK-04 and OBS-01 through OBS-02 into required verification outcomes.
**Warning signs:** `smoke_health.warnings` contains the exact symptoms Phase 3 is supposed to prove or localize.

## Code Examples

Verified patterns from repository sources:

### Runtime Boundary and Port Matching
```python
declared_protocol = summary.get("bridge_protocol_declared")
observed_protocol = summary.get("bridge_protocol")
if isinstance(declared_protocol, dict) and isinstance(observed_protocol, dict):
    summary["bridge_protocol_match"] = declared_protocol == observed_protocol
```
Source: `robot_platform/sim/core/runner.py`

### Validation Target Status Derived from Observed Topics
```python
observed_source_topics = sorted(topic for topic in target.source_topics if topic in observed_topics)
is_observed = len(observed_source_topics) == len(target.source_topics)
target_status = "observed" if is_observed else "declared_only"
if observed_source_topics and not is_observed:
    target_status = "partial"
```
Source: `robot_platform/sim/projects/balance_chassis/validation.py`

### Bridge-Side Runtime Observation Event
```python
payload = {
    "topic": "actuator_command",
    "sample_count": sample_count,
    "command_kind": command_kind,
}
emit_event("runtime_output_observation", payload)
```
Source: `robot_platform/sim/backends/sitl_bridge.py`

### Current SITL Binding Reality
```c
static void bind_sitl_imu(platform_device_layer_t *layer)
{
  platform_bmi088_device_bind(&layer->imu, 0);
}

static void bind_sitl_remote(platform_device_layer_t *layer)
{
  platform_dbus_remote_device_bind(&layer->remote, 0);
}
```
Source: `robot_platform/runtime/device/device_profile_sitl.c`

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| Phase 1 minimal smoke proving only one required runtime output | Phase 2 host verification proves safety behavior on the current task/topic path | 2026-03-31 | Phase 3 can assume the control chain is already testable on host and focus on fake-link/runtime truth |
| Declared-only sim validation targets | Observed-topic validation target status builder in `balance_chassis/validation.py` | 2026-03-31 | The artifact model already supports observed vs declared proof; Phase 3 should expand it |
| Console-only smoke intuition | JSON-first smoke and verification artifacts | 2026-03-31 | Planner should keep machine-readable artifacts primary |

**Deprecated/outdated:**
- Treating `sim` as evidence of real fake-link ingress truth while IMU and remote are still stub-bound: outdated and unsafe for Phase 3 planning.
- Treating missing runtime observations as report decoration instead of a proof failure: outdated for LINK-02 and OBS-01.

## Open Questions

1. **Which exact runtime-backed ingress seam should replace the current SITL IMU and remote stubs?**
   - What we know: Current `device_profile_sitl.c` binds IMU and remote through stub devices, while motor output already crosses UDP.
   - What's unclear: Whether the cleanest Phase 3 move is to rebind those devices directly to existing SITL BSP UDP drivers or to introduce a narrower runtime-fed adapter above them.
   - Recommendation: Plan a first implementation step that audits existing SITL BSP read paths and chooses the thinnest runtime-backed binding that avoids duplicate control logic.

2. **Should Phase 3 add a new `verify phase3` command or extend `sim` plus `verify phase1/phase2`?**
   - What we know: `platform_cli.main` already owns report-driven verification commands and `sim` smoke execution.
   - What's unclear: Whether the cleanest planner shape is a dedicated `verify phase3` artifact or an extension of the current `sim` artifact plus a wrapper verification report.
   - Recommendation: Use a dedicated `verify phase3` wrapper that consumes the smoke artifact, because LINK/OBS requirements need a stable verification verdict separate from raw smoke output.

3. **How much dropped-packet or stale-input accounting exists in runtime code today?**
   - What we know: The bridge already exports counters for IMU sent, MIT seen, wheel seen, and feedback sent; `remote_intent.c` already tracks command freshness using timestamps and repeated samples.
   - What's unclear: Whether runtime-side adapters already expose enough packet-drop/staleness state to populate OBS-02 without additional instrumentation.
   - Recommendation: Plan one discovery/implementation task specifically for adapter diagnostics schema before broadening proof cases.

## Environment Availability

| Dependency | Required By | Available | Version | Fallback |
|------------|------------|-----------|---------|----------|
| `python3` | CLI, runner, Python tests | ✓ | 3.12.3 | — |
| `cmake` | SITL and host configure/build | ✓ | 3.28.3 | — |
| `ctest` | Host verification execution | ✓ | 3.28.3 | — |
| `gcc` | Linux SITL/host compilation | ✓ | 13.3.0 | — |
| `ninja` | Some build flows in repo | ✓ | 1.11.1 | Unix Makefiles already used for SITL/host paths |
| `arm-none-eabi-gcc` | Not required for core Phase 3 proof, but available for broader build coverage | ✓ | 13.2.1 | — |

**Missing dependencies with no fallback:**
- None identified for code/config and host/SITL planning.

**Missing dependencies with fallback:**
- None.

## Validation Architecture

### Test Framework
| Property | Value |
|----------|-------|
| Framework | Python `unittest` + CTest host tests |
| Config file | none — direct module invocation and CMake/CTest |
| Quick run command | `python3 -m unittest robot_platform.sim.tests.test_runner robot_platform.tools.platform_cli.tests.test_main -v` |
| Full suite command | `python3 -m robot_platform.tools.platform_cli.main verify phase3` |

### Phase Requirements → Test Map
| Req ID | Behavior | Test Type | Automated Command | File Exists? |
|--------|----------|-----------|-------------------|-------------|
| LINK-01 | Fake-link ingress drives real runtime path, not stub-only behavior | integration | `python3 -m robot_platform.tools.platform_cli.main verify phase3 --case runtime_binding -x` | ❌ Wave 0 |
| LINK-02 | Verification records observed runtime outputs | integration | `python3 -m robot_platform.tools.platform_cli.main verify phase3 --case runtime_outputs -x` | ❌ Wave 0 |
| LINK-03 | Artifact classifies communication vs control failure | unit/integration | `python3 -m unittest robot_platform.tools.platform_cli.tests.test_main -v` plus `verify phase3 --case classification -x` | ❌ Wave 0 |
| LINK-04 | Boundary/port/topic drift fails explicitly | unit/integration | `python3 -m unittest robot_platform.sim.tests.test_runner -v` plus `verify phase3 --case contract_drift -x` | ❌ Wave 0 |
| OBS-01 | Artifact records adapter binding status, outcomes, and failure reasons | unit/integration | `python3 -m unittest robot_platform.tools.platform_cli.tests.test_main -v` plus `verify phase3 --case artifact_schema -x` | ❌ Wave 0 |
| OBS-02 | Artifact exposes counters/diagnostics for drops, stale inputs, or missing observations | unit/integration | `python3 -m unittest robot_platform.sim.tests.test_runner -v` plus `verify phase3 --case diagnostics -x` | ❌ Wave 0 |

### Sampling Rate
- **Per task commit:** `python3 -m unittest robot_platform.sim.tests.test_runner robot_platform.tools.platform_cli.tests.test_main -v`
- **Per wave merge:** `python3 -m robot_platform.tools.platform_cli.main verify phase3`
- **Phase gate:** Full `verify phase3` green with all required LINK/OBS cases passing

### Wave 0 Gaps
- [ ] `robot_platform/tools/platform_cli/tests/test_main.py` needs Phase 3 verification-report coverage
- [ ] `robot_platform/sim/tests/test_runner.py` needs classification and diagnostics summary coverage
- [ ] A new Phase 3 verification entrypoint is not implemented yet
- [ ] No current tests assert adapter binding truth for SITL IMU/remote ingress

## Sources

### Primary (HIGH confidence)
- `.planning/phases/03-fake-link-runtime-proof/03-CONTEXT.md` - locked decisions, scope, and canonical references
- `.planning/REQUIREMENTS.md` - LINK-01 through LINK-04 and OBS-01 through OBS-02
- `.planning/ROADMAP.md` - Phase 3 goal and success criteria
- `.planning/STATE.md` - Phase sequencing and carry-forward decisions
- `.planning/PROJECT.md` - project trust model and constraints
- `.planning/codebase/CONCERNS.md` - known SITL stub-binding and observability gaps
- `CLAUDE.md` - project constraints and workflow requirements
- `robot_platform/tools/platform_cli/main.py` - authoritative `sim`/`verify` orchestration and artifact writing
- `robot_platform/sim/core/runner.py` - smoke artifact summarization and declared-vs-observed checks
- `robot_platform/sim/projects/balance_chassis/profile.py` - declared runtime boundary and validation targets
- `robot_platform/sim/projects/balance_chassis/validation.py` - observed runtime-output target classification
- `robot_platform/sim/projects/balance_chassis/bridge_adapter.py` - missing runtime-output adapter hook
- `robot_platform/sim/backends/sitl_bridge.py` - bridge counters and observation events
- `robot_platform/runtime/app/balance_chassis/app_bringup/remote_task.c` - runtime ingress path
- `robot_platform/runtime/control/state/observe_task.c` - runtime observation path
- `robot_platform/runtime/app/balance_chassis/app_bringup/chassis_task.c` - runtime control path
- `robot_platform/runtime/control/execution/motor_control_task.c` - runtime execution/output path
- `robot_platform/runtime/app/balance_chassis/app_intent/remote_intent.c` - stale-input and enable logic
- `robot_platform/runtime/app/balance_chassis/app_io/chassis_topics.c` - runtime-output observation seam
- `robot_platform/runtime/control/state/observe_topics.c` - observer topic ingress
- `robot_platform/runtime/control/execution/actuator_topics.c` - actuator topic ingress/feedback publish
- `robot_platform/runtime/device/device_profile_sitl.c` - current SITL binding composition
- `robot_platform/runtime/device/imu/bmi088_device_sitl.c` - current IMU SITL stub
- `robot_platform/runtime/device/remote/dbus_remote_device_sitl.c` - current remote SITL stub
- `robot_platform/runtime/device/actuator/motor/motor_actuator_device_sitl.c` - current UDP-backed output path
- `robot_platform/sim/tests/test_runner.py` - current runner/report regression coverage
- `robot_platform/tools/platform_cli/tests/test_main.py` - current CLI/report regression coverage

### Secondary (MEDIUM confidence)
- Local environment probes on 2026-04-01 for `python3`, `cmake`, `ctest`, `gcc`, `ninja`, and `arm-none-eabi-gcc`

### Tertiary (LOW confidence)
- None

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH - Based on current repo entrypoints, installed toolchain probes, and passing in-repo tests
- Architecture: HIGH - Based on direct inspection of authoritative runtime, bridge, and runner paths named in phase context
- Pitfalls: HIGH - Based on explicit known concerns plus concrete code inspection of current stub bindings and artifact behavior

**Research date:** 2026-04-01
**Valid until:** 2026-04-08
