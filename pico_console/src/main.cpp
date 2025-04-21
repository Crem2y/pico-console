// if pico w board, uncomment below line
#define PICO_W

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "pico/stdlib.h"
//#include "pico/multicore.h"
#include "main.hpp"
#include "LED_STATUS.hpp"
#include "MCP4725.hpp"

LED_STATUS Led = LED_STATUS(8,9,10,11);

//////// dac value ////////
MCP4725 Dac = MCP4725(5,4);

#define ME(op,ch,wav,vol,oc,fre)  (uint32_t)((op<<24)+(ch<<20)+(wav<<16)+(vol<<8)+(oc<<4)+(fre))

// 00000000 0000 0000 00000000 0000 0000
// op code  ch   wave volume   oc   freq

const uint32_t Music_Boot_ex[] = {
  ME(OP_NONE, 0, W_TRIN, 255, 4, S_C), ME(OP_DELAY2, 0, 0, 0, 0, 0),
  ME(OP_NONE, 1, W_TRIN, 255, 4, S_E), ME(OP_DELAY2, 0, 0, 0, 0, 0),
  ME(OP_NONE, 2, W_TRIN, 255, 4, S_G), ME(OP_DELAY2, 0, 0, 0, 0, 0),
  ME(OP_NONE, 3, W_TRIN, 255, 5, S_C), ME(OP_DELAY2, 0, 0, 0, 0, 0),
  ME(OP_DELAY4, 0, 0, 0, 0, 0), ME(OP_STOP, 0, 0, 0, 0, 0)
};

const uint32_t Music_Table0[] = {
  ME(OP_NONE, 0, W_PS50, 255, 4, S_Cs), ME(OP_NONE, 1, W_PS50, 255, 4, S_As), ME(OP_NONE, 2, W_TRIN, 255, 2, S_G ), ME(OP_NONE, 3, W_NOIZ, 63, 4, S_C ), ME(OP_DELAY3, 0, 0, 0, 0, 0), ME(OP_DELAY3, 0, 0, 0, 0, 0),
  ME(OP_NONE, 2, W_TRIN, 255, 2, S_G ), ME(OP_DELAY2, 0, 0, 0, 0, 0),
  ME(OP_NONE, 2, W_TRIN, 255, 2, S_G ), ME(OP_DELAY2, 0, 0, 0, 0, 0),
  ME(OP_NONE, 2, W_TRIN, 255, 2, S_G ), ME(OP_DELAY2, 0, 0, 0, 0, 0),
  ME(OP_NONE, 2, W_TRIN, 255, 2, S_G ), ME(OP_DELAY4, 0, 0, 0, 0, 0),
  ME(OP_NONE, 0, W_PS50, 255, 4, S_Cs), ME(OP_NONE, 1, W_PS50, 255, 4, S_As), ME(OP_DELAY2, 0, 0, 0, 0, 0),
  ME(OP_NONE, 0, W_PS50, 255, 4, S_Cs), ME(OP_NONE, 1, W_PS50, 255, 4, S_As), ME(OP_NONE, 2, W_TRIN, 255, 2, S_G ), ME(OP_NONE, 3, W_NOIZ, 63, 4, S_C ), ME(OP_DELAY2, 0, 0, 0, 0, 0),
  ME(OP_NONE, 0, W_PS50, 255, 4, S_Cs), ME(OP_NONE, 1, W_PS50, 255, 4, S_As), ME(OP_NONE, 2, W_TRIN, 255, 2, S_G ), ME(OP_NONE, 3, W_NOIZ, 63, 4, S_C ), ME(OP_DELAY2, 0, 0, 0, 0, 0),
  ME(OP_NONE, 0, W_PS50, 255, 4, S_Cs), ME(OP_NONE, 1, W_PS50, 255, 4, S_As), ME(OP_NONE, 2, W_TRIN, 255, 2, S_G ), ME(OP_NONE, 3, W_NOIZ, 63, 4, S_C ), ME(OP_DELAY2, 0, 0, 0, 0, 0),
  // 1
  ME(OP_NONE, 0, W_PS50, 255, 4, S_C), ME(OP_NONE, 1, W_PS50, 255, 4, S_As), ME(OP_NONE, 2, W_TRIN, 255, 2, S_G ), ME(OP_NONE, 3, W_NOIZ, 63, 4, S_C ), ME(OP_DELAY2, 0, 0, 0, 0, 0),
  ME(OP_NONE, 0, W_PS50, 0, 4, S_C), ME(OP_NONE, 1, W_PS50, 0, 4, S_As), ME(OP_DELAY2, 0, 0, 0, 0, 0),
  ME(OP_NONE, 0, W_PS50, 255, 4, S_C), ME(OP_NONE, 1, W_PS50, 255, 4, S_Gs), ME(OP_DELAY2, 0, 0, 0, 0, 0),
  ME(OP_NONE, 0, W_PS50, 0, 4, S_C), ME(OP_NONE, 1, W_PS50, 0, 4, S_As), ME(OP_NONE, 2, W_TRIN, 255, 2, S_G ), ME(OP_DELAY2, 0, 0, 0, 0, 0),
  ME(OP_NONE, 2, W_TRIN, 255, 2, S_G ), ME(OP_DELAY2, 0, 0, 0, 0, 0),
  ME(OP_NONE, 2, W_TRIN, 255, 2, S_G ), ME(OP_DELAY2, 0, 0, 0, 0, 0),
  ME(OP_NONE, 2, W_TRIN, 255, 2, S_G ), ME(OP_DELAY4, 0, 0, 0, 0, 0),
  ME(OP_NONE, 0, W_PS50, 255, 4, S_C), ME(OP_NONE, 1, W_PS50, 255, 4, S_As), ME(OP_DELAY2, 0, 0, 0, 0, 0),
  ME(OP_NONE, 0, W_PS50, 255, 4, S_C), ME(OP_NONE, 1, W_PS50, 255, 4, S_As), ME(OP_NONE, 2, W_TRIN, 255, 2, S_G ), ME(OP_NONE, 3, W_NOIZ, 63, 4, S_C ), ME(OP_DELAY2, 0, 0, 0, 0, 0),
  ME(OP_NONE, 0, W_PS50, 255, 4, S_C), ME(OP_NONE, 1, W_PS50, 255, 4, S_As), ME(OP_NONE, 2, W_TRIN, 255, 2, S_G ), ME(OP_NONE, 3, W_NOIZ, 63, 4, S_C ), ME(OP_DELAY2, 0, 0, 0, 0, 0),
  ME(OP_NONE, 0, W_PS50, 255, 4, S_C), ME(OP_NONE, 1, W_PS50, 255, 4, S_As), ME(OP_NONE, 2, W_TRIN, 255, 2, S_G ), ME(OP_NONE, 3, W_NOIZ, 63, 4, S_C ), ME(OP_DELAY2, 0, 0, 0, 0, 0),
  //2

  ME(OP_STOP, 0, 0, 0, 0, 0)
};

