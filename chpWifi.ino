#include "chpWifi.h"

void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println();
  chp_wifi_begin();
}

void loop() {
  chp_wifi_handle();
}
