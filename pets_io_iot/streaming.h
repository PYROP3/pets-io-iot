#ifndef STREAMING_H

#define STREAMING_H

#include "esp_camera.h"
#include <WiFi.h>
#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "fb_gfx.h"
#include "soc/soc.h" //disable brownout problems
#include "soc/rtc_cntl_reg.h"  //disable brownout problems
#include "esp_http_server.h"

#define PART_BOUNDARY "123456789000000000000987654321"

#ifdef DEBUG_STREAMING
#define DEBUG
#endif

static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
//static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace";
//static const char* _STREAM_CONTENT_TYPE = "text/html;boundary=--" PART_BOUNDARY;
//static const char* _STREAM_CONTENT_TYPE = "text/html;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

httpd_handle_t stream_httpd = NULL;

static esp_err_t stream_handler(httpd_req_t *req){
  camera_fb_t * fb = NULL;
  esp_err_t res = ESP_OK;
  size_t _jpg_buf_len = 0;
  uint8_t * _jpg_buf = NULL;
  char * part_buf[64];
  
  Serial.println("New stream handler");

  res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
  if(res != ESP_OK){
    return res;
  }

  while(true){
#ifdef DEBUG
      Serial.println("Stream: get frame");
#endif
    fb = esp_camera_fb_get();
    if (!fb) {
#ifdef DEBUG
      Serial.println("Camera capture failed");
#endif
      res = ESP_FAIL;
    } else {
      if(fb->width > 400){
        if(fb->format != PIXFORMAT_JPEG){
          bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
          esp_camera_fb_return(fb);
          fb = NULL;
          if(!jpeg_converted){
#ifdef DEBUG
            Serial.println("JPEG compression failed");
#endif
            res = ESP_FAIL;
          }
        } else {
          _jpg_buf_len = fb->len;
          _jpg_buf = fb->buf;
        }
      }
    }
    if(res == ESP_OK){
      size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);
      res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
#ifdef DEBUG
      Serial.printf("Send chunk 1: %d\n", res);
#endif
    }
    if(res == ESP_OK){
      res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
#ifdef DEBUG
      Serial.printf("Send chunk 2: %d\n", res);
#endif
    }
    if(res == ESP_OK){
      res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
#ifdef DEBUG
      Serial.printf("Send chunk 3: %d\n", res);
#endif
    }
    if(fb){
      esp_camera_fb_return(fb);
      fb = NULL;
      _jpg_buf = NULL;
    } else if(_jpg_buf){
      free(_jpg_buf);
      _jpg_buf = NULL;
    }
    if(res != ESP_OK){
      break;
    }
  }
  Serial.printf("Stream handler finished: %d (%s)\n", res, esp_err_to_name(res));
  return res;
}

void startCameraServer(){
  httpd_config_t conf = HTTPD_DEFAULT_CONFIG();
  conf.server_port = 80;

  httpd_uri_t index_uri = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = stream_handler,
    .user_ctx  = NULL
  };

#ifdef DEBUG
  Serial.printf("Starting web server on port: '%d'\n", conf.server_port);
#endif
  if (httpd_start(&stream_httpd, &conf) == ESP_OK) {
#ifdef DEBUG
  Serial.printf("Start OK!\n");
#endif
    httpd_register_uri_handler(stream_httpd, &index_uri);
#ifdef DEBUG
  Serial.printf("Register handler OK!\n");
#endif
    setStreamServer(WiFi.localIP().toString());
  } else {
#ifdef DEBUG
  Serial.printf("Start fail!\n");
#endif
  }
}

void setup_streaming() {
  Serial.println("Setting up stream");
  
  // Start streaming web server
  startCameraServer();
  Serial.print("Camera Stream Ready! Go to: http://");
  Serial.print(WiFi.localIP());
  Serial.println("");
}

#ifdef DEBUG
#undef DEBUG
#endif

#endif
