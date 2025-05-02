#pragma once
// SD card (spi interface) lib

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

// for using unicode
#include <wchar.h>
#include <locale.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"

#define SD_SPI_CH spi0

#define SD_RX 16
#define SD_SCK 18
#define SD_TX 19

class SD_SPI {
  public:
  SD_SPI(int pin_cs, int pin_cd);

  void init(void);

  private:
  int _pin_cs, _pin_cd;
};