#ifndef WIFI_ACCESS_H

#define WIFI_ACCESS_H

//const char* ssid = "NET_2GDC3667";
//const char* password = "63DC3667";
//const char* ssid = "VIVOFIBRA-7F90";
//const char* password = "c662727f90";

void connect_to_ap(const char *ssid, const char *password) {
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
}

#endif
