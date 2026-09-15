/*
  RoboHand V2 — hand receiver (ESP-NOW + PCA9685)
  Board: ESP32 inside the forearm
  Servos: 5x MG90S on PCA9685 channels 0-4
  Serial: 115200

  Edit openPulse / closePulse after calibration.
  invert[] = true means the horn is mounted the other way
  (Thumb, Ring, Pinky on V2).
*/

#include <WiFi.h>
#include <esp_now.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);

const int SERVO_COUNT = 5;
const char* NAME[SERVO_COUNT] = {"Thumb", "Index", "Middle", "Ring", "Pinky"};
const int CH[SERVO_COUNT] = {0, 1, 2, 3, 4};

int openPulse[SERVO_COUNT]  = {150, 150, 150, 150, 150};
int closePulse[SERVO_COUNT] = {500, 500, 500, 500, 500};

int joyOpen[SERVO_COUNT]  = {1960, 1960, 1960, 1960, 1960};
int joyClose[SERVO_COUNT] = {800, 800, 800, 800, 800};

bool invert[SERVO_COUNT] = {true, false, false, true, true};

const int PULSE_MIN = 90;
const int PULSE_MAX = 560;

typedef struct __attribute__((packed)) {
  uint16_t raw[5];
} GloveData;

volatile GloveData incoming;
volatile bool gotPacket = false;

int clampPulse(int v) {
  if (v < PULSE_MIN) return PULSE_MIN;
  if (v > PULSE_MAX) return PULSE_MAX;
  return v;
}

int mapConstrained(int x, int inA, int inB, int outA, int outB) {
  if (inA == inB) return outA;
  long v = (long)(x - inA) * (outB - outA) / (inB - inA) + outA;
  if (outA < outB) {
    if (v < outA) v = outA;
    if (v > outB) v = outB;
  } else {
    if (v > outA) v = outA;
    if (v < outB) v = outB;
  }
  return (int)v;
}

void applyFinger(int i, uint16_t raw) {
  int outOpen  = invert[i] ? closePulse[i] : openPulse[i];
  int outClose = invert[i] ? openPulse[i]  : closePulse[i];
  int pulse = mapConstrained(raw, joyOpen[i], joyClose[i], outOpen, outClose);
  pwm.setPWM(CH[i], 0, clampPulse(pulse));
}

#if defined(ESP_NOW_MAX_DATA_LEN)
void onReceive(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
#else
void onReceive(const uint8_t *mac, const uint8_t *data, int len) {
#endif
  if (len < (int)sizeof(GloveData)) return;
  memcpy((void*)&incoming, data, sizeof(GloveData));
  gotPacket = true;
}

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  pwm.begin();
  pwm.setPWMFreq(50);
  delay(200);

  for (int i = 0; i < SERVO_COUNT; i++) {
    pwm.setPWM(CH[i], 0, clampPulse(openPulse[i]));
  }

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  Serial.print("Hand MAC (paste into sender): ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }
  esp_now_register_recv_cb(onReceive);
  Serial.println("Hand receiver ready");
}

void loop() {
  if (!gotPacket) return;
  gotPacket = false;
  GloveData d;
  noInterrupts();
  d = incoming;
  interrupts();
  for (int i = 0; i < SERVO_COUNT; i++) {
    applyFinger(i, d.raw[i]);
  }
}
