#!/usr/bin/bash

# Launch ST debugger with sudo privileges to access the USB devices without needing to add udev rules on the host system.
sudo /opt/st/stm32cubeclt_1.16.0/STLink-gdb-server/bin/ST-LINK_gdbserver \
  -p 50000 \
  -cp /opt/st/stm32cubeclt_1.16.0/STM32CubeProgrammer/bin/ \
  --swd --halt -m 0