#include <BOE180_ST77916.h>

// Pin definitions for ESP32-S3 QSPI mode
#define TFT_SCLK 12
#define TFT_IO0  15
#define TFT_IO1  16
#define TFT_IO2  17
#define TFT_IO3  18
#define TFT_RST  4
#define TFT_CS   6
#define TFT_BL   7

BOE180_ST77916 tft(TFT_SCLK, TFT_IO0, TFT_IO1, TFT_IO2, TFT_IO3, TFT_RST, TFT_CS, TFT_BL);

void setup() {
  Serial.begin(115200);

  if (!tft.begin(20000000)) {
    Serial.println("BOE180_ST77916 initialization failed!");
    while (1) delay(1000);
  }

  // Primary color check
  tft.fillScreen(BOE180_RED);
  delay(600);
  tft.fillScreen(BOE180_GREEN);
  delay(600);
  tft.fillScreen(BOE180_BLUE);
  delay(600);

  // Draw sample round gauge
  tft.fillScreen(BOE180_BLACK);
  tft.drawCircle(180, 180, 178, BOE180_WHITE);
  tft.drawCircle(180, 180, 172, BOE180_CYAN);

  // Draw 12 clock tick marks around the perimeter
  for (int angle = 0; angle < 360; angle += 30) {
    float rad = angle * DEG_TO_RAD;
    int x0 = 180 + cos(rad) * 155;
    int y0 = 180 + sin(rad) * 155;
    int x1 = 180 + cos(rad) * 168;
    int y1 = 180 + sin(rad) * 168;
    tft.drawLine(x0, y0, x1, y1, BOE180_YELLOW);
  }

  tft.setCursor(82, 155);
  tft.setTextColor(BOE180_GREEN);
  tft.setTextSize(3);
  tft.println("BOE 1.8\"");

  tft.setCursor(92, 190);
  tft.setTextColor(BOE180_WHITE);
  tft.setTextSize(2);
  tft.println("QSPI 360x360");
}

void loop() {
  // Blink center dot every second
  tft.fillCircle(180, 230, 8, BOE180_RED);
  delay(500);
  tft.fillCircle(180, 230, 8, BOE180_BLACK);
  delay(500);
}
