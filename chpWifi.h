#ifndef CHP_WIFI_H
#define CHP_WIFI_H

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#endif
#include <time.h>
#include <AutoConnect.h>

#define MDNS_URI "giant"

struct Mqtt_configs {
   String	mqtt_server;
   String	client_name;
   String  mqtt_username;
   String mqtt_password;
   int	mqtt_port;
   int	interval;
};

typedef struct Mqtt_configs Mqtt_config;

void rootPage();
void startPage();
void chp_wifi_begin();
void chp_wifi_handle();
void EEPROM_write(int index, String text);
String EEPROM_read(int index, int length);
void Reset_EEPROM(boolean RESET_EEPROM);
void dvSetup();
void handleNetpie();
Mqtt_config get_mqtt_config();
void set_mqtt_flag(bool __my_flag);
void set_ota_pwd(String my_ota_pwd);

#endif

