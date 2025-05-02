// Li battery checking lib

#include "LI_BATTERY.hpp"

double map_double(double x, double in_min, double in_max, double out_min, double out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

LI_BATTERY::LI_BATTERY(int adc_pin, double res_ratio) {
  _adc_pin = adc_pin;
  _res_ratio = res_ratio;
}

void LI_BATTERY::init(void) {
  adc_init();
  adc_gpio_init(_adc_pin);
}

double LI_BATTERY::get_voltage(void) {
  adc_select_input(BAT_ADC_CH);
  uint16_t adc_raw;
  
  for(int i=0; i<16; i++) {
    adc_raw += adc_read();
  }

  //return (((double)adc_raw * (3.3/4096)) / _res_ratio);
  voltage = (double)adc_raw * (3.3/65536) * 2 + 0.150;
  return voltage;
}

double LI_BATTERY::get_level(void) {
  level = map_double(get_voltage(), BAT_V_MIN, BAT_V_MAX, 0.0, 100.0);
  return level;
}