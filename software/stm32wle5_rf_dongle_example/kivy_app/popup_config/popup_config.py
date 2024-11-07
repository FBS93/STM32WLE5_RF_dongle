import kivy
kivy.require('2.3.0')

from kivy.uix.popup import Popup
from stm32wle5_rf_dongle.stm32wle5_rf_dongle import RadioConfig

class PopupConfig(Popup):
    
    def __init__(self, **kwargs):
        super(PopupConfig, self).__init__(**kwargs)

        self._apply_button_listener = None
        self._cancel_button_listener = None

    def set_listener_on_apply_button_event(self, listener):
        self._apply_button_listener = listener

    def _apply_button_callback(self):
        if self._apply_button_listener is not None:
            self._apply_button_listener()

    def set_listener_on_cancel_button_event(self, listener):
        self._cancel_button_listener = listener

    def _cancel_button_callback(self):
        if self._cancel_button_listener is not None:
            self._cancel_button_listener()
    
    def _get_bandwidth_value(self, bandwidth_text):
        bandwidth_map = {'125 kHz': 0, '250 kHz': 1, '500 kHz': 2}
        return bandwidth_map.get(bandwidth_text, 0)

    def _set_bandwidth_value(self, bandwidth_value):
        bandwidth_map = {0: '125 kHz', 1: '250 kHz', 2: '500 kHz'}
        return bandwidth_map.get(bandwidth_value, '125 kHz')

    def _get_code_rate_value(self, code_rate_text):
        code_rate_map = {'4/5': 1, '4/6': 2, '4/7': 3, '4/8': 4}
        return code_rate_map.get(code_rate_text, 1)

    def _set_code_rate_value(self, code_rate_value):
        code_rate_map = {1: '4/5', 2: '4/6', 3: '4/7', 4: '4/8'}
        return code_rate_map.get(code_rate_value, '4/5')

    def get_config(self):
        config = RadioConfig()

        config.tx_modem = 0 if self.ids.tx_modem.text == 'FSK' else 1
        config.tx_channel = int(self.ids.tx_channel.text)
        config.tx_bandwidth = self._get_bandwidth_value(self.ids.tx_bandwidth.text)
        config.tx_data_rate = int(self.ids.tx_data_rate.text)
        config.tx_code_rate = self._get_code_rate_value(self.ids.tx_code_rate.text)
        config.tx_preamble_len = int(self.ids.tx_preamble_len.text)
        config.tx_fixed_len = self.ids.tx_fixed_len.active
        config.tx_crc_on = self.ids.tx_crc_on.active
        config.tx_iq_inverted = self.ids.tx_iq_inverted.active
        config.tx_power = int(self.ids.tx_power.text)
        config.tx_fdev = int(self.ids.tx_fdev.text)
        config.tx_timeout = int(self.ids.tx_timeout.text)

        config.rx_modem = 0 if self.ids.rx_modem.text == 'FSK' else 1
        config.rx_channel = int(self.ids.rx_channel.text)
        if (self.ids.rx_modem.text == 'LoRa'):
            config.rx_bandwidth = self._get_bandwidth_value(self.ids.rx_bandwidth_lora.text)
        else: #FSK
            config.rx_bandwidth = int(self.ids.rx_bandwidth_fsk.text)
        config.rx_data_rate = int(self.ids.rx_data_rate.text)
        config.rx_code_rate = self._get_code_rate_value(self.ids.rx_code_rate.text)
        config.rx_preamble_len = int(self.ids.rx_preamble_len.text)
        config.rx_fixed_len = self.ids.rx_fixed_len.active
        config.rx_crc_on = self.ids.rx_crc_on.active
        config.rx_iq_inverted = self.ids.rx_iq_inverted.active
        config.rx_payload_len = int(self.ids.rx_payload_len.text)

        return config

    def set_config(self, config):
        self.ids.tx_modem.text = 'FSK' if config.tx_modem == 0 else 'LoRa'
        self.ids.tx_channel.text = str(config.tx_channel)
        self.ids.tx_bandwidth.text = self._set_bandwidth_value(config.tx_bandwidth)
        self.ids.tx_data_rate.text = str(config.tx_data_rate)
        self.ids.tx_code_rate.text = self._set_code_rate_value(config.tx_code_rate)
        self.ids.tx_preamble_len.text = str(config.tx_preamble_len)
        self.ids.tx_fixed_len.active = config.tx_fixed_len
        self.ids.tx_crc_on.active = config.tx_crc_on
        self.ids.tx_iq_inverted.active = config.tx_iq_inverted
        self.ids.tx_power.text = str(config.tx_power)
        self.ids.tx_fdev.text = str(config.tx_fdev)
        self.ids.tx_timeout.text = str(config.tx_timeout)

        self.ids.rx_modem.text = 'FSK' if config.rx_modem == 0 else 'LoRa'
        self.ids.rx_channel.text = str(config.rx_channel)
        if (self.ids.rx_modem.text == 'LoRa'):
            self.ids.rx_bandwidth_lora.text = self._set_bandwidth_value(config.rx_bandwidth)
        else: #FSK
            self.ids.rx_bandwidth_fsk.text = str(config.rx_bandwidth)
        self.ids.rx_data_rate.text = str(config.rx_data_rate)
        self.ids.rx_code_rate.text = self._set_code_rate_value(config.rx_code_rate)
        self.ids.rx_preamble_len.text = str(config.rx_preamble_len)
        self.ids.rx_fixed_len.active = config.rx_fixed_len
        self.ids.rx_crc_on.active = config.rx_crc_on
        self.ids.rx_iq_inverted.active = config.rx_iq_inverted
        self.ids.rx_payload_len.text = str(config.rx_payload_len)


