/*
  RoboHand V2 — per-finger servo values
  ESP32 + PCA9685 (0x40) + 5x MG90S

  Edit the numbers in FINGER VALUES, upload, watch the hand.
  Serial Monitor: 115200

  V2 mounting: Thumb, Ring, Pinky reversed in hardware.
  Put the pulse that actually OPENS that finger in openPulse,
  and the pulse that actually CLOSES it in closePulse.
  Do not power servos from the ESP32.
*/

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);

enum { THUMB = 0, INDEX = 1, MIDDLE = 2, RING = 3, PINKY = 4 };
const int SERVO_COUNT = 5;

int openPulse[SERVO_COUNT] = {
  150, 150, 150, 150, 150
};

int closePulse[SERVO_COUNT] = {
  500, 500, 500, 500, 500
};

const char* NAME[SERVO_COUNT] = {
  "Thumb", "Index", "Middle", "Ring", "Pinky"
};

const int CH[SERVO_COUNT] = {0, 1, 2, 3, 4};

const int TEST_MODE = 2;
const int CYCLE_MS  = 2000;

const int PULSE_MIN = 90;
const int PULSE_MAX = 560;

int clampPulse(int v) {
  if (v < PULSE_MIN) return PULSE_MIN;
  if (v > PULSE_MAX) return PULSE_MAX;
  return v;
}

void setFinger(int finger, bool closed) {
  int pulse = closed ? closePulse[finger] : openPulse[finger];
  pulse = clampPulse(pulse);
  pwm.setPWM(CH[finger], 0, pulse);
}

void setAll(bool closed) {
  for (int i = 0; i < SERVO_COUNT; i++) {
    setFinger(i, closed);
  }
}

void printValues() {
  Serial.println();
  Serial.println("finger   ch   OPEN  CLOSE");
  for (int i = 0; i < SERVO_COUNT; i++) {
    Serial.printf("%-7s  %d   %4d  %4d\n",
                  NAME[i], CH[i], openPulse[i], closePulse[i]);
  }
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Wire.begin(21, 22);
  pwm.begin();
  pwm.setPWMFreq(50);
  delay(200);
  printValues();
  if (TEST_MODE == 1) {
    Serial.println("Holding CLOSE");
    setAll(true);
  } else {
    Serial.println("Holding OPEN");
    setAll(false);
  }
}

void loop() {
  if (TEST_MODE != 2) return;
  Serial.println("CLOSE");
  setAll(true);
  delay(CYCLE_MS);
  Serial.println("OPEN");
  setAll(false);
  delay(CYCLE_MS);
}
