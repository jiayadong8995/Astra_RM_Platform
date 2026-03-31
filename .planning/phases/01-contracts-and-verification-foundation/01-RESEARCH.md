# Phase 1: Contracts and Verification Foundation - Research

**Researched:** 2026-03-30
**Domain:** Repo-native contract hardening, host-side C verification, and minimum SITL smoke closure
**Confidence:** MEDIUM-HIGH

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions
- **D-01:** Phase 1 should build the smallest viable host-side C test loop first, not a full fake ecosystem.
- **D-02:** The first host-side C test coverage surface should prioritize `message_center` and `actuator_gateway` as the smallest viable high-risk seams.
- **D-03:** Phase 1 should define only thin fake/stub seam contracts needed to run those host tests; it should not introduce a broad or highly abstract fake framework yet.
- **D-04:** `device_layer` should not be treated as mandatory first-wave scope. It may enter Phase 1 only if the host harness is already stable enough to absorb a broader seam without bloating the initial closure loop.

- **D-05:** Phase 1 must stop relying on the current fixed `64B` message payload assumption in `message_center`.
- **D-06:** Message transport should move to per-topic declared payload sizing with registration-time validation instead of continuing with global fixed-buffer gambling.
- **D-07:** Unsafe topic registrations should fail explicitly during setup rather than being silently accepted and left to runtime corruption.

- **D-08:** Phase 1 should standardize on one machine-readable JSON result as the primary verification artifact.
- **D-09:** CLI output should be a human-friendly summary layered on top of that JSON artifact, not the primary source of truth.
- **D-10:** Phase 1 should prefer a single top-level report for one verification run rather than a heavier multi-file reporting system.

- **D-11:** Generated-artifact freshness checks should be a hard gate in Phase 1, not just a warning.
- **D-12:** If the checked-in generated STM32 assets are stale relative to their source inputs, firmware output must be treated as untrusted and the pipeline must stop.

- **D-13:** Phase 1 planning should explicitly prioritize proving the minimum live path `build sitl -> launch sitl -> bridge up -> inject one input -> observe one runtime output -> produce passed smoke report`.
- **D-14:** If a Phase 1 task does not contribute to making that minimum live path trustworthy or to hardening its prerequisites, it should be treated as secondary.

### Claude's Discretion
- The specific host C unit test library and fake/stub helper choice may be selected by research/planning as long as the result stays lightweight and compatible with the existing CMake-based build graph.
- The exact JSON schema may be designed by the agent as long as it clearly captures stage identity, pass/fail state, and failure reason for a single verification run.
- The precise stale-artifact detection mechanism may be chosen by the agent as long as it is deterministic, hard-failing, and usable in the default development loop.
- The exact point at which `device_layer` enters the phase may be chosen by the agent based on harness maturity, as long as the smallest viable closure path is not diluted.

### Deferred Ideas (OUT OF SCOPE)
- Broader fake-link runtime proof belongs to Phase 3, not this phase.
- Control safety behavior regression depth beyond the selected `message_center` / `actuator_gateway` first-wave surface belongs mainly to Phase 2.
- Richer per-stage reporting files or a more elaborate report tree can be added later if the single-run JSON report proves too narrow.
</user_constraints>

## Project Constraints (from CLAUDE.md)

