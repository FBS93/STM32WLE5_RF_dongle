import struct
import serial

START_OF_FRAME = 0xA5
SET_CONFIG_CMD = 0xF0
SEND_DATA_CMD = 0xF1
DATA_RECEIVED_CMD = 0xE0
MAX_PAYLOAD_SIZE = 255
MAX_FRAME_SIZE = 259

class RadioConfig:
    
    def __init__(self):
        # ------------------ TX config ------------------
        self.tx_modem = 0           # Radio modem to be used in TX [0: FSK, 1: LoRa]
        self.tx_channel = 0         # TX channel RF frequency (Frequency range: 150 MHz to 960 MHz)
        self.tx_bandwidth = 0       # Sets the TX bandwidth (LoRa only)
                                    # FSK: N/A (set to 0)
                                    # LoRa: [0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3: Reserved]
        self.tx_data_rate = 0       # Sets the TX data rate
                                    # FSK: 600..300000 bits/s
                                    # LoRa: [6: 64, 7: 128, 8: 256, 9: 512, 10: 1024, 11: 2048, 12: 4096 chips]
        self.tx_code_rate = 0       # Sets the TX coding rate (LoRa only)
                                    # FSK: N/A (set to 0)
                                    # LoRa: [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
        self.tx_preamble_len = 0    # Sets the TX preamble length
                                    # FSK: Number of bytes
                                    # LoRa: Length in symbols (the hardware adds 4 more symbols)
        self.tx_fixed_len = False   # TX fixed length packets [False: variable, True: fixed]
        self.tx_crc_on = False      # Enables/Disables the CRC on TX [False: OFF, True: ON]
        self.tx_iq_inverted = False # Inverts IQ signals on TX (LoRa only)
                                    # FSK: N/A (set to 0)
                                    # LoRa: [False: not inverted, True: inverted]
        self.tx_power = 0           # Sets the TX output power [from -9 dBm to +22 dBm]
        self.tx_fdev = 0            # Sets the TX frequency deviation (FSK only)
                                    # FSK: [Hz]
                                    # LoRa: N/A (set to 0)
        self.tx_timeout = 0         # TX timeout [ms]

        # ------------------ RX config ------------------
        self.rx_modem = 0           # Radio modem to be used in RX [0: FSK, 1: LoRa]
        self.rx_channel = 0         # RX channel RF frequency (Frequency range: 150 MHz to 960 MHz)
        self.rx_bandwidth = 0       # Sets the RX bandwidth
                                    # FSK: >= 2600 and <= 250000 Hz
                                    # LoRa: [0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3: Reserved]
        self.rx_data_rate = 0       # Sets the RX data rate
                                    # FSK: 600..300000 bits/s
                                    # LoRa: [6: 64, 7: 128, 8: 256, 9: 512, 10: 1024, 11: 2048, 12: 4096 chips]
        self.rx_code_rate = 0       # Sets the RX coding rate (LoRa only)
                                    # FSK: N/A (set to 0)
                                    # LoRa: [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
        self.rx_preamble_len = 0    # Sets the RX preamble length
                                    # FSK: Number of bytes
                                    # LoRa: Length in symbols (the hardware adds 4 more symbols)
        self.rx_fixed_len = False   # RX fixed length packets [False: variable, True: fixed]
        self.rx_crc_on = False      # Enables/Disables the CRC on RX [False: OFF, True: ON]
        self.rx_iq_inverted = False # Inverts IQ signals on RX (LoRa only)
                                    # FSK: N/A (set to 0)
                                    # LoRa: [False: not inverted, True: inverted]
        self.rx_payload_len = 0     # Sets RX payload length when fixed length is used


