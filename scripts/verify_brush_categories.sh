#!/bin/bash

# Verify Brush Categories Implementation
# This script analyzes logcat output to verify brush categorization

echo "====================================="
echo "Brush Categories Verification Script"
echo "====================================="
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check if adb is available
if ! command -v adb &> /dev/null; then
    echo -e "${RED}Error: adb not found. Please install Android SDK tools.${NC}"
    exit 1
fi

# Check if device is connected
if ! timeout 5 adb devices | grep -q "device$"; then
    echo -e "${RED}Error: No Android device connected.${NC}"
    echo "Please connect a device and enable USB debugging."
    exit 1
fi

echo -e "${GREEN}✓ Android device connected${NC}"
echo ""

# Clear logcat
echo "Clearing logcat..."
timeout 5 adb logcat -c

# Install APK if requested
if [ "$1" == "--install" ]; then
    echo "Installing APK..."
    APK_PATH="app/build/outputs/apk/offPlayStore/debug/app-offPlayStore-debug.apk"
    
    if [ ! -f "$APK_PATH" ]; then
        echo -e "${RED}Error: APK not found at $APK_PATH${NC}"
        exit 1
    fi
    
    if timeout 60 adb install -r "$APK_PATH"; then
        echo -e "${GREEN}✓ APK installed successfully${NC}"
    else
        echo -e "${RED}Error: Failed to install APK${NC}"
        exit 1
    fi
    
    echo ""
    sleep 2
fi

# Start monitoring logcat
echo "Starting logcat monitoring..."
echo "Please open TuxPaint and navigate to the brush tool."
echo "Press Ctrl+C to stop monitoring."
echo ""
echo "-----------------------------------"

# Monitor for brush-related logs
timeout 300 adb logcat | grep --line-buffered -E "(init_sorted_brushes|BRUSH_CATEGORY|CHILD_MODE|draw_brushes|cur_brush|sorted_brush)" | while IFS= read -r line; do
    # Highlight important messages
    if echo "$line" | grep -q "init_sorted_brushes"; then
        echo -e "${GREEN}$line${NC}"
    elif echo "$line" | grep -q "Warning"; then
        echo -e "${YELLOW}$line${NC}"
    elif echo "$line" | grep -q "Error"; then
        echo -e "${RED}$line${NC}"
    else
        echo "$line"
    fi
done

echo ""
echo "-----------------------------------"
echo "Monitoring stopped."
