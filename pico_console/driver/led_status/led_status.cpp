// 4 leds lib

#include "led_status.hpp"

LED_STATUS::LED_STATUS(int led_1, int led_2, int led_3, int led_4) {
  led_pin[0] = led_1;
  led_pin[1] = led_2;
  led_pin[2] = led_3;
  led_pin[3] = led_4;
}

void LED_STATUS::init(void) {
  for(int i=0; i<4; i++) {
    gpio_set_function(led_pin[i], GPIO_FUNC_PWM);
    slice_num[i] = pwm_gpio_to_slice_num(led_pin[i]);
    led_pwm_ch[i] = (led_pin[i] % 2) ? PWM_CHAN_B : PWM_CHAN_A;

    pwm_set_chan_level(slice_num[i], led_pwm_ch[i], 0);
    pwm_set_clkdiv(slice_num[i], SYS_CLK_KHZ * 1000/LED_PWM_MAX);
    pwm_set_wrap(slice_num[i], 1000);
    pwm_set_enabled(slice_num[i], true);
  }
}

void LED_STATUS::set_bright(uint32_t num, uint32_t bright) {
  led_bright[num-1] = LED_PWM_MAX - bright;
  pwm_set_chan_level(slice_num[num-1], led_pwm_ch[num-1], 1000-bright);
}

void LED_STATUS::set_bright_float(uint32_t num, float bright) {
  if (bright < 0) return;

  uint32_t bright_int;
  bright_int = (uint32_t)((1 - bright) * LED_PWM_MAX);

  led_bright[num-1] = bright_int;
  pwm_set_chan_level(slice_num[num-1], led_pwm_ch[num-1], bright_int);
}

void LED_STATUS::set_bright_all(uint32_t* bright_arr) {
  for(int i=0; i<4; i++) {
    led_bright[i] = LED_PWM_MAX - bright_arr[i];
    pwm_set_chan_level(slice_num[i], led_pwm_ch[i], bright_arr[i]);
  }
}

uint32_t LED_STATUS::get_bright(uint32_t num) {
  return led_bright[num-1];
}