#ifndef WIFI_ACCESS_H

#define WIFI_ACCESS_H

#define RETRY_WIFI_COUNT 5

#ifdef DEBUG_WIFI
#define DEBUG
#endif

boolean connect_to_ap(const char *ssid, const char *password) {
  int retryCount = RETRY_WIFI_COUNT;
  boolean result = false;

#ifdef DEBUG 
  Serial.printf("connect_to_ap(%s, %s)\n", ssid, password);
#endif

  if(!strlen(ssid) || !strlen(password)) {
#ifdef DEBUG 
  Serial.printf("ssid or password are empty\n");
#endif
    return false;
  }

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
  Serial.printf("WiFi connected: %d (%d)\n", (int)result, (int)true);
#endif

  if (!result) {
#ifdef DEBUG 
  Serial.printf("disconnecting...\n");
#endif
    WiFi.disconnect();
  }

  return result;
}

#ifdef DEBUG
#undef DEBUG
#endif

#endif
