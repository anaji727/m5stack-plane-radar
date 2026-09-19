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
  tft.setRotation(1); // M5Stack panel offset makes rotation 1 landscape 320x240.
  tft.setBrightness(80);
  tft.setTextWrap(false);
  displayFontInit();
}
