#include "mcp4725.hpp"

mcp4725::mcp4725(int pin_scl, int pin_sda) {
  _pin_scl = pin_scl;
  _pin_sda = pin_sda;
}

void mcp4725::init(void) {

  i2c_init(DAC_I2C_CH, 1000 * 1000);             //Initialize I2C on i2c0 port with 1000kHz
  gpio_set_function(_pin_scl, GPIO_FUNC_I2C); //Use GPIO as I2C
  gpio_set_function(_pin_sda, GPIO_FUNC_I2C);
  gpio_pull_up(_pin_scl);                     //Pull up GPIO
  gpio_pull_up(_pin_sda);

}

void mcp4725::data(uint8_t addr, uint16_t data) {
  uint8_t buf[2] = {0, 0};

  buf[0] = data >> 8;
  buf[1] = data;
  i2c_write_blocking(DAC_I2C_CH, addr, buf, 2, false);
}

void mcp4725::output(uint16_t left, uint16_t right) {
  data(DAC_L, left);
  data(DAC_R, right);
}

void mcp4725::mute(void) {
  data(DAC_L, 0x0300); // power-down mode
  data(DAC_R, 0x0300);
}

void mcp4725::unmute(void) {
  data(DAC_L, 0);
  data(DAC_R, 0);
}