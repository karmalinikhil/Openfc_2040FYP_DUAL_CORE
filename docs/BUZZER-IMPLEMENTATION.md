# Passive Onboard Buzzer Implementation

## Overview
This report details the successful integration of a passive piezoelectric buzzer (Huaneng QMB-09B-03) on the OpenFC2040 (RP2040) flight controller. The standard PX4 `tone_alarm` driver lacked a hardware interface layer for the RP2040 architecture. A custom driver interface was developed to generate high-fidelity square waves using the RP2040’s hardware PWM slices, bypassing CPU-intensive software toggling methods.



## Technical Implementation

### A. Hardware Interface Layer
**File Created:** `platforms/nuttx/src/px4/rpi/rpi_common/tone_alarm/ToneAlarmInterface.cpp`

The core solution is a new C++ interface bridging the generic PX4 driver to the RP2040 microcontroller.

* **Direct Register Access:** Utilized direct memory access to the RP2040’s `PWM_BASE` and `IO_BANK0` registers.
* **Hardware PWM:** Configured **Slice 4 (Channel B)**, physically tied to **GPIO 25**. The driver calculates precise clock dividers to generate frequencies between 20Hz and 20kHz.
* **Reset Controller:** Implemented critical initialization to release the PWM peripheral from its default "Reset" state via the `RESETS_BASE` register.
* **Drive Strength Optimization:** Modified GPIO pad control registers to increase drive strength from **4mA** to **12mA** and enable **Fast Slew Rate**, significantly improving acoustic output.

### B. Build System Integration
**File Modified:** `src/drivers/tone_alarm/CMakeLists.txt`

The build system was updated to conditionally compile the new interface file only when building for the RP2040 architecture, ensuring compatibility with other board targets.

### C. Board Configuration
**File Modified:** `boards/px4/openfc2040/src/board_config.h`  
**File Modified:** `boards/px4/openfc2040/default.px4board`

* Defined physical pin mapping: `#define GPIO_TONE_ALARM_1 PX4_MAKE_GPIO_OUTPUT_CLEAR(25)`
* Enabled driver module: `CONFIG_DRIVERS_TONE_ALARM=y`



## Challenges & Resolutions

| Issue | Root Cause | Resolution |
| :--- | :--- | :--- |
| **Silence on Boot** | PWM peripheral starts in "Reset" state to save power. | Implemented register write to Reset Controller to un-reset the PWM block. |
| **Low Volume** | Default GPIO drive strength (4mA) insufficient for buzzer. | Modified Pad Control registers to boost current to 12mA. |
| **Driver Inactivity** | Index mismatch (`GPIO_TONE_ALARM_0` vs `_1`). | Standardized definition to `GPIO_TONE_ALARM_1`. |
| **Continuous Alarm** | "Preflight Fail" loop due to missing calibration. | Rectified by defining `FLASH_BASED_PARAMS` in `board_config.h`. |



## File Change Log

**1. `platforms/nuttx/src/px4/rpi/rpi_common/tone_alarm/ToneAlarmInterface.cpp`**
* **Action:** Created
* **Content:** `init()`, `start_note()`, `stop_note()` with register manipulation.

**2. `src/drivers/tone_alarm/CMakeLists.txt`**
* **Action:** Modified
* **Content:** Added CMake logic to link the new interface file.

**3. `boards/px4/openfc2040/src/board_config.h`**
* **Action:** Modified
* **Content:** Added `#define GPIO_TONE_ALARM_1` and Flash parameter definitions.

**4. `boards/px4/openfc2040/default.px4board`**
* **Action:** Modified
* **Content:** Uncommented `CONFIG_DRIVERS_TONE_ALARM=y`.

**5. `boards/px4/openfc2040/init/rc.board_defaults`**
* **Action:** Modified
* **Content:** Added `tone_alarm start` to boot sequence.
