#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "hardware/display.h"
#include "services/adsb_client.h"
#include "services/radar_location.h"
#include "services/wifi_setup.h"
#include "ui/radar_display.h"
#include "ui/radar_range.h"
namespace {
uint32_t fetchedAt=0,drawnAt=0;
bool dirty=true,first=true;
bool setupShown=false;
uint8_t brightness=80;
void textStyle() {tft.setFont(&fonts::Font0);tft.setTextSize(1);tft.setTextDatum(textdatum_t::top_left);tft.setTextColor(TFT_WHITE,TFT_BLACK);}
void drawStatus() {
  tft.fillRect(240,0,80,240,TFT_BLACK);textStyle();
  const bool online=WiFi.status()==WL_CONNECTED;
  const bool stale=services::adsb::hasData() && services::adsb::ageMs()>config::kStaleMs;
  tft.setCursor(246,10);tft.print("M5 RADAR");
  tft.setTextColor(online?TFT_GREEN:TFT_ORANGE,TFT_BLACK);
  tft.setCursor(246,34);tft.print(online?"WiFi OK":"OFFLINE");
  tft.setTextColor(stale?TFT_ORANGE:TFT_WHITE,TFT_BLACK);
  tft.setCursor(246,52);tft.print(settingsError()[0]?settingsError():stale?"STALE":services::adsb::status());
  tft.setCursor(246,77);tft.printf("AC %u",static_cast<unsigned>(services::adsb::aircraftCount()));
  tft.setCursor(246,94);
  if(services::adsb::hasData()) tft.printf("Age %lus",static_cast<unsigned long>(services::adsb::ageMs()/1000));else tft.print("Age --");
  tft.setCursor(246,118);tft.print("Outer km");
  tft.setCursor(246,132);tft.printf("%.1f",ui::radar::rangeCurrent().outer_km);
  tft.setCursor(246,160);tft.print("A: Range");
  tft.setCursor(246,177);tft.print("B: Light");
  tft.setCursor(246,194);tft.print("C: Setup");
  tft.setCursor(246,220);tft.print("m5radar");
  tft.setCursor(246,230);tft.print(".local");
}
void setupScreen() {
  tft.fillScreen(TFT_BLACK);textStyle();tft.setTextSize(2);
  tft.setCursor(10,12);tft.print("M5 Plane Radar");
  tft.setTextSize(1);
  tft.setCursor(10,52);tft.print("Connect WiFi: M5Radar-Setup");
  tft.setCursor(10,74);tft.print("Browser: http://192.168.4.1");
  if(WiFi.status()==WL_CONNECTED){tft.setCursor(10,96);tft.printf("LAN: http://%s",WiFi.localIP().toString().c_str());}
  tft.setCursor(10,124);tft.print("Save aircraft.json URL + receiver lat/lon");
  tft.setCursor(10,146);tft.print("Use Setup > WiFi / Setup > Parameters");
  tft.setCursor(10,176);tft.setTextColor(TFT_ORANGE,TFT_BLACK);tft.print(settingsError());
  tft.setCursor(10,210);tft.setTextColor(TFT_WHITE,TFT_BLACK);tft.print("After saving, use Exit to close setup.");
}
}
void setup() {
  Serial.begin(115200);displayInit();
  services::location::init();ui::radar::rangeInit();
  setupScreen();wifiSetupBegin();
  // Network I/O pumps portal only; display and buttons stay on the main loop.
  services::adsb::setPollFn(wifiLoop);
}
void loop() {
  M5.update();wifiLoop();
  if(M5.BtnA.wasClicked()){ui::radar::rangeNext();services::adsb::reset();first=true;dirty=true;}
  if(M5.BtnB.wasClicked()){brightness=brightness==80?160:brightness==160?25:80;tft.setBrightness(brightness);}
  if(M5.BtnC.wasClicked()) wifiOpenSetup();
  bool configuring=wifiPortalActive() || !radarConfigured();
  if(configuring) {
    if(millis()-drawnAt>=1000 || !setupShown){drawnAt=millis();setupScreen();}
    setupShown=true;delay(10);return;
  }
  if(setupShown){setupShown=false;dirty=true;first=true;tft.fillScreen(TFT_BLACK);}
  if(WiFi.status()==WL_CONNECTED && (first || millis()-fetchedAt>=config::kAdsbFetchIntervalMs)) {
    first=false;
    dirty=services::adsb::fetchUpdate(services::location::lat(),services::location::lon(),ui::radar::fetchRadiusKm()) || dirty;
    fetchedAt=millis();
  }
  dirty=services::adsb::expire() || dirty;
  if(dirty || millis()-drawnAt>=1000) {
    if(dirty) ui::radarDisplayDraw();
    drawStatus();dirty=false;drawnAt=millis();
  }
  delay(10);
}
