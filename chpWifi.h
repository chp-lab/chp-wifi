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
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#endif
#include <time.h>
#include <AutoConnect.h>

#define MDNS_URI "giant"
#define URL_START 100
#define URL_LEN 30
#define CLID_LEN 50
#define TOKEN_LEN 50
#define SECRET_LEN 50
#define PORT_LEN 5
#define INTERVAL_LEN 10
#define ROOM_NUM_LEN 32
#define EEPROM_SIZE 512
#define CHP_EEPROM_SCOPE 400
#define  MODE_PIN 19
#define AP_PWD_FK "administrator"
#define WIFI_CONF_V "1.4.16"
#define ota_fw_name "/firmware_pro.bin"
#define ota_port 8100
#define ota_server_uri "giantiot.com"


#if defined(ARDUINO_ARCH_ESP8266)
#define sudo_reboot() ESP.reset()
#elif defined(ARDUINO_ARCH_ESP32)
#define sudo_reboot() ESP.restart()
#endif

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
void Reset_EEPROM();
void dvSetup();
void handleNetpie();
Mqtt_config get_mqtt_config();
void set_mqtt_flag(bool __my_flag);
void set_ota_pwd(String my_ota_pwd);
void connect_to_myap(String my_ssid, String my_password);
String get_room_num();
void set_broker_connection(String my_server_url, String my_client_id, String my_token, String my_secret, int my_port, int my_interval, String room_num);
void change_to_scanner(bool cnt_mode);
void set_mode_pin(int this_mode_pin);
String get_ap_name();
String get_mac();
void eeprom_init();
void reset_wifi_con();
void deleteAllCredentials();
String onUpdate(AutoConnectAux& aux, PageArgument& args);
void apt_update();
void apt_update(const char* uuri, const int uuport, const char* ufw_name);
void set_ap_prefix(String this_prefix);
#endif

