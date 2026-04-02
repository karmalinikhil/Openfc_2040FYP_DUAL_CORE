# Repo Recovery Playbook (RP2040 / PX4)

Use this after a fresh clone if the build or flash flow breaks. This document records exactly what was needed to make the project work in this workspace.

## 1) Known working build target

From the PX4 directory:

```bash
cd /home/karmalinikhil/Openfc_2040FYP_DUAL_CORE/Openfc_2040FYP_DUAL_CORE/PX4-Autopilot
make raspberrypi_pico_default
```

## 2) Issues that occurred and how they were fixed

### A) `YOU HAVE TO USE GIT TO DOWNLOAD THIS REPOSITORY. ABORTING.`

Cause:
- PX4 was inside a parent git repo, so `PX4-Autopilot/.git` did not exist as a local directory.

Fix applied:
- In `PX4-Autopilot/Makefile`, changed git check to accept parent `.git`.

Patch idea:

```make
ifeq ($(wildcard .git ../.git),)
    $(error YOU HAVE TO USE GIT TO DOWNLOAD THIS REPOSITORY. ABORTING.)
endif
```

### B) CMake failure: `is not a git repository`

Cause:
- Version generation expected `.git` only under `PX4-Autopilot`.

Fix applied:
- In `PX4-Autopilot/src/lib/version/CMakeLists.txt`, added fallback to parent `.git`.

Patch idea:

```cmake
else()
    if (EXISTS ${PX4_SOURCE_DIR}/../.git)
        set(git_dir_path ${PX4_SOURCE_DIR}/../.git)
    else()
        message(FATAL_ERROR "is not a git repository")
    endif()
endif()
```

### C) MAVLink generation failure: missing `protocol.h`

Error seen:
- Missing file in `pymavlink/generator/C/include_v2.0/protocol.h`.

Cause:
- Incomplete vendored `pymavlink` content.

Fix applied:
- Copied `include_v1.0` and `include_v2.0` from local Python `pymavlink` package into:
  `PX4-Autopilot/src/modules/mavlink/mavlink/pymavlink/generator/C/`

Command pattern used:

```bash
VENDOR=PX4-Autopilot/src/modules/mavlink/mavlink/pymavlink/generator
SITE=$(python3 - <<'PY'
import os,pymavlink
print(os.path.join(os.path.dirname(pymavlink.__file__),'generator','C'))
PY
)
mkdir -p "$VENDOR/C"
cp -r "$SITE"/include_v1.0 "$VENDOR/C/"
cp -r "$SITE"/include_v2.0 "$VENDOR/C/"
```

### D) NuttX assembler errors: cannot create `bin/*.o`

Errors seen:
- `can't create bin/lib_assert.o: No such file or directory`
- `can't create bin/mm_initialize.o: No such file or directory`

Fix applied:

```bash
cd PX4-Autopilot/platforms/nuttx/NuttX/nuttx
mkdir -p libs/libc/bin mm/bin
```

Then reran build:

```bash
cd /home/karmalinikhil/Openfc_2040FYP_DUAL_CORE/Openfc_2040FYP_DUAL_CORE/PX4-Autopilot
make raspberrypi_pico_default
```

## 3) UF2 conversion and flash flow (known working)

Build (if needed), convert UF2, then copy to `RPI-RP2`:

```bash
cd /home/karmalinikhil/Openfc_2040FYP_DUAL_CORE/Openfc_2040FYP_DUAL_CORE/PX4-Autopilot
make raspberrypi_pico_default
picotool uf2 convert build/raspberrypi_pico_default/raspberrypi_pico_default.bin build/raspberrypi_pico_default/firmware_smp_from_bin.uf2 --platform rp2040
powershell.exe -NoProfile -Command '$src="\\wsl$\Ubuntu\home\karmalinikhil\Openfc_2040FYP_DUAL_CORE\Openfc_2040FYP_DUAL_CORE\PX4-Autopilot\build\raspberrypi_pico_default\firmware_smp_from_bin.uf2"; $vol=Get-Volume | ? { $_.FileSystemLabel -eq "RPI-RP2" } | Select-Object -First 1; if ($null -eq $vol) { Write-Output "RPI-RP2_NOT_FOUND"; exit 11 }; Copy-Item -LiteralPath $src -Destination ($vol.DriveLetter + ":\firmware.uf2") -Force; Start-Sleep -Seconds 3; $still=Get-Volume | ? { $_.FileSystemLabel -eq "RPI-RP2" } | Select-Object -First 1; if ($null -eq $still) { Write-Output "RPI-RP2_DISCONNECTED_POST_COPY" } else { Write-Output ("RPI-RP2_STILL_PRESENT=" + $still.DriveLetter) }'
```

Important corrections:
- In PowerShell filters, use `$_` (not `$.`).
- Use the real WSL path for your current workspace.

If output is `RPI-RP2_DISCONNECTED_POST_COPY`, that is usually expected after successful UF2 copy (board reboots out of BOOTSEL mode).

## 4) Artifact check after successful build

Expected files:

```bash
ls -lh build/raspberrypi_pico_default/raspberrypi_pico_default.{elf,bin,px4}
```

## 5) Notes for future recoveries

- If you restore repo state from your remote and these patch changes disappear, re-apply sections 2A and 2B only if your layout still has PX4 nested inside a parent git repo.
- If MAVLink headers are missing again, re-run section 2C.
- If NuttX `bin/*.o` directory errors return, re-run section 2D.
