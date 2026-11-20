#include "BOE180_ST77916.h"

// Hardware QSPI constructor
BOE180_ST77916::BOE180_ST77916(int8_t sclk, int8_t io0, int8_t io1, int8_t io2, int8_t io3, int8_t rst, int8_t cs, int8_t bl) : Adafruit_GFX(TFT_WIDTH, TFT_HEIGHT), _sclk(sclk), _io0(io0), _io1(io1), _io2(io2), _io3(io3), _rst(rst), _cs(cs), _bl(bl), _useQspi(true), _arduinoSpi(nullptr) {}

// Standard 1-wire SPI constructor (ESP8266 / Arduino / classic ESP32)
BOE180_ST77916::BOE180_ST77916(int8_t rst, int8_t cs, int8_t bl, SPIClass *spi) : Adafruit_GFX(TFT_WIDTH, TFT_HEIGHT), _sclk(-1), _io0(-1), _io1(-1), _io2(-1), _io3(-1), _rst(rst), _cs(cs), _bl(bl), _useQspi(false), _arduinoSpi(spi) {}

bool BOE180_ST77916::begin(uint32_t clockHz) {
  _clockHz = clockHz;

  pinMode(_cs, OUTPUT);
  digitalWrite(_cs, HIGH);

  if (_bl >= 0) {
    pinMode(_bl, OUTPUT);
    digitalWrite(_bl, HIGH);
  }

  // Hardware reset sequence
  if (_rst >= 0) {
    pinMode(_rst, OUTPUT);
    digitalWrite(_rst, HIGH);
    delay(20);
    digitalWrite(_rst, LOW);
    delay(100);
    digitalWrite(_rst, HIGH);
    delay(120);
  }

#if BOE180_HAS_QSPI
  if (_useQspi) {
    spi_bus_config_t buscfg = {};
    buscfg.sclk_io_num = _sclk;
    buscfg.mosi_io_num = _io0;
    buscfg.miso_io_num = _io1;
    buscfg.quadwp_io_num = _io2;
    buscfg.quadhd_io_num = _io3;
    buscfg.max_transfer_sz = 360 * 40 * 2;
    buscfg.flags = SPICOMMON_BUSFLAG_MASTER | SPICOMMON_BUSFLAG_QUAD;

    esp_err_t err = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) return false;

    spi_device_interface_config_t devcfg = {};
    devcfg.clock_speed_hz = _clockHz;
    devcfg.mode = 0;
    devcfg.spics_io_num = -1;
    devcfg.queue_size = 4;
    devcfg.flags = SPI_DEVICE_HALFDUPLEX;

    if (spi_bus_add_device(SPI2_HOST, &devcfg, &_qspiDev) != ESP_OK) return false;
  } else
#endif
  {
    _useQspi = false;
    if (!_arduinoSpi) _arduinoSpi = &SPI;
    _arduinoSpi->begin();
  }

  initPanel();
  fillScreen(BOE180_BLACK);
  return true;
}

void BOE180_ST77916::setBacklight(bool on) {
  if (_bl >= 0) digitalWrite(_bl, on ? HIGH : LOW);
}

void BOE180_ST77916::writeCmd(uint8_t cmd) {
  digitalWrite(_cs, LOW);
#if BOE180_HAS_QSPI
  if (_useQspi) {
    uint8_t pkt[4] = {0x02, 0x00, cmd, 0x00};
    spi_transaction_t t = {};
    t.length = 32;
    t.tx_buffer = pkt;
    spi_device_polling_transmit(_qspiDev, &t);
    digitalWrite(_cs, HIGH);
    return;
  }
#endif
  _arduinoSpi->beginTransaction(SPISettings(_clockHz, MSBFIRST, SPI_MODE0));
  _arduinoSpi->transfer(0x02);
  _arduinoSpi->transfer(0x00);
  _arduinoSpi->transfer(cmd);
  _arduinoSpi->transfer(0x00);
  _arduinoSpi->endTransaction();
  digitalWrite(_cs, HIGH);
}

