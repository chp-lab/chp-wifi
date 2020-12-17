#include "chp.h"

#define ADC0_PIN 32
#define RELAY0_PIN 13

int adc0_pin = ADC0_PIN;
int relay0 = RELAY0_PIN;

void my_callback(char* topic, byte* payload, unsigned int length) 
{
  payload[length] = '\0';
  String topic_str = topic, payload_str = (char*)payload;
  Serial.println("### [" + topic_str + "]: " + payload_str);
  /*
   * example to check incoming message
   * RT_TOPIC + "/" + device_id() is auto subscribed topic
   * You need to add other incoming topic by yourself with sub_data(String topic)
   */
   if(topic_str == RT_TOPIC "/" + device_id())
   {
      Serial.println("Lamp event received!");
      if(payload_str == "on")
      {
        Serial.println("Turn on the lamp!");
        digitalWrite(relay0, HIGH);
      }
      else if(payload_str == "off")
      {
        Serial.println("Turn off the lamp!");
        digitalWrite(relay0, LOW);
      }
      else
      {
        Serial.println("Unknow message!");
      }
   }

   if(topic_str == device_id() + "/myswitch")
   {
      Serial.println("myswitch event received");
      /*
       * TODO
       */
   }
  
}

void setup() {
  Serial.begin(115200);
  pinMode(adc0_pin, INPUT);
  pinMode(relay0, OUTPUT);

  use_saved_config();
  set_ota_pwd("12345678");
  Serial.println("### Interval=" + String(get_interval()));
  chp_wifi_begin();
  chp_init(false);
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

  /*
   * resubscribe and setcallback again when mqtt reconected
   */
   if(mqtt_reconnect())
   {
      Serial.println("Set callback function and subscrib message");
      set_callback(my_callback);
      sub_data(device_id() + "/myswitch");
   }
   delay(100);
}
