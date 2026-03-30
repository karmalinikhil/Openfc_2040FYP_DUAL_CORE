# Suggested Edits Change Log

Last updated: 2026-03-22

Purpose: document the file-by-file changes made during the RP2040 SMP + NSH stdin debugging work, without changing any code.

Scope source:
- Active git diffs in PX4-Autopilot working tree.
- Files visible in the Suggested Edits screenshots.

## Kernel, Scheduler, VFS, and Serial (RP2040/NuttX)

1. platforms/nuttx/NuttX/nuttx/sched/init/nx_start.c
   - Added RP2040 late deferred helper after sched_unlock to repair stdio (fd 0/1/2), clear O_NONBLOCK, and launch init thread.
   - Added extensive marker tracing for startup phase localization.
   - Deferred RP2040 serial/syslog/devnull registration to safer point.

2. platforms/nuttx/NuttX/nuttx/sched/init/nx_bringup.c
   - Exposed init-thread creation function and adjusted RP2040 bring-up flow to defer init launch from nx_start helper.
   - Added marker tracing and RP2040 init-entry wrapper path.

3. platforms/nuttx/NuttX/nuttx/sched/init/init.h
   - Added declaration for nx_create_initthread so nx_start can call it directly.

4. platforms/nuttx/NuttX/nuttx/sched/group/group_setupidlefiles.c
   - Added RP2040 path to skip locked-phase /dev/console open/dup to avoid early deadlock.

5. platforms/nuttx/NuttX/nuttx/sched/task/task_start.c
   - Added marker tracing around task start and kernel/app entry path.

6. platforms/nuttx/NuttX/nuttx/fs/vfs/fs_open.c
   - Added tracked marker instrumentation for /dev/console and /dev/ttyS0 opens.
   - Added non-blocking inode trylock path for tracked console opens.
   - Added RP2040 experimental lockless fallback lookup for non-blocking /dev/ttyS0 when trylock fails.

7. platforms/nuttx/NuttX/nuttx/fs/inode/inode.h
   - Declared non-blocking inode lock API inode_semtrytake.

8. platforms/nuttx/NuttX/nuttx/fs/inode/fs_inode.c
   - Implemented inode_semtrytake using recursive mutex trylock.

9. platforms/nuttx/NuttX/nuttx/drivers/serial/serial.c
   - Added marker instrumentation in uart_open.
   - Added RP2040 non-blocking fast-fail behavior in open path when closesem is contended (returns EAGAIN for O_NONBLOCK callers).

10. platforms/nuttx/NuttX/nuttx/drivers/drivers_initialize.c
   - Deferred RP2040 syslog and /dev/null registration from early initialize stage.

11. platforms/nuttx/NuttX/nuttx/arch/arm/src/common/arm_initialize.c
   - Deferred RP2040 serial driver initialization from up_initialize.

12. platforms/nuttx/NuttX/nuttx/arch/arm/src/rp2040/rp2040_serial.c
   - Avoided re-running up_setup during RP2040 early serial init to preserve stable early console behavior.

13. platforms/nuttx/NuttX/nuttx/arch/arm/src/rp2040/rp2040_start.c
   - Enabled always-on startup marker output and changed pre-nx_start marker to Z for trace disambiguation.

14. platforms/nuttx/NuttX/nuttx/arch/arm/src/rp2040/rp2040_cpustart.c
   - No active diff currently detected in git (file appeared in screenshot list, likely earlier edit or clean state now).

15. platforms/nuttx/NuttX/nuttx/.config
   - No active diff currently detected in git (file appeared in screenshot list, likely regenerated/clean relative to repo state).

## NSH and Console Path (Apps)

16. platforms/nuttx/NuttX/apps/system/nsh/nsh_main.c
   - Added marker tracing.
   - Adjusted RP2040 priority handling path to avoid premature priority drop during sensitive bring-up.

17. platforms/nuttx/NuttX/apps/nshlib/nsh_init.c
   - Added marker tracing.
   - Added RP2040 temporary bypass of blocking ROMFS /etc mount and boardctl init points for progression diagnostics.

18. platforms/nuttx/NuttX/apps/nshlib/nsh_session.c
   - Switched/used fd-based prompt input path (readline_fd/cle_fd).
   - Added RP2040 stdin selection and O_NONBLOCK clear handling.
   - Added marker tracing for prompt and EOF localization.

19. platforms/nuttx/NuttX/apps/nshlib/nsh_stdsession.c
   - Added RP2040-specific EOF retry behavior and marker trace in stdsession path.

