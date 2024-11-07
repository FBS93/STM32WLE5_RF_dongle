#!/usr/bin/bash

# Check if exactly one argument is provided
if [ "$#" -ne 1 ]; then
  echo "Error: You must provide a build preset."
  exit 1
else
  preset=$1
fi

# Define the build directory based on the preset
build_dir="/workspaces/STM32WLE5_RF_dongle/software/stm32wle5_rf_dongle_example/firmware/build/$preset"

# Create the build directory if it does not exist and switch to it
mkdir -p "$build_dir"
cd "$build_dir"

# Run CMake only if the cache file does not exist (avoids re-running CMake every time)
if [ ! -f "$build_dir/CMakeCache.txt" ]; then
  cmake \
    -DCMAKE_BUILD_TYPE="$preset" \
    -DCMAKE_TOOLCHAIN_FILE=/workspaces/STM32WLE5_RF_dongle/software/stm32wle5_rf_dongle_example/firmware/cmake/gcc-arm-none-eabi.cmake \
    -S /workspaces/STM32WLE5_RF_dongle/software/stm32wle5_rf_dongle_example/firmware \
    -B "$build_dir" \
    -G "Unix Makefiles"
fi

# Build
make
