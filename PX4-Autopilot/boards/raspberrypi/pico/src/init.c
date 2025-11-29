/****************************************************************************
 *
 *   Copyright (c) 2021 PX4 Development Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/**
 * @file init.c
 *
 * board specific early startup code.  This file implements the
 * board_app_initialize() function that is called early by nsh during startup.
 *
 * Code here is run before the rcS script is invoked; it should start required
 * subsystems and perform board-specific initialization.
 */

/****************************************************************************
 * Included Files
 ****************************************************************************/

 #include <px4_platform_common/px4_config.h>
 #include <px4_platform_common/tasks.h>
 
 #include <stdbool.h>
 #include <stdio.h>
 #include <string.h>
 #include <debug.h>
 #include <errno.h>
 #include <syslog.h>
 
 #include <nuttx/board.h>
 #include <nuttx/spi/spi.h>
 #include <nuttx/i2c/i2c_master.h>
 #include <nuttx/mmcsd.h>
 #include <nuttx/analog/adc.h>
 #include <nuttx/mm/gran.h>
 
 #include "board_config.h"
 #include <rp2040_uart.h>
 
 #include <arch/board/board.h>
 
 #include <drivers/drv_hrt.h>
 #include <drivers/drv_board_led.h>
 
 #include <systemlib/px4_macros.h>
 
 #include <px4_arch/io_timer.h>
 #include <px4_platform_common/init.h>
 #include <px4_platform/board_dma_alloc.h>

 #include <rp2040_spi.h>
 #include <rp2040_gpio.h>
 
 __BEGIN_DECLS
 extern void led_init(void);
 extern void led_on(int led);
 extern void led_off(int led);
 __END_DECLS

 /****************************************************************************
  * Name: rp2040_spiinitialize
  * 
  * Description:
  *   Configure GPIO pins for SPI0 and SPI1 functions.
  *   On RP2040, each GPIO can be mapped to multiple peripherals.
  *   We must explicitly set the function to SPI.
  *
  *   SPI0 (SD Card): CLK=18, MOSI=19, MISO=16, CS=17
  *   SPI1 (Sensors): CLK=10, MOSI=11, MISO=8, IMU_CS=9, BARO_CS=12
  ****************************************************************************/
 void rp2040_spiinitialize(void)
 {
     /* ------------------------------------------------------------------- */
     /* SPI0: SD Card                                                       */
     /* CLK: GPIO18, MOSI: GPIO19, MISO: GPIO16, CS: GPIO17                */
     /* ------------------------------------------------------------------- */
 #ifdef CONFIG_RP2040_SPI0
     /* Configure SPI0 pins for SPI function */
     rp2040_gpio_set_function(16, RP2040_GPIO_FUNC_SPI);  /* MISO/RX */
     rp2040_gpio_set_function(18, RP2040_GPIO_FUNC_SPI);  /* SCK */
     rp2040_gpio_set_function(19, RP2040_GPIO_FUNC_SPI);  /* MOSI/TX */
     
     /* CS pin as GPIO output, active low, initially high (deselected) */
     rp2040_gpio_init(17);
     rp2040_gpio_setdir(17, true);   /* Output */
     rp2040_gpio_put(17, true);      /* High = deselected */
 #endif

     /* ------------------------------------------------------------------- */
     /* SPI1: Internal Sensors (IMU LSM6DS3 + Baro DPS310)                  */
     /* CLK: GPIO10, MOSI: GPIO11, MISO: GPIO8                             */
     /* IMU CS: GPIO9, BARO CS: GPIO12                                      */
     /* ------------------------------------------------------------------- */
 #ifdef CONFIG_RP2040_SPI1
     /* Configure SPI1 pins for SPI function */
     rp2040_gpio_set_function(8, RP2040_GPIO_FUNC_SPI);   /* MISO/RX */
     rp2040_gpio_set_function(10, RP2040_GPIO_FUNC_SPI);  /* SCK */
     rp2040_gpio_set_function(11, RP2040_GPIO_FUNC_SPI);  /* MOSI/TX */
     
     /* IMU CS pin (GPIO9) as GPIO output, active low, initially high */
     rp2040_gpio_init(9);
     rp2040_gpio_setdir(9, true);    /* Output */
     rp2040_gpio_put(9, true);       /* High = deselected */
     
     /* Baro CS pin (GPIO12) as GPIO output, active low, initially high */
     rp2040_gpio_init(12);
     rp2040_gpio_setdir(12, true);   /* Output */
     rp2040_gpio_put(12, true);      /* High = deselected */
 #endif

     syslog(LOG_INFO, "[boot] SPI GPIO pins configured\n");
 }

 /****************************************************************************
  * SPI Register Callbacks - Required by NuttX SPI driver for SD card
  * media change detection (CONFIG_MMCSD_SPI)
  ****************************************************************************/
 #ifdef CONFIG_RP2040_SPI0
 int rp2040_spi0register(FAR struct spi_dev_s *dev,
                         spi_mediachange_t callback, FAR void *arg)
 {
     /* No media change detection on SPI0 (SD Card) - stub */
     (void)dev;
     (void)callback;
     (void)arg;
     return OK;
 }
 #endif

 #ifdef CONFIG_RP2040_SPI1
 int rp2040_spi1register(FAR struct spi_dev_s *dev,
                         spi_mediachange_t callback, FAR void *arg)
 {
     /* No media change detection on SPI1 (Sensors) - stub */
     (void)dev;
     (void)callback;
     (void)arg;
     return OK;
 }
 #endif

 /****************************************************************************
  * SPI Chip Select Functions - Required by NuttX SPI driver
  * 
  * These functions are called by the NuttX SPI driver to assert/deassert
  * the chip select line before/after SPI transactions.
  * 
  * For SPI0 (SD Card): Only one device, use CONFIG_RP2040_SPI0_CS_GPIO (17)
  * For SPI1 (Sensors): Multiple devices - IMU on GPIO9, Baro on GPIO12
  *                     Use devid to determine which CS to control
  ****************************************************************************/

 /* GPIO definitions for SPI chip selects */
 #define GPIO_SPI0_CS_SD     17   /* SD Card CS */
 #define GPIO_SPI1_CS_IMU    9    /* IMU (LSM6DS3) CS */
 #define GPIO_SPI1_CS_BARO   12   /* Barometer (DPS310) CS */

 #ifdef CONFIG_RP2040_SPI0
 void rp2040_spi0select(FAR struct spi_dev_s *dev, uint32_t devid, bool selected)
 {
     (void)dev;
     syslog(LOG_DEBUG, "SPI0 select: devid=%lu selected=%d\n", devid, selected);
     
     /* SD Card is the only device on SPI0 */
     rp2040_gpio_put(GPIO_SPI0_CS_SD, !selected);  /* Active LOW */
 }

 uint8_t rp2040_spi0status(FAR struct spi_dev_s *dev, uint32_t devid)
 {
     (void)dev;
     (void)devid;
     /* Always report card present for now */
     return SPI_STATUS_PRESENT;
 }
 #endif

 /* Device type definitions from PX4 drv_sensor.h */
 #define DRV_IMU_DEVTYPE_ST_LSM9DS1_AG   0x44
 #define DRV_BARO_DEVTYPE_DPS310         0x68

 #ifdef CONFIG_RP2040_SPI1
 void rp2040_spi1select(FAR struct spi_dev_s *dev, uint32_t devid, bool selected)
 {
     (void)dev;
     
     /* 
      * PX4 SPI device ID format: 
      *   Bits 16-31: PX4_SPI_DEVICE_ID (0x1000)
      *   Bits 8-15:  Instance counter (for multiple devices of same type)
      *   Bits 0-7:   Device type (DRV_xxx_DEVTYPE_xxx)
      * 
      * For internal SPI bus with initSPIDevice:
      *   IMU (LSM9DS1):  devid = 0x10000044
      *   Baro (DPS310):  devid = 0x10000068
      */
     
     /* Extract device type from lower 8 bits */
     uint8_t device_type = devid & 0xFF;
     
     syslog(LOG_DEBUG, "SPI1 select: devid=0x%08lx type=0x%02x selected=%d\n", 
            devid, device_type, selected);
     
     /* First, deselect both devices */
     if (!selected) {
         rp2040_gpio_put(GPIO_SPI1_CS_IMU, true);   /* Deselect IMU */
         rp2040_gpio_put(GPIO_SPI1_CS_BARO, true);  /* Deselect Baro */
         return;
     }
     
     /* Select the appropriate device based on device type */
     switch (device_type) {
     case DRV_IMU_DEVTYPE_ST_LSM9DS1_AG:  /* 0x44 - IMU */
         rp2040_gpio_put(GPIO_SPI1_CS_IMU, false);  /* Select IMU */
         rp2040_gpio_put(GPIO_SPI1_CS_BARO, true);  /* Ensure Baro deselected */
         syslog(LOG_DEBUG, "SPI1: Selected IMU (GPIO9 LOW)\n");
         break;
     case DRV_BARO_DEVTYPE_DPS310:  /* 0x68 - Barometer */
         rp2040_gpio_put(GPIO_SPI1_CS_IMU, true);   /* Ensure IMU deselected */
         rp2040_gpio_put(GPIO_SPI1_CS_BARO, false); /* Select Baro */
         syslog(LOG_DEBUG, "SPI1: Selected Baro (GPIO12 LOW)\n");
         break;
     default:
         /* Unknown device, deselect all */
         rp2040_gpio_put(GPIO_SPI1_CS_IMU, true);
         rp2040_gpio_put(GPIO_SPI1_CS_BARO, true);
         syslog(LOG_WARNING, "SPI1: Unknown device type 0x%02x (devid=0x%08lx)\n", 
                device_type, devid);
         break;
     }
 }

 uint8_t rp2040_spi1status(FAR struct spi_dev_s *dev, uint32_t devid)
 {
     (void)dev;
     (void)devid;
     /* Sensors are always present */
     return SPI_STATUS_PRESENT;
 }
 #endif

 /************************************************************************************
  * Name: board_peripheral_reset
  ************************************************************************************/
 __EXPORT void board_peripheral_reset(int ms)
 {
     UNUSED(ms);
 }
 
 /************************************************************************************
  * Name: board_on_reset
  ************************************************************************************/
 __EXPORT void board_on_reset(int status)
 {
     // Configure the PWM GPIO pins to outputs and keep them low.
     for (int i = 0; i < DIRECT_PWM_OUTPUT_CHANNELS; ++i) {
         px4_arch_configgpio(io_timer_channel_get_gpio_output(i));
     }
 
     if (status >= 0) {
         up_mdelay(400);
     }
 }
 
 /************************************************************************************
  * Name: board_read_VBUS_state
  * Returns -  0 if connected.
  ************************************************************************************/
 int board_read_VBUS_state(void)
 {
     // Based on board_config.h, this is hardcoded to TRUE (0 returned = connected)
     return BOARD_ADC_USB_CONNECTED ? 0 : 1;
 }
 
 /****************************************************************************
  * Name: rp2040_boardearlyinitialize
  ****************************************************************************/
 void rp2040_boardearlyinitialize(void)
 {
     /* Set default UART pin functions */
 #if defined(CONFIG_RP2040_UART0) && CONFIG_RP2040_UART0_GPIO >= 0
     rp2040_gpio_set_function(CONFIG_RP2040_UART0_GPIO, RP2040_GPIO_FUNC_UART);     /* TX */
     rp2040_gpio_set_function(CONFIG_RP2040_UART0_GPIO + 1, RP2040_GPIO_FUNC_UART);  /* RX */
 #endif
 
 #if defined(CONFIG_RP2040_UART1) && CONFIG_RP2040_UART1_GPIO >= 0
     rp2040_gpio_set_function(CONFIG_RP2040_UART1_GPIO, RP2040_GPIO_FUNC_UART);     /* TX */
     rp2040_gpio_set_function(CONFIG_RP2040_UART1_GPIO + 1, RP2040_GPIO_FUNC_UART);  /* RX */
 #endif
 }
 
 /************************************************************************************
  * Name: rp2040_boardinitialize
  ************************************************************************************/
 __EXPORT void
 rp2040_boardinitialize(void)
 {
     /* configure LEDs */
     board_autoled_initialize();
 
     /* * Configure ADC Pins (GPIO 27 & 28)
      * We must disable the digital input/output buffers for these pins to work as Analog
      */
     // GPIO 27: Battery Voltage
     rp2040_gpioconfig(27 | GPIO_FUN(RP2040_GPIO_FUNC_NULL));        
     clrbits_reg32(RP2040_PADS_BANK0_GPIO_IE, RP2040_PADS_BANK0_GPIO(27));    
     setbits_reg32(RP2040_PADS_BANK0_GPIO_OD, RP2040_PADS_BANK0_GPIO(27));    
 
     // GPIO 28: Battery Current
     rp2040_gpioconfig(28 | GPIO_FUN(RP2040_GPIO_FUNC_NULL));        
     clrbits_reg32(RP2040_PADS_BANK0_GPIO_IE, RP2040_PADS_BANK0_GPIO(28));    
     setbits_reg32(RP2040_PADS_BANK0_GPIO_OD, RP2040_PADS_BANK0_GPIO(28));    
 
     /* Set default I2C1 pin (For GPS Mag) */
     // Ensure this matches board_config.h (GPIO 6/7)
 #if defined(CONFIG_RP2040_I2C1) &&  CONFIG_RP2040_I2C1_GPIO >= 0
     rp2040_gpio_set_function(CONFIG_RP2040_I2C1_GPIO, RP2040_GPIO_FUNC_I2C);      /* SDA */
     rp2040_gpio_set_function(CONFIG_RP2040_I2C1_GPIO + 1, RP2040_GPIO_FUNC_I2C);  /* SCL */
     rp2040_gpio_set_pulls(CONFIG_RP2040_I2C1_GPIO, true, false);  /* Pull up */
     rp2040_gpio_set_pulls(CONFIG_RP2040_I2C1_GPIO + 1, true, false);
 #endif
 
     /* Configure SPI GPIOs (Calls the function in board_config.h) */
     rp2040_spiinitialize();
 }
 
 /****************************************************************************
  * Name: board_app_initialize
  ****************************************************************************/
 
 static struct spi_dev_s *spi_sd;
 static struct spi_dev_s *spi_sensors;
 
 __EXPORT int board_app_initialize(uintptr_t arg)
 {
     px4_platform_init();
 
     /* configure the DMA allocator */
     if (board_dma_alloc_init() < 0) {
         syslog(LOG_ERR, "DMA alloc FAILED\n");
     }
 
     /* initial LED state - turn on RED as "alive" indicator */
     drv_led_start();
     led_on(LED_RED);
 
     /* --------------------------------------------------------- */
     /* Initialize SPI0 - SD CARD                                 */
     /* --------------------------------------------------------- */
     // Hardware SPI0 is often Index 0 in NuttX RP2040 drivers
     spi_sd = rp2040_spibus_initialize(0); 
 
     if (!spi_sd) {
         syslog(LOG_ERR, "[boot] FAILED to initialize SPI0 for SD Card\n");
         led_off(LED_BLUE);
     } else {
         /* Now bind the SPI interface to the MMCSD driver */
         // Assuming Minor 0, Slot 0
         int result = mmcsd_spislotinitialize(0, 0, spi_sd);
         if (result != OK) {
             led_off(LED_BLUE);
             syslog(LOG_ERR, "[boot] FAILED to bind SPI0 to the MMCSD driver\n");
         }
     }
 
     up_udelay(20);
 
     /* --------------------------------------------------------- */
     /* Initialize SPI1 - SENSORS (IMU/Baro)                      */
     /* --------------------------------------------------------- */
     // Hardware SPI1 is Index 1
     spi_sensors = rp2040_spibus_initialize(1);
 
     if (!spi_sensors) {
         syslog(LOG_ERR, "[boot] FAILED to initialize SPI1 for Sensors\n");
         led_off(LED_BLUE);
     } else {
         /* Default SPI1 to 10MHz */
         SPI_SETFREQUENCY(spi_sensors, 10000000);
         SPI_SETBITS(spi_sensors, 8);
         SPI_SETMODE(spi_sensors, SPIDEV_MODE3);
     }
     
     up_udelay(20);
 
     /* Configure the HW based on the manifest */
     px4_platform_configure();
 
     return OK;
 }
