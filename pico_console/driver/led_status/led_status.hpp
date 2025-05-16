#pragma once
// 4 leds lib

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"

#define LED_PWM_MAX 1000

class ledStatus {
  public:
  ledStatus(int led_1, int led_2, int led_3, int led_4);

  void init(void);
  void set_bright(uint32_t num, uint32_t bright);
  void set_bright_float(uint32_t num, float bright);
  void set_bright_all(uint32_t* bright_arr);
  uint32_t get_bright(uint32_t num);

  private:
    int led_pin[4];
    uint slice_num[4];
    int led_bright[4];
    int led_pwm_ch[4];
};