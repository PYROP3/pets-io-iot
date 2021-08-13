#ifndef HTTP_CLIENT_H

#define HTTP_CLIENT_H

//Your Domain name with URL path or IP address with path
#ifdef ENV_LOCAL
#define EVENT_API "http://192.168.15.27:5000/eventTriggered"
#else
#define EVENT_API "https://pets-io.herokuapp.com/eventTriggered"
#endif

#define PIO_DEVICE_ID "PIOFB00001"

String generateMessage(String picture_base64) {
  String objStart = "{";
  String objEnd = "}";
  String colon = ":";
  String quotes = "\"";
  String comma = ",";
  String imgTag = "Img";
  String devTag = "DeviceID";
  String devStr = PIO_DEVICE_ID;
  String extTag = "Extra";
  String nulStr = "null";
  String message = objStart + 
    quotes + imgTag + quotes + colon + quotes + picture_base64 + quotes + comma +
    quotes + devTag + quotes + colon + quotes + devStr + quotes + comma +
    quotes + extTag + quotes + colon + quotes + nulStr + quotes +
    objEnd;
  return message;
}

int sendEvent(String picture_base64) {
  HTTPClient http;
  
  // Your Domain name with URL path or IP address with path
  http.begin(EVENT_API);

  // Specify headers
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Content-Disposition", "inline; filename=capture.jpg");
  http.addHeader("Access-Control-Allow-Origin", "*");
  
  // Send HTTP POST request
  // TODO try to remove background from photo
  // TODO add event extra (time in litterbox[s]/food consumed[g])
  String httpMessage = generateMessage(picture_base64);
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

#endif
