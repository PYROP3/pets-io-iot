#ifndef CAMERA_H

#define CAMERA_H
#define CAMERA_MODEL_AI_THINKER

//#define PRINT_B64
#define CAM_HD

#include "camera_pins.h"

// TODO move b64 stuff to utils
#define BASE64_ENCODING "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"

#ifdef DEBUG_CAM
#define DEBUG
#endif

String bytesToB64(uint8_t *bytes, int len) {
  String result = "";
  int i = 0;

#ifdef PRINT_B64
  Serial.printf("bytesToB64 in %d\n", len);
#endif
  
  for (i = 0; i < len / 3; i++) {
    result += BASE64_ENCODING[(bytes[3*i] & 0xFC) >> 2];
    result += BASE64_ENCODING[((bytes[3*i] & 0x03) << 4) | ((bytes[3*i+1] & 0xF0) >> 4)];
    result += BASE64_ENCODING[((bytes[3*i+1] & 0x0F) << 2) | ((bytes[3*i+2] & 0xC0) >> 6)];
    result += BASE64_ENCODING[bytes[3*i+2] & 0x3F];
#ifdef PRINT_B64
  Serial.printf("%c%c%c%c", 
      BASE64_ENCODING[(bytes[3*i] & 0xFC) >> 2],
      BASE64_ENCODING[((bytes[3*i] & 0x03) << 4) | ((bytes[3*i+1] & 0xF0) >> 4)],
      BASE64_ENCODING[((bytes[3*i+1] & 0x0F) << 2) | ((bytes[3*i+2] & 0xC0) >> 6)],
      BASE64_ENCODING[bytes[3*i+2] & 0x3F]);
#endif
  }

  if (len % 3 == 2) {
    result += BASE64_ENCODING[(bytes[3*i] & 0xFC) >> 2];
    result += BASE64_ENCODING[((bytes[3*i] & 0x03) << 4) | ((bytes[3*i+1] & 0xF0) >> 4)];
    result += BASE64_ENCODING[(bytes[3*i+1] & 0x0F) << 2];
    result += "=";
#ifdef PRINT_B64
    Serial.printf("%c%c%c%c", 
      BASE64_ENCODING[(bytes[3*i] & 0xFC) >> 2],
      BASE64_ENCODING[((bytes[3*i] & 0x03) << 4) | ((bytes[3*i+1] & 0xF0) >> 4)],
      BASE64_ENCODING[((bytes[3*i+1] & 0x0F) << 2)],
      '=');
#endif
  } else if (len % 3 == 1) {
    result += BASE64_ENCODING[(bytes[3*i] & 0xFC) >> 2];
    result += BASE64_ENCODING[(bytes[3*i] & 0x03) << 4];
    result += "==";
#ifdef PRINT_B64
    Serial.printf("%c%c%c%c", 
      BASE64_ENCODING[(bytes[3*i] & 0xFC) >> 2],
      BASE64_ENCODING[((bytes[3*i] & 0x03) << 4)],
      '=',
      '=');
#endif
  }

#ifdef PRINT_B64
  Serial.println("");
#endif

  return result;
}

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
//  config.frame_size = FRAMESIZE_UXGA;
//  config.jpeg_quality = 10;
//  config.fb_count = 2;

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
    return err;
  }

  sensor_t * s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1); // flip it back
    s->set_brightness(s, 1); // up the brightness just a bit
    s->set_saturation(s, -2); // lower the saturation
  }
  // drop down frame size for higher initial frame rate
  s->set_framesize(s, FRAMESIZE_QVGA);

  Serial.println("Camera init success");

  return 0;
}

String take_picture() {
  camera_fb_t * fb = NULL;
  int64_t fr_start = esp_timer_get_time();

  fb = esp_camera_fb_get();
  if (!fb) {
      Serial.println("Camera capture failed");
      return "";
  }

  String pic = bytesToB64(fb->buf, fb->len);

  esp_camera_fb_return(fb);
  int64_t fr_end = esp_timer_get_time();

#ifdef DEBUG
  Serial.printf("JPG: %uB %ums\n", (uint32_t)(fb->len), (uint32_t)((fr_end - fr_start)/1000));
#endif

  return pic;
}

#ifdef DEBUG
#undef DEBUG
#endif

#endif
