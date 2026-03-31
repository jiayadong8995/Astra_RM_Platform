---
phase: 01-contracts-and-verification-foundation
plan: "02"
subsystem: testing
tags: [c, cmake, sanitizers, message_center, contracts]
requires:
  - phase: 01-contracts-and-verification-foundation
    provides: Host test entrypoint and sanitizer-backed CTest target from Plan 01-01
provides:
  - Declared-size `message_center` transport with explicit registration failures
  - Host regressions for mismatched topic sizes, null publish inputs, and real runtime contract round-trips
  - Payload-size coverage beyond the legacy 8-bit ceiling via `platform_device_input_t`
affects: [phase-01, phase-02, balance_chassis, runtime-contracts]
tech-stack:
  added: []
  patterns: [static topic payload pool, generation-based subscriber reads, host-side runtime contract regression tests]
key-files:
  created: [.planning/phases/01-contracts-and-verification-foundation/01-02-SUMMARY.md]
  modified: [robot_platform/runtime/module/message_center/message_center.h, robot_platform/runtime/module/message_center/message_center.c, robot_platform/runtime/tests/host/test_message_center.c]
key-decisions:
  - "Store one declared-size payload buffer per topic in a static byte pool, then let subscribers track read generation instead of owning fixed local buffers."
  - "Widen topic payload sizing from `uint8_t` to `size_t` so current runtime contracts, including `platform_device_input_t`, are representable."
patterns-established:
  - "Host transport regressions use real runtime contract headers instead of synthetic stand-ins."
  - "Unsafe topic registrations fail during setup when names, sizes, or static storage are invalid."
requirements-completed: [HOST-01, HOST-04, ARCH-02]
duration: 2min
completed: 2026-03-31
---

# Phase 1 Plan 02: Message-center transport hardening summary

**Declared-size `message_center` transport with static per-topic storage, explicit registration failure, and host regressions for the real `balance_chassis` contracts**

## Performance

- **Duration:** 2 min
- **Started:** 2026-03-31T14:01:32+08:00
- **Completed:** 2026-03-31T06:03:19Z
- **Tasks:** 2
- **Files modified:** 3

## Accomplishments
- Replaced the bootstrap host test with transport regressions that cover mismatched registration, null publish inputs, and round-trips for `platform_robot_state_t`, `platform_actuator_command_t`, `platform_device_feedback_t`, and `platform_device_input_t`.
- Removed the fixed `64B` subscriber buffer design and widened declared payload sizing so the transport can represent contracts larger than `255` bytes.
- Kept the implementation static-memory only while making invalid topic setup fail deterministically instead of deferring errors into runtime corruption.

## Task Commits

Each task was committed atomically:

1. **Task 1: Replace the bootstrap `message_center` test with failing contract-size and registration tests** - `dd099fdc` (test)
2. **Task 2: Implement declared-size topic storage and explicit registration failures** - `fcda0d99` (fix)

## Files Created/Modified
- `robot_platform/runtime/tests/host/test_message_center.c` - Host transport regression spec using the real runtime contract headers and a compile-time assertion that `platform_device_input_t` exceeds the legacy 8-bit size ceiling.
- `robot_platform/runtime/module/message_center/message_center.h` - Public API widened to `size_t` payload sizing with static-pool configuration for declared-size transport.
- `robot_platform/runtime/module/message_center/message_center.c` - Static topic payload pool, registration validation, null-input guards, and generation-based subscriber reads.

## Decisions Made
- Used one topic-owned payload buffer plus subscriber generation tracking instead of per-subscriber copies. This preserved zero-malloc behavior and removed the fixed 64-byte transport ceiling without multiplying storage for each subscriber.
- Made registration reject invalid names, zero-length payloads, mismatched duplicate sizes, and payload-pool exhaustion. That moves failure to setup time, which is the trust boundary this phase needed.

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered

- `ctest` initially passed because the existing host test binary was stale. Rebuilding `test_message_center` before rerunning restored the expected TDD RED step and then verified the GREEN step after implementation.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

- `message_center` no longer depends on the overflow-prone `64B` payload assumption and now rejects unsafe declarations before publish-time corruption can occur.
- The host harness now proves the real Phase 1 contracts, including `platform_device_input_t`, through sanitizer-backed transport coverage and is ready to support later control-path verification work.

## Self-Check: PASSED

- Found `.planning/phases/01-contracts-and-verification-foundation/01-02-SUMMARY.md`.
- Found task commits `dd099fdc` and `fcda0d99` in `git log --oneline --all`.
- No blocking stubs were detected in the files modified by this plan.
