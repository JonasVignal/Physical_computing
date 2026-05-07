// -----------------
//     ROBOT (4WD)
// -----------------
#include <esp_now.h>
#include <WiFi.h>
#include <ESP32Servo.h>

// Wheel pins
#define FL_PIN 27
#define FR_PIN 26
#define BL_PIN 21
#define BR_PIN 18

Servo fl, fr, bl, br;

// 🔧 TRIM THESE until robot stays still at rest
int trimFL = 5.8;
int trimFR = 0;
int trimBL = 0;
int trimBR = 0;

typedef struct {
  int x;
  int y;
} Data;

Data data;

// -----------------
int clamp(int v) {
  if (v < -100) return -100;
  if (v > 100)  return 100;
  return v;
}

// convert speed → servo signal
int toServo(int v) {
  return 90 + v * 0.4;  // adjust power here
}

// -----------------
void onReceive(const esp_now_recv_info *info, const uint8_t *incomingData, int len) {
  memcpy(&data, incomingData, sizeof(data));
  Serial.println("DATA RECEIVED");
  int forward = clamp(data.y);
  int turn    = clamp(data.x);

  // tank drive mix
  int left  = clamp(forward + turn);
  int right = clamp(forward - turn);

  // apply to wheels
  fl.write(toServo(left)  + trimFL);
  bl.write(toServo(left)  + trimBL);

  fr.write(toServo(right) + trimFR);
  br.write(toServo(right) + trimBR);
  Serial.printf("BL servo value = %d\n", toServo(left) + trimBL);
  Serial.printf("F:%d T:%d | L:%d R:%d\n", forward, turn, left, right);
}

// -----------------
void setup() {
  Serial.begin(115200);

  fl.attach(FL_PIN);
  fr.attach(FR_PIN);
  bl.attach(BL_PIN);
  br.attach(BR_PIN);

  // STOP all wheels
  fl.write(91.2 + trimFL);
  fr.write(90 + trimFR);
  bl.write(90 + trimBL);
  br.write(90 + trimBR);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_register_recv_cb(onReceive);
}

void loop() {}