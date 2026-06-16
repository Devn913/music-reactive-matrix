# Music-Reactive LED Matrix Project

This project creates a real-time audio visualization on an 8x8 LED matrix using an ESP32 (LilyGO T-Display) and Python for audio processing.

## 📁 Repository Structure

- `/arduino`: Contains the `.ino` firmware for the ESP32.
- `/python`: Contains the `audio_to_leds.py` script and virtual environment setup.
- `handout.md`: Original project instructions and technical specs.

## 🔌 Hardware Configuration

### Pin Mapping (Current Setup)
| LED Matrix | ESP32 Pin | Wire Color |
|------------|-----------|------------|
| VCC        | 5V        | RED        |
| GND        | GND       | BLACK      |
| **DIN**    | **G13**   | YELLOW     |
| **CLK**    | **G17**   | GREEN      |
| **CS**     | **G15**   | BLUE       |

**Note:** If the matrix does not display data, consider moving CLK to **G14** as G17 can sometimes conflict with internal PSRAM on specific LilyGO boards.

## 🚀 Getting Started

### 1. Arduino Setup
1. Open `arduino/esp32_max7219_matrix.ino` in Arduino IDE.
2. Install library: `MD_MAX72XX`.
3. Board: `ESP32 Dev Module` (or LilyGO T-Display specific).
4. **Hold the BOOT button** on the ESP32 while clicking Upload. Release when "Connecting..." appears.

### 2. Python Setup (Mac)
1. Navigate to the project folder.
2. Create and activate virtual environment:
   ```bash
   python3 -m venv venv
   source venv/bin/activate
   pip install pyserial numpy scipy sounddevice
   ```

### 3. Running the Visualization
1. Activate venv: `source venv/bin/activate`
2. Run script: `python3 python/audio_to_leds.py`
3. Ensure your Mac volume is up. The script currently uses the **MacBook Pro Microphone** to capture audio from your speakers.

## ✨ Features
- **Falling Peaks:** Professional audio visualization effect.
- **Auto-Smoothing:** Prevents flickering for a clean look.
- **Frequency Analysis:** Uses Fast Fourier Transform (FFT) to split music into 8 distinct frequency bands.

## 🛠 Troubleshooting
- **No Output:** Check G17/G13/G15 wiring. Ensure RED is on 5V.
- **Serial Error:** Ensure the Arduino Serial Monitor is CLOSED before running the Python script.
- **Quiet Visualization:** Increase your Mac volume or get closer to the mic.
