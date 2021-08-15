#include "debug.h"
#include "esp_camera.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include "ble_server.h"
#include "camera.h"
#include "http_client.h"
#include "state_machine.h"
#include "ultrasound.h"
#include "wifi_access.h"
#include "registrar.h"
#include "qr.h"

//#define SCAN_QR
//#define REGISTER

#define PIO_DEVICE_ID "PIOFB00001"

void onEvent(void);

void setup() {
  boolean apConnected = false;
#ifdef REGISTER
  int registerResult;
  String registerToken;
#endif
  
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

#ifdef DEBUG
  Serial.print("Testing B64: ");
  Serial.print(bytesToB64((uint8_t*)"Man", 3));
  Serial.println(" (should be \"TWFu\")");
  Serial.print("Testing B64: ");
  Serial.print(bytesToB64((uint8_t*)"Ma", 2));
  Serial.println(" (should be \"TWE=\")");
  Serial.print("Testing B64: ");
  Serial.print(bytesToB64((uint8_t*)"M", 1));
  Serial.println(" (should be \"TQ==\")");
#endif

  init_ble();

  do {
    delay(500);
    apConnected = connect_to_ap(getSSID().c_str(), getPass().c_str());
  } while (!apConnected);

#ifdef REGISTER
  do {
    delay(500);
    registerResult = registerDevice(getToken().c_str());
  } while (registerResult != 200);
#endif

  init_camera(false);
  init_ultrasound();
  
  execute(onEvent);
}

void loop() {
  // Loops automatically on setup
}

void onEvent(void) {
  String pic = take_picture();
  
#ifdef DEBUG
  Serial.println("onEvent!");
  Serial.println("pic=\"" + pic + "\"");
#endif

  sendEvent(pic);
}
