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
bool setupDismissed=false;
uint8_t brightness=80;
void textStyle() {tft.setFont(&fonts::Font0);tft.setTextSize(1);tft.setTextDatum(textdatum_t::top_left);tft.setTextColor(TFT_WHITE,TFT_BLACK);}
void drawStatus() {
  tft.fillRect(0,240,240,80,TFT_BLACK);textStyle();
  const bool online=WiFi.status()==WL_CONNECTED;
  const bool stale=services::adsb::hasData() && services::adsb::ageMs()>config::kStaleMs;
  tft.setCursor(6,246);tft.print("M5 RADAR");
  tft.setTextColor(online?TFT_GREEN:TFT_ORANGE,TFT_BLACK);
  tft.setCursor(84,246);tft.print(online?"WiFi OK":"OFFLINE");
  tft.setTextColor(stale?TFT_ORANGE:TFT_WHITE,TFT_BLACK);
  tft.setCursor(162,246);tft.print(settingsError()[0]?settingsError():!radarConfigured()?"SETUP NEEDED":stale?"STALE":services::adsb::status());
  tft.setCursor(6,264);tft.printf("AC %u",static_cast<unsigned>(services::adsb::aircraftCount()));
  tft.setCursor(84,264);
  if(services::adsb::hasData()) tft.printf("Age %lus",static_cast<unsigned long>(services::adsb::ageMs()/1000));else tft.print("Age --");
  tft.setCursor(162,264);tft.printf("R %.1fkm",ui::radar::rangeCurrent().outer_km);
  tft.setCursor(6,284);tft.print("A: Range");
  tft.setCursor(84,284);tft.print("B: Light");
  tft.setCursor(162,284);tft.print("C: Setup");
  tft.setCursor(6,306);tft.print("m5radar.local");
}
void setupScreen() {
  tft.fillScreen(TFT_BLACK);textStyle();tft.setTextSize(2);
  tft.setCursor(10,12);tft.print("M5 Plane Radar");
  tft.setTextSize(1);
  tft.setCursor(10,52);tft.print("Connect WiFi: M5Radar-Setup");
  tft.setCursor(10,74);tft.print("Browser: http://192.168.4.1");
  if(WiFi.status()==WL_CONNECTED){tft.setCursor(10,96);tft.printf("LAN: http://%s",WiFi.localIP().toString().c_str());}
  tft.setCursor(10,124);tft.print("Save aircraft.json URL and");
  tft.setCursor(10,140);tft.print("receiver latitude / longitude.");
  tft.setCursor(10,170);tft.print("Use Configure WiFi / Setup.");
  tft.setCursor(10,204);tft.setTextColor(TFT_ORANGE,TFT_BLACK);tft.print(settingsError());
  tft.setCursor(10,250);tft.setTextColor(TFT_WHITE,TFT_BLACK);tft.print("Hold C (1 sec): Back to radar");
  tft.setCursor(10,266);tft.print("Save in browser before leaving.");
}
}
void setup() {
  Serial.begin(115200);displayInit();
  M5.BtnC.setHoldThresh(1000);
  services::location::init();ui::radar::rangeInit();
  setupScreen();wifiSetupBegin();
  // Network I/O pumps portal only; display and buttons stay on the main loop.
  services::adsb::setPollFn(wifiLoop);
}
void loop() {
  M5.update();wifiLoop();
  if(M5.BtnA.wasClicked()){ui::radar::rangeNext();services::adsb::reset();first=true;dirty=true;}
  if(M5.BtnB.wasClicked()){brightness=brightness==80?160:brightness==160?25:80;tft.setBrightness(brightness);}
  // Hold is distinct from click, so releasing C after exit cannot reopen setup.
  if(M5.BtnC.wasHold() && (setupShown || wifiPortalActive())) {
    wifiCloseSetup();setupDismissed=true;
  } else if(M5.BtnC.wasClicked()) {
    setupDismissed=false;wifiOpenSetup();
  }
  bool configuring=wifiPortalActive() || (!radarConfigured() && !setupDismissed);
  if(configuring) {
    if(millis()-drawnAt>=1000 || !setupShown){drawnAt=millis();setupScreen();}
    setupShown=true;delay(10);return;
  }
  if(setupShown){setupShown=false;dirty=true;first=true;tft.fillScreen(TFT_BLACK);}
  if(radarConfigured() && WiFi.status()==WL_CONNECTED && (first || millis()-fetchedAt>=config::kAdsbFetchIntervalMs)) {
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
