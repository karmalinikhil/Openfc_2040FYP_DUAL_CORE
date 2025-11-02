px4_add_board(
    PLATFORM nuttx
    VENDOR rsp_2040
    MODEL rsp_2040
    LABEL default
    TOOLCHAIN arm-none-eabi
    ARCHITECTURE cortex-m0plus
    ROMFSROOT px4fmu_common
    
    SERIAL_PORTS
        GPS1:/dev/ttyS1
        TEL1:/dev/ttyS0
    
    DRIVERS
        adc/board_adc
        barometer/dps310
        gps
        imu/st/lsm6ds3
        pwm_out
        rc_input
        rgbled
    
    MODULES
        attitude_estimator_q
        battery_status
        commander
        control_allocator
        dataman
        ekf2
        events
        flight_mode_manager
        gyro_calibration
        land_detector
        load_mon
        logger
        mavlink
        mc_att_control
        mc_pos_control
        mc_rate_control
        navigator
        rc_update
        sensors
    
    SYSTEMCMDS
        bl_update
        dmesg
        hardfault_log
        i2cdetect
        led_control
        mft
        mixer
        motor_ramp
        motor_test
        mtd
        nshterm
        param
        perf
        pwm
        reboot
        reflect
        sd_bench
        system_time
        top
        topic_listener
        tune_control
        uorb
        usb_connected
        ver
        work_queue
)
