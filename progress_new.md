# OpenFC2040 Simple Progress Update

Last updated: 2026-03-23 (latest session)

## What we are trying to do
We are trying to run PX4 + NuttX on RP2040 using both cores (SMP / dual-core), and still keep a working command console (NSH) on UART0.

## Good news
- Dual-core boot is now much better than before.
- The firmware goes deep into startup and reaches NSH code.
- Build and flashing process is stable now.
- The previous UAVCAN Python build error is fixed.

## Current problem (main blocker)
Boot can now get deep into task creation and, on the farthest known-good run,
reach the RP2040 NSH console path. The current active blocker in the latest
diagnostic branch is the inode-lock semaphore holder release path during task
file inheritance.

- The old "NSH prompt path then immediate EOF" failure has been bypassed.
- Late stdio repair succeeds and init task creation now gets through:
  - direct `/dev/ttyS0` open
  - file descriptor allocation
  - `dup2(srcfd, 0)` for stdin
  - task file inheritance
  - stack/TLS/scheduler setup
  - RP2040 init trampoline
  - `nsh_main()`
  - `nsh_initialize()`
  - entry into `nsh_consolemain()`
- The strongest latest-good marker is now:
  - `ABCEZNuvxyzZHJKruvstLmbVpRb2rumnpqdxywkC3rumnpqdxywkS4rumnpqdxywkTBSdxyYdxyZcOfykghopq@$bmnwxiLD6f7dqKYAabghiru0drtapvstkuvxyzZlmnopdTARIN01rsbd235679a`
- In simple terms: boot now reaches the RP2040 NSH console path, gets past the
  old stop at `7`, and reaches the new `9a` markers in `nsh_consolemain()`.
- Yes: this `...35679a` run is still the farthest point reached so far in the
  overall bring-up work.
- What `...35679a` means:
  - `TARIN01rsbd23567` shows boot reached:
    - RP2040 init trampoline / application handoff
    - `nsh_main()`
    - `nsh_initialize()`
    - `nsh_consolemain()`
  - `9` shows the RP2040 NSH path got past the old stop right after marker `7`
    and entered the new pre-session stdio repair/rebind logic.
  - `a` shows the RP2040 stdio rebind helper started, which means boot had
    advanced into the final NSH console preparation stage instead of failing
    earlier in task creation.
- A later run showed the older marker tail
  `...dqKYAabghiru0drs` again. Most likely explanation: the board was running
  an older UF2 after a failed or partial reflash, not a logical regression in
  the current source tree.
- In the newest diagnostic branch, the repeated live tail is now:
  - `...drsuIPQhik`
- In simple terms: the current branch no longer stops vaguely in inode addref.
  It now proves the active blocker is inside `nxsem_release_holder()` while
  releasing the inode lock semaphore, after entering `nxsem_post()` and after
  passing the initial holder-scan checks.
- A later reverted run reached:
  - `ABCEZNPQhorRSTXYZPQhorRSTXYZPQhorRSTXYZPQhorRSTXYZPQhorRSTXYZPQhorRSTXYZuvxPQhorRSTXYZyzZPQhorRSTXYZPQhorRSTXYZPQhorRSTXYZPQhorRSTXYZPQhorRSTXYZHJKruvPQhorRSTXYZstLmPQhorRSTXYZPQhorRSTXYZbVpRb2rumnpqdxPQhorRSTXYZPQhorRSTXYZywkC3rumnpqdxPQhorRSTXYZywkS4rumnpqdxPQhorRSTXYZywkTBSdxyYdxPQhorRSTXYZyZcOfykghopq@$bmnwPQhorRSTXYZxiLPQhorRSTXYZPQhorRSTXYZPQhorRSTXYZD6fPQhorRSTXYZ7dqKYAaPQhorRSTXYZbgPQhorRSTXYZPQhorRSTXYZhPQhorRSTXYZPQhorRSTXYZiru0PQhorRSTXYZPQhorRSTXYZdrtBCFGWapvPQhorRSTXYZstkuvxPQhorRSTXYZyzZlmnopdTPQhiklmRSTXYZPQhiklmRSTXYZPQhiklmRSTXYZAPQhiklmRSTXYZPQhiklmRSTXYZPQhiklmRSTXYZPQhiklmRSTXYZRIN01rsbd235PQhiklmRSTXYZ679aPQhiklmRSTXYZPQhiklmRSTXYZ`
- In simple terms: after backing out the unsafe global inode-lock PI-disable
  experiment, the system climbed back to the farthest-known `...679a` level.
  This confirms the direct stdio-clone path still works and the bad early
  regressions were caused by the PI-disable experiment itself, not by the
  underlying stdio/task creation fixes.

