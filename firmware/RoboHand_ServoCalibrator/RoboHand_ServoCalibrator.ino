/*
  RoboHand V2 — live servo calibrator
  Board: ESP32 on the hand (PCA9685 + 5x MG90S)
  Serial Monitor: 115200, Newline

  Commands: 0-4 select, t i m r p, 150 set pulse,
  + - ++ --, o save OPEN, c save CLOSE, open close,
  allopen allclose mid print help
*/

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);

const int SERVO_COUNT = 5;
const char* NAMES[SERVO_COUNT] = {"Thumb", "Index", "Middle", "Ring", "Pinky"};
const int PULSE_ABS_MIN = 90;
const int PULSE_ABS_MAX = 560;
const int START_PULSE   = 300;

int servoOpen[SERVO_COUNT]  = {300, 300, 300, 300, 300};
int servoClose[SERVO_COUNT] = {300, 300, 300, 300, 300};
int currentPulse[SERVO_COUNT];
int selected = 0;

int clampPulse(int v) {
  if (v < PULSE_ABS_MIN) return PULSE_ABS_MIN;
  if (v > PULSE_ABS_MAX) return PULSE_ABS_MAX;
  return v;
}

void moveServo(int ch, int pulse) {
  pulse = clampPulse(pulse);
  currentPulse[ch] = pulse;
  pwm.setPWM(ch, 0, pulse);
}

void printStatus() {
  Serial.println();
  Serial.println(" ch  name     now   OPEN  CLOSE");
  for (int i = 0; i < SERVO_COUNT; i++) {
    Serial.printf(" %s%d  %-7s %4d  %4d  %4d\n",
                  (i == selected) ? ">" : " ",
                  i, NAMES[i], currentPulse[i], servoOpen[i], servoClose[i]);
  }
}

void printPasteBlock() {
  Serial.print("int openPulse[5]  = {");
  for (int i = 0; i < SERVO_COUNT; i++) {
    Serial.print(servoOpen[i]);
    if (i < SERVO_COUNT - 1) Serial.print(", ");
  }
  Serial.println("};");
  Serial.print("int closePulse[5] = {");
  for (int i = 0; i < SERVO_COUNT; i++) {
    Serial.print(servoClose[i]);
    if (i < SERVO_COUNT - 1) Serial.print(", ");
  }
  Serial.println("};");
}

void printHelp() {
  Serial.println("0-4 / t i m r p  select");
  Serial.println("150  + - ++ --  o c  open close  allopen allclose mid print");
}

void setup() {
  Serial.begin(115200);
  delay(400);
  Wire.begin(21, 22);
  pwm.begin();
  pwm.setPWMFreq(50);
  delay(200);
  for (int i = 0; i < SERVO_COUNT; i++) {
    moveServo(i, START_PULSE);
    delay(80);
  }
  printHelp();
  printStatus();
}

void handleLine(String line) {
  line.trim();
  line.toLowerCase();
  if (line.length() == 0) return;
  if (line.length() == 1 && line[0] >= '0' && line[0] <= '4') {
    selected = line[0] - '0';
    printStatus();
    return;
  }
  if (line == "t" || line == "thumb") selected = 0;
  else if (line == "i" || line == "index") selected = 1;
  else if (line == "m" || line == "middle") selected = 2;
  else if (line == "r" || line == "ring") selected = 3;
  else if (line == "p" || line == "pinky") selected = 4;
  if (line == "t" || line == "i" || line == "m" || line == "r" || line == "p" ||
      line == "thumb" || line == "index" || line == "middle" || line == "ring" || line == "pinky") {
    printStatus();
    return;
  }
  if (line == "+") { moveServo(selected, currentPulse[selected] + 5); printStatus(); return; }
  if (line == "-") { moveServo(selected, currentPulse[selected] - 5); printStatus(); return; }
  if (line == "++") { moveServo(selected, currentPulse[selected] + 20); printStatus(); return; }
  if (line == "--") { moveServo(selected, currentPulse[selected] - 20); printStatus(); return; }
  if (line == "o") { servoOpen[selected] = currentPulse[selected]; Serial.println("saved OPEN"); return; }
  if (line == "c") { servoClose[selected] = currentPulse[selected]; Serial.println("saved CLOSE"); return; }
  if (line == "open") { moveServo(selected, servoOpen[selected]); printStatus(); return; }
  if (line == "close") { moveServo(selected, servoClose[selected]); printStatus(); return; }
  if (line == "allopen") { for (int i = 0; i < SERVO_COUNT; i++) moveServo(i, servoOpen[i]); printStatus(); return; }
  if (line == "allclose") { for (int i = 0; i < SERVO_COUNT; i++) moveServo(i, servoClose[i]); printStatus(); return; }
  if (line == "mid") { for (int i = 0; i < SERVO_COUNT; i++) moveServo(i, START_PULSE); printStatus(); return; }
  if (line == "print") { printPasteBlock(); return; }
  if (line == "help" || line == "?") { printHelp(); printStatus(); return; }
  bool numeric = true;
  for (unsigned i = 0; i < line.length(); i++) if (!isDigit(line[i])) numeric = false;
  if (numeric) { moveServo(selected, line.toInt()); printStatus(); return; }
  Serial.println("Unknown. Type help");
}

void loop() {
  if (Serial.available()) {
    String line = Serial.readStringUntil('\n');
    handleLine(line);
  }
}
