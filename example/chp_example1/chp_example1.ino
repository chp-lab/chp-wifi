#include "chp.h"

#define ADC0_PIN 32

int adc0_pin = ADC0_PIN;

void setup() {
  Serial.begin(115200);
  pinMode(adc0_pin, INPUT);
  String n_broker = "broker.netpie.io";
  String n_client_id = "586ef1b2-7460-420d-a98f-303b29696364";
  String n_token = "8d68AwFuQ8fYZwTNhpfDZLjsdmcx4Bbd";
  String n_secret = "*hkKXQ8U_!iW7G1SC9UNsSqU-S1D*xGD";
  // number of reconnecting to mqtt brocker
  int n_max_reconnect = 3;

//  set_mqtt(String m_server, int m_port, String m_clid, String m_uname, String m_pwd, int m_max_rec)
  set_mqtt(n_broker, 1883, n_client_id, n_token, n_secret, n_max_reconnect);
  chp_wifi_begin();
  chp_init(false);
  set_sync_time(30*1000);
  Serial.println("Sync time=" + String(get_sync_time()));
  // set upload schedule
  set_interval(1000);
  Serial.println("Interval=" + String(get_interval()));
}

void loop() {
  chp_wifi_handle();
  listen_for_fw();
  chp_loop();
  time_to_sync();

  int adc_v = analogRead(adc0_pin);
  float v_t = 3.3*adc_v/4095;
//  Serial.println("adc_v=" + String(adc_v));

  if(time_to_send() || real_time_req())
  {
    // payload format {"data":{...your data...}}
    String res = "{\"data\":{\"v_t\":" + String(v_t) + "}}";
    Serial.println(res);
    // topic @shadow/data/update
    pubData(res, "@shadow/data/update");
  }
  delay(100);
}