//////// function ////////

/*
void core1_entry() {
  while (1) {
    
  }
}
*/

int main() {
  stdio_init_all();
  built_in_led_init();

  sleep_ms(100);
  built_in_led_on();

  // multicore code
  //multicore_launch_core1(core1_entry);
  //alarm_pool_t alarm_dac = alarm_pool_create(8, 16);
  //alarm_dac.core_num = 1;
  //alarm_pool_add_repeating_timer_us(alarm_dac, (1000000/WAVE_FREQ), timer_callback, NULL, &timer_delay);

  // boot sequence start

  sleep_ms(1000);
  Led.init();
  Dac.init();
  sleep_ms(100);
  Dac.set_waning(32);
  Dac.play_music_ex(Music_Boot_ex, 12, 100);
  //dac.play_music_ex(Music_Table0, 255, 33);

  // boot sequence end

  Led.set_bright_float(1, 1.0);
  Led.set_bright_float(2, 1.0);
  Led.set_bright_float(3, 1.0);
  Led.set_bright_float(4, 1.0);

  while (true) {
    for(int i=0; i<100; i++) {
      float b = (float)i/100;
      Led.set_bright_float(1, b);
      Led.set_bright_float(2, b);
      Led.set_bright_float(3, b);
      Led.set_bright_float(4, b);

      sleep_ms(10);
    }

    for(int i=0; i<100; i++) {
      float b = (float)(100-i)/100;
      Led.set_bright_float(1, b);
      Led.set_bright_float(2, b);
      Led.set_bright_float(3, b);
      Led.set_bright_float(4, b);

      sleep_ms(10);
    }

  }

  return 0;
}