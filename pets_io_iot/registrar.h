#ifndef REGISTRAR_H

#define REGISTRAR_H

#ifdef ENV_LOCAL
#define REGISTER_API "http://192.168.15.27:5000/confirmDeviceRegistration?registerToken="
#else
#define REGISTER_API "http://pets-io.herokuapp.com/confirmDeviceRegistration?registerToken="
#endif

#define PIO_DEVICE_ID "PIOFB00001"

#ifdef DEBUG_REGISTER
#define DEBUG
#endif

String generateRegistrationMessage(String registerToken) {
  String objStart = "{";
  String objEnd = "}";
  String colon = ":";
  String quotes = "\"";
  String comma = ",";
  String tknTag = "Token";
  String devTag = "DeviceID";
  String devStr = PIO_DEVICE_ID;
  String message = objStart + 
    quotes + tknTag + quotes + colon + quotes + registerToken + quotes + comma +
    quotes + devTag + quotes + colon + quotes + devStr + quotes +
    objEnd;
  return message;
}

int registerDevice(String registerToken) {
  HTTPClient http;
  
#ifdef DEBUG
  Serial.printf("Registering device [%s]\n", registerToken.c_str());
#endif
  
  // Your Domain name with URL path or IP address with path
  http.begin((REGISTER_API + registerToken).c_str());
  
  // Send HTTP POST request
  String httpMessage = generateRegistrationMessage(registerToken);
#ifdef DEBUG
  Serial.print("HTTP message: ");
  Serial.println(httpMessage);
#endif
  int httpResponseCode = http.POST(httpMessage);

#ifdef DEBUG
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    String payload = http.getString();
    Serial.println(payload);
    // TODO write to file if already registered, on next boot if file exists skip register step
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
#endif
  // Free resources
  http.end();
  
  return httpResponseCode;
}

#ifdef DEBUG
#undef DEBUG
#endif

#endif
