#ifndef BLE_SERVER_H

#define BLE_SERVER_H

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID             "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define SSID_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define PASS_CHARACTERISTIC_UUID "995a473b-498e-43d0-9be3-7f3c37ba005f"
#define TOKN_CHARACTERISTIC_UUID "70461f4d-4142-4135-9582-8d9a9fc50ea4"
#define STRM_CHARACTERISTIC_UUID "1a014454-e990-438b-91f5-fa11173ca7b0"
#define STATUS_CON_CHARACTERISTIC_UUID "55246fe7-1105-4395-a472-f26201b3d75c"
#define STATUS_CAM_CHARACTERISTIC_UUID "dd1e4f59-d6c5-4d65-a2c5-ed2b6d2570dd"
#define STATUS_REG_CHARACTERISTIC_UUID "2f6255b5-c4fa-403c-9b18-11f40ffee21b"

#define PIO_DEVICE_ID "PIOFB00001"

#ifdef DEBUG_BLE
#define DEBUG
#endif

BLEServer *pServer;
BLEService *pService;
BLECharacteristic *pSSIDCharacteristic, *pPassCharacteristic, *pToknCharacteristic, *pStrmCharacteristic;
BLECharacteristic *pStatusConnectionCharacteristic, *pStatusCameraCharacteristic, *pStatusRegisterCharacteristic;

void setConnectionStatus(String connectionStatus) {
  pStatusConnectionCharacteristic->setValue(connectionStatus.c_str());
}

void setCameraStatus(String cameraStatus) {
  pStatusCameraCharacteristic->setValue(cameraStatus.c_str());
}

void setRegisterStatus(String registerStatus) {
  pStatusRegisterCharacteristic->setValue(registerStatus.c_str());
}

void setStreamServer(String streamServer) {
  pStrmCharacteristic->setValue(streamServer.c_str());
}

void init_ble() {
#ifdef DEBUG
  Serial.println("Starting BLE work!");
#endif

  BLEDevice::init("Pets.io_" PIO_DEVICE_ID);
  pServer = BLEDevice::createServer();
  pService = pServer->createService(SERVICE_UUID);
  pSSIDCharacteristic = pService->createCharacteristic(
                                         SSID_CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  pPassCharacteristic = pService->createCharacteristic(
                                         PASS_CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  pToknCharacteristic = pService->createCharacteristic(
                                         TOKN_CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  pStrmCharacteristic = pService->createCharacteristic(
                                         STRM_CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  pStatusConnectionCharacteristic = pService->createCharacteristic(
                                         STATUS_CON_CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  pStatusCameraCharacteristic = pService->createCharacteristic(
                                         STATUS_CAM_CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  pStatusRegisterCharacteristic = pService->createCharacteristic(
                                         STATUS_REG_CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pService->start();
  
  setConnectionStatus("not initialized");
  setCameraStatus("not initialized");
  setRegisterStatus("not initialized");

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  
#ifdef DEBUG
  Serial.println("Characteristics defined!");
#endif
}

String getSSID() {
  // TODO apply cryptography to protect sensitive data
  return pSSIDCharacteristic->getValue().c_str();
}

String getPass() {
  // TODO apply cryptography to protect sensitive data
  return pPassCharacteristic->getValue().c_str();
}

String getToken() {
  return pToknCharacteristic->getValue().c_str(); // "A1B2C3"
}

#ifdef DEBUG
#undef DEBUG
#endif

#endif
