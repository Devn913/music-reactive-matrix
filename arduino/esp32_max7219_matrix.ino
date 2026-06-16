#include <MD_MAX72xx.h>
#include <SPI.h>

// Pin Configuration
#define DIN_PIN 13
#define CLK_PIN 17
#define CS_PIN 15

#define MAX_DEVICES 1
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW

// Serial Configuration
#define BAUD_RATE 115200
#define NUM_BANDS 8
#define START_MARKER 255
#define END_MARKER 254

MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, DIN_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

// Data arrays
float smoothing = 0.4;
float smoothed_bands[NUM_BANDS] = {0};
uint8_t peak_heights[NUM_BANDS] = {0};
unsigned long last_peak_drop = 0;
const int PEAK_DELAY = 80;

// Pattern management
uint8_t currentPattern = 0;
unsigned long lastPatternSwitch = 0;
const unsigned long PATTERN_DURATION = 15000; // Switch every 15 seconds

// Circular pattern variables
float angle = 0;

void setup() {
  Serial.begin(BAUD_RATE);
  mx.begin();
  mx.control(MD_MAX72XX::INTENSITY, 2);
  mx.clear();
  
  // Quick startup: Vortex
  for (int r = 0; r < 4; r++) {
    for (int i = 0; i < 8; i++) {
       mx.setPoint(r, i, true);
       mx.setPoint(7-r, i, true);
       mx.setPoint(i, r, true);
       mx.setPoint(i, 7-r, true);
       delay(20);
    }
  }
  mx.clear();
}

void loop() {
  if (Serial.available() >= 10) {
    if (Serial.read() == START_MARKER) {
      uint8_t raw_bands[NUM_BANDS];
      for (int i = 0; i < NUM_BANDS; i++) raw_bands[i] = Serial.read();
      if (Serial.read() == END_MARKER) {
        processBands(raw_bands);
      }
    }
  }

  // Auto-switch patterns
  if (millis() - lastPatternSwitch > PATTERN_DURATION) {
    currentPattern = (currentPattern + 1) % 3;
    lastPatternSwitch = millis();
    mx.clear();
  }

  // Universal peak animation
  if (millis() - last_peak_drop > PEAK_DELAY) {
    for (int i = 0; i < NUM_BANDS; i++) {
      if (peak_heights[i] > 0) peak_heights[i]--;
    }
    last_peak_drop = millis();
    drawCurrentPattern();
  }
}

void processBands(uint8_t* raw_bands) {
  for (int i = 0; i < NUM_BANDS; i++) {
    smoothed_bands[i] = (raw_bands[i] * (1.0 - smoothing)) + (smoothed_bands[i] * smoothing);
    int height = map(smoothed_bands[i], 0, 255, 0, 8);
    if (height > peak_heights[i]) peak_heights[i] = height;
  }
  drawCurrentPattern();
}

void drawCurrentPattern() {
  mx.clear();
  switch (currentPattern) {
    case 0: drawFallingPeaks(); break;
    case 1: drawOrbitalScan(); break;
    case 2: drawBeatPulse(); break;
  }
}

// PATTERN 0: Standard bars with falling peaks (Classic)
void drawFallingPeaks() {
  for (int x = 0; x < 8; x++) {
    int height = map(smoothed_bands[x], 0, 255, 0, 8);
    for (int y = 0; y < height; y++) mx.setPoint(7-y, x, true);
    if (peak_heights[x] > 0) mx.setPoint(8 - peak_heights[x], x, true);
  }
}

// PATTERN 1: Orbital Scan (A line that rotates, its length depends on volume)
void drawOrbitalScan() {
  float avg_vol = 0;
  for(int i=0; i<8; i++) avg_vol += smoothed_bands[i];
  avg_vol /= 8;
  
  int radius = map(avg_vol, 0, 255, 1, 5);
  angle += 0.2; // Speed of rotation
  
  int centerX = 3, centerY = 4;
  for (int r = 0; r < radius; r++) {
    int x = centerX + r * cos(angle);
    int y = centerY + r * sin(angle);
    if (x >= 0 && x < 8 && y >= 0 && y < 8) mx.setPoint(y, x, true);
    
    // Symmetric point
    int x2 = centerX - r * cos(angle);
    int y2 = centerY - r * sin(angle);
    if (x2 >= 0 && x2 < 8 && y2 >= 0 && y2 < 8) mx.setPoint(y2, x2, true);
  }
}

// PATTERN 2: Beat Pulse (Expanding/Contracting Square from center)
void drawBeatPulse() {
  // Use Bass band (Band 0-1) for the main pulse
  float bass = (smoothed_bands[0] + smoothed_bands[1]) / 2;
  int size = map(bass, 0, 255, 0, 4);
  
  for (int s = 0; s <= size; s++) {
    // Draw hollow square frames
    for (int i = 3 - s; i <= 4 + s; i++) {
      mx.setPoint(3 - s, i, true);
      mx.setPoint(4 + s, i, true);
      mx.setPoint(i, 3 - s, true);
      mx.setPoint(i, 4 + s, true);
    }
  }
  
  // Use Treble (Bands 6-7) for "Sparkles" on edges
  float treble = (smoothed_bands[6] + smoothed_bands[7]) / 2;
  if (treble > 150) {
    mx.setPoint(0, 0, true); mx.setPoint(0, 7, true);
    mx.setPoint(7, 0, true); mx.setPoint(7, 7, true);
  }
}
