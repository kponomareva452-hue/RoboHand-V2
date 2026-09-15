/*
  RoboHand V2 — glove controller (ESP-NOW sender)
  Board: ESP32 on the glove
  Serial: 115200

  Pins from the controller schematic (drawing order J1→J5):
    J1 GPIO39, J2 GPIO34, J3 GPIO35, J4 GPIO32, J5 GPIO33
  Default finger map (edit if a finger is swapped on the glove):
    Pinky=39  Ring=34  Middle=35  Index=32  Thumb=33
*/

#include <WiFi.h>
#include <esp_now.h>

uint8_t receiverMac[] = {0xD4, 0xE9, 0xF4, 0xC4, 0x0E, 0xD4}; // hand ESP32 MAC

const int PIN[5] = {33, 32, 35, 34, 39}; // Thumb, Index, Middle, Ring, Pinky

typedef struct __attribute__((packed)) {
  uint16_t raw[5];
} GloveData;

GloveData data;

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, receiverMac, 6);
  peer.channel = 0;
  peer.encrypt = false;
  esp_now_add_peer(&peer);

  Serial.println("Controller ready");
  Serial.print("This MAC: ");
  Serial.println(WiFi.macAddress());
}

void loop() {
  for (int i = 0; i < 5; i++) {
    data.raw[i] = analogRead(PIN[i]);
  }
  esp_now_send(receiverMac, (uint8_t*)&data, sizeof(data));

  static uint32_t lastPrint = 0;
  if (millis() - lastPrint > 200) {
    lastPrint = millis();
    Serial.printf("T:%4u I:%4u M:%4u R:%4u P:%4u\n",
                  data.raw[0], data.raw[1], data.raw[2], data.raw[3], data.raw[4]);
  }
  delay(20);
}
