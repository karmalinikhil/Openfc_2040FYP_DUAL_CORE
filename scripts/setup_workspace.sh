#!/bin/bash
#
# FYP Workspace Setup Script
# Run this once after cloning/reorganizing workspace
#

set -e

GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${GREEN}Setting up OpenFC2040 FYP Workspace...${NC}"
echo ""

# Navigate to firmware directory and run its setup
if [ -d "firmware/openfc2040" ]; then
    cd firmware/openfc2040
    if [ -f "scripts/setup.sh" ]; then
        echo -e "${GREEN}Running firmware setup...${NC}"
        ./scripts/setup.sh
    else
        echo -e "${YELLOW}Warning: firmware/openfc2040/scripts/setup.sh not found${NC}"
    fi
else
    echo -e "${YELLOW}Warning: firmware/openfc2040 directory not found${NC}"
fi

echo ""
echo -e "${GREEN}Workspace setup complete!${NC}"
echo ""
echo "Next steps:"
echo "1. cd firmware/openfc2040"
echo "2. ./scripts/build.sh"
echo "3. ./scripts/flash.sh"
