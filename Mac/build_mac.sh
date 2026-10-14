#!/bin/bash
set -e

# Script location: <project>/Mac/build_mac.sh
MAC_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$MAC_DIR/.." && pwd)"
BUILD_DIR="$PROJECT_DIR/build/mac"
OUTPUT="$BUILD_DIR/alchemistra"

mkdir -p "$BUILD_DIR"

SOURCES=$(find "$PROJECT_DIR/src" -name "*.cpp" | sort)
INCLUDES=$(find "$PROJECT_DIR/include" -type d | sed 's/^/-I /' | tr '\n' ' ')
LIBS=$(find "$PROJECT_DIR/lib/macos" -type d 2>/dev/null | sed 's/^/-L /' | tr '\n' ' ')

g++ $SOURCES -o "$OUTPUT" -std=c++17 $INCLUDES $LIBS -lraylib \
  -framework CoreVideo \
  -framework IOKit \
  -framework Cocoa \
  -framework GLUT \
  -framework OpenGL

# The game loads assets with relative paths such as "assets/ball_basic.png".
# Copy assets next to the executable so it can be launched from the build folder.
rm -rf "$BUILD_DIR/assets"
cp -R "$PROJECT_DIR/assets" "$BUILD_DIR/assets"

echo "Mac build completed: $OUTPUT"
echo "Run with: cd '$BUILD_DIR' && ./alchemistra"
