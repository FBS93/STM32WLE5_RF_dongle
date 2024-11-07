import kivy
kivy.require('2.3.0')

from kivy.app import App
from kivy.lang import Builder

from rf_console.rf_console import RFConsole

# Load kv file
kv = Builder.load_file('rf_console/rf_console.kv')

class MainApp(App):
    def build(self):
        return kv

if __name__ == '__main__':
    MainApp().run()
