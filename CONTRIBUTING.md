# Contributing to OpenFC2040 Firmware# Contributing to OpenFC2040 Project



**Guidelines for collaborating on the PX4 autopilot firmware for OpenFC2040****Thank you for contributing!** This guide ensures smooth collaboration.



> 🤝 **Purpose**: Ensure clean collaboration, avoid conflicts, maintain code quality  ---

> ⚠️ **CRITICAL**: Always update `docs/NEXT_STEPS.md` after EVERY commit

## 🚦 Before You Start

---

### ⚠️ CRITICAL: Always Check NEXT_STEPS.md First!

## Table of Contents

Before making ANY changes, read:

- [Getting Started](#getting-started)```bash

- [Git Workflow](#git-workflow) `cat docs/NEXT_STEPS.md`

- [Code Standards](#code-standards)```

- [Testing Requirements](#testing-requirements)

- [Commit Guidelines](#commit-guidelines)This shows:

- [Pull Request Process](#pull-request-process)- ✅ What was done last

- [Documentation Updates](#documentation-updates)- ⚠️ What's in progress

- [Common Issues](#common-issues)- ❌ Known blockers

- 🎯 What needs to be done next

---

**Why?** Avoid duplicate work and conflicts!

## Getting Started

---

### Prerequisites

## 🔄 Git Workflow

Before contributing, ensure you have:

### Daily Workflow

1. **Development environment set up** (see [README.md](README.md))

   - arm-none-eabi-gcc toolchain```bash

   - CMake, Ninja# 1. Start your day: Get latest changes

        - Python 3.x
        - From repo root: `cd path/to/your/local/clone`

   - Gitgit pull origin main

git submodule update --init --recursive

2. **Hardware for testing**:

   - OpenFC2040 board# 2. Create feature branch

   - USB-to-Serial adapter (for console)git checkout -b feature/your-feature-name

   - Picoprobe (for debugging)# Example: feature/lsm6ds3-driver

# Example: fix/led-polarity

3. **Working build**:

   ```bash# 3. Make your changes

   cd firmware/openfc2040/rsp_2040# ... edit files ...

   make raspberrypi_pico_minimal

   # Should build without errors# 4. Test your changes

   ```cd firmware/openfc2040

./scripts/build.sh           # Must build successfully!

### Fork and Clone

# 5. Update NEXT_STEPS.md (REQUIRED!)

If working from remote repository: `nano docs/NEXT_STEPS.md`

# Add your work to "Most Recent Work" section

```bash

# Fork on GitHub first, then:# 6. Commit with good message

git clone git@github.com:YOUR_USERNAME/FYP.gitgit add .

cd FYPgit commit -m "feat: Add LSM6DS3 driver



# Add upstream remote- Implemented SPI communication

git remote add upstream git@github.com:ORIGINAL_OWNER/FYP.git- Added sensor calibration

- Updated board_config.h

# Verify remotes

git remote -vRef: #issue_number"

# origin    git@github.com:YOUR_USERNAME/FYP.git (fetch)

# origin    git@github.com:YOUR_USERNAME/FYP.git (push)# 7. Push your branch

# upstream  git@github.com:ORIGINAL_OWNER/FYP.git (fetch)git push origin feature/your-feature-name

# upstream  git@github.com:ORIGINAL_OWNER/FYP.git (push)

```# 8. Create Pull Request (if using GitHub)

```

---

---

## Git Workflow

## 📝 Commit Message Format

### 1. Sync Before Starting Work

### Structure

**Always sync with latest code before making changes!**

```

```bash<type>: <subject>

# Switch to main branch

git checkout main<body>



# Fetch latest from upstream<footer>

git fetch upstream```



# Merge upstream changes### Types

git merge upstream/main

- `feat`: New feature

# Push to your fork- `fix`: Bug fix

git push origin main- `docs`: Documentation only

```- `refactor`: Code restructure (no behavior change)

- `test`: Adding tests

### 2. Create Feature Branch- `chore`: Build scripts, dependencies

- `style`: Code formatting (no logic change)

**Never work directly on `main`!**

### Examples

```bash

# Create and switch to feature branch**Good**:

git checkout -b feature/descriptive-name```

feat: Add LSM6DS3 IMU driver

# Examples of good branch names:

# feature/lsm6ds3-driver       (adding IMU driver)Implements SPI-based communication with LSM6DS3 sensor.

# fix/battery-voltage-divider  (fixing bug)Includes:

# docs/update-pin-connections  (documentation)- SPI initialization

# test/spi-peripherals         (testing)- Sensor configuration

```- Data reading

- Calibration constants

### Branch Naming Convention

Tested on hardware, sensor detected and reporting values.

| Type | Prefix | Example |

|------|--------|---------|Closes #42

| New feature | `feature/` | `feature/mavlink-telemetry` |```

| Bug fix | `fix/` | `fix/uart-console-baud` |

| Documentation | `docs/` | `docs/add-debug-guide` |**Good**:

| Testing | `test/` | `test/gpio-outputs` |```

| Refactoring | `refactor/` | `refactor/spi-driver` |fix: Correct RGB LED polarity to active-LOW

| Hardware update | `hardware/` | `hardware/pcb-v2` |

LEDs were configured as active-HIGH causing inverted behavior.

### 3. Make ChangesChanged GPIO_LED_* definitions in board_config.h to use

PX4_MAKE_GPIO_OUTPUT_CLEAR for active-LOW operation.

```bash

# Edit filesFixes #38

vim firmware/openfc2040/rsp_2040/board/board_config.h```



# Test build frequently**Bad** (too vague):

make raspberrypi_pico_minimal```

fix stuff

# Check what changed```

git status

git diff**Bad** (no context):

``````

update files

### 4. Commit Changes```



```bash---

# Stage specific files (don't use `git add .`)

git add firmware/openfc2040/rsp_2040/board/board_config.h## 🎯 NEXT_STEPS.md Update (REQUIRED!)



# Commit with descriptive message (see Commit Guidelines below)### After EVERY commit, update this file:

git commit -m "fix: correct battery voltage divider to 5.09:1

**Location**: `docs/NEXT_STEPS.md`

- Update BOARD_BATTERY_V_DIV from 1.74 to 5.09

- Supports 4S LiPo max 16.8V safely within 3.3V ADC limit### Template to Use

- Verified against schematic: R6=2kΩ, R7=2.7kΩ

- References issue #42"```markdown

## Most Recent Work

# Push to your fork

git push origin feature/battery-voltage-divider### [YYYY-MM-DD] - [Your Name]

```

**What I did**:

### 5. Create Pull Request- Implemented X

- Fixed Y

1. Go to your fork on GitHub- Tested Z

2. Click "Pull Request"

3. Base: `main` ← Compare: `feature/battery-voltage-divider`**Files modified**:

4. Fill out PR template (see below)- path/to/file1.cpp

5. Request review- path/to/file2.h

- path/to/config.h

### 6. Address Review Comments

**Current state**:

```bash- Feature X: ✅ Complete and tested

# Make requested changes- Feature Y: ⚠️ In progress (70% done)

vim some_file.c- Feature Z: ❌ Blocked (reason: need hardware)



# Commit changes**Next person should**:

git add some_file.c1. Complete Y by implementing...

git commit -m "fix: address review comments - add input validation"2. Test Z when hardware arrives

3. Verify X works with...

# Push to update PR4. Document findings in...

git push origin feature/battery-voltage-divider

```**Known issues**:

- Issue A: workaround is...

### 7. Merge and Cleanup- Issue B: investigating...

```

After PR merged:

---

```bash

# Switch back to main## ✅ Testing Requirements

git checkout main

### Before You Commit

# Sync with upstream

git pull upstream main**Required Checks**:

- [ ] Code compiles without errors

# Delete feature branch- [ ] Code compiles without warnings (or warnings justified)

git branch -d feature/battery-voltage-divider- [ ] Tested on hardware (if hardware changes)

- [ ] Documentation updated

# Delete remote branch- [ ] NEXT_STEPS.md updated

git push origin --delete feature/battery-voltage-divider

```### Testing Commands



---```bash

cd firmware/openfc2040

## Code Standards

# Clean build (recommended before commit)

### C/C++ Style./scripts/build.sh clean

./scripts/build.sh

Follow **PX4 coding style**:

# Flash and test (if possible)

#### Indentation./scripts/flash.sh

- **Tabs**: 8 spaces (PX4 convention, not 4!)# Connect to console, verify functionality

- **Braces**: K&R style (opening brace on same line)```



```c### If You Can't Test on Hardware

// GOOD

if (condition) {**It's OK!** Just document it:

        do_something();```markdown

}**Testing status**:

- ✅ Compiles successfully

// BAD- ⚠️ Not tested on hardware (no board available)

if (condition)- 📝 Needs verification by someone with hardware

{```

    do_something();

}---

```

## 🗂️ File Organization

#### Naming Conventions

### Where to Put New Files

```c

// Functions: lowercase with underscores| File Type | Location | Example |

void initialize_sensors(void);|-----------|----------|---------|

| **New driver** | `firmware/openfc2040/drivers/<category>/` | `drivers/imu/lsm6ds3/` |

// Variables: lowercase with underscores| **Board config** | `firmware/openfc2040/board/src/` | `board/src/board_config.h` |

int sensor_count = 0;| **Documentation** | `docs/<category>/` | `docs/hardware/IMU_SETUP.md` |

| **Test scripts** | `testing/bench-tests/` | `testing/bench-tests/imu_test.py` |

// Constants: UPPERCASE| **Build scripts** | `firmware/openfc2040/scripts/` | `scripts/test.sh` |

#define MAX_SENSORS 10| **Hardware docs** | `hardware/` | `hardware/datasheets/lsm6ds3.pdf` |



// Structs: lowercase with _s suffix---

struct sensor_data_s {

        int value;## 📖 Code Standards

        uint64_t timestamp;

};### C/C++ Style



// Enums: lowercase with _e suffixFollow [PX4 Coding Style](https://docs.px4.io/main/en/contribute/code.html):

enum sensor_type_e {- Use **tabs** (not spaces) for indentation

        SENSOR_GYRO,- Max line length: **120 characters**

        SENSOR_ACCEL- Always add **comments** explaining WHY, not WHAT

};- Use **meaningful variable names**

```

**Example**:

#### Comments```cpp

// Good: Explains why, not what

```c// Set active-LOW because RGB LED has common-anode configuration

/**#define GPIO_LED_RED PX4_MAKE_GPIO_OUTPUT_CLEAR(13)

 * Initialize SPI peripheral for IMU

 *// Bad: States the obvious

 * @param spi_bus SPI bus number (0 or 1)// Set GPIO 13

 * @return 0 on success, negative errno on failure#define GPIO_LED_RED PX4_MAKE_GPIO_OUTPUT_CLEAR(13)

 */```

int spi_init(int spi_bus);

### Python Style

// Single-line comment for brief explanations

- Follow **PEP 8**

/* - Use **type hints**

 * Multi-line comment- Add **docstrings**

 * for longer explanations

 */---

```

## 🤝 Remote Collaboration

### File Structure

### For Remote Team Members

#### New Driver Files

**1. Daily Sync**

``````bash

firmware/openfc2040/rsp_2040/# Every morning before starting work

├── drivers/git pull --rebase origin main

│   └── imu/git submodule update --init --recursive

│       ├── lsm6ds3/```

│       │   ├── LSM6DS3.hpp          # Class definition

│       │   ├── LSM6DS3.cpp          # Implementation**2. Communicate via NEXT_STEPS.md**

│       │   └── lsm6ds3_registers.h  # Register definitions- Read it before starting

```- Update it after finishing

- Use it to coordinate

#### Header Guards

**3. Avoid Merge Conflicts**

```c- Pull before starting work

#ifndef LSM6DS3_REGISTERS_H- Work on different files when possible

#define LSM6DS3_REGISTERS_H- Communicate in team chat if editing same files

- Push frequently (at least daily)

// Content here

### Handling Merge Conflicts

#endif // LSM6DS3_REGISTERS_H

``````bash

# If pull fails with conflicts

### Build Integrationgit status                  # See conflicted files



When adding new files, update **CMakeLists.txt**:# Edit files to resolve conflicts

# Look for <<<<<<< HEAD markers

```cmake

# firmware/openfc2040/rsp_2040/drivers/imu/CMakeLists.txt# After resolving

add_library(lsm6ds3git add <resolved_files>

        LSM6DS3.cppgit rebase --continue

        lsm6ds3_registers.h

)# Or abort if needed

git rebase --abort

target_include_directories(lsm6ds3 PUBLIC .)```

```

---

---

## 📁 Documentation Standards

## Testing Requirements

### Every Code Change Should Update

### Before Every Commit

1. **Inline comments** (in code)

#### 1. Verify Build2. **README.md** (if adding new feature)

3. **Relevant docs/ file** (technical details)

```bash4. **NEXT_STEPS.md** (current status)

# Clean build

make clean### Documentation Should

make raspberrypi_pico_minimal

- ✅ Be beginner-friendly

# Should complete without errors- ✅ Include examples

# Check output:- ✅ Explain WHY not just WHAT

ls -lh px4-autopilot/build/raspberrypi_pico_minimal/raspberrypi_pico_minimal.uf2- ✅ Stay up-to-date

```- ✅ Have clear section headers

- ✅ Use proper markdown formatting

#### 2. Check for Errors

---

```bash

# No compiler warnings## 🎯 Contribution Checklist

make raspberrypi_pico_minimal 2>&1 | grep -i "warning"

# Should be empty or only warnings from PX4 upstreamBefore creating a pull request:



# Check file size- [ ] Code compiles without errors

ls -lh *.uf2- [ ] Code compiles without warnings (or justified)

# Should be < 2MB (flash size)- [ ] Followed coding standards

```- [ ] Added/updated comments

- [ ] Tested (or documented why not tested)

#### 3. Test on Hardware (if possible)- [ ] Updated relevant documentation

- [ ] **Updated NEXT_STEPS.md**

```bash- [ ] Commit message follows format

# Flash firmware- [ ] Branch name is descriptive

cp raspberrypi_pico_minimal.uf2 /media/$USER/RPI-RP2/

---

# Connect UART console

picocom -b 115200 /dev/ttyUSB0## 🆘 Questions?



# Verify boot1. **Check documentation first**: [docs/](docs/)

# Should see: "NuttShell (NSH) NuttX-12.x.x"2. **Read NEXT_STEPS.md**: May already be answered

3. **Check archived investigations**: [docs/development/archived/](docs/development/archived/)

# Test basic commands4. **Ask team**: Team chat or create GitHub issue

nsh> help

nsh> ps       # Show processes---

nsh> free     # Show memory

```## 🎉 Thank You!



### Required Tests for Specific ChangesYour contributions make this project better. Following these guidelines helps everyone work together smoothly.



| Change Type | Required Tests |**Happy coding!** 🚀

|-------------|----------------|
| **New driver** | Build test, hardware test, sensor data validation |
| **GPIO change** | Verify no conflicts, test with multimeter/scope |
| **UART/SPI config** | Test communication with peripheral |
| **PWM output** | Test with oscilloscope or servo |
| **Battery monitoring** | Test with voltage divider, verify ADC reading |
| **Build system** | Clean build, verify all configs still work |

### Hardware Testing Checklist

When testing on real hardware:

- [ ] Firmware builds without errors
- [ ] Firmware flashes successfully
- [ ] Board boots (LED blinks or console accessible)
- [ ] No smoke, no overheating
- [ ] UART console shows boot messages
- [ ] New feature works as expected
- [ ] Existing features still work (no regression)
- [ ] No excessive current draw (measure with multimeter)

---

## Commit Guidelines

### Commit Message Format

Use **Conventional Commits** format:

```
<type>(<scope>): <subject>

<body>

<footer>
```

#### Type

Must be one of:

- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation only
- `style`: Code style (formatting, no logic change)
- `refactor`: Code refactoring (no feature/fix)
- `test`: Adding tests
- `chore`: Build system, dependencies

#### Scope (Optional)

Component affected:

- `imu`: IMU driver
- `baro`: Barometer
- `spi`: SPI driver
- `uart`: UART driver
- `pwm`: PWM outputs
- `build`: Build system
- `board`: Board configuration

#### Subject

- Imperative mood ("add" not "added")
- Lowercase (except proper nouns)
- No period at end
- Max 50 characters

#### Body (Optional but Recommended)

- Wrap at 72 characters
- Explain **what** and **why**, not **how**
- Reference issues: `Fixes #123` or `Closes #456`

### Examples

#### Good Commits

```
feat(imu): add LSM6DS3 driver

- Implement SPI communication with LSM6DS3
- Support gyro range ±2000 dps, accel ±16g
- Poll at 100Hz, publish to uORB topic
- Tested on hardware, verified sensor data

Closes #15
```

```
fix(board): correct battery voltage divider

BOARD_BATTERY_V_DIV was 1.74 (max 5.74V) but 4S LiPo is 16.8V.
This would exceed RP2040 ADC limit (3.3V) and damage the chip.

Updated to 5.09:1 based on schematic analysis:
- R6 = 2kΩ (to ADC)
- R7 = 2.7kΩ (to GND)
- Divider ratio: 2kΩ / (2kΩ + 2.7kΩ) = 0.4255
- Max voltage: 3.3V / 0.4255 = 7.75V (still too low!)

WARNING: Current divider CANNOT handle 4S LiPo safely.
Board should only be used with 2S LiPo (max 8.4V) until hardware fixed.

Fixes #42
```

```
docs: add comprehensive debugging guide

- Complete SWD setup instructions
- OpenOCD + GDB configuration
- Picoprobe wiring diagrams
- Common troubleshooting steps
- Alternative debugging methods

Closes #38
```

```
chore(build): update NuttX submodule to v12.4.0

- Includes fix for UART console stability
- Improves USB CDC-ACM reliability
- No breaking changes
```

#### Bad Commits (Don't Do This!)

```
# ❌ Too vague
fix: update stuff

# ❌ Not descriptive
feat: add driver

# ❌ Multiple unrelated changes
fix: fix battery voltage and add IMU driver and update docs

# ❌ No capitalization, ends with period
feat: Added new feature.

# ❌ Past tense
fix: fixed the bug
```

### Atomic Commits

**One logical change per commit!**

**Good** (3 separate commits):
1. `fix(board): correct battery voltage divider`
2. `feat(imu): add LSM6DS3 driver`
3. `docs: update PIN_CONNECTIONS.md with IMU specs`

**Bad** (1 messy commit):
```
fix: lots of changes
- battery voltage divider
- added IMU driver
- updated docs
- fixed typo in README
```

---

## Pull Request Process

### PR Template

When creating PR, include:

```markdown
## Description
Brief summary of changes.

## Type of Change
- [ ] Bug fix
- [ ] New feature
- [ ] Documentation update
- [ ] Refactoring

## Testing
- [ ] Built successfully
- [ ] Tested on hardware
- [ ] No regressions

## Checklist
- [ ] Code follows style guidelines
- [ ] Committed with descriptive messages
- [ ] Updated docs/NEXT_STEPS.md
- [ ] Updated relevant documentation
- [ ] Added tests (if applicable)

## Related Issues
Fixes #123
Closes #456
```

### Review Process

**Reviewers will check**:
1. Code quality and style
2. Test coverage
3. Documentation updates
4. No breaking changes
5. Commit messages follow guidelines

**Contributor responsibilities**:
- Respond to review comments within 48 hours
- Make requested changes promptly
- Re-request review after updates
- Keep PR scope focused (don't add unrelated changes)

### Approval and Merge

- **Requires**: 1 approval from maintainer
- **Merge method**: Squash and merge (for clean history)
- **After merge**: Delete feature branch

---

## Documentation Updates

### ⚠️ CRITICAL: Always Update NEXT_STEPS.md

**After EVERY commit that gets merged**, update `docs/NEXT_STEPS.md`:

```markdown
## What Was Done Last
**Date**: 2024-12-XX
**Author**: Your Name

### Completed Tasks
- ✅ Fixed battery voltage divider (commit abc123)
- ✅ Added LSM6DS3 driver (commit def456)

### Issues Encountered
- Battery divider still insufficient for 4S LiPo
- Need hardware revision

### Changes Made
- Updated board_config.h with new BOARD_BATTERY_V_DIV
- Added drivers/imu/lsm6ds3/ with full driver implementation
- Updated PIN_CONNECTIONS.md with verified schematic data
```

### Update Other Docs

When making changes, update relevant documentation:

| Change | Update |
|--------|--------|
| New GPIO usage | `docs/PIN_CONNECTIONS.md` |
| Build process change | `docs/PROJECT_CONTEXT.md` |
| New debugging method | `docs/DEBUG_GUIDE.md` |
| Project structure | `README.md` |
| Dependencies added | `README.md` |

---

## Common Issues

### Merge Conflicts

```bash
# Sync with upstream first
git fetch upstream
git checkout main
git merge upstream/main

# Rebase your feature branch
git checkout feature/my-feature
git rebase main

# Resolve conflicts in editor
vim conflicted_file.c

# Mark as resolved
git add conflicted_file.c
git rebase --continue

# Force push (rebase rewrites history)
git push origin feature/my-feature --force
```

### Accidentally Committed to Main

```bash
# Move commit to new branch
git branch feature/accidental-commit
git reset --hard HEAD~1

# Switch to feature branch
git checkout feature/accidental-commit

# Push feature branch
git push origin feature/accidental-commit
```

### Large Binary Files

**Don't commit**:
- ❌ Build artifacts (*.o, *.elf, *.uf2)
- ❌ IDE files (.vscode/, *.swp)
- ❌ Large datasheets (> 1MB)

**Already in .gitignore**:
```
build/
*.uf2
*.elf
*.o
.vscode/
```

If accidentally committed:

```bash
# Remove from git but keep file
git rm --cached large_file.pdf

# Add to .gitignore
echo "large_file.pdf" >> .gitignore

# Commit removal
git commit -m "chore: remove large binary from git"
```

### Syncing Forks

Keep your fork updated weekly:

```bash
# Fetch all changes from upstream
git fetch upstream

# Merge upstream changes to main
git checkout main
git merge upstream/main

# Push to your fork
git push origin main

# Update feature branch if needed
git checkout feature/my-feature
git rebase main
```

---

## Code Review Guidelines

### As a Contributor

**Before requesting review**:
- [ ] Code is self-documented (clear variable names, comments)
- [ ] No debug print statements left in
- [ ] No commented-out code blocks
- [ ] Formatting is consistent
- [ ] All tests pass

**During review**:
- Be open to feedback
- Explain your reasoning if you disagree
- Make requested changes promptly
- Ask questions if unclear

### As a Reviewer

**Check for**:
- Code correctness and logic
- Edge cases handled
- Error handling present
- Memory leaks or buffer overflows
- Performance implications
- Code readability

**Provide**:
- Constructive feedback
- Specific suggestions
- Explanations for requested changes

---

## Communication

### Where to Discuss

- **GitHub Issues**: Bug reports, feature requests
- **Pull Requests**: Code-specific discussions
- **Discord/Slack** (if applicable): General questions, quick help

### Creating Issues

Use issue templates:

**Bug Report**:
```markdown
**Describe the bug**
Firmware doesn't boot after flashing.

**To Reproduce**
1. Flash raspberrypi_pico_minimal.uf2
2. Connect UART console
3. See no output

**Expected behavior**
Should see NuttShell prompt.

**Environment**
- Board: OpenFC2040 rev 1.0
- Firmware: commit abc123
- Console: picocom 115200 baud

**Additional context**
LED blinks 3 times then stops.
```

**Feature Request**:
```markdown
**Is your feature related to a problem?**
Need to support PPM RC input, currently only SBUS.

**Describe the solution**
Add PPM decoding on GPIO24 using PIO.

**Additional context**
Many old RC receivers only output PPM.
```

---

## Quick Reference

```
┌─────────────────────────────────────────────────────────┐
│ Git Workflow Quick Reference                             │
├─────────────────────────────────────────────────────────┤
│ BEFORE STARTING:                                         │
│   git checkout main                                      │
│   git pull upstream main                                 │
│   git checkout -b feature/my-feature                     │
│                                                           │
│ DURING WORK:                                             │
│   make raspberrypi_pico_minimal    # Build frequently    │
│   git status                       # Check changes       │
│   git diff                         # Review changes      │
│                                                           │
│ COMMITTING:                                              │
│   git add specific_file.c          # Stage files         │
│   git commit -m "type: subject"    # Descriptive message │
│   git push origin feature/my-feature                     │
│                                                           │
│ PULL REQUEST:                                            │
│   1. Create PR on GitHub                                 │
│   2. Fill out template                                   │
│   3. Request review                                      │
│   4. Address comments                                    │
│   5. Merge after approval                                │
│                                                           │
│ AFTER MERGE:                                             │
│   git checkout main                                      │
│   git pull upstream main                                 │
│   git branch -d feature/my-feature                       │
│   UPDATE docs/NEXT_STEPS.md ← REQUIRED!                  │
│                                                           │
│ COMMIT MESSAGE FORMAT:                                   │
│   type(scope): subject                                   │
│                                                           │
│   body (what and why)                                    │
│                                                           │
│   Fixes #123                                             │
│                                                           │
│ TYPES: feat, fix, docs, style, refactor, test, chore     │
└─────────────────────────────────────────────────────────┘
```

---

## Additional Resources

- **PX4 Contribution Guide**: https://docs.px4.io/main/en/contribute/
- **Git Best Practices**: https://git-scm.com/book/en/v2
- **Conventional Commits**: https://www.conventionalcommits.org/

---

**Thank you for contributing!** 🚀

For questions, open an issue or reach out to maintainers.
