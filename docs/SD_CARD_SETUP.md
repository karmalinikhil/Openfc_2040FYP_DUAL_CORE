# MicroSD Card Parameter Storage Setup for Raspberry Pi Pico

## Overview

This guide explains how to enable persistent parameter storage on MicroSD cards for the Raspberry Pi Pico running PX4 firmware. This allows QGroundControl to save airframe configurations, sensor calibrations, and other settings that survive power cycles.

---

## Problem Solved

❌ **Before:** Parameters reset after every reboot  
✅ **After:** Parameters persist on MicroSD card even after power cycle

---

## Changes Required

### Required Runtime Selection (Recommended)

On this RP2040 Pico PX4 tree, parameter storage is most reliably switched at runtime using the PX4 shell command:

```
param select /fs/microsd/params
```

This makes PX4 store parameters (including all calibration values) on the microSD card mounted at `/fs/microsd`.

---

## Step-by-Step Instructions

### Step 1: Locate the Configuration File
```bash
cd PX4-Autopilot
nano boards/raspberrypi/pico/init/rc.board_defaults
```

### Step 2: Make the Change

Add the following line near the top of the script (after the shebang is fine):

```sh
param select /fs/microsd/params
```

Save the file (Ctrl+O, Enter, Ctrl+X).

### Step 3: Rebuild Firmware
```bash
make raspberrypi_pico_default
```

### Step 4: Convert to UF2 Format
```bash
/usr/local/bin/picotool uf2 convert \
  build/raspberrypi_pico_default/raspberrypi_pico_default.bin \
  firmware/test-firmware/sd_card.uf2 \
  --family rp2040 --offset 0x10000000
```

### Step 5: Flash to Pico
1. Hold **BOOTSEL** button on Pico
2. Plug in USB cable (keep holding BOOTSEL)
3. Release BOOTSEL
4. Drag and drop `sd_card.uf2` to the Pico mass storage device
5. Wait for reboot

### Step 6: Insert MicroSD Card
Insert a **formatted MicroSD card** (FAT32) into your board's MicroSD slot

---

## How It Works

```
QGroundControl (PC)
        ↓
   MAVLink Protocol (USB)
        ↓
PX4 Firmware on Pico
        ↓
Parameter Manager
        ↓
Auto-save to MicroSD Card (/fs/microsd/params)
        ↓
Persistent Storage Across Reboots
```

---

## Parameter Storage Flow

| Action | What Happens |
|--------|--------------|
| **Select Airframe in QGC** | QGC sends parameter via MAVLink |
| **PX4 Receives Parameter** | Parameter Manager validates & applies |
| **Auto-save to MicroSD** | `params` file created/updated automatically |
| **Power Cycle/Reboot** | PX4 boots, loads params from MicroSD |
| **Airframe Still Set!** | ✅ Configuration persisted successfully |

---

## Testing the Setup

### Test 1: Set Airframe
1. Connect Pico to QGroundControl
2. Go to **Vehicle Setup** → **Airframe**
3. Select an airframe type
4. Click **Apply and Restart**
5. Wait for reboot
6. **Verify:** Airframe should still be selected! ✅

### Test 2: Verify Parameter File
Connect to Pico's telemetry port and check:
```bash
ls -la /fs/microsd/
# You should see: params (or params.bak for backup)
```

If the SD card is mounted and parameter storage is selected correctly, you should also see:

```bash
ls -la /fs/microsd/log
```

---

## Important Requirements

✅ **MicroSD Card Required**
- Must be inserted before first parameter save
- Recommended: 4GB-32GB, FAT32 format
- Format on Windows/Mac if new card

✅ **PX4 Firmware Updated**
- Must rebuild with the configuration change
- Old firmware won't save to MicroSD

✅ **QGroundControl**
- Any recent version works (handles parameter saving automatically)
- No special configuration needed

---

## Troubleshooting

### Problem: Parameters Not Saving
**Solution:** 
- Verify MicroSD card is inserted
- Check card is formatted as FAT32
- Verify the card is mounted at `/fs/microsd`:

```sh
ls /fs
ls /fs/microsd
```

- Verify param storage selection:

```sh
param status
```

### Problem: MicroSD Card Not Detected
**Solution:**
- Power cycle the board
- Try a different MicroSD card
- Check SPI0 pin connections (CLK, MOSI, MISO, CS)

### Problem: Build Fails
**Solution:**
```bash
# Clean build (fixes most issues)
make raspberrypi_pico_default clean
make raspberrypi_pico_default
```

---

## Advanced: Backup & Restore Parameters

### Backup Parameters to PC
```bash
# Copy from board via telemetry or USB
# File location: /fs/microsd/params
cp /fs/microsd/params ~/my_parameters_backup
```

### Restore Parameters from Backup
```bash
# Copy backup back to board
cp ~/my_parameters_backup /fs/microsd/params
```

### Share Parameters with Team
- Save the `params` file
- Send via email/USB to teammates
- They can restore it on their boards

---

## File Structure After Setup

```
MicroSD Card
├── params              (Parameter file - created automatically)
├── params.bak          (Backup - created on first save)
└── [other files]
```

---

## Key Benefits

✅ **Persistent Configuration**  
Parameters survive power cycles and USB disconnects

✅ **Easy Backup & Restore**  
Copy a single file to backup all settings

✅ **Team Collaboration**  
Share parameter files with teammates easily

✅ **No Manual Saving**  
PX4 automatically saves when QGC changes parameters

✅ **Quick Recovery**  
Reset to last saved configuration instantly

---

## Reference Information

### Configuration Files Modified
- `boards/raspberrypi/pico/init/rc.board_defaults` (adds `param select /fs/microsd/params`)

### Firmware Components Used
- MicroSD Card Driver: Already enabled
- SPI0 Interface: Already configured for SD card
- Parameter Manager: Already included
- MAVLink Protocol: Already enabled

### Storage Location
- **Path:** `/fs/microsd/params`
- **Format:** Binary BSON format (PX4 standard)
- **Size:** ~4-8 KB (varies by number of parameters)

### Logging Location

PX4 logs (ULog) are stored on the SD card at:

- `/fs/microsd/log/`

QGroundControl can download/view PX4 logs. Mission Planner is primarily an ArduPilot tool and is not the recommended log workflow for PX4.

---

## Support

If you encounter issues:
1. Check that MicroSD card is properly inserted
2. Verify firmware was rebuilt with updated config
3. Try power cycling the board
4. Check connection to QGroundControl

---

## Summary

**Just change one line in the configuration file and rebuild!**

The rest happens automatically:
- QGC saves parameters → PX4 receives → Auto-saved to MicroSD
- Power cycle → PX4 loads from MicroSD → Parameters restored!

That's it! 🎉

---

*Last Updated: December 17, 2025*  
*For: OpenFC2040 FYP Project*  
*Platform: Raspberry Pi Pico + PX4*