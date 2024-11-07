import kivy
kivy.require('2.3.0')

from kivy.uix.relativelayout import RelativeLayout
from kivy.clock import Clock

import serial.tools.list_ports

from scrollable_log_window.scrollable_log_window import ScrollableLogWindow
from popup_device_selection.popup_device_selection import PopupDeviceSelection
from popup_config.popup_config import PopupConfig
from stm32wle5_rf_dongle.stm32wle5_rf_dongle import RFDongle, RadioConfig


class RFConsole(RelativeLayout):

    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self._ftdi_vid = 0x0403  # FTDI Vendor ID
        self._ftdi_pid = 0x6015  # Product ID for FT230X
        self._rf_dongle = None
        self._popup_config = PopupConfig()
        self._popup_config.set_listener_on_apply_button_event(self._apply_conf_button_callback)
        self._popup_config.set_listener_on_cancel_button_event(self._cancel_conf_button_callback)

        # Get a list of available serial ports
        ports = serial.tools.list_ports.comports()

        # Loop through available ports to find the matching FTDI devices
        device_list = []
        for port in ports:
            if ((port.vid == self._ftdi_vid) and (port.pid == self._ftdi_pid)):
                device_list.append(port.device)

        self._popup_device_selection = PopupDeviceSelection(devices= device_list)
        self._popup_device_selection.set_listener_on_apply_button_event(self._apply_selec_button_callback)
        self._popup_device_selection.set_listener_on_cancel_button_event(self._cancel_selec_button_callback)

        # Default configuration for FSK modulation
        self._fsk_default_config = RadioConfig()
        self._fsk_default_config.tx_modem = 0                 # FSK (0)
        self._fsk_default_config.tx_channel = 433000000       # TX frequency (433 MHz)
        self._fsk_default_config.tx_bandwidth = 0             # N/A for FSK, set to 0
        self._fsk_default_config.tx_data_rate = 1200          # FSK data rate (1200 bps)
        self._fsk_default_config.tx_code_rate = 0             # N/A for FSK, set to 0
        self._fsk_default_config.tx_preamble_len = 8          # TX preamble length (8 bytes)
        self._fsk_default_config.tx_fixed_len = False         # Variable length packets for TX
        self._fsk_default_config.tx_crc_on = True             # Enable CRC for TX
        self._fsk_default_config.tx_iq_inverted = False       # N/A for FSK
        self._fsk_default_config.tx_power = 10                # TX power (10 dBm)
        self._fsk_default_config.tx_fdev = 500                # Frequency deviation for TX (500Hz)
        self._fsk_default_config.tx_timeout = 1000            # TX timeout (in ms)

        self._fsk_default_config.rx_modem = 0                 # FSK (0)
        self._fsk_default_config.rx_channel = 433000000       # RX frequency (433 MHz)
        self._fsk_default_config.rx_bandwidth = 20000         # RX bandwidth (20 kHz)
        self._fsk_default_config.rx_data_rate = 1200          # FSK data rate (1200 bps)
        self._fsk_default_config.rx_code_rate = 0             # N/A for FSK, set to 0
        self._fsk_default_config.rx_preamble_len = 8          # RX preamble length (8 bytes)
        self._fsk_default_config.rx_fixed_len = False         # Variable length packets for RX
        self._fsk_default_config.rx_crc_on = True             # Enable CRC for RX
        self._fsk_default_config.rx_iq_inverted = False       # N/A for FSK
        self._fsk_default_config.rx_payload_len = 255         # Maximum RX payload length (255 bytes)

        # Default configuration for LoRa modulation
        self._lora_default_config = RadioConfig()
        self._lora_default_config.tx_modem = 1                # LoRa
        self._lora_default_config.tx_channel = 868000000      # Example frequency for LoRa (868 MHz)
        self._lora_default_config.tx_bandwidth = 0            # 125 kHz bandwidth
        self._lora_default_config.tx_data_rate = 7            # Spreading factor 7 (128 chips)
        self._lora_default_config.tx_code_rate = 1            # Coding rate 4/5
        self._lora_default_config.tx_preamble_len = 8         # Preamble length in symbols
        self._lora_default_config.tx_fixed_len = False        # Fixed length for LoRa
        self._lora_default_config.tx_crc_on = True            # CRC enabled
        self._lora_default_config.tx_iq_inverted = False      # Normal IQ signals for LoRa
        self._lora_default_config.tx_power = 14               # Example TX power (14 dBm)
        self._lora_default_config.tx_fdev = 0                 # N/A for LoRa, set to 0
        self._lora_default_config.tx_timeout = 5000           # TX timeout (in ms)

        self._lora_default_config.rx_modem = 1                # LoRa
        self._lora_default_config.rx_channel = 868000000      # Example frequency for LoRa (868 MHz)
        self._lora_default_config.rx_bandwidth = 0            # 125 kHz bandwidth
        self._lora_default_config.rx_data_rate = 7            # Spreading factor 7 (128 chips)
        self._lora_default_config.rx_code_rate = 1            # Coding rate 4/5
        self._lora_default_config.rx_preamble_len = 8         # Preamble length in symbols
        self._lora_default_config.rx_fixed_len = False        # Fixed length for LoRa
        self._lora_default_config.rx_crc_on = True            # CRC enabled
        self._lora_default_config.rx_iq_inverted = False      # Normal IQ signals for LoRa
        self._lora_default_config.rx_payload_len = 64         # Example payload length (64 bytes)

        # Set default config
        self._popup_config.set_config(self._fsk_default_config)
        self._current_config = self._popup_config.get_config()

        # Schedule self._after_init_callback
        Clock.schedule_once(self._after_init_callback)

    def _after_init_callback(self, *largs):
        self._popup_device_selection.open()
    
    def _check_received_data_callback(self, *largs):
        data = self._rf_dongle.receive()

        if (data != None):
            converted_data = data.decode('ascii')
            message = f"[color=00BFFF]RX: [/color][color=00ff00]{converted_data}[/color]"
            self.ids.ScrollableLogWindow.add_message(message)

    def _apply_selec_button_callback(self):
        device_selected = self._popup_device_selection.get_selected_device()
        self._popup_device_selection.dismiss()

        if device_selected == None:
            raise RuntimeError("STM32WLE5_RF_Dongle not found")

        self._rf_dongle = RFDongle(device_selected)
        self._rf_dongle.set_config(self._current_config)
        Clock.schedule_interval(self._check_received_data_callback, 1)
        
    def _cancel_selec_button_callback(self):
        self._popup_device_selection.dismiss()
        raise RuntimeError("STM32WLE5_RF_Dongle not found")

    def _set_config_button_callback(self):
        self._popup_config.open()
    
    def _apply_conf_button_callback(self):
        self._current_config = self._popup_config.get_config()
        self._rf_dongle.set_config(self._current_config)
        self._popup_config.dismiss()

    def _cancel_conf_button_callback(self):
        self._popup_config.set_config(self._current_config)
        self._popup_config.dismiss()

    def _set_default_FSK_config_button_callback(self):
        self._popup_config.set_config(self._fsk_default_config)
        self._current_config = self._popup_config.get_config()
        self._rf_dongle.set_config(self._current_config)
        
    def _set_default_LoRa_config_button_callback(self):
        self._popup_config.set_config(self._lora_default_config)
        self._current_config = self._popup_config.get_config()
        self._rf_dongle.set_config(self._current_config)

    def _send_button_callback(self):
        data = self.ids.message_input.text

        if (data != ''):
            message = "[color=FFA500]TX: [/color][color=00ff00]" + data + "[/color]"
            self.ids.ScrollableLogWindow.add_message(message)
            self._rf_dongle.send(data)
            self.ids.message_input.text = ''
        