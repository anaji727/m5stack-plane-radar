# Upstream and modifications

Based on [MatixYo/ESP32-Plane-Radar](https://github.com/MatixYo/ESP32-Plane-Radar),
commit `cd7e60620e457f286d5db41dab5c10d45cbefe95`, retrieved 2026-09-19.
The original MIT license is preserved in `LICENSE`.

Retained: radar drawing, range persistence, coordinate settings, airport/runway
data and overlay, and embedded font. The original renderer is directly adapted,
not independently recreated.

Changed: M5Unified/M5GFX hardware layer; 320x240 layout; 8-bit radar sprite;
physical A/B/C buttons; local dump1090 HTTP client; mutability/FA field adapter;
freshness expiry; bounded JSON/body memory; nearest-target selection; nonblocking
WiFi reconnection; editable endpoint and coordinates; status sidebar.

No adsb.fi/FR24 cloud requests or API keys are used by this firmware.

Dependencies are pinned in `platformio.ini`. Their own licenses remain applicable
to linked firmware. M5Unified and M5GFX are MIT licensed; M5GFX includes LovyanGFX
code and bundled font components with their own notices. WiFiManager is MIT;
ArduinoJson 6 is MIT; Arduino ESP32 contains multiple components and licenses.
Consult their distributed license files before redistributing binaries.

Technical references:

- https://github.com/flightaware/dump1090/blob/master/README-json.md
- https://github.com/M5Stack/M5Unified
- https://docs.platformio.org/en/latest/boards/espressif32/m5stack-core-esp32.html

The user's live mutability endpoint was read to verify `aircraft`, `now`,
`altitude`, `speed`, `track`, and `seen_pos`. Live snapshots and home coordinates
are not bundled with the release.