## What has already been tested
- Several stdin/console rebind methods were tried.
- ALT console mode was tested and then disabled for current fixed-UART strategy.
- Direct open of `/dev/ttyS0` from NSH session was tested and caused an earlier stall, so it was reverted.
- Deferred helper retry window was increased significantly (2000 attempts x 5ms), still no stdin recovery.
- NSH-stage bounded retry fallback was added (`u` start, `v` success marker). Latest run showed `...kuf...` with no `v`, so it still failed to get a valid stdin.
- RP2040 hardware UARTs are now explicitly marked connected under
  `CONFIG_SERIAL_REMOVABLE`, fixing the false `@#` / `-ENOTCONN` failure.
- Late stdio repair was simplified to fix stdin only; stdout/stderr rebinding
  and final `fcntl()` normalization were removed from the hot path in
  `nx_start.c`.
- RP2040 stdio inheritance for child task stdio slots now bypasses the fragile
  `file_dup2()` reopen path and clones the already-open UART-backed file
  structure after `inode_addref()`.
- `nsh_consolemain()` was patched to skip the RP2040-only stdio `fcntl()`
  normalization block that was the likely stop right after marker `7`.
- New semaphore/inode-lock diagnostics added in this session:
  - `inode_addref()` markers around `inode_semgive()`
  - `inode_semgive()` wrapper markers in `fs_inode.c`
  - `nxsem_post()` markers in `sem_post.c`
  - `nxsem_release_holder()` markers in `sem_holder.c`
- A temporary experiment disabled PI for the inode lock globally. That caused
  very early regressions (`...yzZ`, then `ABCEZN`) and was reverted.
- Reverting that experiment restored the later boot path and confirmed the
  project can still reach `...679a` on the current source tree.
- Latest diagnostic narrowing from this session:
  - `...drsuI`  = entered `inode_semgive()`
  - `...drsuIPQ` = entered `nxsem_post()` and passed `enter_critical_section()`
  - `...drsuIPQhik` = entered `nxsem_release_holder()`, passed holder entry
    validity/count checks, and now blocks around the holder comparison /
    holder metadata path
  - After reverting the unsafe PI-disable experiment, the system again reached
    the farthest NSH-side milestone `...679a`, so the inode-lock PI issue is
    not blocking every branch equally. The active next step is to instrument
    the RP2040 NSH rebind helper after `a`.

## What is considered fixed already
- Early serial initialization ordering issues.
- SMP-related PX4 compatibility items (`TSTATE_TASK_ASSIGNED`, scheduler instrumentation callbacks).
- Build pipeline issue from missing `dronecan.dsdl` import.

## Recommended next step
Keep the inode-lock semaphore findings in the notes, but move active debugging
back to the later NSH-side path after `...679a` and instrument the RP2040
stdio rebind helper after marker `a`.

Reason:
- The farthest known-good milestone is still the `...35679a` NSH run.
- The active latest diagnostic branch narrowed a real inode-lock PI issue to
  `nxsem_release_holder()`, but the unsafe global PI-disable test caused early
  regressions and was reverted.
- After reverting that experiment, the system again reached the farthest-known
  `...679a` NSH milestone.
- The highest-value next signal now is what happens immediately after `a` in
  the RP2040 NSH stdio rebind helper.

## Where we paused
- We have already confirmed:
  - late `/dev/ttyS0` open succeeds
  - `files_allocate()` succeeds
  - `dup2(srcfd, 0)` succeeds
  - init task creation succeeds through file inheritance, stack, TLS, and task
    activation
  - boot reaches `nsh_main()`, `nsh_initialize()`, and `nsh_consolemain()`
- A known-good later marker reached:
  - `...TARIN01rsbd235679a`
- This remains the farthest known progress point in the project so far.
- A later rerun emitted the older tail:
  - `...dqKYAabghiru0drs`
- Newer diagnostics then narrowed the current blocker further to:
  - `...drsuIPQhik`
- A later reverted run confirmed the tree still reaches:
  - `...679a`
- Resume plan: rebuild/flash the current source tree again, verify the board is
  on the intended image for each branch, and continue by instrumenting the
  NSH rebind helper after marker `a`.

## Quick status for teammates
- Not an early boot crash anymore.
- It is now a late NSH console / image-consistency issue, not an early boot
  crash.
- The strongest current evidence is that the tree has progressed further than
  the board is sometimes actually running after failed flashes.
