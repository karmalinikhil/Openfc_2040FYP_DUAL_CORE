px4_add_board(
  PLATFORM nuttx
  VENDOR raspberrypi
  MODEL pico_debug
  LABEL default
  TOOLCHAIN arm-none-eabi
  ARCHITECTURE cortex-m0
  ROMFSROOT px4fmu_common
  TESTING
  DRIVERS
    # Minimal set
    board_core
    rgbled
  MODULES
    minimal.c
)
