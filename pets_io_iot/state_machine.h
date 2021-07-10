#ifndef STATE_MACHINE_H

#define STATE_MACHINE_H

#include "ultrasound.h"

#define POOLING_SLEEP_MS 2000
#define TRIGGER_SLEEP_MS 5000

#define ABS(x) x & 0x7fffffff

void execute(void (*on_trigger)(void)) {
  int threshold_dist;
  boolean alive = true;

  // Wait for 5 seconds
  delay(5000);

  threshold_dist = getAvgReading();
  #ifdef DEBUG
  Serial.printf("Set threshold_dist to %d", threshold_dist);
  #endif

  while (alive) {
    while (getAvgReading() > threshold_dist - ULTRASOUND_THRESHOLD) {
      #ifdef DEBUG
      Serial.printf("[OUT/LOW] Sleeping...");
      #endif
      delay(POOLING_SLEEP_MS);
    }
    
    #ifdef DEBUG
    Serial.printf("Detected trigger %d", threshold_dist);
    #endif

    // TODO pre-scan picture to check if trigger is false alarm
    on_trigger();

    delay(TRIGGER_SLEEP_MS);
    
//    while (getAvgReading() > threshold_dist - ULTRASOUND_THRESHOLD) {
//      #ifdef DEBUG
//      Serial.printf("[OUT/LOW] Sleeping...");
//      #endif
//      delay(POOLING_SLEEP_MS);
//    }
//
//    #ifdef DEBUG
//    Serial.printf("Detected possible trigger", threshold_dist);
//    #endif
//
//    while (getAvgReading() < threshold_dist - ULTRASOUND_THRESHOLD) {
//      #ifdef DEBUG
//      Serial.printf("[OUT/HIGH] Sleeping...");
//      #endif
//      delay(POOLING_SLEEP_MS);
//    }
//
//    // TODO pre-scan picture to check if trigger is false alarm
//    on_trigger();
//
//    while (getAvgReading() > threshold_dist - ULTRASOUND_THRESHOLD) {
//      #ifdef DEBUG
//      Serial.printf("[IN/LOW] Sleeping...");
//      #endif
//      delay(POOLING_SLEEP_MS);
//    }
//
//    #ifdef DEBUG
//    Serial.printf("Detected exit start", threshold_dist);
//    #endif
//
//    while (getAvgReading() < threshold_dist - ULTRASOUND_THRESHOLD) {
//      #ifdef DEBUG
//      Serial.printf("[IN/HIGH] Sleeping...");
//      #endif
//      delay(POOLING_SLEEP_MS);
//    }
//
//    #ifdef DEBUG
//    Serial.printf("Detected exit end", threshold_dist);
//    #endif
//
//    delay(TRIGGER_SLEEP_MS);
  }
}

#endif
