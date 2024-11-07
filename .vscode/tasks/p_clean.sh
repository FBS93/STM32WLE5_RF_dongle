#!/usr/bin/bash

# Remove __pycache__
find /workspaces/STM32WLE5_RF_dongle/software/stm32wle5_rf_dongle_example/kivy_app -type d -name '__pycache__' -exec rm -r {} +