- Preserve the reusable platform direction; do not collapse Phase 1 into a one-off `balance_chassis` hack.
- Treat pre-hardware verification as a safety gate, not as optional convenience tooling.
- Keep fake-link and host validation scoped as staged evidence, not as a substitute for real hardware proof.
- Preserve existing runtime layering where it has leverage, but actively reduce implementation-level trust gaps.
- Work within the current toolchain reality: C, Python, CMake, STM32CubeMX, cross-compilers, and SITL.
- Use `balance_chassis` as the proving path without redefining the whole platform around it.
- Keep changes inside a GSD workflow and align with repo conventions: CMake-based builds, Python `unittest`, sparse comments, plain-text CLI logs, and `snake_case`.

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|------------------|
| PIPE-02 | Build/test/generate failures return machine-readable results that identify which stage failed | Extend `robot_platform.tools.platform_cli.main` to orchestrate a single verification run and emit one JSON artifact with per-stage results. |
| PIPE-03 | The project detects when generated STM32 artifacts are stale relative to the checked-in board source inputs before firmware output is trusted | Add deterministic freshness manifest/check logic around `generate` and hardware-producing commands. |
| HOST-01 | Developer can run host-native C tests for safety-critical runtime modules without requiring robot hardware | Add a Linux host C test target in `robot_platform/CMakeLists.txt` for `message_center` and `actuator_gateway`. |
| HOST-04 | Host-side verification reports sanitizer failures for memory-safety or undefined-behavior defects in supported test targets | Build host C tests with ASan and UBSan enabled by default in the host test path. |
| ARCH-02 | Runtime contracts and message transport reject unsafe payload sizing instead of silently allowing overflow-prone behavior | Replace the global 64-byte subscriber buffer assumption in `message_center` with per-topic sizing and explicit registration-time failure. |
</phase_requirements>

## Summary

Phase 1 should stay narrow and foundational. The repo already has a Python smoke-report path and a working SITL build path, but it does not yet have a host-native C harness, does not hard-fail on stale generated STM32 assets, and still relies on a memory-unsafe fixed `64B` topic payload assumption. Empirical inspection confirms the risk: current runtime contracts are much larger than `64B` (`platform_robot_state_t=196`, `platform_actuator_command_t=204`, `platform_device_feedback_t=212`, `platform_device_input_t=284`), so the present `message_center` implementation is structurally unsafe.

The shortest trustworthy Phase 1 plan is:
1. Make `message_center` reject unsafe sizing and support per-topic declared payload storage.
2. Add a minimal host C test harness inside the existing CMake graph, with sanitizer-on coverage for `message_center` and `actuator_gateway`.
3. Unify stage reporting into one JSON verification artifact emitted by the CLI.
4. Add a deterministic generated-artifact freshness gate for hardware-producing commands.
5. Close the minimum live path by proving one real runtime output that already exists in the runtime, instead of keeping the current sim declaration for `chassis_state` / `leg_left` / `leg_right`, which is not backed by real observation.

**Primary recommendation:** Plan Phase 1 around one closure loop: harden `message_center`, stand up sanitizer-backed host C tests, then align the smoke path to one actually published runtime output (`robot_state` or `actuator_command`) and report the whole run through a single JSON artifact.

## Standard Stack

### Core
| Library / Tool | Version | Purpose | Why Standard |
|---------|---------|---------|--------------|
| CMake | 3.22+ in repo, `3.23.0` installed | Build graph for firmware, SITL, and new host C tests | Already owns all C targets; no second build system needed |
| GCC (host) | `13.4.0` installed | Compile Linux host C tests with sanitizers | Already available and supports ASan/UBSan |
| Python `unittest` | stdlib on Python `3.11.9` | CLI/report regression tests | Already used in `robot_platform/tools/platform_cli/main.py` |
| Existing `platform_cli` | repo-local | Stage orchestration and report emission | Already the single command surface |

### Supporting
| Library / Tool | Version | Purpose | When to Use |
|---------|---------|---------|-------------|
| Ninja | `1.10.1` installed | Hardware build generator | Hardware-producing build path |
| Unix Makefiles | CMake built-in | SITL and host-test generator | Existing Linux paths already use it |
| `arm-none-eabi-gcc` | `10.3.1` installed | Hardware firmware builds | Hardware path only |
| STM32CubeMX | `6.17.0` in repo/docs/backend | Code generation source of truth | Generation and freshness metadata |
| OpenJDK | `21.0.10` installed | CubeMX runtime dependency | Codegen only |

### Alternatives Considered
| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| Plain CMake test executables + CTest | Unity/CMock or another C test framework | Extra vendoring and framework surface before the repo has even one stable host harness |
| Single JSON verification artifact | Per-stage JSON files | Heavier artifact management with no Phase 1 payoff |
| Content-manifest freshness gate | mtime-only stale detection | mtime is not deterministic across git checkouts and is too weak for a trust gate |

