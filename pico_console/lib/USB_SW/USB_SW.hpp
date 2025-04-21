#pragma once
// software USB port lib
// original source
// https://github.com/sekigon-gonnoc/Pico-PIO-USB

#include "pico/stdlib.h"
#include "hardware/gpio.h"

class USB_SW {
  public:
  USB_SW(int pin_dp, int pin_dn);

  void init(void);

  private:
  int _pin_dp, _pin_dn;
};