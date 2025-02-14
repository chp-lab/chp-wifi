#ifndef CHP_H
#define CHP_H

#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Time.h>
#include "chpWifi.h"

#define MODEL_NAME "v2024_1"
#define FW_VERSION "FW Version " WIFI_CONF_V
#define WIFI_TIMEOUT 60*1000

//#define MQTT_SERVER "172.31.0.99"
#define MQTT_SERVER "18.140.173.239"
#define MQTT_PORT 1883
#define MQTT_USERNAME "chp-lab"
#define MQTT_PASSWORD "atop3352"

#define TOPIC "@msg/set/status/GfdHrdhgfIBMKGXBZt6nXkcw"
#define MQTT_MAX_RECONNECT 5
#define NUM_PHASE 3
#define INTERVAL 5*60*1000
#define MAX_LEN 1000
#define INFLUX_DOOR "mm_"
#define LOCATION "Hatyai"
#define SYNC_TIME 60*1000
#define PREPARE 100
#define NUM_PREPARE 10

#define DEBUG_MODE true

#define RT_TOPIC "rt_req"

#define OTA_PWD "admin"
#define build_in_led 2

#define WIFI_RESET_BTN 15
#define SW1 16
#define SW2 14

void callback(char* topic, byte* payload, unsigned int length) ;
void pubData(String payload, String topic);
void mqtt_connect();
void giantOta();
String get_client_name();
String randomMessage();
String uart_read();
String influx_inline(String j_str, bool req_ts);
String get_time();
void chp_init(bool en_log);
bool time_to_sync();
void listen_for_fw();
void chp_loop();
bool time_to_send();
String device_id();
unsigned long get_sync_time();
void set_sync_time(unsigned long sync_t);
unsigned long get_interval();
void set_interval(unsigned long sch_t);
String msg_construct(String payload);
void sub_data(String topic);
bool real_time_req();
void set_mqtt(String m_server, int m_port, String m_clid, String m_uname, String m_pwd);
void use_saved_config();
void set_callback(void (*func1)(char* topic, byte* payload, unsigned int length));
bool mqtt_reconnect();
bool time_to_reboot(String reboot_time);
void reboot_now();
String get_time_format();
String get_date_fmrt();
int get_days();
int get_wifi_rssi();
void is_fact_reset(int d_before_rest);
void sub_data(String topic, int QoS);
#endif

