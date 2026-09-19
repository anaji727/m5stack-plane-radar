#pragma once
#include <ArduinoJson.h>
#include <cmath>
#include <cstdio>
#include <cstring>
#include "services/adsb_client.h"
namespace services::adsb {
inline bool number(JsonVariantConst v, float& out) {
  if (!v.is<double>()) return false;
  out=v.as<float>(); return std::isfinite(out);
}
inline void textField(JsonVariantConst value, char* out, size_t len) {
  if (!len) return;
  const char* s=value.is<const char*>() ? value.as<const char*>() : "";
  while (*s==' ') ++s;
  size_t n=0;
  while(s[n] && n+1<len) {out[n]=(s[n]>=32 && s[n]<=126)?s[n]:'?';++n;}
  while(n && out[n-1]==' ') --n;
  out[n]=0;
}
inline float distanceKm(float lat,float lon,double clat,double clon) {
  constexpr double r=0.0174532925199433;
  const double a=(lat-clat)*r, b=(lon-clon)*r;
  double h=sin(a/2)*sin(a/2)+cos(clat*r)*cos(lat*r)*sin(b/2)*sin(b/2);
  h=fmax(0.0,fmin(1.0,h));return 12742*asin(sqrt(h));
}
inline bool parseAircraft(JsonObjectConst obj,Aircraft& ac,bool showGround,float maxAge) {
  ac={};
  if (!number(obj["lat"],ac.lat)||!number(obj["lon"],ac.lon)||fabs(ac.lat)>90||fabs(ac.lon)>180) return false;
  if (!number(obj["seen_pos"],ac.position_age_sec)||ac.position_age_sec<0||ac.position_age_sec>maxAge) return false;
  auto alt=obj["alt_baro"].isNull()?obj["altitude"]:obj["alt_baro"];
  bool ground=alt.is<const char*>() && strcmp(alt.as<const char*>(),"ground")==0;
  if (ground && !showGround) return false;
  if (!number(obj["track"],ac.track_deg)) ac.track_deg=NAN;
  if (!number(obj["true_heading"],ac.nose_deg)) ac.nose_deg=ac.track_deg;
  if (!std::isfinite(ac.track_deg)) ac.track_deg=ac.nose_deg;
  if (!number(obj["gs"],ac.gs_knots) && !number(obj["speed"],ac.gs_knots)) ac.gs_knots=0;
  ac.gs_knots=fmaxf(0,fminf(2000,ac.gs_knots));
  textField(obj["flight"],ac.callsign,sizeof(ac.callsign));
  if(!ac.callsign[0]) textField(obj["hex"],ac.callsign,sizeof(ac.callsign));
  textField(obj["t"],ac.type,sizeof(ac.type));
  float feet;
  if(ground) snprintf(ac.alt,sizeof(ac.alt),"GND");
  else if(number(alt,feet)||number(obj["alt_geom"],feet)) {
    if (feet>=-2000 && feet<=100000) snprintf(ac.alt,sizeof(ac.alt),"%.0f ft",feet);
  }
  return true;
}
}
