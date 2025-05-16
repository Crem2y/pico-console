// software USB port lib
// original source
// https://github.com/sekigon-gonnoc/Pico-PIO-USB

#include "usb_sw.hpp"

USB_SW::USB_SW(int pin_dp, int pin_dn) {
  _pin_dp = pin_dp;
  _pin_dn = pin_dn;
}

void USB_SW::init(void) {
  gpio_init(_pin_dp);
  gpio_set_dir(_pin_dp, GPIO_IN);
  gpio_pull_down(_pin_dp);

  gpio_init(_pin_dn);
  gpio_set_dir(_pin_dn, GPIO_IN);
  gpio_pull_down(_pin_dn);
}