void BOE180_ST77916::writeCmdData(uint8_t cmd, uint8_t data) { writeCmdBytes(cmd, &data, 1); }

void BOE180_ST77916::writeCmdBytes(uint8_t cmd, const uint8_t *data, size_t len) {
  digitalWrite(_cs, LOW);
#if BOE180_HAS_QSPI
  if (_useQspi) {
    uint8_t pkt[4] = {0x02, 0x00, cmd, 0x00};
    spi_transaction_t t = {};
    t.length = 32;
    t.tx_buffer = pkt;
    spi_device_polling_transmit(_qspiDev, &t);

    if (data && len > 0) {
      spi_transaction_t td = {};
      td.length = len * 8;
      td.tx_buffer = data;
      spi_device_polling_transmit(_qspiDev, &td);
    }
    digitalWrite(_cs, HIGH);
    return;
  }
#endif
  _arduinoSpi->beginTransaction(SPISettings(_clockHz, MSBFIRST, SPI_MODE0));
  _arduinoSpi->transfer(0x02);
  _arduinoSpi->transfer(0x00);
  _arduinoSpi->transfer(cmd);
  _arduinoSpi->transfer(0x00);
  for (size_t i = 0; i < len; i++) {
    _arduinoSpi->transfer(data[i]);
  }
  _arduinoSpi->endTransaction();
  digitalWrite(_cs, HIGH);
}

