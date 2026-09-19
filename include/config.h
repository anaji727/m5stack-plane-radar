#pragma once
#include <cstdint>
namespace config {
constexpr char kPortalApName[] = "M5Radar-Setup";
constexpr char kPortalIp[] = "192.168.4.1";
constexpr char kPortalHostname[] = "m5radar";
constexpr char kPortalHostUrl[] = "m5radar.local";
constexpr int kDisplayWidth = 320, kDisplayHeight = 240;
constexpr bool kDisplayRgbOrder = false;
// Coordinates must be explicitly saved before aircraft are fetched.
constexpr double kDefaultRadarLat = 0, kDefaultRadarLon = 0;
constexpr unsigned long kAdsbFetchIntervalMs = 3000;
constexpr unsigned long kStaleMs = 15000;
constexpr float kMaxPositionAgeSec = 15;
constexpr bool kAdsbShowGroundAircraft = false;
constexpr uint16_t kColorBlack = 0, kColorYellow = 0xFFE0;
constexpr uint16_t kTextOnYellow = 0, kTextOnBlack = 0xFFFF;
}
