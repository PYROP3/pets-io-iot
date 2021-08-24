#ifndef STATE_MACHINE_H

#define STATE_MACHINE_H

#include "ultrasound.h"

#define POOLING_SLEEP_MS 2000
#define TRIGGER_SLEEP_MS 20000

#define ABS(x) x & 0x7fffffff

#ifdef DEBUG_STATES
#define DEBUG
#endif

enum machine_state {
  state_sleeping_low = 0,
  state_trigger,
  state_sleeping_high
};

static unsigned long last_loop_millis = millis();
static machine_state prev_state = state_sleeping_low;

void execute(void (*on_trigger)(void)) {
  machine_state next_state = prev_state;
  unsigned long now_millis = millis();
#ifdef DEBUG
  Serial.printf("Loop : %d @ %lu (%lu)\n", prev_state, now_millis, last_loop_millis);
#endif
  
  switch (prev_state) {
    case state_sleeping_low:
      if (now_millis - last_loop_millis >= POOLING_SLEEP_MS) {
        if (getAvgReading() > ULTRASOUND_THRESHOLD) {
#ifdef DEBUG
          Serial.printf("[OUT/LOW] Sleeping...\n");
#endif
        } else {
#ifdef DEBUG
          Serial.printf("[OUT/LOW] Change state\n");
#endif
          next_state = state_trigger;
          last_loop_millis = now_millis;
        }
      }
      break;
    case state_trigger:
#ifdef DEBUG
      Serial.printf("Detected trigger\n");
#endif
      on_trigger();
      next_state = state_sleeping_high;
    break;
    case state_sleeping_high:
      if (now_millis - last_loop_millis >= POOLING_SLEEP_MS) {
        if (getAvgReading() < ULTRASOUND_THRESHOLD) {
#ifdef DEBUG
          Serial.printf("[OUT/HIGH] Sleeping...\n");
#endif
        } else {
#ifdef DEBUG
          Serial.printf("[OUT/HIGH] Change state\n");
#endif
          next_state = state_sleeping_low;
          last_loop_millis = now_millis;
        }
      }
      break;
  }
  prev_state = next_state;
}

#ifdef DEBUG
#undef DEBUG
#endif

#endif
