#pragma once
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "sound_system.hpp"

#define DAC_L 0x60 // dac (L) address
#define DAC_R 0x61 // dac (R) address

#define DAC_HW_MAX 4095
#define DAC_SAMPLING_FREQ 10000.0


struct mcp4725_data_t {
  i2c_inst_t* i2c; // I2C instance
  struct sound_system_data_t sound_data; // pointer to sound system data
};

extern struct mcp4725_data_t dac_data;

static bool timer_callback_dac(struct repeating_timer *t);

class mcp4725{
  public:
    mcp4725(i2c_inst_t* i2c, int pin_scl, int pin_sda, struct mcp4725_data_t* dac_data);

    void init(void);

  private:
    int _pin_scl, _pin_sda;
    i2c_inst_t* _i2c;
};