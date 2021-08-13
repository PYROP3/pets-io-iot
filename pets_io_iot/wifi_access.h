#ifndef WIFI_ACCESS_H

#define WIFI_ACCESS_H

//const char* ssid = "NET_2GDC3667";
//const char* password = "63DC3667";
//const char* ssid = "VIVOFIBRA-7F90";
//const char* password = "c662727f90";

#define RETRY_WIFI_COUNT 5

boolean connect_to_ap(const char *ssid, const char *password) {
  int retryCount = RETRY_WIFI_COUNT;
  boolean result = false;

#ifdef DEBUG 
  Serial.printf("connect_to_ap(%s, %s)\n", ssid, password);
#endif

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED && retryCount-- > 0) {
    delay(500);
#ifdef DEBUG
    Serial.print(".");
#endif
  }

  result = WiFi.status() == WL_CONNECTED;
  
#ifdef DEBUG
  Serial.println("");
  Serial.printf("WiFi connected: %d\n", (int)result);
#endif

  if (!result) {
    WiFi.disconnect();
  }

  return result;
}

#endif
