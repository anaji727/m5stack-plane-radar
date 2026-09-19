#pragma once
#include <cstddef>
#include <cstdint>
namespace services::adsb {
struct Aircraft {
  float lat,lon,nose_deg,track_deg,gs_knots,position_age_sec;
  char callsign[9],type[5],alt[12];
};
constexpr size_t kMaxAircraft=64;
size_t aircraftCount();
const Aircraft* aircraftList();
using PollFn=void(*)();
void setPollFn(PollFn fn);
bool fetchUpdate(double center_lat,double center_lon,float fetch_radius_km);
bool expire();
void reset();
bool hasData();
uint32_t ageMs();
const char* status();
}
