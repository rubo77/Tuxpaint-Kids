#!/bin/bash
# Build signed release APK for Tux Paint Kids

echo "================================================"
echo "  Building Signed Release APK"
echo "================================================"
echo ""

# Prompt for keystore passwords
read -sp "Enter Keystore Password: " KEYSTORE_PASSWORD
echo ""
read -sp "Enter Key Password: " KEY_PASSWORD
echo ""
echo ""

# Export passwords as environment variables
export KEYSTORE_PASSWORD
export KEY_PASSWORD

# Build both flavors
echo "Building PlayStore Release APK..."
./gradlew assemblePlayStoreRelease

echo ""
echo "Building OffPlayStore Release APK..."
./gradlew assembleOffPlayStoreRelease

echo ""
echo "================================================"
echo "  Build Complete!"
echo "================================================"
echo ""
echo "Signed APKs created:"
echo ""
ls -lh app/build/outputs/apk/*/release/*.apk 2>/dev/null | grep -v unsigned || echo "No signed APKs found"
echo ""
echo "Location:"
echo "  PlayStore:    app/build/outputs/apk/playStore/release/"
echo "  OffPlayStore: app/build/outputs/apk/offPlayStore/release/"
echo ""
