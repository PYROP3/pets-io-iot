#ifndef CAMERA_H

#define CAMERA_H
#define CAMERA_MODEL_AI_THINKER

#define CAM_HD

#include "camera_pins.h"

#ifdef DEBUG_CAM
#define DEBUG
#endif

int init_camera(boolean grayscale) {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = grayscale ? PIXFORMAT_GRAYSCALE : PIXFORMAT_JPEG;
  
  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.

#ifdef CAM_HD
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
#else
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
#endif

  // camera init
  esp_err_t err;
  int count = 5;

  do {
    err = esp_camera_init(&config);
    if (err != ESP_OK) {
      Serial.printf("Camera init failed with error 0x%x", err);
      delay(500);
    }
  } while (err != ESP_OK && count-- > 0);

  if (err != ESP_OK) {
    setCameraStatus("error");
    return err;
  }

  sensor_t * s = esp_camera_sensor_get();
  s->set_vflip(s, 1); // flip it back
  // drop down frame size for higher initial frame rate
  //s->set_framesize(s, FRAMESIZE_QVGA);

  Serial.println("Camera init success");
  setCameraStatus("success");

  return 0;
}

#ifdef DEBUG
#undef DEBUG
#endif

#endif
