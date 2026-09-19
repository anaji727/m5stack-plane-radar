#pragma once
#include <Arduino.h>
void wifiSetupBegin();
void wifiLoop();
void wifiOpenSetup();
bool wifiPortalActive();
bool radarConfigured();
const String& aircraftUrl();
const char* settingsError();
