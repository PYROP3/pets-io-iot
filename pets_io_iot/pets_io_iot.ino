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
#include "streaming.h"

//#define SCAN_QR
//#define REGISTER

#define PIO_DEVICE_ID "PIOFB00001"

void onEvent(void);

void setup() {
  boolean apConnected = false;
#ifdef REGISTER
  int registerResult;
#endif
  
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  init_ble();
  init_camera(false);
  init_ultrasound();

  setConnectionStatus("waiting");
  do {
    delay(500);
    apConnected = connect_to_ap(getSSID().c_str(), getPass().c_str());
  } while (!apConnected);
  setConnectionStatus("success");

#ifdef REGISTER
  setRegisterStatus("registering");
  do {
    delay(500);
    registerResult = registerDevice(getToken().c_str());
  } while (registerResult != 200);
  setRegisterStatus("success");
#else
  setRegisterStatus("skipped");
#endif

  setup_streaming();
}

void loop() {
  // Loops automatically on setup
  delay(1);
  execute(onEvent);
}

void onEvent(void) {
  Serial.println("onEvent!");
  sendEvent();
}
