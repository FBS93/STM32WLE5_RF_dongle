#!/usr/bin/bash

# Check if exactly one argument is provided
if [ "$#" -ne 1 ]; then
  echo "Error: You must provide a build preset."
  exit 1
else
  preset=$1
fi

# Build the defined preset
/workspaces/STM32WLE5_RF_dongle/.vscode/tasks/c_build.sh $preset

# Define firmware path
firmware_path="/workspaces/STM32WLE5_RF_dongle/software/stm32wle5_rf_dongle_example/firmware/build/$preset/STM32WLE5_RF_dongle.elf"

# Launch STM32_Programmer_CLI with sudo privileges to access the USB devices without needing to add udev rules on the host system.
sudo /opt/st/stm32cubeclt_1.16.0/STM32CubeProgrammer/bin/STM32_Programmer_CLI -c port=SWD -w "$firmware_path" -v -hardRst