// Vendor initialization sequence for BOE 1.8" (TI1LG1) with ST77916 driver
void BOE180_ST77916::initPanel() {
  writeCmdData(0xF0, 0x28);
  writeCmdData(0xF2, 0x28);
  writeCmdData(0x73, 0xF0);
  writeCmdData(0x76, 0x0F);
  writeCmdData(0x7C, 0xD1);
  writeCmdData(0x83, 0xE0);
  writeCmdData(0x84, 0x61);
  writeCmdData(0xF2, 0x82);
  writeCmdData(0xF0, 0x00);
  writeCmdData(0xF0, 0x01);
  writeCmdData(0xF1, 0x01);
  writeCmdData(0xB0, 0x52);
  writeCmdData(0xB1, 0x49);
  writeCmdData(0xB2, 0x24);
  writeCmdData(0xB3, 0x01);
  writeCmdData(0xB4, 0x66);
  writeCmdData(0xB5, 0x44);
  writeCmdData(0xB6, 0xC5);
  writeCmdData(0xB7, 0x40);
  writeCmdData(0xB8, 0x86);
  writeCmdData(0xB9, 0x15);
  writeCmdData(0xBA, 0x00);
  writeCmdData(0xBB, 0x08);
  writeCmdData(0xBC, 0x08);
  writeCmdData(0xBD, 0x00);
  writeCmdData(0xBE, 0x00);
  writeCmdData(0xBF, 0x07);
  writeCmdData(0xC0, 0x80);
  writeCmdData(0xC1, 0x10);
  writeCmdData(0xC2, 0x37);
  writeCmdData(0xC3, 0x80);
  writeCmdData(0xC4, 0x10);
  writeCmdData(0xC5, 0x37);
  writeCmdData(0xC6, 0xA9);
  writeCmdData(0xC7, 0x41);
  writeCmdData(0xC8, 0x01);
  writeCmdData(0xC9, 0xA9);
  writeCmdData(0xCA, 0x41);
  writeCmdData(0xCB, 0x01);
  writeCmdData(0xCC, 0x7F);
  writeCmdData(0xCD, 0x7F);
  writeCmdData(0xCE, 0xFF);
  writeCmdData(0xD0, 0x91);
  writeCmdData(0xD1, 0x68);
  writeCmdData(0xD2, 0x68);
  {
    const uint8_t d[2] = {0x00, 0xA5};
    writeCmdBytes(0xF5, d, 2);
  }
  writeCmdData(0xF1, 0x10);
  writeCmdData(0xF0, 0x00);
  writeCmdData(0xF0, 0x02);
  {
    const uint8_t d[14] = {0xF0, 0x0E, 0x14, 0x0B, 0x0B, 0x16, 0x3A, 0x44, 0x4E, 0x18, 0x14, 0x13, 0x2F, 0x35};
    writeCmdBytes(0xE0, d, 14);
  }
  {
    const uint8_t d[14] = {0xF0, 0x0D, 0x13, 0x0B, 0x0A, 0x16, 0x39, 0x43, 0x4E, 0x17, 0x13, 0x13, 0x2E, 0x34};
    writeCmdBytes(0xE1, d, 14);
  }
  writeCmdData(0xF0, 0x10);
  writeCmdData(0xF3, 0x10);
  writeCmdData(0xE0, 0x09);
  writeCmdData(0xE1, 0x00);
  writeCmdData(0xE2, 0x03);
  writeCmdData(0xE3, 0x00);
  writeCmdData(0xE4, 0xE0);
  writeCmdData(0xE5, 0x06);
  writeCmdData(0xE6, 0x21);
  writeCmdData(0xE7, 0x00);
  writeCmdData(0xE8, 0x05);
  writeCmdData(0xE9, 0x82);
  writeCmdData(0xEA, 0xDE);
  writeCmdData(0xEB, 0xC0);
  writeCmdData(0xEC, 0x40);
  writeCmdData(0xED, 0x84);
  writeCmdData(0xEE, 0xFF);
  writeCmdData(0xEF, 0x71);
  writeCmdData(0xF8, 0xFF);
  writeCmdData(0xF9, 0x50);
  writeCmdData(0xFA, 0xFF);
  writeCmdData(0xFB, 0xF3);
  writeCmdData(0xFC, 0x00);
  writeCmdData(0xFD, 0x00);
  writeCmdData(0xFE, 0x00);
  writeCmdData(0xFF, 0x00);
  writeCmdData(0x60, 0x42);
  writeCmdData(0x61, 0xDF);
  writeCmdData(0x62, 0x40);
  writeCmdData(0x63, 0x40);
  writeCmdData(0x64, 0x02);
  writeCmdData(0x65, 0x00);
  writeCmdData(0x66, 0x00);
  writeCmdData(0x67, 0x00);
  writeCmdData(0x68, 0x00);
  writeCmdData(0x69, 0x00);
  writeCmdData(0x6A, 0x00);
  writeCmdData(0x6B, 0x00);
  writeCmdData(0x70, 0x42);
  writeCmdData(0x71, 0xDF);
  writeCmdData(0x72, 0x40);
  writeCmdData(0x73, 0x40);
  writeCmdData(0x74, 0x01);
  writeCmdData(0x75, 0x00);
  writeCmdData(0x76, 0x00);
  writeCmdData(0x77, 0x00);
  writeCmdData(0x78, 0x00);
  writeCmdData(0x79, 0x00);
  writeCmdData(0x7A, 0x00);
  writeCmdData(0x7B, 0x00);
  writeCmdData(0x80, 0x48);
  writeCmdData(0x81, 0x00);
  writeCmdData(0x82, 0x04);
  writeCmdData(0x83, 0x02);
  writeCmdData(0x84, 0xDC);
  writeCmdData(0x85, 0x00);
  writeCmdData(0x86, 0x00);
  writeCmdData(0x87, 0x00);
  writeCmdData(0x88, 0x48);
  writeCmdData(0x89, 0x00);
  writeCmdData(0x8A, 0x06);
  writeCmdData(0x8B, 0x02);
  writeCmdData(0x8C, 0xDE);
  writeCmdData(0x8D, 0x00);
  writeCmdData(0x8E, 0x00);
  writeCmdData(0x8F, 0x00);
  writeCmdData(0x90, 0x48);
  writeCmdData(0x91, 0x00);
  writeCmdData(0x92, 0x08);
  writeCmdData(0x93, 0x02);
  writeCmdData(0x94, 0xE0);
  writeCmdData(0x95, 0x00);
  writeCmdData(0x96, 0x00);
  writeCmdData(0x97, 0x00);
  writeCmdData(0x98, 0x48);
  writeCmdData(0x99, 0x00);
  writeCmdData(0x9A, 0x0A);
  writeCmdData(0x9B, 0x02);
  writeCmdData(0x9C, 0xE2);
  writeCmdData(0x9D, 0x00);
  writeCmdData(0x9E, 0x00);
  writeCmdData(0x9F, 0x00);
  writeCmdData(0xA0, 0x48);
  writeCmdData(0xA1, 0x00);
  writeCmdData(0xA2, 0x03);
  writeCmdData(0xA3, 0x02);
  writeCmdData(0xA4, 0xDB);
  writeCmdData(0xA5, 0x00);
  writeCmdData(0xA6, 0x00);
  writeCmdData(0xA7, 0x00);
  writeCmdData(0xA8, 0x48);
  writeCmdData(0xA9, 0x00);
  writeCmdData(0xAA, 0x05);
  writeCmdData(0xAB, 0x02);
  writeCmdData(0xAC, 0xDD);
  writeCmdData(0xAD, 0x00);
  writeCmdData(0xAE, 0x00);
  writeCmdData(0xAF, 0x00);
  writeCmdData(0xB0, 0x48);
  writeCmdData(0xB1, 0x00);
  writeCmdData(0xB2, 0x07);
  writeCmdData(0xB3, 0x02);
  writeCmdData(0xB4, 0xDF);
  writeCmdData(0xB5, 0x00);
  writeCmdData(0xB6, 0x00);
  writeCmdData(0xB7, 0x00);
  writeCmdData(0xB8, 0x48);
  writeCmdData(0xB9, 0x00);
  writeCmdData(0xBA, 0x09);
  writeCmdData(0xBB, 0x02);
  writeCmdData(0xBC, 0xE1);
  writeCmdData(0xBD, 0x00);
  writeCmdData(0xBE, 0x00);
  writeCmdData(0xBF, 0x00);
  writeCmdData(0xC0, 0x65);
  writeCmdData(0xC1, 0x74);
  writeCmdData(0xC2, 0x47);
  writeCmdData(0xC3, 0x56);
  writeCmdData(0xC4, 0xAA);
  writeCmdData(0xC5, 0x11);
  writeCmdData(0xC6, 0x00);
  writeCmdData(0xC7, 0x2A);
  writeCmdData(0xC8, 0xA2);
  writeCmdData(0xC9, 0x33);
  writeCmdData(0xD0, 0x65);
  writeCmdData(0xD1, 0x74);
  writeCmdData(0xD2, 0x47);
  writeCmdData(0xD3, 0x56);
  writeCmdData(0xD4, 0xAA);
  writeCmdData(0xD5, 0x11);
  writeCmdData(0xD6, 0x00);
  writeCmdData(0xD7, 0x2A);
  writeCmdData(0xD8, 0xA2);
  writeCmdData(0xD9, 0x33);
  writeCmdData(0xF3, 0x01);
  writeCmdData(0xF0, 0x00);
  writeCmdData(0x36, 0x00);
  writeCmd(0x21);  // Display inversion ON
  writeCmd(0x11);  // Sleep out
  delay(120);
  writeCmd(0x29);            // Display ON
  writeCmdData(0x3A, 0x55);  // 16-bit RGB565 pixel format
  writeCmd(0x2C);            // Memory write
}