class RFDongle():

    def __init__(self, device):
        self._input_buffer = bytearray()
        self._device_connection = serial.Serial(device, baudrate=115200, timeout=0.1)
        
    def set_config(self, radio_config):          
        payload = struct.pack(
            '!B I I I B H ? ? ? b I I B I I I B H ? ? ? B',
            
            # ------------------ TX config ------------------
            radio_config.tx_modem,        # uint8_t -> B
            radio_config.tx_channel,      # uint32_t -> I
            radio_config.tx_bandwidth,    # uint32_t -> I
            radio_config.tx_data_rate,    # uint32_t -> I
            radio_config.tx_code_rate,    # uint8_t -> B
            radio_config.tx_preamble_len, # uint16_t -> H
            radio_config.tx_fixed_len,    # bool -> ?
            radio_config.tx_crc_on,       # bool -> ?
            radio_config.tx_iq_inverted,  # bool -> ?
            radio_config.tx_power,        # int8_t -> b
            radio_config.tx_fdev,         # uint32_t -> I
            radio_config.tx_timeout,      # uint32_t -> I

            # ------------------ RX config ------------------
            radio_config.rx_modem,        # uint8_t -> B
            radio_config.rx_channel,      # uint32_t -> I
            radio_config.rx_bandwidth,    # uint32_t -> I
            radio_config.rx_data_rate,    # uint32_t -> I
            radio_config.rx_code_rate,    # uint8_t -> B
            radio_config.rx_preamble_len, # uint16_t -> H
            radio_config.rx_fixed_len,    # bool -> ?
            radio_config.rx_crc_on,       # bool -> ?
            radio_config.rx_iq_inverted,  # bool -> ?
            radio_config.rx_payload_len   # uint8_t -> B
        )

        self._send_frame(SET_CONFIG_CMD, payload)

    def send(self, data):

        if not isinstance(data, bytes):
            data = data.encode('utf-8')

        self._send_frame(SEND_DATA_CMD, data)

    def _send_frame(self, command, payload):
        start_of_frame = START_OF_FRAME
        payload_len = len(payload)

        if payload_len > MAX_PAYLOAD_SIZE:
            print("Payload too large")
            return

        checksum = (start_of_frame + command + payload_len + sum(payload)) & 0xFF
        frame = struct.pack('BBB', start_of_frame, command, payload_len) + payload + struct.pack('B', checksum)

        if self._device_connection is None:
            raise Exception("Serial device not connected")
        
        self._device_connection.write(frame)

    def receive(self):
        new_data = self._receive_data()

        if new_data:
            self._input_buffer.extend(new_data)

        return self._try_decode_frame()
    
    def _try_decode_frame(self):
        """Start of frame = START_OF_FRAME | Command (1 Byte) |Payload length (1 Byte) | Payload | Checksum (1 Byte) |"""
        
        # We need at least 5 bytes: start of frame (1 byte), command (1 byte), payload length (1 byte), checksum (1 byte)
        if len(self._input_buffer) < 5:
            return None  # Not enough data to decode

        # Look for the start of frame byte (START_OF_FRAME)
        try:
            start_index = self._input_buffer.index(START_OF_FRAME)
        except ValueError:
            # Start of frame byte not found, clear the buffer
            self._input_buffer.clear()
            return None

        # Remove any bytes before the start of the frame
        if start_index > 0:
            self._input_buffer = self._input_buffer[start_index:]

        # Check if we have enough bytes to read the command and length
        if len(self._input_buffer) < 3:
            return None  # Still not enough bytes for the header

        # Read the command and payload length
        _, command, payload_len = struct.unpack('BBB', self._input_buffer[:3])

        # Check if there are enough bytes to read the full payload and checksum
        frame_size = 3 + payload_len + 1  # 3 bytes for header + payload + 1 byte for checksum
        if len(self._input_buffer) < frame_size:
            return None  # We still don't have the full frame

        # Extract the complete frame
        frame = self._input_buffer[:frame_size]
        self._input_buffer = self._input_buffer[frame_size:]  # Remove the frame from the buffer

        # Extract the payload and checksum
        payload = frame[3:3 + payload_len]
        checksum_received = frame[-1]

        # Verify the checksum
        checksum_calculated = (START_OF_FRAME + command + payload_len + sum(payload)) & 0xFF
        if checksum_received != checksum_calculated:
            return None
        
        if (command != DATA_RECEIVED_CMD):
            return None

        return payload

    def _receive_data(self):

        if self._device_connection is None:
            raise Exception("Serial device not connected")
        
        return self._device_connection.read(MAX_FRAME_SIZE)