**Installation:** reuse the toolchain already documented in `robot_platform/docs/wsl_environment_setup.md`; no new package manager should enter Phase 1.

## Architecture Patterns

### Recommended Project Structure
```text
robot_platform/
├── CMakeLists.txt                       # add host test targets and sanitizer options
├── runtime/
│   ├── module/message_center/          # contract-safe transport changes
│   ├── control/execution/              # actuator_gateway target under test
│   └── tests/host/                     # new minimal host C harness files
├── sim/
│   ├── core/runner.py                  # consume unified smoke/verification results
│   └── projects/balance_chassis/       # align one observed runtime output to real runtime behavior
└── tools/
    ├── platform_cli/main.py            # stage orchestration + top-level JSON artifact
    └── cubemx_backend/main.py          # write/read freshness manifest
```

### Pattern 1: Host C Tests Live Inside The Main CMake Graph
**What:** Add Linux-only host test executables under the existing `robot_platform/CMakeLists.txt`, not a parallel harness.
**When to use:** For `message_center` and `actuator_gateway` first; expand only after the harness is stable.
**Example:**
```cmake
option(PLATFORM_HOST_TESTS "Build host-native runtime tests" OFF)
option(PLATFORM_HOST_TEST_SANITIZERS "Enable ASan/UBSan for host tests" ON)

add_executable(test_message_center
  runtime/tests/host/test_message_center.c
  runtime/module/message_center/message_center.c
)
add_test(NAME test_message_center COMMAND test_message_center)
```

### Pattern 2: Thin Stubs, Not A Fake Framework
**What:** For `actuator_gateway`, provide only the fake `platform_device_*` functions needed to capture written commands and seed feedback.
**When to use:** Whenever a control/execution module depends on `device_layer` entrypoints but Phase 1 does not need full device-layer behavior.
**Example:**
```c
static platform_device_command_t g_last_command;

platform_device_result_t platform_device_write_default_command(
    const platform_device_command_t *command)
{
    g_last_command = *command;
    return PLATFORM_DEVICE_RESULT_OK;
}
```

### Pattern 3: JSON-First Verification Run
**What:** Introduce one top-level verification report that records every stage (`freshness`, `build_sitl`, `host_tests`, `smoke`) with `status`, `exit_code`, `duration_s`, and `failure_reason`.
**When to use:** For the new authoritative Phase 1 command path and for any CLI stage that needs machine-readable failure output.
**Example:**
```json
{
  "verification_run_version": 1,
  "project": "balance_chassis",
  "overall_status": "failed",
  "stages": [
    {"name": "build_sitl", "status": "passed", "exit_code": 0},
    {"name": "smoke", "status": "failed", "exit_code": 1, "failure_reason": "bridge_startup_error"}
  ]
}
```

### Pattern 4: Phase 1 Smoke Must Observe One Real Existing Runtime Output
**What:** Collapse the minimum live proof to one runtime topic that the C runtime already publishes. Today that means `robot_state` or `actuator_command`, not the sim-declared `chassis_state` / `leg_left` / `leg_right`.
**When to use:** Before adding any richer runtime output taxonomy.
**Example:**
```c
// In a SITL-only observation hook, emit a tiny summary when robot_state publishes.
printf("[RuntimeOutput] topic=robot_state sequence=%lu valid=%d\n",
       (unsigned long)robot_state->sequence,
       robot_state->health.state_valid ? 1 : 0);
```

### Anti-Patterns to Avoid
- **Broad fake runtime abstraction:** It dilutes the smallest viable closure loop and violates D-01 through D-04.
- **Keeping the `64B` ceiling and just adding checks:** That still fails `ARCH-02` because the actual contracts are 196-284 bytes.
- **Using declared sim outputs as proof without observation:** The current `balance_chassis` adapter returns no runtime observations at all.
- **Treating CLI stdout as the authoritative artifact:** It blocks `PIPE-02` automation.

## Recommended Sequencing

