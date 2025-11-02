#!/bin/bash
#
# OpenFC2040 Documentation Reorganization Script
#
# This script reorganizes scattered documentation into proper structure
# Run from: /home/rishi-patil/Desktop/FYP/rsp_2040/
#

set -e

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}OpenFC2040 Documentation Reorganization${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

# Get project root
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$PROJECT_ROOT"

echo -e "${GREEN}[1/5]${NC} Creating directory structure..."

# Create new directory structure
mkdir -p docs/getting-started
mkdir -p docs/hardware/schematics
mkdir -p docs/development/archived
mkdir -p docs/testing
mkdir -p docs/reference

echo -e "${GREEN}[2/5]${NC} Moving documentation files..."

# Move and rename files
[ -f "PROJECT_CONTEXT.md" ] && mv -v PROJECT_CONTEXT.md docs/PROJECT_STATUS.md
[ -f "BUILD_LOG.md" ] && mv -v BUILD_LOG.md docs/development/
[ -f "NEXT_STEPS.md" ] && mv -v NEXT_STEPS.md docs/development/
[ -f "TESTING_CHECKLIST.md" ] && mv -v TESTING_CHECKLIST.md docs/testing/TEST_PROCEDURES.md
[ -f "UART_CONNECTION_GUIDE.md" ] && mv -v UART_CONNECTION_GUIDE.md docs/hardware/UART_CONSOLE_SETUP.md
[ -f "USB_CONSOLE_INVESTIGATION.md" ] && mv -v USB_CONSOLE_INVESTIGATION.md docs/development/archived/
[ -f "USB_CONSOLE_FAILURE_ANALYSIS.md" ] && mv -v USB_CONSOLE_FAILURE_ANALYSIS.md docs/development/archived/
[ -f "FIRMWARE_BUILDS.md" ] && mv -v FIRMWARE_BUILDS.md docs/development/FIRMWARE_VARIANTS.md
[ -f "HARDWARE_REVIEW_SUMMARY.md" ] && mv -v HARDWARE_REVIEW_SUMMARY.md docs/hardware/COMPONENT_VERIFICATION.md
[ -f "MIGRATION_PLAN.md" ] && mv -v MIGRATION_PLAN.md docs/development/archived/
[ -f "DIAGNOSIS_SUMMARY.md" ] && mv -v DIAGNOSIS_SUMMARY.md docs/development/archived/
[ -f "IMPLEMENTATION_SUMMARY.md" ] && mv -v IMPLEMENTATION_SUMMARY.md docs/development/archived/

# Remove duplicate/outdated files
echo -e "${GREEN}[3/5]${NC} Removing outdated files..."
[ -f "QUICK_START.md" ] && rm -v QUICK_START.md
[ -f "QUICK_START_AFTER_REVIEW.md" ] && rm -v QUICK_START_AFTER_REVIEW.md
[ -f "NEXT_STEPS_UART_CONSOLE.md" ] && mv -v NEXT_STEPS_UART_CONSOLE.md docs/hardware/

echo -e "${GREEN}[4/5]${NC} Creating documentation index..."

# Create docs/README.md
cat > docs/README.md << 'EOF'
# OpenFC2040 Documentation Index

**Quick Navigation**: Find what you need fast!

---

## 🚀 Getting Started

**New to the project?** Start here:
1. [Project Status](PROJECT_STATUS.md) - What works, what doesn't
2. [Architecture Overview](ARCHITECTURE.md) - How PX4/NuttX/RP2040 fit together
3. [Build System Explained](BUILD_SYSTEM_EXPLAINED.md) - Understanding compilation
4. [First Build Guide](getting-started/FIRST_BUILD.md) - Step-by-step

---

## 📖 Documentation Categories

### Getting Started
- [Dependencies Guide](getting-started/DEPENDENCIES.md) - What each tool does and why
- [First Build](getting-started/FIRST_BUILD.md) - Beginner-friendly build guide
- [Flashing Guide](getting-started/FLASHING_GUIDE.md) - All flashing methods
- [Troubleshooting](getting-started/TROUBLESHOOTING.md) - Common issues + solutions

### Hardware
- [GPIO Pin Mapping](hardware/GPIO_PIN_MAPPING.md) - Complete pin assignments
- [Hardware Analysis](hardware/HARDWARE_ANALYSIS.md) - Component breakdown
- [UART Console Setup](hardware/UART_CONSOLE_SETUP.md) - Console configuration
- [Component Verification](hardware/COMPONENT_VERIFICATION.md) - Testing results
- [Power Rails](hardware/POWER_RAILS.md) - Voltage levels and distribution
- [Schematics](hardware/schematics/) - Board schematics and diagrams

### Development
- [Build Log](development/BUILD_LOG.md) - Build history and changes
- [Next Steps](development/NEXT_STEPS.md) - **⚠️ UPDATE AFTER EVERY COMMIT!**
- [Firmware Variants](development/FIRMWARE_VARIANTS.md) - Different build configurations
- [Debugging Guide](development/DEBUGGING.md) - SWD, OpenOCD, GDB
- [Code Style](development/CODE_STYLE.md) - Coding standards
- [Driver Development](development/DRIVER_DEVELOPMENT.md) - How to add drivers
- [Archived Investigations](development/archived/) - Past troubleshooting docs

### Testing
- [Test Procedures](testing/TEST_PROCEDURES.md) - Systematic testing checklist
- [Bench Testing](testing/BENCH_TESTING.md) - Hardware verification
- [Flight Testing](testing/FLIGHT_TESTING.md) - Flight test protocols

### Reference
- [PX4/RP2040 Integration](reference/PX4_RP2040_INTEGRATION.md) - Platform specifics
- [NuttX Configuration](reference/NUTTX_CONFIGURATION.md) - OS configuration details
- [API Reference](reference/API_REFERENCE.md) - Custom function documentation

---

## 🔍 Quick Search

**Looking for:**
- **Pin assignments?** → [GPIO Pin Mapping](hardware/GPIO_PIN_MAPPING.md)
- **Build errors?** → [Troubleshooting](getting-started/TROUBLESHOOTING.md)
- **What to work on next?** → [Next Steps](development/NEXT_STEPS.md)
- **How to contribute?** → [Contributing Guide](../CONTRIBUTING.md)
- **Why USB console fails?** → [Archived: USB Investigation](development/archived/USB_CONSOLE_INVESTIGATION.md)
- **How compilation works?** → [Build System Explained](BUILD_SYSTEM_EXPLAINED.md)

---

## 📝 Documentation Standards

When updating documentation:
1. **Be beginner-friendly** - Explain WHY not just WHAT
2. **Include examples** - Show, don't just tell
3. **Keep it current** - Update after code changes
4. **Use clear titles** - Make it searchable
5. **Add to this index** - Help others find your docs

---

## 🤝 Contributing

See [CONTRIBUTING.md](../CONTRIBUTING.md) for:
- How to update NEXT_STEPS.md
- Documentation workflow
- Review process

---

**Last Updated**: 2024-11-01
EOF

echo -e "${GREEN}[5/5]${NC} Creating placeholder files for future documentation..."

# Create placeholder files
cat > docs/ARCHITECTURE.md << 'EOF'
# OpenFC2040 Architecture Overview

**Status**: 🚧 To be written (see REORGANIZATION_PLAN.md for outline)

This document will explain:
- How PX4, NuttX, and RP2040 layers interact
- Data flow through the system
- Compilation process
- Why we need multiple layers

**Coming soon!**
EOF

cat > docs/BUILD_SYSTEM_EXPLAINED.md << 'EOF'
# Build System Explained

**Status**: 🚧 To be written (see REORGANIZATION_PLAN.md for outline)

This document will explain:
- What actually happens when you run `./scripts/build.sh`
- Which files are involved in compilation
- How CMake, Ninja, and compilers work together
- Common build misconceptions

**Coming soon!**
EOF

echo ""
echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}✅ Reorganization Complete!${NC}"
echo -e "${GREEN}========================================${NC}"
echo ""
echo -e "${YELLOW}Next steps:${NC}"
echo "1. Review docs/README.md for navigation"
echo "2. Update docs/PROJECT_STATUS.md with current state"
echo "3. Follow REORGANIZATION_PLAN.md to create missing docs"
echo "4. Update root README.md to reference new structure"
echo ""
echo -e "${BLUE}Pro tip:${NC} Run 'tree docs/' to see new structure"
echo ""
