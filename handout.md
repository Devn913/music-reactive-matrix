# Music-Reactive LED Matrix: Complete Handout

## Table of Contents
1. [Project Overview](#project-overview)
2. [Hardware Setup](#hardware-setup)
3. [Arduino Configuration](#arduino-configuration)
4. [Mac Configuration](#mac-configuration)
5. [Running the System](#running-the-system)
6. [Troubleshooting](#troubleshooting)
7. [Customization](#customization)

---

## Project Overview

### What You're Building

A music-reactive 8×8 LED matrix display that:
- Captures audio from your Mac (Spotify, YouTube, etc.)
- Analyzes audio frequency spectrum
- Displays real-time visualization on the LED matrix
- Shows bass on the left, treble on the right

### How It Works

```
Your Mac Audio (Spotify/YouTube)
           ↓
Python Script (FFT Analysis)
           ↓
8 Frequency Bands (0-255 each)
           ↓
USB Serial (115200 baud)
           ↓
ESP32 Microcontroller
           ↓
MAX7219 LED Matrix Driver
           ↓
8×8 LED Matrix Display
```

### System Architecture

```
┌─────────────────────────────────────────────────┐
│ Your Mac (macOS)                                │
│ ┌──────────────────────────────────────────┐   │
│ │ Spotify / YouTube (Audio Source)         │   │
│ └────────────┬─────────────────────────────┘   │
│              │ System Audio                     │
│ ┌────────────▼─────────────────────────────┐   │
│ │ audio_to_leds.py (Python Script)         │   │
│ │ - Captures audio via sounddevice         │   │
│ │ - Performs FFT analysis                  │   │
│ │ - Generates 8 frequency bands (0-255)    │   │
│ │ - Sends via USB Serial (115200 baud)     │   │
│ └────────────┬─────────────────────────────┘   │
└─────────────┼──────────────────────────────────┘
              │ USB Serial
┌─────────────▼──────────────────────────────────┐
│ ESP32-LilyGO T-Display                          │
│ ┌──────────────────────────────────────────┐   │
│ │ Arduino Firmware (esp32_max7219_matrix)  │   │
│ │ - Receives serial packets                │   │
│ │ - Parses frequency band values           │   │
│ │ - Applies smoothing                      │   │
│ │ - Outputs to MAX7219 via SPI             │   │
│ └────────────┬─────────────────────────────┘   │
│              │ SPI (GPIO 13, 14, 15)           │
│              │ Power (5V, GND)                 │
└──────────────┼──────────────────────────────────┘
               │
┌──────────────▼──────────────────────────────────┐
│ MAX7219 8×8 LED Matrix Driver                   │
│ ┌──────────────────────────────────────────┐   │
│ │ LED Matrix (8 columns × 8 rows)          │   │
│ │ Column = Frequency Band                  │   │
│ │ Height = Magnitude (0-8 LEDs)            │   │
│ └──────────────────────────────────────────┘   │
└──────────────────────────────────────────────────┘
```

---

## Hardware Setup

### Components Required

| Item | Quantity | Notes |
|------|----------|-------|
| ESP32-LilyGO T-Display | 1 | Microcontroller with built-in screen |
| MAX7219 8×8 LED Matrix Module | 1 | LED driver + display |
| USB Cable (USB-A to Micro-USB) | 1 | For power and serial communication |
| Jumper Wires | 7 | Male-to-female, ~15cm each |
| Breadboard (optional) | 1 | For organizing connections |

### Wire Colors Used

- **RED** = Power (5V)
- **BLACK** = Ground (GND)
- **YELLOW** = Data (DIN/GPIO 13)
- **GREEN** = Clock (CLK/GPIO 14)
- **BLUE** = Chip Select (CS/GPIO 15)

### Pin Connections

| LED Matrix | Wire | ESP32 Pin | Pin Type |
|-----------|------|-----------|----------|
| VCC | RED | 5V | Power |
| GND | BLACK | GND | Ground |
| DIN | YELLOW | GPIO 13 | Data |
| CLK | GREEN | GPIO 14 | Clock |
| CS | BLUE | GPIO 15 | Chip Select |

### Wiring Steps

1. **Power Connection (RED wire)**
   - Plug RED wire into LED Matrix VCC pin
   - Plug other end into ESP32 5V pin (top-right corner)

2. **Ground Connection (BLACK wire)**
   - Plug BLACK wire into LED Matrix GND pin
   - Plug other end into ESP32 GND pin (right side)

3. **Data Signal (YELLOW wire)**
   - Plug YELLOW wire into LED Matrix DIN pin
   - Plug other end into ESP32 GPIO 13 (right side, middle)

4. **Clock Signal (GREEN wire)**
   - Plug GREEN wire into LED Matrix CLK pin
   - Plug other end into ESP32 GPIO 14 (right side, next to GPIO 13)

5. **Chip Select (BLUE wire)**
   - Plug BLUE wire into LED Matrix CS pin
   - Plug other end into ESP32 GPIO 15 (LEFT side, top)

### Critical Notes

⚠️ **IMPORTANT:**
- RED wire MUST go to 5V, NOT 3.3V
- GREEN wire MUST be on GPIO 14, NOT GPIO 17
- All wires should be firmly seated
- No wires should be touching each other

---

## Arduino Configuration

### Board Information

- **Board:** LilyGO T-Display (ESP32-based)
- **Chip:** ESP32-D0WD-V3
- **Board Selection in Arduino IDE:** ESP32

### Arduino IDE Settings

```
Tools → Board → ESP32 → ESP32
Tools → Upload Speed → 115200
Tools → Port → /dev/cu.usbserial-XXXXX
Tools → Flash Mode → DIO
Tools → Flash Size → 4MB
Tools → Flash Frequency → 80MHz
Tools → Erase All Flash Before Sketch Upload → Enabled
```

### Required Libraries

Install via Arduino IDE: **Sketch → Include Library → Manage Libraries**

1. **MD_MAX72XX** by majicDesigns
   - Search for: "MD_MAX72XX"
   - Version: Latest (1.4.2+)
   - Used for: LED matrix control

2. **MD_Parola** by majicDesigns
   - Search for: "MD_Parola"
   - Version: Latest
   - Used for: Text/pattern display support

### Uploading Code

#### Method 1: Automatic (if RTS pin works)
1. Select board and port in Arduino IDE
2. Click Upload button
3. Wait for "Done uploading"

#### Method 2: Manual BOOT Mode (if RTS pin broken)

**Your board uses this method:**

1. Enable: **Tools → Erase All Flash Before Sketch Upload → Enabled**
2. **Hold down the BOOT button** (left button on right side of board)
3. Click **Upload** in Arduino IDE
4. Wait for **"Connecting...."** message
5. **Release BOOT button**
6. Wait 60 seconds for upload to complete

### Code Structure

```cpp
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

// Pin Configuration
#define CLK_PIN 14        // Clock signal
#define CS_PIN 15         // Chip select
#define DIN_PIN 13        // Data input
#define MAX_DEVICES 1     // One 8x8 matrix

// Serial Configuration
#define BAUD_RATE 115200
#define NUM_BANDS 8       // 8 frequency bands

// Smoothing Factor
float smoothing = 0.6;    // 0.0 = no smoothing, 1.0 = full smoothing

// Serial Packet Format
// Byte 0: 255 (start marker)
// Bytes 1-8: Band values (0-255 each)
// Byte 9: 254 (end marker)
```

### Serial Communication Protocol

**Packet Structure:**

```
[START] [BAND1] [BAND2] [BAND3] [BAND4] [BAND5] [BAND6] [BAND7] [BAND8] [END]
  255     0-255   0-255   0-255   0-255   0-255   0-255   0-255   0-255   254
```

**Timing:**
- Baud Rate: 115200
- Packet Size: 10 bytes
- Update Rate: ~50ms (20 Hz)
- Data Rate: ~1920 bytes/sec

### Verification

After upload, open **Serial Monitor** (Tools → Serial Monitor):

```
Set Baud Rate: 115200

Expected Output:
MAX7219 LED Matrix Ready!
Waiting for audio data...
```

---

## Mac Configuration

### System Requirements

- **OS:** macOS 10.15 or later
- **Python:** Python 3.6+
- **USB Port:** Available USB-A port

### Installation

#### Step 1: Open Terminal

```bash
Cmd + Space → type "terminal" → Enter
```

#### Step 2: Create Project Directory

```bash
mkdir -p ~/music-led-matrix
cd ~/music-led-matrix
```

#### Step 3: Create Python Virtual Environment

```bash
python3 -m venv venv
source venv/bin/activate
```

You should see `(venv)` in your Terminal prompt.

#### Step 4: Upgrade pip and Install Dependencies

```bash
pip install --upgrade pip
pip install pyserial numpy scipy sounddevice
```

**Package Descriptions:**

| Package | Purpose |
|---------|---------|
| pyserial | USB serial communication with ESP32 |
| numpy | Numerical computing, audio processing |
| scipy | Signal processing (FFT, windowing) |
| sounddevice | Real-time audio capture from Mac |

#### Step 5: Create Python Script

Save the following as `~/music-led-matrix/audio_to_leds.py`:

```python
import serial
import numpy as np
from scipy import signal
import sounddevice as sd
import glob
import time

class AudioProcessor:
    def __init__(self, port=None, sample_rate=44100, fft_size=512):
        self.sample_rate = sample_rate
        self.fft_size = fft_size
        self.num_bands = 8
        
        # Auto-detect ESP32 port
        if port is None:
            ports = glob.glob('/dev/tty.usbserial*')
            if ports:
                port = ports[0]
            else:
                raise Exception("Could not find ESP32. Make sure it's plugged in!")
        
        # Connect to ESP32
        try:
            self.serial = serial.Serial(port, 115200, timeout=1)
            print(f"Connected to ESP32 on {port}")
        except Exception as e:
            raise Exception(f"Could not connect to ESP32 on {port}: {e}")
        
        # Define frequency band edges (Hz)
        self.freq_edges = [0, 100, 200, 400, 800, 1600, 3200, 6400, 22050]
        time.sleep(0.5)
    
    def get_frequency_bands(self, audio_chunk):
        """Extract 8 frequency bands from audio using FFT"""
        # Apply Hann window to reduce spectral leakage
        windowed = audio_chunk * signal.windows.hann(len(audio_chunk))
        
        # Compute FFT
        fft = np.fft.rfft(windowed)
        magnitudes = np.abs(fft)
        frequencies = np.fft.rfftfreq(len(windowed), 1/self.sample_rate)
        
        # Extract band values
        band_values = []
        for i in range(self.num_bands):
            # Find frequencies in this band
            mask = (frequencies >= self.freq_edges[i]) & (frequencies < self.freq_edges[i+1])
            
            if np.any(mask):
                band_magnitude = np.mean(magnitudes[mask])
            else:
                band_magnitude = 0
            
            # Apply logarithmic scaling
            if band_magnitude > 0:
                log_val = 20 * np.log10(band_magnitude)
                normalized = np.clip(log_val / 80, 0, 1)
            else:
                normalized = 0
            
            # Map to 0-255
            value = int(normalized * 255)
            band_values.append(max(0, min(255, value)))
        
        return band_values
    
    def send_to_esp32(self, band_values):
        """Send frequency bands to ESP32 via serial"""
        packet = bytearray([255])  # Start marker
        packet.extend(band_values)
        packet.append(254)  # End marker
        
        try:
            self.serial.write(packet)
        except Exception as e:
            print(f"Error sending to ESP32: {e}")
    
    def process_audio(self, audio_chunk):
        """Process audio chunk and send to ESP32"""
        if len(audio_chunk) >= self.fft_size:
            band_values = self.get_frequency_bands(audio_chunk[:self.fft_size])
            self.send_to_esp32(band_values)
    
    def start(self):
        """Start audio capture loop"""
        print(f"Sample rate: {self.sample_rate} Hz")
        print(f"FFT size: {self.fft_size}")
        print(f"Number of bands: {self.num_bands}")
        print("\nStarting audio capture...")
        print("Playing music on your Mac will sync with the LED matrix.")
        print("Press Ctrl+C to stop.\n")
        
        try:
            with sd.InputStream(channels=1, samplerate=self.sample_rate, 
                              blocksize=self.fft_size, callback=self.audio_callback):
                while True:
                    time.sleep(0.05)
        except KeyboardInterrupt:
            print("\n\nStopped.")
        finally:
            self.serial.close()
    
    def audio_callback(self, indata, frames, time_info, status):
        """Callback for audio stream"""
        if status:
            print(f"Audio status: {status}")
        
        audio_data = indata[:, 0]
        self.process_audio(audio_data)

if __name__ == "__main__":
    try:
        processor = AudioProcessor()
        processor.start()
    except Exception as e:
        print(f"Error: {e}")
```

---

## Running the System

### Quick Start

**Every time you want to use the system:**

```bash
cd ~/music-led-matrix
source venv/bin/activate
python3 audio_to_leds.py
```

### Expected Output

```
Connected to ESP32 on /dev/tty.usbserial-XXXXX
Sample rate: 44100 Hz
FFT size: 512
Number of bands: 8

Starting audio capture...
Playing music on your Mac will sync with the LED matrix.
Press Ctrl+C to stop.
```

### Testing

1. **Start the Python script** (see above)
2. **Open Spotify** on your Mac
3. **Play a song** (bass-heavy music works best)
4. **Observe LED matrix:**
   - Columns light up for each frequency band
   - Height shows magnitude (0-8 LEDs)
   - Left = Bass frequencies
   - Right = Treble frequencies

### Stopping

Press **Ctrl+C** in Terminal to stop the script.

---

## Troubleshooting

### Issue 1: "Could not find ESP32"

**Symptom:** Script shows error about finding ESP32

**Causes:**
- ESP32 not plugged into Mac
- USB cable not connected
- USB cable is data cable (not power-only cable)

**Solutions:**
1. Check USB cable is plugged into both Mac and ESP32
2. Try different USB port on Mac
3. Try different USB cable
4. In Terminal: `ls /dev/tty.usbserial*`
   - Should list a port
   - If nothing appears, USB not detected

### Issue 2: "Could not connect to ESP32"

**Symptom:** Script says USB port found but can't communicate

**Causes:**
- Wrong baud rate
- Serial Monitor open on Arduino IDE (blocks communication)
- ESP32 firmware issue

**Solutions:**
1. Close Serial Monitor in Arduino IDE
2. Wait 5 seconds
3. Run script again
4. Check Arduino IDE Serial Monitor shows "MAX7219 LED Matrix Ready!"

### Issue 3: LEDs Not Lighting Up

**Symptom:** Script runs but no LEDs light up

**Causes:**
- Wiring problem (RED on 3.3V, GREEN on GPIO 17, etc.)
- ESP32 firmware not uploaded
- Power not reaching LED matrix

**Solutions:**
1. Verify all 5 wires connected correctly
2. Check RED wire on 5V (NOT 3.3V)
3. Check GREEN wire on GPIO 14 (NOT GPIO 17)
4. Verify all wires pushed in firmly
5. Check Serial Monitor shows "MAX7219 LED Matrix Ready!"

### Issue 4: LEDs Light But Don't Sync with Music

**Symptom:** LEDs stay on or blink randomly, don't follow music

**Causes:**
- Audio not being captured from Mac
- System audio input set incorrectly
- Music volume too low

**Solutions:**
1. **System Preferences → Sound → Input:** Select "Built-in Microphone"
2. **Turn Mac volume ALL THE WAY UP**
3. **Play loud bass-heavy music** (not quiet/soft music)
4. Check Terminal shows no audio errors

### Issue 5: Pattern Looks Wrong

**Symptom:** LEDs light up but pattern seems odd

**Causes:**
- Different music = different pattern (this is normal!)
- Smoothing too high/low
- Audio levels need adjustment

**Solutions:**
- This is NORMAL behavior. Try different music.
- Bass-heavy songs (electronic, hip-hop) work better than acoustic
- Turn up volume for more responsive display

### Issue 6: Occasional Glitches or Stuttering

**Symptom:** LEDs flicker or skip beats

**Causes:**
- USB communication intermittent
- Loose wires
- Mac CPU overloaded

**Solutions:**
1. Check all wires are firmly connected
2. Close other applications on Mac
3. Try different USB port
4. Reduce other system load (close Spotify web player, etc.)

---

## Customization

### Adjusting Sensitivity

Edit `audio_to_leds.py` and find this line:

```python
normalized = np.clip(log_val / 80, 0, 1)
```

- **Increase 80 to 100+** = Less sensitive (need louder music)
- **Decrease 80 to 60** = More sensitive (responds to quiet music)

### Changing Frequency Bands

The script divides audio into 8 frequency bands:

```python
self.freq_edges = [0, 100, 200, 400, 800, 1600, 3200, 6400, 22050]
```

Current bands:
1. 0-100 Hz (ultra bass)
2. 100-200 Hz (bass)
3. 200-400 Hz (low-mid)
4. 400-800 Hz (mid)
5. 800-1600 Hz (mid-high)
6. 1600-3200 Hz (high)
7. 3200-6400 Hz (very high)
8. 6400-22050 Hz (ultra high)

To adjust, modify the numbers. For example, to emphasize bass:

```python
self.freq_edges = [0, 50, 100, 200, 400, 800, 1600, 3200, 22050]
```

### Changing Smoothing

In Arduino code (`esp32_max7219_matrix.ino`):

```cpp
float smoothing = 0.6;
```

- **0.0** = No smoothing (very jumpy)
- **0.5** = Medium smoothing
- **0.9** = Heavy smoothing (very smooth, lags behind music)

Try values between 0.5 and 0.8 for best results.

### Changing Update Rate

In Arduino code:

```cpp
const uint16_t UPDATE_INTERVAL = 50;  // milliseconds
```

- Smaller = Faster updates (20ms = 50 Hz)
- Larger = Slower updates (100ms = 10 Hz)

50ms is recommended for good response without flickering.

---

## Performance Specs

### Audio Processing

- **Sample Rate:** 44100 Hz
- **FFT Size:** 512 samples
- **Update Frequency:** 20 Hz (50ms)
- **Latency:** ~50-100ms (imperceptible)
- **Frequency Resolution:** ~86 Hz per bin

### Data Communication

- **Baud Rate:** 115200
- **Packet Size:** 10 bytes
- **Data Rate:** ~1.92 KB/sec
- **Error Checking:** Start/End markers

### Display

- **Resolution:** 8×8 LEDs
- **Colors:** On/Off (red, depends on LED matrix)
- **Brightness:** Fixed (determined by LED matrix specs)
- **Refresh Rate:** 20 Hz

---

## Tips & Tricks

### Best Music for Testing

- **Electronic/EDM** - Clear bass and treble separation
- **Hip-hop/Rap** - Strong bass response
- **Pop** - Balanced frequencies
- **Classical** - Good dynamic range
- **Avoid:** Quiet acoustic music, speech-heavy content

### Optimizing Display Response

1. **More responsive:** Increase volume, play bass-heavy music
2. **Smoother animation:** Increase smoothing factor (0.8-0.9)
3. **Faster response:** Decrease smoothing factor (0.3-0.5)
4. **Better bass detection:** Adjust freq_edges to emphasize low frequencies

### Troubleshooting Audio Issues

1. **System Preferences → Sound:**
   - Input: "Built-in Microphone"
   - Output: "Built-in Speaker" (or your preferred output)

2. **In Spotify/YouTube:**
   - Play from app (not web player)
   - Use internal audio (not external speakers if possible)
   - Maximize volume

3. **On Mac:**
   - Close unnecessary applications
   - Avoid VPN/proxy services
   - Disable Bluetooth if experiencing interference

---

## Project Timeline

### Total Time Required

- **Hardware Setup:** 30 minutes
- **Arduino Upload:** 15 minutes
- **Mac Configuration:** 10 minutes
- **Testing & Debugging:** 15-30 minutes
- **Total:** 1-2 hours

### Files & Locations

```
~/music-led-matrix/
├── venv/                          # Virtual environment
├── audio_to_leds.py              # Main Python script
└── [Arduino sketch uploaded to ESP32]
```

---

## Reference

### Key Commands

```bash
# Activate virtual environment
source ~/music-led-matrix/venv/bin/activate

# Run the main script
python3 ~/music-led-matrix/audio_to_leds.py

# Check available USB ports
ls /dev/tty.usbserial*

# Check Python installation
python3 --version

# List installed packages
pip list
```

### Important Files

- **Arduino Sketch:** `esp32_max7219_matrix.ino`
- **Python Script:** `audio_to_leds.py`
- **Libraries:** MD_MAX72XX, MD_Parola (Arduino)
- **Dependencies:** pyserial, numpy, scipy, sounddevice (Python)

### Hardware Specifications

- **ESP32-LilyGO T-Display:**
  - CPU: Dual-core 240MHz
  - RAM: 520 KB SRAM
  - Storage: 4 MB Flash
  - GPIO Pins: 34 (28 usable)
  - UART Ports: 3
  - SPI Ports: 4

- **MAX7219 LED Matrix:**
  - Display: 8×8 LED array
  - Interface: SPI 3-wire
  - Max Brightness: ~100mA
  - Operating Voltage: 5V

---

## Support & Debugging

### Common Issues Quick Reference

| Problem | Check | Solution |
|---------|-------|----------|
| ESP32 not found | USB connection | Try different port/cable |
| LEDs dark | Wiring | Verify all 5 wires, RED on 5V |
| No sync with music | Audio settings | System Preferences → Sound |
| Pattern odd | Normal behavior | Try different music/volume |
| Stuttering/glitches | Loose wires | Reseat all connections |

### Getting Help

1. **Check DEBUG_NOT_SYNCING.md** for detailed debugging
2. **Review wiring** using PIN_MAPPING_STEP_BY_STEP.md
3. **Verify Arduino upload** using LILIYGO_T_DISPLAY_SETUP.md
4. **Test audio** using test_audio.py script

---

## Conclusion

You now have a fully functional music-reactive LED matrix display! 

**Enjoy your project! 🎵✨**

---

**Last Updated:** June 2026
**Version:** 1.0
**Status:** Complete & Tested

