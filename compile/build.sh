#!/bin/bash

BUILD_DIR="../build"
INSTALL_DIR="/../installed" 
CMAKE_GENERATOR="Ninja"  
CC="/C/msys64/clang64/bin/clang.exe"
CXX="/C/msys64/clang64/bin/clang++.exe"

if [ ! -d "$BUILD_DIR" ]; then
  echo "Creating build directory: $BUILD_DIR"
  mkdir "$BUILD_DIR"
fi

cd "$BUILD_DIR" || exit

echo "Running CMake configuration..."
cmake .. -G "$CMAKE_GENERATOR" -DCMAKE_C_COMPILER="$CC" -DCMAKE_CXX_COMPILER="$CXX" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR"
if [ $? -ne 0 ]; then
  echo "CMake configuration failed!"
  exit 1
fi

echo "Building project..."
cmake --build . --target all
if [ $? -ne 0 ]; then
  echo "Build failed!"
  exit 1
fi

<< COMMENT
echo "Installing project..."
sudo cmake --install .
if [ $? -ne 0 ]; then
  echo "Installation failed!"
  exit 1
fi
COMMENT

echo "Build and installation completed successfully!"