20. platforms/nuttx/NuttX/apps/nshlib/nsh_consolemain.c
   - Added RP2040 rebind helper to recover stdin from available descriptors.
   - Added bounded non-blocking /dev/ttyS0 retry fallback and markers (u/v path).
   - Added pre-session stdio O_NONBLOCK normalization.

21. platforms/nuttx/NuttX/apps/nshlib/nsh_altconsole.c
   - Added RP2040-safe alt-console handling and marker tracing.
   - Added non-blocking probe + blocking restore behavior for alt stdin path.

## PX4 Platform Integration (SMP compatibility)

22. platforms/nuttx/src/px4/common/print_load.cpp
   - Added TSTATE_TASK_ASSIGNED handling for SMP-aware task-state reporting.

23. platforms/nuttx/src/px4/common/cpuload.cpp
   - Added SMP scheduler note callback stubs required by instrumentation linkage.

## Build Tooling Fix

24. src/drivers/uavcan/libdronecan/libuavcan/dsdl_compiler/libuavcan_dsdlc
   - Fixed local pydronecan precedence logic to only use local path when dronecan/dsdl exists, avoiding ModuleNotFoundError on incomplete local tree.

## Board/Config and Application Files Seen Earlier (Not in current active root git diff snapshot)

25. progress.md
   - Progress handoff content exists in workspace history/suggested edits, but file is currently untracked at workspace root.

26. progress_simple.md
   - Updated during session with simplified status and pause/resume notes.

27. Any additional board-level files shown in older screenshot states (for example board defaults, sensor scripts, clock/timer/board headers)
   - These were part of earlier iterations, but are not all present as active diffs in the current root snapshot.
   - If needed, generate a second document from full git history with commit-by-commit details.

## Additional RP2040 SMP Progress Since Original Pause

28. platforms/nuttx/NuttX/nuttx/fs/vfs/fs_dup2.c
   - Added RP2040 marker instrumentation around file descriptor duplication.
   - Latest markers show inherited `fd 0` duplication reaches `inode_addref()`
     before stalling.

29. platforms/nuttx/NuttX/nuttx/fs/inode/fs_files.c
   - Added RP2040 marker instrumentation for `files_duplist()` to show which
     inherited descriptor (`0/1/2`) is being cloned.

30. platforms/nuttx/NuttX/nuttx/fs/inode/fs_inodeaddref.c
   - Added RP2040 marker instrumentation around `inode_addref()`.
   - Added RP2040 trylock / best-effort addref fallback so inode lock
     contention can be separated from later duplication blockers.

31. platforms/nuttx/NuttX/nuttx/sched/group/group_setuptaskfiles.c
   - Added RP2040 marker instrumentation around task file inheritance and
     stream setup.

32. platforms/nuttx/NuttX/nuttx/sched/task/task_create.c
   - Added RP2040 marker instrumentation around `nxthread_create()` and task
     activation.

33. platforms/nuttx/NuttX/nuttx/sched/task/task_init.c
   - Added RP2040 marker instrumentation across `nxtask_init()` stages:
     group allocation, env duplication, file inheritance, stack setup, TLS,
     scheduler setup, argv setup, and group finalize.

34. platforms/nuttx/NuttX/nuttx/arch/arm/src/common/arm_createstack.c
   - Added RP2040 marker instrumentation around ARM stack allocation and
     post-allocation setup.

35. platforms/nuttx/NuttX/nuttx/sched/init/nx_bringup.c
   - Added RP2040 marker instrumentation around `nx_start_application()`,
     `nxtask_create()`, and the RP2040 init-entry trampoline.

36. progress_new.md
   - Updated with the newest marker progression and current blocker location
     in task file inheritance after stdin repair succeeds.

## Current Pause State

- Work has progressed beyond late stdio repair and into init task creation.
- Current live blocker is inherited `fd 0` duplication for the init task:
  markers currently reach `...abghiru0dfgrs`.
- Next active investigation point is the `file_dup2()` / `inode_addref()`
  path while duplicating the repaired UART stdin inode.

## Additional Progress After Init-Task FD Fix

37. platforms/nuttx/NuttX/nuttx/fs/inode/fs_files.c
   - Added RP2040 stdio inheritance bypass for child stdio slots (`fd 0/1/2`)
     so task creation can clone the already-open UART-backed file structures
     without re-entering the fragile `file_dup2()` reopen path.

38. platforms/nuttx/NuttX/apps/nshlib/nsh_consolemain.c
   - Added RP2040 markers around `nsh_consolemain()`, including `5/6/7/9/A/8`
     and rebind helper markers `a/b/c/d/k/m/u/h/i`.
   - Skipped RP2040-only stdio `fcntl()` normalization before `nsh_session()`
     after markers showed the old stop point was immediately after `7`.

