// -----------------
//   CONTROLLER
// -----------------
#include <esp_now.h>
#include <WiFi.h>

// Joystick pins
#define JOY_X 34
#define JOY_Y 35

// Replace with your ROBOT ESP32 MAC
uint8_t receiverMac[] = {0x88,0x13,0xBF,0x08,0x60,0x2C};

typedef struct {
  int x; // turn
  int y; // forward
} Data;

Data data;

esp_now_peer_info_t peerInfo;

// ---- Calibration values ----
int centerX = 2048;
int centerY = 2048;

// ---- Read joystick axis with calibration ----
int readAxis(int pin, int center) {
  int raw = analogRead(pin);
  int centered = raw - center;

  // Deadzone
  if (abs(centered) < 150) centered = 0;

  // Map to -100 → 100
  int mapped = map(centered, -2048, 2048, -100, 100);

  // Clamp
  if (mapped > 100) mapped = 100;
  if (mapped < -100) mapped = -100;

  return mapped;
}

void setup() {
  Serial.begin(115200);

  // -------------------------
  // AUTO‑CALIBRATE JOYSTICK
  // -------------------------
  long sumX = 0, sumY = 0;
  for (int i = 0; i < 100; i++) {
    sumX += analogRead(JOY_X);
    sumY += analogRead(JOY_Y);
    delay(5);
  }
  centerX = sumX / 100;
  centerY = sumY / 100;

  Serial.printf("Joystick Center → X=%d  Y=%d\n", centerX, centerY);

  // -------------------------
  // ESP‑NOW SETUP
  // -------------------------
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  memcpy(peerInfo.peer_addr, receiverMac, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  esp_now_add_peer(&peerInfo);
}

void loop() {
  data.x = readAxis(JOY_X, centerX);
  data.y = readAxis(JOY_Y, centerY);

  esp_now_send(receiverMac, (uint8_t *)&data, sizeof(data));

  Serial.printf("X:%d  Y:%d\n", data.x, data.y);

  delay(20); 
  }
