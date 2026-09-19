#include "services/wifi_setup.h"
#include <WiFi.h>
#include <WiFiManager.h>
#include <Preferences.h>
#include <ESPmDNS.h>
#include "config.h"
#include "services/radar_location.h"
#include "services/adsb_client.h"
#include "ui/radar_range.h"
namespace {
WiFiManager wm;
String url;
bool configured = false;
const char* error = "";
unsigned long retryAt = 0;
WiFiManagerParameter urlParam("dump_url", "dump1090 aircraft.json URL (http://)", "", 240);
WiFiManagerParameter latParam("radar_lat", "Receiver latitude (-90..90)", "", 20);
WiFiManagerParameter lonParam("radar_lon", "Receiver longitude (-180..180)", "", 20);
bool validUrl(const String& value) {
  if (!value.startsWith("http://") || value.length() < 9) return false;
  for (size_t i=0; i<value.length(); ++i)
    if (value[i] <= ' ' || value[i] == '"' || value[i] == '<' || value[i] == '>' || value[i] == '\'' || value[i] == '&') return false;
  int end = value.indexOf('/', 7);
  String host = value.substring(7, end < 0 ? value.length() : end);
  return host.length() > 0 && host.indexOf('@') < 0 && value.indexOf('#') < 0;
}
void save() {
  String candidate(urlParam.getValue()); candidate.trim();
  if (!validUrl(candidate)) { error = "BAD URL"; return; }
  if (!services::location::saveFromStrings(latParam.getValue(), lonParam.getValue())) {
    error = "BAD LAT/LON"; return;
  }
  url = candidate;
  Preferences prefs; prefs.begin("m5radar", false);
  prefs.putString("url", url); prefs.putBool("ready", true); prefs.end();
  configured = true; error = "";
  services::adsb::reset();
}
void defaults() {
  urlParam.setValue(url.c_str(), 240);
  char value[24];
  if (configured) {
    snprintf(value,sizeof(value),"%.6f",services::location::lat());latParam.setValue(value,20);
    snprintf(value,sizeof(value),"%.6f",services::location::lon());lonParam.setValue(value,20);
  }
}
}
void wifiOpenSetup() {
  if (wm.getConfigPortalActive()) return;
  wm.stopWebPortal(); defaults();
  wm.startConfigPortal(config::kPortalApName);
}
void wifiCloseSetup() {
  if (wm.getConfigPortalActive()) wm.stopConfigPortal();
}
void wifiSetupBegin() {
  Preferences prefs; prefs.begin("m5radar",true);
  url=prefs.getString("url","http://192.168.1.101/dump1090/data/aircraft.json"); configured=prefs.getBool("ready",false) && validUrl(url);prefs.end();
  defaults();
  wm.setHostname(config::kPortalHostname);
  wm.setConfigPortalBlocking(false);
  wm.setConfigPortalTimeout(0);
  wm.setConnectTimeout(12);
  wm.setSaveParamsCallback(save);
  wm.addParameter(&urlParam);wm.addParameter(&latParam);wm.addParameter(&lonParam);
  std::vector<const char*> menu={"wifi", "param", "info", "exit"};
  wm.setMenu(menu);
  WiFi.mode(WIFI_STA); WiFi.setSleep(false); WiFi.setAutoReconnect(true);
  // A receiver that boots before the home router must recover unattended.
  // Keep trying saved WiFi instead of getting stuck in the setup AP.
  if (wm.getWiFiIsSaved()) WiFi.begin();
  else wm.autoConnect(config::kPortalApName);
}
void wifiLoop() {
  wm.process();
  if (WiFi.status()==WL_CONNECTED) {
    if (!wm.getConfigPortalActive() && !wm.getWebPortalActive()) {
      defaults();wm.startWebPortal();
      MDNS.end();if(MDNS.begin(config::kPortalHostname)) MDNS.addService("http","tcp",80);
    }
  } else if (!wm.getConfigPortalActive() && millis()-retryAt>=15000) {
    retryAt=millis();WiFi.reconnect();
  }
}
bool wifiPortalActive() { return wm.getConfigPortalActive(); }
bool radarConfigured() { return configured; }
const String& aircraftUrl() { return url; }
const char* settingsError() { return error; }
