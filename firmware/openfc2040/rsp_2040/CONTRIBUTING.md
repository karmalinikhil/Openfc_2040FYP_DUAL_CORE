# Contributing to OpenFC2040 Firmware

Thank you for your interest in contributing to the OpenFC2040 firmware project!

## Getting Started

### Prerequisites
- Linux development environment (Ubuntu 20.04+ recommended)
- ARM GCC toolchain (`arm-none-eabi-gcc`)
- Git with submodule support
- Python 3.8+

### Initial Setup

1. **Clone the repository with submodules:**
   ```bash
   git clone --recursive https://github.com/yourteam/rsp_2040.git
   cd rsp_2040
   ```

2. **Run the setup script:**
   ```bash
   ./scripts/setup.sh
   ```

3. **Build the firmware:**
   ```bash
   ./scripts/build.sh
   ```

## Development Workflow

### Creating a Feature Branch

```bash
# Update your local repository
git checkout main
git pull origin main
git submodule update --init --recursive

# Create a new feature branch
git checkout -b feature/your-feature-name
```

### Making Changes

1. **Edit files in the appropriate directories:**
   - `board/rsp_2040/src/` - Board-specific source code
   - `drivers/` - Custom drivers
   - `configs/` - Configuration files
   - `docs/` - Documentation

2. **Build and test locally:**
   ```bash
   ./scripts/build.sh
   ./scripts/flash.sh  # Flash to hardware
   ```

3. **Run tests:**
   ```bash
   # Add your test procedures here
   ```

### Committing Changes

```bash
# Stage your changes
git add board/rsp_2040/src/your_file.c

# Commit with a descriptive message
git commit -m "Add support for new sensor XYZ"

# Push to your fork
git push origin feature/your-feature-name
```

### Commit Message Guidelines

- Use present tense ("Add feature" not "Added feature")
- Use imperative mood ("Move cursor to..." not "Moves cursor to...")
- Limit first line to 72 characters
- Reference issues and pull requests when relevant

Examples:
```
Add LSM6DS3 gyroscope calibration routine

- Implement auto-calibration on startup
- Add parameter for calibration timeout
- Update documentation

Fixes #123
```

## Code Style

### C/C++ Code
- Follow PX4 coding standards
- Use tabs for indentation (width: 8)
- Maximum line length: 120 characters
- Use descriptive variable names

### Shell Scripts
- Use `#!/bin/bash` shebang
- Add comments for complex logic
- Use `set -e` for error handling

## Testing

Before submitting a pull request:

1. **Build test:**
   ```bash
   ./scripts/build.sh
   ```

2. **Hardware test:**
   - Flash firmware to board
   - Verify all sensors are detected
   - Test PWM outputs
   - Check MAVLink communication

3. **Documentation:**
   - Update relevant documentation
   - Add comments to complex code
   - Update README if needed

## Pull Request Process

1. **Ensure your branch is up to date:**
   ```bash
   git checkout main
   git pull origin main
   git checkout feature/your-feature-name
   git rebase main
   ```

2. **Create a pull request on GitHub:**
   - Provide a clear description of changes
   - Reference related issues
   - Include test results
   - Add screenshots/logs if applicable

3. **Code review:**
   - Address reviewer feedback
   - Make requested changes
   - Push updates to your branch

4. **Merge:**
   - Once approved, your PR will be merged
   - Delete your feature branch after merge

## Updating PX4 Submodule

If you need to update the PX4-Autopilot version:

```bash
cd px4-autopilot
git fetch
git checkout v1.15.0  # or desired version/commit
cd ..
git add px4-autopilot
git commit -m "Update PX4-Autopilot to v1.15.0"
```

## Directory Structure

```
rsp_2040/
├── board/rsp_2040/      # Board configuration (edit these)
├── drivers/             # Custom drivers (edit these)
├── px4-autopilot/       # PX4 submodule (don't edit directly)
├── scripts/             # Build scripts
├── docs/                # Documentation
└── configs/             # Configuration files
```

## Reporting Issues

When reporting bugs, please include:
- Firmware version (`ver all` in NuttX console)
- Hardware setup description
- Steps to reproduce
- Expected vs actual behavior
- Console logs/error messages
- Build environment details

## Questions?

- Open an issue for bugs or feature requests
- Use discussions for questions
- Check existing issues before creating new ones

## License

By contributing, you agree that your contributions will be licensed under the BSD 3-Clause License.
