// SD card (spi interface) lib

#include "SD_SPI.hpp"

SD_SPI::SD_SPI(int pin_cs, int pin_cd) {
  _pin_cs = pin_cs;
  _pin_cd = pin_cd;
}

void SD_SPI::init(void) {
  gpio_init(_pin_cs);
  gpio_set_dir(_pin_cs, GPIO_OUT);
  gpio_put(_pin_cs, 1);

  gpio_init(_pin_cd);
  gpio_set_dir(_pin_cd, GPIO_IN);
  gpio_pull_up(_pin_cd);

  spi_init(SD_SPI_CH, 25 * 1000000);
  gpio_set_function(SD_RX, GPIO_FUNC_SPI);
  gpio_pull_up(SD_RX);
  gpio_set_function(SD_SCK, GPIO_FUNC_SPI);
  gpio_set_function(SD_TX, GPIO_FUNC_SPI);
}