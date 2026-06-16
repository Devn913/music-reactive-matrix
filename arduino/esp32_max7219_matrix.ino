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

// Display settings
MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, DIN_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

// Data arrays
float smoothing = 0.5;
float smoothed_bands[NUM_BANDS] = {0};
uint8_t peak_heights[NUM_BANDS] = {0};
unsigned long last_peak_drop = 0;
const int PEAK_DELAY = 100; // Peak drop speed (ms)

void setup() {
  Serial.begin(BAUD_RATE);
  mx.begin();
  mx.control(MD_MAX72XX::INTENSITY, 2);
  mx.clear();
  
  // Startup animation: Scanning line
  for (int col=0; col<8; col++) {
    mx.setColumn(col, 0xFF);
    delay(50);
    mx.setColumn(col, 0x00);
  }
}

void loop() {
  // 1. Process Serial Data
  if (Serial.available() >= 10) {
    if (Serial.read() == START_MARKER) {
      uint8_t raw_bands[NUM_BANDS];
      for (int i = 0; i < NUM_BANDS; i++) {
        raw_bands[i] = Serial.read();
      }
      if (Serial.read() == END_MARKER) {
        processBands(raw_bands);
      }
    }
  }

  // 2. Animate Peaks (drop them over time)
  if (millis() - last_peak_drop > PEAK_DELAY) {
    for (int i = 0; i < NUM_BANDS; i++) {
      if (peak_heights[i] > 0) peak_heights[i]--;
    }
    last_peak_drop = millis();
    drawDisplay();
  }
}

void processBands(uint8_t* raw_bands) {
  for (int i = 0; i < NUM_BANDS; i++) {
    // Apply smoothing
    smoothed_bands[i] = (raw_bands[i] * (1.0 - smoothing)) + (smoothed_bands[i] * smoothing);
    
    // Map to 0-8
    int height = map(smoothed_bands[i], 0, 255, 0, 8);
    
    // Update peaks
    if (height > peak_heights[i]) {
      peak_heights[i] = height;
    }
  }
  drawDisplay();
}

void drawDisplay() {
  mx.clear();
  for (int x = 0; x < NUM_BANDS; x++) {
    int height = map(smoothed_bands[x], 0, 255, 0, 8);
    
    // Pattern 1: Solid bars with falling peaks
    for (int y = 0; y < height; y++) {
      mx.setPoint(7-y, x, true);
    }
    
    // Draw Peak (individual LED at the top)
    if (peak_heights[x] > 0) {
      mx.setPoint(8 - peak_heights[x], x, true);
    }
  }
}
