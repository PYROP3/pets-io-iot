#ifndef STATE_MACHINE_H

#define STATE_MACHINE_H

#include "ultrasound.h"

#define POOLING_SLEEP_MS 2000
#define TRIGGER_SLEEP_MS 20000

#define ABS(x) x & 0x7fffffff

#ifdef DEBUG_STATES
#define DEBUG
#endif

void execute(void (*on_trigger)(void)) {
  int threshold_dist;
  boolean alive = true;

  // Wait for 10 seconds
  delay(10000);

  while (alive) {
    while (getAvgReading() > ULTRASOUND_THRESHOLD) {
#ifdef DEBUG
      Serial.printf("[OUT/LOW] Sleeping...\n");
#endif
      delay(POOLING_SLEEP_MS);
    }
    
#ifdef DEBUG
    Serial.printf("Detected trigger\n");
#endif

    // TODO pre-scan picture to check if trigger is false alarm
    on_trigger();

    delay(TRIGGER_SLEEP_MS);
    
    while (getAvgReading() < ULTRASOUND_THRESHOLD) {
#ifdef DEBUG
      Serial.printf("[OUT/HIGH] Sleeping...\n");
#endif
      delay(POOLING_SLEEP_MS);
    }
  }
}

#ifdef DEBUG
#undef DEBUG
#endif

#endif
