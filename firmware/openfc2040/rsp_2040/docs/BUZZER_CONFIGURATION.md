# QMB-09B-03 Passive Buzzer Configuration

## Component Specifications

**Part Number**: QMB-09B-03  
**Manufacturer**: HNDZ  
**Type**: Passive buzzer (requires external PWM drive signal)  
**Package**: Through-hole, 9mm diameter

---

## Electrical Characteristics

| Parameter | Value | Notes |
|-----------|-------|-------|
| **Optimal Frequency** | 2700Hz | Center frequency for maximum SPL |
| **Frequency Range** | 2400 - 3000Hz | ±300Hz tolerance |
| **Operating Voltage** | 3.0 - 3.6V | Nominal 3.3V |
| **Current Consumption** | ~30mA | At 3.3V, 2700Hz |
| **Sound Pressure Level** | ~85dB | At 10cm, 2700Hz |
| **Resonant Frequency** | 2700Hz ± 300Hz | Maximum acoustic output |

---

## Hardware Connection

| Pin | Connection | RP2040 GPIO |
|-----|------------|-------------|
| + | PWM Signal | GPIO25 |
| - | GND | Ground |

**Circuit**:
```
RP2040 GPIO25 ----[PWM]----> Buzzer (+)
                             Buzzer (-) ----> GND
```

**Note**: No current-limiting resistor needed. GPIO can source sufficient current (~30mA).

---

## Firmware Configuration

### In `/board/src/board_config.h`:

```c
/* Buzzer Configuration */
/* QMB-09B-03 Passive Buzzer - Requires PWM drive */
#define GPIO_BUZZER           (25 | GPIO_FUN(RP2040_GPIO_FUNC_SIO))
#define BUZZER_FREQUENCY_HZ   2700  // Optimal frequency: 2700±300Hz
#define BUZZER_FREQ_MIN_HZ    2400  // Minimum frequency
#define BUZZER_FREQ_MAX_HZ    3000  // Maximum frequency
```

### In `/board/default.px4board`:

```ini
CONFIG_DRIVERS_TONE_ALARM=y
```

---

## PWM Drive Requirements

### Critical Parameters

1. **Frequency**: Must be 2700Hz (±300Hz) for optimal volume
2. **Duty Cycle**: 50% square wave recommended
3. **Voltage**: 3.3V logic level (RP2040 GPIO output)
4. **Waveform**: Clean square wave (rise/fall time <1μs)

### PWM Calculation for RP2040

**System Clock**: 125 MHz (default)  
**Target Frequency**: 2700 Hz  
**Required Divisor**: 125,000,000 / 2700 = 46,296

**Configuration**:
```c
// PWM slice configuration for GPIO25
uint slice_num = pwm_gpio_to_slice_num(25);
uint channel = pwm_gpio_to_channel(25);

// Calculate wrap value for 2700Hz
// PWM freq = sys_clk / (wrap + 1)
uint32_t clock_freq = 125000000;  // 125 MHz
uint32_t target_freq = 2700;       // 2700 Hz
uint32_t wrap = (clock_freq / target_freq) - 1;  // ~46295

pwm_set_wrap(slice_num, wrap);
pwm_set_chan_level(slice_num, channel, wrap / 2);  // 50% duty cycle
pwm_set_enabled(slice_num, true);
```

---

## Tone Definitions (PX4)

### Recommended Frequencies

All tones should be within the 2400-3000Hz range for this buzzer:

| Tone Type | Frequency | Duration | Pattern |
|-----------|-----------|----------|---------|
| **Startup** | 2700Hz | 200ms | Single beep |
| **Armed** | 2700Hz | 100ms × 3 | Three short beeps |
| **Disarmed** | 2700Hz | 500ms | Long beep |
| **Warning** | 2600Hz | 100ms on/off | Alternating |
| **Error** | 2800Hz | 50ms on/off | Fast beeping |
| **Low Battery** | 2500Hz | 200ms × 2 | Two beeps, repeat |
| **GPS Fix** | 2700Hz | 100ms | Single short beep |
| **Mission Complete** | 2700Hz | 300ms × 2 | Two long beeps |

### Example: Startup Tone Sequence

