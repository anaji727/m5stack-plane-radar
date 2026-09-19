#include "services/aircraft_parser.h"
#include "services/freshness.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>
using namespace services::adsb;
int main(int argc,char** argv) {
  DynamicJsonDocument d(16384);
  auto parse=[&](const char* json,Aircraft& ac,bool ground=false) {
    assert(!deserializeJson(d,json));
    return parseAircraft(d.as<JsonObjectConst>(),ac,ground,15);
  };
  Aircraft ac;
  assert(parse(R"({"lat":35,"lon":140,"seen_pos":0.5,"flight":"ANA123  ","altitude":12345,"speed":250,"track":0})",ac));
  assert(!strcmp(ac.callsign,"ANA123") && !strcmp(ac.alt,"12345 ft"));
  assert(ac.gs_knots==250 && ac.nose_deg==0);
  assert(parse(R"({"lat":0,"lon":0,"seen_pos":0,"flight":"   ","hex":"abcdef","alt_baro":23456,"gs":310,"true_heading":85,"track":90})",ac));
  assert(!strcmp(ac.callsign,"abcdef") && !strcmp(ac.alt,"23456 ft"));
  assert(ac.nose_deg==85 && ac.track_deg==90 && ac.gs_knots==310);
  assert(!parse(R"({"lat":35,"lon":140,"seen_pos":16})",ac));
  assert(!parse(R"({"lat":35,"lon":140})",ac));
  assert(!parse(R"({"lon":140,"seen_pos":0})",ac));
  assert(!parse(R"({"lat":"35","lon":140,"seen_pos":0})",ac));
  assert(!parse(R"({"lat":91,"lon":140,"seen_pos":0})",ac));
  assert(!parse(R"({"lat":35,"lon":140,"seen_pos":-1})",ac));
  const char* ground=R"({"lat":35,"lon":140,"seen_pos":0,"altitude":"ground"})";
  assert(!parse(ground,ac));assert(parse(ground,ac,true));assert(!strcmp(ac.alt,"GND"));
  assert(parse(R"({"lat":35,"lon":140,"seen_pos":0})",ac));
  assert(std::isnan(ac.nose_deg) && std::isnan(ac.track_deg) && ac.gs_knots==0);
  assert(distanceKm(0,1,0,0)>111 && distanceKm(0,1,0,0)<112);
  assert(distanceKm(0,-179.9,0,179.9)<23);
  // A recently fetched target can already have an old position.
  assert(positionFresh(14,1000,15,15000));
  assert(!positionFresh(14,1001,15,15000));
  assert(!positionFresh(0,15001,15,15000));
  assert(!positionFresh(NAN,0,15,15000));
  assert(!positionFresh(-1,0,15,15000));
  // millis() rollover does not make a 16-second-old snapshot fresh again.
  uint32_t beforeWrap=0xFFFFFF00u,afterWrap=beforeWrap+16000u;
  assert(!positionFresh(0,afterWrap-beforeWrap,15,15000));
  if(argc>1) {
    std::ifstream f(argv[1]);assert(f.good());std::stringstream s;s<<f.rdbuf();
    assert(!deserializeJson(d,s.str()));
    assert(d["aircraft"].is<JsonArray>());
    size_t valid=0,located=0;
    for(JsonObjectConst obj:d["aircraft"].as<JsonArrayConst>()) {
      if(obj["lat"].is<double>() && obj["lon"].is<double>()) ++located;
      if(parseAircraft(obj,ac,false,15)) {++valid;assert(ac.callsign[0]);}
    }
    assert(located>0 && valid>0);
    std::cout<<"Live feed: "<<d["aircraft"].size()<<" aircraft, "<<located<<" positioned, "<<valid<<" fresh airborne.\n";
  }
  std::cout<<"Parser tests passed.\n";
}
