#include <Arduino.h>
#include <M5Unified.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <Preferences.h>
#include <ArduinoJson.h>

#include "../eta_logic.h"
#include "../eta_animation.h"

namespace {
constexpr uint32_t kExpectedWidth = 320;
constexpr uint32_t kExpectedHeight = 240;
constexpr uint16_t kCream = 0xFF7A;
constexpr uint16_t kInk = 0x21E6;
constexpr uint16_t kRed = 0xCA07;
constexpr uint16_t kMap = 0xD71D;

WebServer server(80);
Preferences prefs;
bool displayReady = false;

void drawScaffold() {
  auto &display = M5.Display;
  display.fillScreen(kCream);
  display.setTextColor(kInk, kCream);
  display.setTextDatum(top_left);
  display.setTextSize(1);
  display.drawString("KMB BUS CLOCK", 12, 12, &fonts::Font2);
  display.setTextColor(kRed, kCream);
  display.drawString("CoreS3", 12, 42, &fonts::Font4);
  display.setTextColor(kInk, kCream);
  display.drawString("PORT SCAFFOLD", 12, 80, &fonts::Font2);
  display.fillRoundRect(12, 112, 296, 76, 10, kMap);
  display.setTextColor(kInk, kMap);
  display.drawCentreString("NO LIVE ETA", 160, 132, &fonts::Font4);
  display.drawCentreString("touch + power acceptance pending", 160, 166, &fonts::Font0);
  display.setTextColor(kInk, kCream);
  display.drawString(String(M5.Power.getBatteryLevel()) + "%", 12, 210, &fonts::Font2);
  display.drawRightString(M5.Power.isCharging() ? "CHARGING" : "BATTERY", 308, 210, &fonts::Font2);
}

void sendStatus() {
  StaticJsonDocument<384> out;
  out["target"] = "cores3";
  out["scaffold"] = true;
  out["liveEta"] = false;
  out["width"] = M5.Display.width();
  out["height"] = M5.Display.height();
  out["displayReady"] = displayReady;
  out["touchPoints"] = M5.Touch.getCount();
  out["battery"] = M5.Power.getBatteryLevel();
  out["charging"] = M5.Power.isCharging();
  String body;
  serializeJson(out, body);
  server.send(200, "application/json", body);
}
}  // namespace

void setup() {
  auto config = M5.config();
  M5.begin(config);
  if (M5.Display.width() < M5.Display.height()) M5.Display.setRotation(1);
  displayReady = M5.Display.width() == kExpectedWidth && M5.Display.height() == kExpectedHeight;

  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  prefs.begin("busclock", false);
  server.on("/status", HTTP_GET, sendStatus);
  server.begin();
  drawScaffold();

  Serial.printf("CORES3_PORT_SCAFFOLD width=%d height=%d displayReady=%s\n",
                M5.Display.width(), M5.Display.height(), displayReady ? "true" : "false");
}

void loop() {
  M5.update();
  server.handleClient();
  if (M5.Touch.getDetail().wasClicked()) drawScaffold();
  delay(5);
}
