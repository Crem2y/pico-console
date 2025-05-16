#pragma once
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"

#define DAC_I2C_CH i2c0
#define DAC_L 0x60 // dac (L) address
#define DAC_R 0x61 // dac (R) address

class mcp4725 {
  public:
    mcp4725(int pin_scl, int pin_sda);

    void init(void);
    void data(uint8_t addr, uint16_t data);

    void output(uint16_t left, uint16_t right);
    void mute(void);
    void unmute(void);

  private:
    int _pin_scl, _pin_sda;
};