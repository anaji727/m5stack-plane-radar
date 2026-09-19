#include "ui/aircraft_list.h"
#include "hardware/display.h"
#include "services/adsb_client.h"
#include "services/aircraft_parser.h"
#include "services/radar_location.h"
#include <algorithm>
#include <cstring>

namespace ui {
void aircraftListDraw(int x, int y, int width, int height) {
  const auto* planes = services::adsb::aircraftList();
  const size_t count = services::adsb::aircraftCount();
  struct Row { size_t index; float distance; } rows[services::adsb::kMaxAircraft];
  for (size_t i = 0; i < count; ++i) {
    rows[i] = {i, services::adsb::distanceKm(planes[i].lat, planes[i].lon,
               services::location::lat(), services::location::lon())};
  }
  std::sort(rows, rows + count, [planes](const Row& a, const Row& b) {
    if (a.distance != b.distance) return a.distance < b.distance;
    return std::strcmp(planes[a.index].callsign, planes[b.index].callsign) < 0;
  });

  tft.setClipRect(x, y, width, height);
  tft.fillRect(x, y, width, height, TFT_BLACK);
  tft.setFont(&fonts::Font0); tft.setTextSize(1);
  tft.setTextDatum(textdatum_t::top_left);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setCursor(x + 4, y + 3); tft.print("NEAREST");
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  const size_t capacity = height > 28 ? (height - 28) / 12 : 0;
  const size_t shown = std::min(count, capacity);
  for (size_t i = 0; i < shown; ++i) {
    tft.setCursor(x + 4, y + 18 + i * 12);
    tft.print(planes[rows[i].index].callsign);
  }
  if (!count) {
    tft.setCursor(x + 4, y + 18); tft.print("--");
  } else if (shown < count) {
    tft.setCursor(x + 4, y + height - 9);
    tft.printf("+%u more", static_cast<unsigned>(count - shown));
  }
  tft.clearClipRect();
}
}