1. **Contract safety first:** land `message_center` sizing redesign and registration failure behavior before any smoke-proof expansion.
2. **Host harness second:** add host C test targets and sanitizer defaults for `message_center`, then `actuator_gateway`.
3. **Single verification report third:** add a CLI path that runs staged checks and emits one top-level JSON artifact.
4. **Freshness hard gate fourth:** make hardware-producing commands fail when generated artifacts are stale or manifest data is missing.
5. **Minimum live path last:** align smoke validation to one actual runtime output and prove the `build sitl -> launch sitl -> bridge up -> inject one input -> observe one runtime output -> passed report` chain.

**Prove first:** `message_center` safety, sanitizer-capable host harness, single JSON artifact, and one real observable runtime output.

**Defer:** `device_layer` host coverage, broad fake-link fidelity, multi-output runtime observation trees, and deeper control-safety assertions.

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Host test execution | A second custom shell-based harness | CMake test executables + CTest | The repo already trusts CMake; a second harness adds drift |
| `actuator_gateway` dependencies | A full fake device ecosystem | Two or three thin C stubs for `platform_device_*` APIs | First-wave coverage only needs seam capture |
| Verification artifacts | A report directory tree | One JSON file plus human summary print | Matches D-08 through D-10 |
| Stale generated detection | mtime heuristics | Manifest with source hash, generated hash set, and tool version | Deterministic across checkouts |

**Key insight:** Phase 1 should build trust in the current repo shape, not create a new framework that itself becomes untrusted.

## Common Pitfalls

### Pitfall 1: Fixing `message_center` by only rejecting payloads `>64B`
**What goes wrong:** Registration fails for the actual runtime contracts, but the transport architecture remains unusable.
**Why it happens:** The current contracts are already far above `64B`.
**How to avoid:** Store payload size per topic and allocate subscriber/topic storage to that declared size or an explicit per-topic storage class.
**Warning signs:** `robot_state`, `actuator_command`, or `device_feedback` topics can no longer register after the “fix.”

### Pitfall 2: Designing the smoke proof around fake declared topics
**What goes wrong:** The report passes declaration checks but still observes no real runtime output.
**Why it happens:** `balance_chassis` currently publishes `robot_state` and `actuator_command`, while the sim profile declares `chassis_state` / `leg_left` / `leg_right`.
**How to avoid:** Pick one output that already exists in runtime code and align the sim proof to that first.
**Warning signs:** `validation_targets_status` stays `declared_only`.

### Pitfall 3: Letting freshness checks live only in documentation
**What goes wrong:** Developers keep producing hardware artifacts from stale generated code.
**Why it happens:** The current generate/build path has no hard trust boundary.
**How to avoid:** Put the freshness gate directly in CLI paths that lead to firmware output.
**Warning signs:** `build hw_elf` succeeds after an `.ioc` change without regeneration.

### Pitfall 4: Confusing sandbox UDP restrictions with a repo defect
**What goes wrong:** The plan “fixes” the bridge when the failure is environmental.
**Why it happens:** In this research session, `sim --duration 1` built successfully but the bridge failed to initialize UDP sockets with `[Errno 1] Operation not permitted`.
**How to avoid:** Treat that specific observation as environment-sensitive unless reproduced outside sandbox restrictions.
**Warning signs:** Build is green, SITL starts, but bridge dies before stats with a socket permission error.

## Code Examples

Verified patterns from current repo code and repo-aligned implementation sketches:

### Machine-readable smoke writer already exists
```python
def write_report(report_path: Path, payload: dict) -> None:
    report_path.parent.mkdir(parents=True, exist_ok=True)
    report_path.write_text(json.dumps(payload, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
```
Source: `robot_platform/sim/reports/report_writer.py`

### Current unsafe topic registration pattern that Phase 1 must harden
```c
bus->robot_state_pub = PubRegister("robot_state", sizeof(platform_robot_state_t));
bus->actuator_command_pub = PubRegister("actuator_command", sizeof(platform_actuator_command_t));
```
Source: `robot_platform/runtime/app/balance_chassis/app_io/chassis_topics.c`

