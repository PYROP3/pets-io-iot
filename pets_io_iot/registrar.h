#ifndef REGISTRAR_H

#define REGISTRAR_H

#define REGISTER_API "http://192.168.15.11:5000/eventTriggered?registerToken="

int registerDevice(String registerToken) {
  HTTPClient http;
  
  // Your Domain name with URL path or IP address with path
  http.begin((REGISTER_API + registerToken).c_str());
  
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  
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
  // Free resources
  http.end();
  
  return httpResponseCode;
}

#endif
