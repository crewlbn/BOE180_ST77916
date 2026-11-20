#include <BOE180_ST77916.h>

// Control pins (connect SCL -> hardware SCK, IO0 -> hardware MOSI, IO2 & IO3 -> 3.3V)
#if defined(ESP8266)
  #define TFT_RST D1
  #define TFT_CS  D2
  #define TFT_BL  D3
#else
  #define TFT_RST 8
  #define TFT_CS  10
  #define TFT_BL  9
#endif

BOE180_ST77916 tft(TFT_RST, TFT_CS, TFT_BL);

void setup() {
  Serial.begin(115200);

  if (!tft.begin(20000000)) {
    Serial.println("BOE180_ST77916 initialization failed!");
    while (1) delay(1000);
  }

  tft.fillScreen(BOE180_BLACK);
  tft.drawCircle(180, 180, 175, BOE180_YELLOW);
  tft.drawCircle(180, 180, 168, BOE180_WHITE);

  tft.setCursor(88, 165);
  tft.setTextColor(BOE180_CYAN);
  tft.setTextSize(3);
  tft.println("1-SPI OK!");
}

void loop() {
  delay(1000);
}
