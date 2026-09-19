#pragma once
#include <Arduino.h>
void wifiSetupBegin();
void wifiLoop();
void wifiOpenSetup();
void wifiCloseSetup();
bool wifiPortalActive();
bool radarConfigured();
const String& aircraftUrl();
const char* settingsError();
