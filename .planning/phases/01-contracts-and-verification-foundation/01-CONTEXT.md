# Phase 1: Contracts and Verification Foundation - Context

**Gathered:** 2026-03-30
**Status:** Ready for planning

<domain>
## Phase Boundary

This phase establishes the minimum trustworthy verification foundation for the existing platform before deeper control-path validation begins. It covers host-native C test entrypoints, machine-readable stage reporting, generated-artifact freshness enforcement, and contract-safe message transport behavior, but does not yet expand into the broader fake-link runtime proof or full safety-control verification phases.

</domain>

<decisions>
## Implementation Decisions

### Host Test Harness Scope
- **D-01:** Phase 1 should build the smallest viable host-side C test loop first, not a full fake ecosystem.
- **D-02:** The first host-side C test coverage surface should include `message_center`, `device_layer`, and `actuator_gateway`.
- **D-03:** Phase 1 should define only thin fake/stub seam contracts needed to run those host tests; it should not introduce a broad or highly abstract fake framework yet.

### Message Contract Safety
- **D-04:** Phase 1 must stop relying on the current fixed `64B` message payload assumption in `message_center`.
- **D-05:** Message transport should move to per-topic declared payload sizing with registration-time validation instead of continuing with global fixed-buffer gambling.
- **D-06:** Unsafe topic registrations should fail explicitly during setup rather than being silently accepted and left to runtime corruption.

### Verification Reporting
- **D-07:** Phase 1 should standardize on one machine-readable JSON result as the primary verification artifact.
- **D-08:** CLI output should be a human-friendly summary layered on top of that JSON artifact, not the primary source of truth.
- **D-09:** Phase 1 should prefer a single top-level report for one verification run rather than a heavier multi-file reporting system.

### Generated Artifact Freshness
- **D-10:** Generated-artifact freshness checks should be a hard gate in Phase 1, not just a warning.
- **D-11:** If the checked-in generated STM32 assets are stale relative to their source inputs, firmware output must be treated as untrusted and the pipeline must stop.

### the agent's Discretion
- The specific host C unit test library and fake/stub helper choice may be selected by research/planning as long as the result stays lightweight and compatible with the existing CMake-based build graph.
- The exact JSON schema may be designed by the agent as long as it clearly captures stage identity, pass/fail state, and failure reason for a single verification run.
- The precise stale-artifact detection mechanism may be chosen by the agent as long as it is deterministic, hard-failing, and usable in the default development loop.

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Phase Scope and Requirements
- `.planning/ROADMAP.md` — Phase 1 goal, requirement mapping, and success criteria
- `.planning/REQUIREMENTS.md` — Phase 1 requirements `PIPE-02`, `PIPE-03`, `HOST-01`, `HOST-04`, and `ARCH-02`
- `.planning/PROJECT.md` — project-level constraints and the overall v1/v2 trust model
- `.planning/STATE.md` — current project focus and recent roadmap decisions

### Existing Build and Verification Entry Points
- `robot_platform/tools/platform_cli/main.py` — current CLI command surface, `_run_tests`, `generate`, and stage orchestration entrypoints
- `robot_platform/tools/platform_cli/README.md` — current command expectations for `generate` and `test sim`
- `robot_platform/sim/README.md` — current smoke-test expectations and verification behavior
- `robot_platform/CMakeLists.txt` — active build graph for hardware and SITL targets

### Message Transport and Contract Risk
- `.planning/codebase/CONCERNS.md` — current message sizing, generated-artifact drift, and verification-gap risks
- `.planning/codebase/TESTING.md` — current automated testing surface and gaps
- `robot_platform/runtime/module/message_center/message_center.h` — current message-center limits and API
- `robot_platform/runtime/module/message_center/message_center.c` — current transport behavior and sizing assumptions
- `robot_platform/runtime/control/contracts/robot_state.h` — representative runtime contract larger than the current fixed buffer assumption
- `robot_platform/runtime/control/contracts/actuator_command.h` — representative actuator command contract carried through message transport
- `robot_platform/runtime/control/contracts/device_feedback.h` — representative feedback contract carried through message transport
- `robot_platform/runtime/control/contracts/device_input.h` — representative input contract carried through message transport

### Generated Artifact Rules and Codegen Constraints
- `robot_platform/tools/cubemx_backend/main.py` — current STM32CubeMX generation backend and output behavior
- `robot_platform/tools/cubemx_backend/README.md` — current assumptions and limitations of the CubeMX generation flow
- `robot_platform/docs/generated_import_rules.md` — rules governing generated code boundaries and regeneration expectations
- `robot_platform/docs/wsl_environment_setup.md` — current documented environment and generation workflow constraints
- `Astra_RM2025_Balance/Chassis/CtrlBoard-H7_IMU.ioc` — current board source input for generated STM32 artifacts

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- `robot_platform/tools/platform_cli/main.py`: already centralizes CLI stages and is the natural integration point for Phase 1 verification entrypoints and final JSON report emission.
- `robot_platform/sim/core/runner.py`: already builds structured smoke summaries and can inform the shape of a single machine-readable report for later pipeline stages.
- `robot_platform/sim/tests/test_runner.py` and `robot_platform/tools/platform_cli/tests/test_main.py`: existing deterministic Python tests provide examples of report-oriented assertions and command-surface checks.
- `robot_platform/runtime/module/message_center/message_center.c`: the highest-value first target for host-side C coverage because it is central and currently unsafe.
- `robot_platform/runtime/device/device_layer.c` and `robot_platform/runtime/control/execution/actuator_gateway.c`: the next thin seams for initial host-side C coverage because they sit on phase-relevant transport and execution boundaries.

### Established Patterns
- The repository already prefers a single CLI entrypoint rather than many separate scripts.
- Python-side verification logic currently favors deterministic, data-driven tests and machine-readable summaries rather than complex integration harnesses.
- The C build already runs through CMake, so new host-side C tests should fit that graph rather than introduce an unrelated build system.
- Runtime code is largely static-allocation and pointer-based, so host tests should stay close to that style instead of requiring heavy runtime indirection.

### Integration Points
- New host C tests should plug into the existing build/test path rooted at `robot_platform/tools/platform_cli/main.py` and `robot_platform/CMakeLists.txt`.
- Message-size validation work should anchor in `robot_platform/runtime/module/message_center/` and the contract headers under `robot_platform/runtime/control/contracts/`.
- Generated-artifact freshness enforcement should sit on the path between `generate`, `build`, and firmware-producing commands rather than live as a passive documentation rule only.

</code_context>

<specifics>
## Specific Ideas

- The initial host verification surface should stay intentionally narrow: bring up a minimal loop first, prove it is useful, then widen later phases rather than designing a complete fake universe upfront.
- The project should stop "betting on 64B" for transport payloads; Phase 1 is the point to make that unsafe assumption illegal.
- Verification artifacts should be JSON-first so later phases can build more automation and gating on top of them.

</specifics>

<deferred>
## Deferred Ideas

- Broader fake-link runtime proof belongs to Phase 3, not this phase.
- Control safety behavior regression depth beyond the selected `message_center` / `device_layer` / `actuator_gateway` surface belongs mainly to Phase 2.
- Richer per-stage reporting files or a more elaborate report tree can be added later if the single-run JSON report proves too narrow.

</deferred>

---
*Phase: 01-contracts-and-verification-foundation*
*Context gathered: 2026-03-30*
