#ifndef CHP_WIFI_H
#define CHP_WIFI_H

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#include <WebServer.h>
#endif
#include <time.h>
#include <AutoConnect.h>

void rootPage();
void startPage();
void chp_wifi_begin();
void chp_wifi_handle();

#endif

