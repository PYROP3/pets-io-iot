#include "esp_camera.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include "camera.h"
#include "http_client.h"
#include "state_machine.h"
#include "ultrasound.h"
#include "wifi_access.h"
#include "registrar.h"
#include "qr.h"

#define SCAN_QR

void onEvent(void);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

#ifdef SCAN_QR
  parsed_json_t *scanned_data = NULL;
  uint8_t *scanned_data_raw = NULL;
  int scanned_data_size = 0;
#endif

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

  init_ultrasound();

#ifdef SCAN_QR
  init_camera(true);
  do {
    scanned_data_size = scanQR(&scanned_data_raw);
  } while (scanned_data_size <= 0);
#else
  init_camera(false);
#endif
  
  // TODO load via QR code (take picture, scan, parse text)
#ifdef SCAN_QR
  const char* ssid = scanned_data->ap_ssid.c_str();
  const char* password = scanned_data->ap_password.c_str();
#else
  const char* ssid = "VIVOFIBRA-7F90";
  const char* password = "c662727f90";
#endif
  connect_to_ap(ssid, password);

#ifdef DEBUG
  Serial.println("debug pic");
  String pic = take_picture();
  Serial.println("debug pic=\"" + pic + "\"");
#endif

#ifdef REGISTER
  int registerResult;
  String registerToken;

  do {
    // TODO load via QR code (take picture, scan, parse text)
#ifdef SCAN_QR
    registerToken = scanned_data->registerToken;
#else
    registerToken = "A1B2C3"
#endif
    registerResult = registerDevice(registerToken);
  } while (registerResult != 200);
#endif

#ifdef SCAN_QR
  free(scanned_data);

  // Restart camera in color mode
  esp_camera_deinit();
  init_camera(false);
#endif
  
  execute(onEvent);
}

void loop() {
  // Loops automatically on setup
}

void onEvent(void) {
  Serial.println("onEvent!");
  String pic = take_picture();
  Serial.println("pic=\"" + pic + "\"");
  if (pic.length()) {
    sendEvent(pic);
  }
}
