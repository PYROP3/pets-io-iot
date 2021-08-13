#ifndef ULTRASOUND_H

#define ULTRASOUND_H

#define TRIG_PIN 12
#define ECHO_PIN 13
#define ULTRASOUND_THRESHOLD 20

void init_ultrasound() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

int getReading() {
  long duration;
  
  // The PING))) is triggered by a HIGH pulse of 2 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // The same pin is used to read the signal from the PING))): a HIGH pulse
  // whose duration is the time (in microseconds) from the sending of the ping
  // to the reception of its echo off of an object.
  duration = pulseIn(ECHO_PIN, HIGH);

  // convert the time into a distance// The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the object we
  // take half of the distance travelled.
  return duration / 59;
}

int getAvgReading() {
  return (getReading() + getReading() + getReading()) / 3;
}

#endif