### Current minimal CLI test entry shape to extend, not replace
```python
return _run(
    [
        sys.executable,
        "-m",
        "unittest",
        "robot_platform.sim.tests.test_runner",
        "robot_platform.tools.platform_cli.tests.test_main",
        "-v",
    ],
    cwd=repo_root,
)
```
Source: `robot_platform/tools/platform_cli/main.py`

## State of the Art

| Old Approach | Current Recommended Approach | When Changed | Impact |
|--------------|------------------------------|--------------|--------|
| Fixed `64B` subscriber buffer for all topics | Per-topic declared payload sizing with explicit registration failure | Phase 1 | Removes silent overflow risk and meets `ARCH-02` |
| Python-only parser/report tests | Add host C tests for runtime seams inside CMake | Phase 1 | Makes runtime refactors safer before hardware |
| Ad hoc smoke JSON owned only by `sim` | One top-level verification artifact spanning all stages | Phase 1 | Meets `PIPE-02` and becomes automation-friendly |
| Trust checked-in generated files by convention | Hard freshness manifest gate for hardware-producing commands | Phase 1 | Prevents untrusted firmware outputs |

**Deprecated/outdated:**
- The current assumption that “topic buffer must be >= largest struct” is not acceptable as a safety rule.
- The current declared output boundary in `sim/projects/balance_chassis/profile.py` is too ambitious for Phase 1 because it is not connected to actual runtime observation.

## Open Questions

1. **Which single runtime output should Phase 1 observe first: `robot_state` or `actuator_command`?**
   - What we know: both are already published in `chassis_topics.c`.
   - What's unclear: which one is easier to expose from SITL with minimal instrumentation and least semantic ambiguity.
   - Recommendation: choose one and make it the only required observed runtime output for the smoke gate; `robot_state` is the cleaner default because it is already the main state contract.

2. **Where should freshness metadata live?**
   - What we know: the generate path already owns the `.ioc` source and output directory.
   - What's unclear: whether the manifest should live under `runtime/generated/...` or under `build/`.
   - Recommendation: store the authoritative manifest next to the checked-in generated tree so trust is tied to the committed artifact set, not a transient build directory.

## Environment Availability

| Dependency | Required By | Available | Version | Fallback |
|------------|------------|-----------|---------|----------|
| `python3` | CLI, tests, smoke runner | ✓ | `3.11.9` | — |
| `cmake` | SITL, hardware, host C tests | ✓ | `3.23.0` | — |
| `ninja` | hardware build path | ✓ | `1.10.1` | Unix Makefiles for non-hardware paths |
| `gcc` | host C tests | ✓ | `13.4.0` | — |
| `arm-none-eabi-gcc` | hardware firmware build | ✓ | `10.3.1` | — |
| `java` | CubeMX backend | ✓ | `21.0.10` | — |
| `STM32CubeMX` | generation/freshness authority | Repo/backend documents `6.17.0`; binary not re-probed in this session | `6.17.0` expected | Use existing checked-in generated tree for non-generate tasks |
| Local UDP socket permissions | SITL bridge live smoke path | ✗ in this sandboxed session | — | Rerun smoke outside sandbox to validate the real bridge path |

**Missing dependencies with no fallback:**
- None for planning. A real Phase 1 live-smoke proof still needs an environment where the UDP bridge can open sockets.

**Missing dependencies with fallback:**
- Local UDP socket permissions are blocked in this sandbox, but host-side planning and non-live validation can proceed.

## Validation Architecture

### Test Framework
| Property | Value |
|----------|-------|
| Framework | Python `unittest` plus new CMake/CTest host C tests |
| Config file | none — hard-coded in `robot_platform/tools/platform_cli/main.py` today |
| Quick run command | `python3 -m robot_platform.tools.platform_cli.main test sim` |
| Full suite command | `python3 -m robot_platform.tools.platform_cli.main test sim` plus new host C test command exposed through `platform_cli` |

