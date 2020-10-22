#include "chpWifi.h"

void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println();
  esp_wifi_begin();
}

void loop() {
  esp_wifi_handle();
}