39. progress_new.md
   - Updated to reflect the later milestone marker:
     `...TARIN01rsbd235679a`
   - Recorded that a subsequent older marker tail (`...dqKYAabghiru0drs`) is
     most likely due to the board running an older image after an incomplete
     flash, not a source-level regression.

## Newest Known-Good Marker Milestone

- Latest strongest progress marker:
  `ABCEZNuvxyzZHJKruvstLmbVpRb2rumnpqdxywkC3rumnpqdxywkS4rumnpqdxywkTBSdxyYdxyZcOfykghopq@$bmnwxiLD6f7dqKYAabghiru0drtapvstkuvxyzZlmnopdTARIN01rsbd235679a`
- This proves boot now reaches:
  - `nsh_main()`
  - `nsh_initialize()`
  - `nsh_consolemain()`
  - the new RP2040 `9a` path in NSH console setup

## Current Practical Status

- Source tree still contains the later NSH patch set.
- The most likely current issue is inconsistent board image state after one or
  more failed or partial BOOTSEL reflashes.
- Immediate next step is to rebuild/flash the current tree and resume from the
  `...35679a` milestone rather than the older `...drs` path.

## Additional Debug Progress From Current Chat

40. platforms/nuttx/NuttX/nuttx/fs/inode/fs_inodeaddref.c
   - Added markers around the inode-lock release edge after `inode->i_crefs++`.
   - Latest narrowing from this file showed:
     - `...drsu` means execution reaches the point immediately before
       `inode_semgive()`.

41. platforms/nuttx/NuttX/nuttx/fs/inode/fs_inode.c
   - Added wrapper markers around `inode_semgive()`.
   - Latest narrowing showed:
     - `...drsuI` means `inode_semgive()` is entered.
     - Missing `J` means the stop is inside `nxrmutex_unlock()` / underlying
       semaphore post path.

42. platforms/nuttx/NuttX/nuttx/sched/semaphore/sem_post.c
   - Added `P..Z` markers across `nxsem_post()`.
   - Latest narrowing showed:
     - `...drsuIPQ` means `nxsem_post()` starts and passes
       `enter_critical_section()`.
     - Missing `R` means the stop is inside `nxsem_release_holder()`.

43. platforms/nuttx/NuttX/nuttx/sched/semaphore/sem_holder.c
   - Added `h..r` markers inside `nxsem_release_holder()`.
   - Latest narrowing showed:
     - `...drsuIPQhik` means holder release enters the scan loop, reaches a
       valid holder entry, and gets past the non-null/count checks.
     - The active stop is now around the `pholder->htcb == rtcb` comparison /
       holder metadata path.

44. platforms/nuttx/NuttX/nuttx/fs/inode/fs_inode.c
   - Temporary experiment added a global inode-lock PI disable using
     `nxsem_set_protocol(..., SEM_PRIO_NONE)`.
   - That experiment caused early regressions (`...yzZ`, then `ABCEZN`) and
     was reverted.
   - Important conclusion: the early regressions came from the PI-disable
     experiment itself, not from the earlier stdio/task inheritance fixes.

45. progress_new.md
   - Updated to record that reverting the unsafe global inode-lock PI-disable
     experiment restored the later boot path and reached the farthest NSH-side
     milestone again.

## Restored Farthest Progress After Revert

- After reverting the unsafe global inode-lock PI-disable experiment, the
  system again reached:
  `...RIN01rsbd235...679a...`
- This confirms:
  - the direct stdio-clone path remains valid
  - the earlier inode/file inheritance work was not lost
  - the project is back at the farthest known milestone
  - the best next target is the RP2040 NSH rebind helper after marker `a`

## Farthest Known-Good Progress Point

- The farthest marker reached so far is still:
  `ABCEZNuvxyzZHJKruvstLmbVpRb2rumnpqdxywkC3rumnpqdxywkS4rumnpqdxywkTBSdxyYdxyZcOfykghopq@$bmnwxiLD6f7dqKYAabghiru0drtapvstkuvxyzZlmnopdTARIN01rsbd235679a`
- This confirms the system reached:
  - RP2040 init/app handoff
  - `nsh_main()`
  - `nsh_initialize()`
  - `nsh_consolemain()`
  - the newer RP2040 NSH console path up to the `9a` markers
- Meaning of `9a` in that run:
  - `9` shows boot got past the old `nsh_consolemain()` stop right after `7`
    and entered the RP2040 NSH pre-session stdio handling path.
  - `a` shows the RP2040 stdio rebind helper started, which is later than the
    old task-creation / inode-addref failure path.
