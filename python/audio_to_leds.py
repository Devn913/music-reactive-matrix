import serial
import serial.tools.list_ports
import numpy as np
from scipy import signal
import sounddevice as sd
import glob
import time
import sys

class AudioProcessor:
    def __init__(self, port=None, sample_rate=44100, fft_size=1024): # Increased FFT size for better resolution
        self.sample_rate = sample_rate
        self.fft_size = fft_size
        self.num_bands = 8
        
        # Auto-detect ESP32 port
        if port is None:
            ports = glob.glob('/dev/tty.usbserial*') + glob.glob('/dev/cu.usbserial*')
            if ports:
                port = ports[0]
            else:
                print("Error: Could not find ESP32.")
                sys.exit(1)
        
        try:
            self.serial = serial.Serial(port, 115200, timeout=1)
            print(f"Connected to ESP32 on {port}")
        except Exception as e:
            print(f"Error: {e}")
            sys.exit(1)
            
        # Try to find Microphone
        self.device_id = None
        devices = sd.query_devices()
        for i, dev in enumerate(devices):
            if "Microphone" in dev['name']:
                self.device_id = i
                print(f"Using Audio Device: {dev['name']} (ID: {i})")
                break
        
        if self.device_id is None:
            print("Using default input.")
            self.device_id = sd.default.device[0]

        # Freq edges tuned for "cool" response
        self.freq_edges = [0, 80, 200, 400, 800, 1600, 3200, 6400, 12000]
        
    def get_frequency_bands(self, audio_chunk):
        windowed = audio_chunk * signal.windows.hann(len(audio_chunk))
        fft = np.fft.rfft(windowed)
        magnitudes = np.abs(fft)
        frequencies = np.fft.rfftfreq(len(windowed), 1/self.sample_rate)
        
        band_values = []
        for i in range(self.num_bands):
            mask = (frequencies >= self.freq_edges[i]) & (frequencies < self.freq_edges[i+1])
            if np.any(mask):
                # Using max instead of mean for more "punchy" visual
                band_magnitude = np.max(magnitudes[mask])
            else:
                band_magnitude = 0
            
            if band_magnitude > 0:
                log_val = 20 * np.log10(band_magnitude)
                # Sensitive mapping
                normalized = np.clip((log_val + 50) / 50, 0, 1)
            else:
                normalized = 0
            
            band_values.append(int(normalized * 255))
        return band_values
    
    def send_to_esp32(self, band_values):
        # Packet: [START, B1, B2, B3, B4, B5, B6, B7, B8, END]
        packet = bytearray([255]) + bytearray(band_values) + bytearray([254])
        try:
            # Debug: print band values occasionally
            if time.time() % 1 < 0.05: # Print roughly once per second
                print(f"Bands: {band_values}")
            self.serial.write(packet)
            self.serial.flush()
        except Exception as e:
            print(f"Serial Error: {e}")
    
    def audio_callback(self, indata, frames, time_info, status):
        audio_data = indata[:, 0]
        if len(audio_data) >= self.fft_size:
            bands = self.get_frequency_bands(audio_data[:self.fft_size])
            self.send_to_esp32(bands)

    def start(self):
        print("\nStarting enhanced visualization...")
        print("Note: Set Mac Sound Output to 'BlackHole 2ch' to visualize music.")
        try:
            with sd.InputStream(device=self.device_id, channels=1, samplerate=self.sample_rate, 
                              blocksize=self.fft_size, callback=self.audio_callback):
                while True:
                    time.sleep(1)
        except KeyboardInterrupt:
            print("\nStopped.")
        finally:
            if hasattr(self, 'serial') and self.serial.is_open:
                self.serial.close()

if __name__ == "__main__":
    processor = AudioProcessor()
    processor.start()
