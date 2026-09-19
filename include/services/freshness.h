#pragma once
#include <cstdint>
#include <cmath>
namespace services::adsb {
inline bool positionFresh(float ageAtSnapshot, uint32_t elapsedMs,
                          float maxPositionAgeSec, uint32_t maxSnapshotAgeMs) {
  return std::isfinite(ageAtSnapshot) && ageAtSnapshot >= 0 &&
         elapsedMs <= maxSnapshotAgeMs &&
         ageAtSnapshot + elapsedMs / 1000.0f <= maxPositionAgeSec;
}
}
