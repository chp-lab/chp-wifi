#include "chp.h"

#define OUT1 26
#define RY_REG_ID 90
#define RY_REG_LEN 4
#define LIGHT_ON LOW
#define LIGHT_OFF HIGH
#define RELAY0_PIN OUT1
#define nexpie_topic "@msg/#"
#define update_topic "@shadow/data/update-dev"
#define this_device_topic "this_device"
#define relay_topic "@msg/control/relay-dev"
#define refresh_topic "@msg/control/refresh-dev"

int relay0 = RELAY0_PIN;
int ry_reg_start = RY_REG_ID;
int ry_reg_end = RY_REG_ID + RY_REG_LEN;

void light_cnt(String cmd) {
  if(cmd == "ON") 
  {
    digitalWrite(relay0, LIGHT_ON);
    EEPROM_write(ry_reg_start, "1");
    Serial.println("Lighht on.");
  }
  else if(cmd == "OFF")
  {
    digitalWrite(relay0, LIGHT_OFF);
    EEPROM_write(ry_reg_start, "0");
    Serial.println("Lighht off.");
  }
  else
  {
    Serial.println("Light cnt command not found");
  }
  String lst = EEPROM_read(ry_reg_start, ry_reg_end);
  Serial.println("$$$ lst=" + lst);
}

String new_shadow() {
  float a_temp, s_moist, s_temp, a_humid;
  int light;
  randomSeed(analogRead(0));
  a_temp = 25 + random(0, 10);
  s_moist = random(0, 20);
  s_temp = 23 + random(0, 5);
  light = 5000 + random(1000);
  a_humid = 50 + random(0, 50);

  String lst = EEPROM_read(ry_reg_start, ry_reg_end);
  if(lst != "0" && lst != "1") {
    Serial.println("Wrong light status on refresh request!! Reset lst to zero.");
    lst = "0";
    light_cnt("OFF");
  }
  
  if(isnan(a_humid)) {
    a_humid = 0;
  }
  if(isnan(a_temp)) {
    a_temp = 0;
  }
  
  String shadow_json = "{\"data\":{\"a_temp\":" + String(a_temp) + ", \"light\":" + String(light) +
  ", \"s_moist\":" + String(s_moist) + ", \"s_temp\":" + String(s_temp) + ", \"light_status\":" + String(lst) + ", \"a_humid\":" + String(a_humid) + "}}";

  // Serial.println("shadow_json= " + shadow_json);
  return shadow_json;
}

void sync_light(String light_st) {
  Serial.println("Sync light...");
  if (light_st == "1") {
    String ry_update = "{\"data\":{\"light_status\":1}}";
    pubData(ry_update, update_topic);
    Serial.println("Bright.");
  }
  else if (light_st == "0") {
    String ry_update = "{\"data\":{\"light_status\":0}}";
    pubData(ry_update, update_topic);
    Serial.println("Dark.");
  }
  else {
    Serial.println("Sync light failed, Unknow light status.");
  }
}

void my_callback(char* topic, byte* payload, unsigned int length)
{
  payload[length] = '\0';
  String topic_str = topic, payload_str = (char*)payload;
  Serial.println("### [" + topic_str + "]: " + payload_str);
  /*
     example to check incoming message
     RT_TOPIC + "/" + device_id() is auto subscribed topic
     You need to add other incoming topic by yourself with sub_data(String topic)
  */
  if (topic_str == RT_TOPIC "/" + device_id())
  {
    Serial.println("rt request recv");
  }

  if (topic_str == device_id() + this_device_topic)
  {
    Serial.println("myswitch event received");
    /*
       TODO
    */
  }

  // this will turn on or off relay of all devices connected with mqtt broker
  if(topic_str == relay_topic)
  {
    Serial.println("Relay control command");
    Serial.println("Lamp event received!");
    if (payload_str == "1")
    {
      Serial.println("Turn on the lamp!");
      light_cnt("ON");
      sync_light("1");
    }
    else if (payload_str == "0")
    {
      Serial.println("Turn off the lamp!");
      light_cnt("OFF");
      sync_light("0");
    }
    else {
      Serial.println("Bad light control payload. Do not thing!");
    }
  }

  if(topic_str == refresh_topic)
  {
    Serial.println("Shadow update request!");
    String lst = EEPROM_read(ry_reg_start, ry_reg_end);
    Serial.println("$$$ lst=" + lst);
    String shadow_msg = new_shadow();
    Serial.println("pub shadow_msg=" + shadow_msg);
    pubData(shadow_msg, update_topic);
  }
}

void setup() {
  Serial.begin(115200);
  eeprom_init();
  String ry_reg = EEPROM_read(ry_reg_start, ry_reg_end);
  Serial.println("### ry_reg=" + ry_reg);
  
  if(ry_reg == "1") {
    Serial.println("Light back to on.");
    digitalWrite(relay0, LIGHT_ON);
  }
  else if(ry_reg == "0") {
    Serial.println("Light back to off.");
    digitalWrite(relay0, LIGHT_OFF);
  }
  else {
    Serial.println("Unknow light status. Turn light off.");
    light_cnt("OFF");
  }
  Serial.println("MAC ADDR->" + get_mac());
  set_ota_pwd("12345678");
  Serial.println("### Interval=" + String(get_interval()));
  chp_wifi_begin();
  use_saved_config();
  Serial.println("Mqtt Loading...");
  chp_init(false);
  set_sync_time(30 * 1000);
  Serial.println("Sync time=" + String(get_sync_time()));
  Serial.println("Interval=" + String(get_interval()));
  sub_data(nexpie_topic);
  Serial.println("Mqtt load complete.");
}

void loop() {
  chp_wifi_handle();
  listen_for_fw();
  chp_loop();
  time_to_sync();
  // reset wifi
  int wifi_reset_presed = digitalRead(WIFI_RESET_BTN);
  if(wifi_reset_presed == 0) {
//    reset_wifi_con();
    Serial.println("Factory reset pressed, wait 5 sec...");
    delay(5000);
    wifi_reset_presed = digitalRead(WIFI_RESET_BTN);
    if(wifi_reset_presed == 0) {
      Serial.println("WiFi Reset pressed!");
      reset_wifi_con();
      delay(1000);
    }
    else {
      Serial.println("Button released before 5 sec. No factory reset.");
    }
  }

  if (time_to_send())
  {
    String shadow_msg = new_shadow();
    Serial.println("Time to send "+ get_time_format() + shadow_msg);
    pubData(shadow_msg, update_topic);
  }

  /*
    resubscribe and setcallback again when mqtt reconected
  */
  if (mqtt_reconnect())
  {
    Serial.println("Set callback function and subscrib message");
    set_callback(my_callback);
    sub_data(device_id() + this_device_topic);
    sub_data(nexpie_topic);
  }

  if (time_to_reboot("00:07"))
  {
    reboot_now();
  }
}
