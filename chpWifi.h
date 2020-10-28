#ifndef CHP_WIFI_H
#define CHP_WIFI_H

//#include <ArduinoOTA.h>
//#include <PubSubClient.h>
//#include <ArduinoJson.h>
//#include <Time.h>

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#include <WebServer.h>
#endif
#include <time.h>
#include <AutoConnect.h>

//#define MODEL_NAME "STD2021"
//#define FW_VERSION "FW Version 1.1.0"
//#define WIFI_TIMEOUT 60*1000
//#define MQTT_SERVER "nexpie-giant.ddns.net"
//#define MQTT_PORT 1883
//#define MQTT_USERNAME "chp-lab"
//
//#define MQTT_PASSWORD "atop3352"
//
//#define TOPIC "test/powermeter"
//#define MQTT_MAX_RECONNECT 10
//#define NUM_PHASE 3
//#define INTERVAL 5*60*1000
//#define MAX_LEN 1000
//#define INFLUX_DOOR "mm_"
//#define LOCATION "Hatyai"
//#define SYNC_TIME 60*1000
//#define PREPARE 100
//#define NUM_PREPARE 10
//
//#define DEBUG_MODE true

//void callback(char* topic, byte* payload, unsigned int length) ;
//void pubData(String payload, String topic);
//void mqtt_connect();
//void giantOta();
//String get_client_name();
//String randomMessage();
//String uart_read();
//String influx_inline(String j_str);
//String get_time();
//void chp_init(bool en_log);
//bool time_to_sync();
//void listen_for_fw();
//void chp_loop();
//bool time_to_send();
//String device_id();
//unsigned long get_sync_time();
//void set_sync_time(unsigned long sync_t);
//unsigned long get_interval();
//void set_interval(unsigned long sch_t);
//String msg_construct(String payload);

void rootPage();
void startPage();
void chp_wifi_begin();
void chp_wifi_handle();

#endif

