import kivy
kivy.require('2.3.0')

from kivy.uix.boxlayout import BoxLayout
from kivy.uix.label import Label
from kivy.core.window import Window

class ScrollableLogWindow(BoxLayout):

    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        Window.bind(on_resize=self._on_window_resize)
        self._max_lines = 100

    def _on_window_resize(self, window, width, height):
        log_container = self.ids.log_container

        for label in log_container.children:
            # Adjust text width on resize
            label.text_size = (self.width - 10, None)

    def set_max_lines(self, lines):
        self._max_lines = lines
        log_container = self.ids.log_container

        # Remove excess lines if the new max is smaller than current log size
        while len(log_container.children) > self._max_lines:
            log_container.remove_widget(log_container.children[-1])

    def add_message(self, message):
        log_container = self.ids.log_container
        log_scroll = self.ids.log_scroll

        new_label = Label(
            text=message,
            size_hint_y=None,  # Adjust height to content
            font_name='Roboto',
            font_size='14sp',
            markup=True, 
            halign='left',     
            valign='top',      
            padding=(5, 5),    
            text_size=(self.width - 10, None)  # Ensure text spans the full width
        )

        # Adjust the size of the label to its text content
        new_label.bind(texture_size=new_label.setter('size'))

        # Add the new message to the container
        log_container.add_widget(new_label)

        # Remove the oldest message if self._max_lines is exceeded
        if len(log_container.children) > self._max_lines:
            log_container.remove_widget(log_container.children[-1])

        # Scroll down if content height exceeds the ScrollView height
        if log_container.height > log_scroll.height:
            log_scroll.scroll_y = 0