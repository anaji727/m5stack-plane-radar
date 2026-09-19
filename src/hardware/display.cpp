#include "hardware/display.h"
#include "hardware/display_font.h"
M5GFX& tft = M5.Display;
void displayInit() {
  auto cfg = M5.config();
  cfg.internal_imu = false;
  cfg.internal_rtc = false;
  cfg.internal_spk = false;
  cfg.internal_mic = false;
  M5.begin(cfg);
  tft.setRotation(1); // Rotate clockwise 90 degrees: portrait 240x320.
  tft.setBrightness(80);
  tft.setTextWrap(false);
  displayFontInit();
}
