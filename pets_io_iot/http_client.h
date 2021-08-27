#ifndef HTTP_CLIENT_H

#define HTTP_CLIENT_H

#include <string.h>

//#define ENV_LOCAL

#ifdef ENV_LOCAL
#define EVENT_API "http://192.168.15.27:5000/eventTriggered"
#else
#define EVENT_API "http://pets-io.herokuapp.com/eventTriggered"
#endif

#define PIO_DEVICE_ID "PIOFB00001"

#ifdef DEBUG_HTTP
#define DEBUG
#endif

int sendEvent() {
  camera_fb_t * fb = NULL;
  HTTPClient http;

  fb = esp_camera_fb_get();
  
  // Your Domain name with URL path or IP address with path
#ifdef DEBUG
  Serial.print("HTTP addr: ");
  Serial.println(EVENT_API);
#endif
  http.begin(EVENT_API);

  // Specify headers
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Content-Disposition", "inline; filename=capture.jpg");
  http.addHeader("Access-Control-Allow-Origin", "*");
  http.addHeader("Device-ID", PIO_DEVICE_ID);
  
  // Send HTTP POST request
  // TODO try to remove background from photo
  // TODO add event extra (time in litterbox[s]/food consumed[g])
  
#ifdef DEBUG
  Serial.printf("Sending %d bytes\n", fb->len);
#endif
  int httpResponseCode = http.POST(fb->buf, fb->len);

#ifdef DEBUG
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    String payload = http.getString();
    Serial.println(payload);
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
#endif
  // Free resources
  http.end();
  esp_camera_fb_return(fb);
  
  return httpResponseCode;
}

#ifdef DEBUG
#undef DEBUG
#endif

#endif