void BOE180_ST77916::setRotation(uint8_t r) {
  rotation = r & 3;
  uint8_t madctl = 0x00;
  switch (rotation) {
    case 0:
      madctl = 0x00;
      break;
    case 1:
      madctl = 0x60;
      break;
    case 2:
      madctl = 0xC0;
      break;
    case 3:
      madctl = 0xA0;
      break;
  }
  writeCmdData(0x36, madctl);
}

void BOE180_ST77916::invertDisplay(bool i) { writeCmd(i ? 0x21 : 0x20); }

void BOE180_ST77916::setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
  uint8_t col[4] = {(uint8_t)(x0 >> 8), (uint8_t)(x0 & 0xFF), (uint8_t)(x1 >> 8), (uint8_t)(x1 & 0xFF)};
  uint8_t row[4] = {(uint8_t)(y0 >> 8), (uint8_t)(y0 & 0xFF), (uint8_t)(y1 >> 8), (uint8_t)(y1 & 0xFF)};
  writeCmdBytes(0x2A, col, 4);
  writeCmdBytes(0x2B, row, 4);
}
void BOE180_ST77916::pushColorBlock(uint16_t color, uint32_t count) {
  if (count == 0) return;
  uint8_t hi = color >> 8;
  uint8_t lo = color & 0xFF;
  // Small line buffer in internal RAM (safe for ESP8266 and AVR)
  const uint32_t CHUNK_PIXELS = 360;
  uint8_t buf[CHUNK_PIXELS * 2];
  uint32_t fillLen = (count < CHUNK_PIXELS) ? count : CHUNK_PIXELS;
  for (uint32_t i = 0; i < fillLen; i++) {
    buf[i * 2] = hi;
    buf[i * 2 + 1] = lo;
  }
  digitalWrite(_cs, LOW);
#if BOE180_HAS_QSPI
  if (_useQspi) {
    // 1-line opcode prefix (0x32 = Quad Input Page Program)
    uint8_t hdr[4] = {0x32, 0x00, 0x2C, 0x00};
    spi_transaction_t th = {};
    th.length = 32;
    th.tx_buffer = hdr;
    spi_device_polling_transmit(_qspiDev, &th);
    // 4-line pixel data stream (IO0-IO3)
    while (count > 0) {
      uint32_t n = (count > CHUNK_PIXELS) ? CHUNK_PIXELS : count;
      spi_transaction_t td = {};
      td.flags = SPI_TRANS_MODE_QIO;
      td.length = n * 16;
      td.tx_buffer = buf;
      spi_device_polling_transmit(_qspiDev, &td);
      count -= n;
    }
    digitalWrite(_cs, HIGH);
    return;
  }
#endif
  // Standard 1-wire SPI transfer (ESP8266, classic ESP32, Arduino)
  _arduinoSpi->beginTransaction(SPISettings(_clockHz, MSBFIRST, SPI_MODE0));
  _arduinoSpi->transfer(0x02);
  _arduinoSpi->transfer(0x00);
  _arduinoSpi->transfer(0x2C);
  _arduinoSpi->transfer(0x00);
  while (count > 0) {
    uint32_t n = (count > CHUNK_PIXELS) ? CHUNK_PIXELS : count;
#if defined(ESP8266) || defined(ESP32)
    _arduinoSpi->writeBytes(buf, n * 2);
#else
    for (uint32_t i = 0; i < n * 2; i++) _arduinoSpi->transfer(buf[i]);
#endif
    count -= n;
  }
  _arduinoSpi->endTransaction();
  digitalWrite(_cs, HIGH);
}
void BOE180_ST77916::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if (x < 0 || y < 0 || x >= _width || y >= _height) return;
  setAddrWindow(x, y, x, y);
  pushColorBlock(color, 1);
}
void BOE180_ST77916::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  if (x >= _width || y >= _height || w <= 0 || h <= 0) return;
  if (x < 0) {
    w += x;
    x = 0;
  }
  if (y < 0) {
    h += y;
    y = 0;
  }
  if ((x + w) > _width) w = _width - x;
  if ((y + h) > _height) h = _height - y;
  setAddrWindow(x, y, x + w - 1, y + h - 1);
  pushColorBlock(color, (uint32_t)w * (uint32_t)h);
}
void BOE180_ST77916::fillScreen(uint16_t color) { fillRect(0, 0, _width, _height, color); }
void BOE180_ST77916::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) { fillRect(x, y, 1, h, color); }
void BOE180_ST77916::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) { fillRect(x, y, w, 1, color); }