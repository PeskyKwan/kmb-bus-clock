#pragma once
#include <ctime>
#include <cmath>
#include <algorithm>

// An illustrative countdown position, not GPS or inferred stop passage.
constexpr float approachWindowSeconds = 600.0f;
inline bool animationFresh(bool connected, int code, std::time_t now,
                           std::time_t stamp, std::time_t eta) {
  return connected && code == 2 && stamp > 0 &&
         std::abs(double(now - stamp)) <= 120 && eta >= now - 30 &&
         eta <= now + 10800;
}
inline float approachFraction(std::time_t now, std::time_t eta) {
  return std::max(0.0f, std::min(1.0f, 1.0f - float(eta - now) / approachWindowSeconds));
}
struct ApproachMotion {
  bool visible = false;
  float fraction = 0;
  std::time_t previousEta = 0;
  void reset() { visible = false; fraction = 0; previousEta = 0; }
  void update(bool valid, std::time_t now, std::time_t eta, float seconds) {
    if (!valid) { reset(); return; }
    const float target = approachFraction(now, eta);
    // A major forecast change may represent another service; reposition instead
    // of drawing the previous marker driving backwards through the whole map.
    if (!visible || std::abs(double(eta - previousEta)) > 180) fraction = target;
    else {
      const float limit = std::max(0.0f, std::min(seconds, 1.0f)) * .15f;
      fraction += std::max(-limit, std::min(limit, target - fraction));
    }
    previousEta = eta; visible = true;
  }
};
