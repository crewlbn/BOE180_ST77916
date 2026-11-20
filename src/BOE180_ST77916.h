#pragma once

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <SPI.h>

#if defined(CONFIG_IDF_TARGET_ESP32S3) || defined(CONFIG_IDF_TARGET_ESP32S2) || defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32C6)
  #define BOE180_HAS_QSPI 1
  #include "driver/spi_master.h"
#else
  #define BOE180_HAS_QSPI 0
#endif

// Standard RGB565 color definitions
#define BOE180_BLACK   0x0000
#define BOE180_WHITE   0xFFFF
#define BOE180_RED     0xF800
#define BOE180_GREEN   0x07E0
#define BOE180_BLUE    0x001F
#define BOE180_YELLOW  0xFFE0
#define BOE180_CYAN    0x07FF
#define BOE180_MAGENTA 0xF81F

class BOE180_ST77916 : public Adafruit_GFX {
public:
  static const int16_t TFT_WIDTH  = 360;
  static const int16_t TFT_HEIGHT = 360;

  // Hardware QSPI constructor (ESP32-S3 / S2 / C3 / C6)
  BOE180_ST77916(int8_t sclk, int8_t io0, int8_t io1, int8_t io2, int8_t io3,
                 int8_t rst, int8_t cs, int8_t bl = -1);

  // Standard 1-wire SPI constructor (ESP8266, classic ESP32, Arduino AVR)
  BOE180_ST77916(int8_t rst, int8_t cs, int8_t bl = -1, SPIClass *spi = &SPI);

  bool begin(uint32_t clockHz = 20000000);
  void setRotation(uint8_t r) override;
  void invertDisplay(bool i) override;
  void setBacklight(bool on);

  // Core Adafruit_GFX drawing primitives
  void drawPixel(int16_t x, int16_t y, uint16_t color) override;
  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override;
  void fillScreen(uint16_t color) override;
  void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) override;
  void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) override;

private:
  int8_t _sclk, _io0, _io1, _io2, _io3, _rst, _cs, _bl;
  bool _useQspi;
  SPIClass *_arduinoSpi;
  uint32_t _clockHz;

#if BOE180_HAS_QSPI
  spi_device_handle_t _qspiDev = nullptr;
#endif

  void initPanel();
  void setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
  void writeCmd(uint8_t cmd);
  void writeCmdData(uint8_t cmd, uint8_t data);
  void writeCmdBytes(uint8_t cmd, const uint8_t *data, size_t len);
  void pushColorBlock(uint16_t color, uint32_t count);
};