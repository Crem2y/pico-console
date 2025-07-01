// Li battery checking lib

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

#define BAT_V_MAX 4.2
#define BAT_V_MIN 3.2

#define BAT_ADC_CH 2

class liBattery {
  public:
  double voltage;
  double level;
  
  liBattery(int adc_pin, double res_ratio);

  void init(void);
  double get_voltage(void);
  double get_level(void);

  private:
    int _adc_pin;
    double _res_ratio;
};