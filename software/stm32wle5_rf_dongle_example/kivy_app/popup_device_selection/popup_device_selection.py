import kivy
kivy.require('2.3.0')

from kivy.uix.popup import Popup
from kivy.properties import ListProperty

class PopupDeviceSelection(Popup):
    devices = ListProperty([])
    
    def __init__(self, devices, **kwargs):
        super(PopupDeviceSelection, self).__init__(**kwargs)
        self._apply_button_listener = None
        self._cancel_button_listener = None
        self.devices = devices

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

    def get_selected_device(self):
        return self.ids.device_spinner.text if self.ids.device_spinner.text != 'Select Device' else None


