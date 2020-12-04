#include "chp.h"

#define ADC0_PIN 32

int adc0_pin = ADC0_PIN;

void my_callback(char* topic, byte* payload, unsigned int length) 
{
  payload[length] = '\0';
  String topic_str = topic, payload_str = (char*)payload;
  Serial.println("### [" + topic_str + "]: " + payload_str);
  
}

void setup() {
  Serial.begin(115200);
  pinMode(adc0_pin, INPUT);

//  use_saved_config();
  set_ota_pwd("12345678");
  Serial.println("### Interval=" + String(get_interval()));
  chp_wifi_begin();
  chp_init(false);
  mqtt_connect();
  set_callback(my_callback);
  
  set_sync_time(30*1000);
  
  Serial.println("Sync time=" + String(get_sync_time()));
  Serial.println("Interval=" + String(get_interval()));
}

void loop() {
  chp_wifi_handle();
  listen_for_fw();
  chp_loop();
  time_to_sync();

  int adc_v = analogRead(adc0_pin);
  float v_t = 3.3*adc_v/4095;

  if(time_to_send())
  {
    // payload format {"data":{...your data...}}
    String res = "{\"data\":{\"v_t\":" + String(v_t) + "}}";
    Serial.println(res);
    // topic @shadow/data/update
    pubData(res, "@shadow/data/update");
  }
  delay(100);
}
