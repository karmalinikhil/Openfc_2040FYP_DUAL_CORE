# OpenFC2040 Progress Report (Dual-Core / SMP Bring-up)

Last updated: 2026-03-22

## Goal
Bring up PX4 + NuttX on RP2040 with dual-core scheduling enabled (SMP), while preserving a usable NSH console on UART0.

## What Is Working
- SMP boot path now reaches deep kernel bring-up reliably.
- System consistently transitions through `nx_start`, scheduler unlock, app/init task launch, and NSH entry path.
- Build and BOOTSEL flash pipeline is stable.
- UF2 address issue is handled by converting BIN -> UF2 for RP2040 base `0x10000000`.

## Major Confirmed Fixes (Not Just Bypasses)
- RP2040 early serial init behavior corrected: do not re-run UART setup in early phase.
- RP2040 serial/syslog/devnull initialization ordering adjusted to avoid early bring-up lock issues.
- SMP-related PX4 compatibility fixes added:
  - Handling `TSTATE_TASK_ASSIGNED` in cpuload output path.
  - Required `sched_note_cpu_*` callbacks for `CONFIG_SCHED_INSTRUMENTATION_EXTERNAL=y`.
- UAVCAN DSDL build blocker fixed:
  - Local `pydronecan` path is now only forced when `dronecan/dsdl` exists.
  - Python package fallback works with installed `dronecan`.

## Current Runtime State
- Current stable marker tail remains:
  - `ABCEZNHJKLmbVpYTARI01rsbd235679abcdkefagpE8yyU`
- Interpretation:
  - Execution reaches `nsh_session` and prompt path (`...agp...`).
  - Input read returns immediate EOF (`E8`), session exits, kernel continues (`...yyU`).
- Core unresolved issue:
  - NSH does not inherit a valid readable stdin in this RP2040 SMP path.

## Key Findings From Debug Iterations
- Open-based console recovery attempts in sensitive phases can block/hang.
- `readline_fd` maps non-`EINTR` read errors to EOF; non-blocking stdin can surface as immediate EOF.
- Direct `open("/dev/ttyS0", O_RDWR|O_NONBLOCK)` from `nsh_session` caused an earlier stall (`...kefa`) and was reverted.
- ALT console path was tested extensively but not retained for the current fixed-UART strategy.

## Temporary/Diagnostic Workarounds Still In Play
- RP2040 path in idle stdio setup currently skips locked-phase open/dup of `/dev/console` to avoid deadlock risk.
- Several NSH startup steps were temporarily bypassed during isolation (ROMFS init, board init, scripts) in selected test iterations.


## Recommended Next Engineering Step
Implement deferred, kernel-owned stdio inheritance restoration (fd `0/1/2`) in a safe unlocked point before NSH starts, then remove ad-hoc NSH rebind logic once stdin validity is proven.

## Handoff Summary For Teammates
- This is no longer an early-boot crash problem.
- It is now a late bring-up console inheritance problem (stdin validity at NSH session time).
- Avoid adding new direct open/rebind calls in locked/early contexts; these have repeatedly caused hangs/regressions.
- Use marker strings as the primary acceptance signal for each patch.