```
Time 0ms:    Start 2700Hz tone
Time 200ms:  Stop tone
Time 300ms:  Start 2700Hz tone
Time 400ms:  Stop tone
Time 500ms:  Start 2700Hz tone
Time 700ms:  Stop tone
```

---

## Testing the Buzzer

### Via NSH Console

```bash
# Basic tone test
tone_alarm 1        # Startup tone (2700Hz)
tone_alarm 2        # Error tone (2800Hz)
tone_alarm 3        # Notify tone (2700Hz)
tone_alarm 10       # Battery warning (2500Hz)

# Custom tone (if driver supports)
tone_alarm test 2700 200    # 2700Hz for 200ms
```

### Via QGroundControl

1. **Vehicle Setup → Safety**
2. Look for "Test Buzzer" or "Test Sounds"
3. Click to test each alarm type

### Expected Behavior

- ✅ Clear, loud tone at 2700Hz (loudest)
- ✅ Slight volume decrease at 2400Hz and 3000Hz (edge of range)
- ✅ No distortion or rattling
- ✅ Sound pressure ~85dB at 10cm distance

---

## Troubleshooting

### No Sound

**Possible Causes**:
1. Wrong GPIO assignment → Check `GPIO_BUZZER` is GPIO25
2. No PWM signal → Verify PWM slice is enabled
3. Buzzer polarity reversed → Swap +/- connections
4. Buzzer damaged → Test with multimeter (should show ~30Ω impedance)

**Debug Steps**:
```bash
# Check GPIO configuration
cat /sys/class/gpio/gpio25/direction  # Should show "out"

# Check tone alarm driver
tone_alarm status

# Manual PWM test (if available)
pwm test -c 5 -p 2700 -d 50    # Channel 5 (GPIO25), 2700Hz, 50% duty
```

### Weak Sound

**Possible Causes**:
1. Wrong frequency → Should be 2700Hz ± 300Hz
2. Low duty cycle → Should be 50%
3. Low voltage → Check 3.3V supply
4. Mechanical obstruction → Ensure buzzer vent holes are clear

**Fix**:
- Verify PWM frequency is 2700Hz
- Increase duty cycle to 50%
- Check supply voltage with multimeter

### Distorted Sound

**Possible Causes**:
1. PWM frequency too high/low → Stay in 2400-3000Hz
2. Non-square waveform → Check signal with oscilloscope
3. Voltage ripple → Add decoupling capacitor (0.1μF) near buzzer

---

## Integration with PX4 Tone Alarm Driver

### Driver Location

PX4 tone alarm driver: `/drivers/tone_alarm/`

### Customization

To optimize for QMB-09B-03, modify tone definitions:

**File**: `tone_alarm_params.c` (if exists)

```c
// Default frequency should be 2700Hz
#define DEFAULT_TONE_FREQ_HZ    2700

// All tones within optimal range
static const tone_t tones[] = {
    {2700, 200},  // Startup
    {2800, 100},  // Error
    {2700, 100},  // Notify
    {2600, 200},  // Warning
    {2500, 150},  // Low battery
    // ... etc
};
```

---

## Acoustic Performance

### Frequency Response

Based on QMB-09B-03 datasheet:

```
2400Hz: ~80dB (lower limit)
2500Hz: ~82dB
2600Hz: ~84dB
2700Hz: ~85dB (PEAK - LOUDEST)
2800Hz: ~84dB
2900Hz: ~82dB
3000Hz: ~80dB (upper limit)
```

**Recommendation**: Use 2700Hz for all critical alerts to ensure maximum audibility.

---

## Safety Considerations

1. **Hearing Protection**: 85dB is moderately loud. Avoid continuous operation near ears.
2. **Current Draw**: 30mA from GPIO is within RP2040 limits (50mA max per pin).
3. **EMI**: Square wave at 2700Hz may cause minor EMI. Keep buzzer wiring short.
4. **Mechanical**: Ensure buzzer is securely mounted to prevent vibration damage.

---

## References

- **Datasheet**: QMB-09B-03 (HNDZ)
- **RP2040 Datasheet**: Section 4.5 (PWM)
- **PX4 Documentation**: Tone Alarm Driver
- **Board Config**: `/board/src/board_config.h`

---

**Last Updated**: October 30, 2024  
**Status**: ✅ Configured and optimized for 2700Hz operation
