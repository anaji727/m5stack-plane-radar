#include "services/adsb_client.h"
#include "services/aircraft_parser.h"
#include "services/freshness.h"
#include "services/wifi_setup.h"
#include "config.h"
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <algorithm>
namespace services::adsb {
namespace {
Aircraft planes[kMaxAircraft], pending[kMaxAircraft];
float distances[kMaxAircraft];
size_t count=0;
bool received=false;
uint32_t receivedAt=0;
double sourceNow=0;
const char* state="WAIT DATA";
PollFn poll=nullptr;
constexpr size_t maxBody=64*1024;
bool body(HTTPClient& http,String& payload) {
  auto* stream=http.getStreamPtr();
  const int length=http.getSize();
  if (!stream || length>static_cast<int>(maxBody)) return false;
  if (!payload.reserve(length>0?length:4096)) return false;
  uint32_t start=millis();char buf[768];
  while(millis()-start<4000) {
    if(poll) poll();
    int available=stream->available();
    if (available>0) {
      int n=stream->readBytes(buf,std::min(available,static_cast<int>(sizeof(buf))));
      if(n>0 && (payload.length()+n>maxBody || !payload.concat(buf,n))) return false;
    }
    if(length>=0 && payload.length()==static_cast<size_t>(length)) return length>0;
    if(!http.connected() && !stream->available()) return length<0 && payload.length()>0;
    delay(1);
  }
  return false;
}
}
size_t aircraftCount(){return count;}
const Aircraft* aircraftList(){return planes;}
void setPollFn(PollFn fn){poll=fn;}
bool hasData(){return received;}
uint32_t ageMs(){return received ? millis()-receivedAt : UINT32_MAX;}
const char* status(){return state;}
void reset(){count=0;received=false;sourceNow=0;state="WAIT DATA";}
bool expire(){
  const uint32_t elapsed=ageMs();
  size_t n=0;
  for(size_t i=0;i<count;++i)
    if(positionFresh(planes[i].position_age_sec,elapsed,config::kMaxPositionAgeSec,config::kStaleMs)) planes[n++]=planes[i];
  bool changed=n!=count;count=n;return changed;
}
bool fetchUpdate(double clat,double clon,float radius) {
  const uint32_t requestedAt=millis();
  WiFiClient client; HTTPClient http;
  if(!http.begin(client,aircraftUrl())){state="BAD URL";return false;}
  http.useHTTP10(true);http.setReuse(false);http.setConnectTimeout(800);http.setTimeout(1500);
  const char* keys[]={"Transfer-Encoding","Content-Encoding"};http.collectHeaders(keys,2);
  http.addHeader("Accept-Encoding","identity");http.addHeader("Cache-Control","no-cache");
  int code=http.GET();
  if(code!=200){state=code==404?"HTTP 404":"HTTP ERROR";Serial.printf("HTTP %d\n",code);http.end();return false;}
  if(http.header("Transfer-Encoding").length() || (http.header("Content-Encoding").length() && http.header("Content-Encoding")!="identity")) {
    state="ENCODING";http.end();return false;
  }
  String payload;
  if(!body(http,payload)){state="BODY ERROR";http.end();return false;}
  http.end();
  // Fixed capacities bound memory use even on malformed/oversized feeds.
  DynamicJsonDocument filter(2048),doc(49152);
  filter["now"]=true;
  const char* fields[]={"lat","lon","seen_pos","flight","hex","alt_baro","alt_geom","altitude","gs","speed","track","true_heading","t"};
  for(auto key:fields) filter["aircraft"][0][key]=true;
  auto err=deserializeJson(doc,payload,DeserializationOption::Filter(filter));
  if(err){state="JSON ERROR";Serial.println(err.c_str());return false;}
  if(!doc["aircraft"].is<JsonArray>() || !doc["now"].is<double>()) {state="BAD SCHEMA";return false;}
  double stamp=doc["now"].as<double>();
  if(!std::isfinite(stamp)||stamp<=0){state="BAD TIME";return false;}
  // Unchanged snapshots must not extend the age of aircraft indefinitely.
  if(received && stamp==sourceNow){state="UNCHANGED";return false;}
  size_t n=0;
  for(JsonObjectConst obj:doc["aircraft"].as<JsonArrayConst>()) {
    Aircraft ac;
    if(!parseAircraft(obj,ac,config::kAdsbShowGroundAircraft,config::kMaxPositionAgeSec)) continue;
    float distance=distanceKm(ac.lat,ac.lon,clat,clon);
    if(distance>radius) continue;
    size_t at=n;
    if(n==kMaxAircraft){at=std::max_element(distances,distances+n)-distances;if(distance>=distances[at]) continue;}
    else ++n;
    pending[at]=ac;distances[at]=distance;
  }
  memcpy(planes,pending,n*sizeof(Aircraft));count=n;received=true;receivedAt=requestedAt;sourceNow=stamp;state="LIVE";
  return true;
}
}