### Phase Requirements → Test Map
| Req ID | Behavior | Test Type | Automated Command | File Exists? |
|--------|----------|-----------|-------------------|-------------|
| PIPE-02 | One JSON artifact reports stage-level pass/fail and failure reason | Python unit + CLI integration | `python3 -m unittest robot_platform.tools.platform_cli.tests.test_main -v` plus new artifact-shape tests | ❌ Wave 0 |
| PIPE-03 | Stale generated artifacts hard-fail hardware trust path | Python unit | new freshness-check tests under `robot_platform/tools/platform_cli/tests/` | ❌ Wave 0 |
| HOST-01 | Host-native C tests run without hardware | CTest smoke | new `platform_cli test host` or equivalent | ❌ Wave 0 |
| HOST-04 | Sanitizer failures are surfaced for host targets | CTest + failing sample/manual verifier | same host-test command with sanitizers enabled | ❌ Wave 0 |
| ARCH-02 | Unsafe payload sizing is rejected at registration | Host C unit | host `message_center` tests | ❌ Wave 0 |

### Sampling Rate
- **Per task commit:** `python3 -m robot_platform.tools.platform_cli.main test sim` and the new host test command for touched Phase 1 paths
- **Per wave merge:** run the full Phase 1 verification command that emits the single JSON artifact
- **Phase gate:** successful minimum live path report plus green host C tests with sanitizers enabled

### Wave 0 Gaps
- [ ] `robot_platform/runtime/tests/host/test_message_center.c` — covers `ARCH-02` and the `HOST-01` entrypoint
- [ ] `robot_platform/runtime/tests/host/test_actuator_gateway.c` — covers `HOST-01` and sanitizer plumbing
- [ ] `robot_platform/tools/platform_cli/tests/test_verification_report.py` — locks the Phase 1 JSON artifact schema for `PIPE-02`
- [ ] `robot_platform/tools/platform_cli/tests/test_freshness_gate.py` — locks `PIPE-03`
- [ ] CLI command wiring for host tests and top-level verification in `robot_platform/tools/platform_cli/main.py`
- [ ] CMake/CTest integration for host runtime tests in `robot_platform/CMakeLists.txt`

## Sources

### Primary (HIGH confidence)
- `CLAUDE.md` - project constraints, stack, architecture, and workflow expectations
- `.planning/phases/01-contracts-and-verification-foundation/01-CONTEXT.md` - locked scope and phase decisions
- `.planning/REQUIREMENTS.md` - authoritative requirement text and phase mapping
- `robot_platform/tools/platform_cli/main.py` - current orchestration entrypoints
- `robot_platform/CMakeLists.txt` - current build graph and sanitizer precedent
- `robot_platform/runtime/module/message_center/message_center.h`
- `robot_platform/runtime/module/message_center/message_center.c`
- `robot_platform/runtime/control/execution/actuator_gateway.c`
- `robot_platform/runtime/app/balance_chassis/app_io/chassis_topics.c`
- `robot_platform/sim/core/runner.py`
- `robot_platform/sim/projects/balance_chassis/profile.py`
- `robot_platform/sim/projects/balance_chassis/bridge_adapter.py`
- `robot_platform/sim/reports/report_writer.py`
- `robot_platform/tools/cubemx_backend/main.py`

### Secondary (MEDIUM confidence)
- `.planning/codebase/CONCERNS.md` - repo risk inventory, confirmed against code reads
- `.planning/codebase/TESTING.md` - current validation gaps and run commands
- `robot_platform/tools/platform_cli/README.md`
- `robot_platform/sim/README.md`
- `robot_platform/docs/generated_import_rules.md`
- `robot_platform/docs/wsl_environment_setup.md`

### Tertiary (LOW confidence)
- `python3 -m robot_platform.tools.platform_cli.main sim --duration 1` execution in this sandbox - useful for current command behavior, but the UDP socket failure may be environment-specific

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH - derived from repo build files, installed tools, and current command execution
- Architecture: MEDIUM-HIGH - strong repo evidence, but the exact smoke output observation hook still needs a Phase 1 design choice
- Pitfalls: HIGH - directly confirmed by current code and observed command behavior

**Research date:** 2026-03-30
**Valid until:** 2026-04-29
