// if pico w board, uncomment below line
#define PICO_W

#include <stdio.h>

// for using unicode
#include <wchar.h>
#include <locale.h>

// pico libraries
#include "pico/stdlib.h"
#include "pico/multicore.h"

// headers
#include "built_in_led.h"
#include "pico_chiptune.hpp"

// drivers
#include "uart_log.hpp"
#include "led_status.hpp"
#include "li_battery.hpp"
#include "mcp4725.hpp"
#include "pca9554.hpp"
#include "tm022hdh26.hpp"
#include "sd_spi.hpp"

// middlewares
#include "sound_system.hpp"

// hw lib init
ledStatus Led = ledStatus(8,9,10,11);
mcp4725 Dac = mcp4725(i2c0, 5,4);
soundSystem Sound = soundSystem();
tm022hdh26 Lcd = tm022hdh26(spi0, 19,16,18, 13,14,15,12);
pca9554 Key = pca9554(i2c1, 3,2);
liBattery Bat = liBattery(28, ((double)1/2));
sdSpi Sdcard = sdSpi(spi0, 19,16,18, 17,20);

//////// dac value ////////
#define ME(op,ch,wav,vol,oc,fre)  (uint32_t)((op<<24)+(ch<<20)+(wav<<16)+(vol<<8)+(oc<<4)+(fre))

// 00000000 0000 0000 00000000 0000 0000
// op code  ch   wave volume   oc   freq

const uint32_t Music_Boot_ex[] = {
  ME(OP_NONE, 0, W_TRIN, 63, 6, S_C), ME(OP_DELAY1, 0, 0, 0, 0, 0),
  ME(OP_NONE, 1, W_TRIN, 63, 6, S_G), ME(OP_DELAY4, 0, 0, 0, 0, 0),
  ME(OP_DELAY4, 0, 0, 0, 0, 0), ME(OP_STOP, 0, 0, 0, 0, 0)
};

const uint32_t Music_Test[] = {
  ME(OP_NONE, 0, W_TRIN, 255, 4, S_C), ME(OP_DELAY1, 0, 0, 0, 0, 0),
  ME(OP_NONE, 1, W_TRIN, 255, 4, S_E), ME(OP_DELAY1, 0, 0, 0, 0, 0),
  ME(OP_NONE, 2, W_TRIN, 255, 4, S_G), ME(OP_DELAY1, 0, 0, 0, 0, 0),
  ME(OP_NONE, 3, W_TRIN, 255, 5, S_C), ME(OP_DELAY1, 0, 0, 0, 0, 0),
  ME(OP_DELAY4, 0, 0, 0, 0, 0), ME(OP_STOP, 0, 0, 0, 0, 0)
};

void core1_entry();
void battery_task(void);
void button_task(void);
void led_task(void);

//////// function ////////
void dac_output_wrapper(uint16_t l, uint16_t r) {
    Dac.output(l, r);
}

void dac_mute_wrapper() {
    Dac.mute();
}

void dac_unmute_wrapper() {
    Dac.unmute();
}

int main() { // uses core 0 to sub core
  uartLog_init(uart0, 0, 1, 115200);
  built_in_led_init();

  sleep_ms(100);
  built_in_led_on();

  multicore_launch_core1(core1_entry);

  // initalizing hardwares
  Led.init();
  LOG_PRINTF("LED ok\n");
  Lcd.begin();
  Lcd.fillScreen(0x0000);
  Lcd.set_bright(500);
  LOG_PRINTF("LCD ok\n");
  Dac.init();
  LOG_PRINTF("DAC ok\n");
  Sound.init(dac_output_wrapper, dac_mute_wrapper, dac_unmute_wrapper);
  Sound.init_timer();
  Key.init();
  LOG_PRINTF("KEY ok\n");
  Bat.init();
  LOG_PRINTF("BAT ok\n");
  Sdcard.init();
  LOG_PRINTF("SD card ok\n");
  LOG_PRINTF("all HWs ok!\n");
  LOG_PRINTF("core freq = %ld hz\n", SYS_CLK_KHZ * 1000);
  // hardware initalized

  // boot sequence start
  Lcd.setTextColor(0xFFFF, 0x0000);
  Lcd.setTextSize(4);
  Lcd.setCursor(115,85);
  Lcd.print_5x8("PICO");
  Lcd.setTextSize(2);
  Lcd.setCursor(115,130);
  Lcd.print_5x8("CHIPTUNE");

  Lcd.setTextSize(1);
  Lcd.setCursor(128,200);
  Lcd.print_5x8("press START");
  Lcd.setCursor(64,210);
  Lcd.print_5x8("press SELECT+START to quiet boot");
  Lcd.setCursor(64,220);
  Lcd.print_5x8("press ZL/ZR to change brightness");

  while (1) {
    char string_buf[32];

    sleep_ms(100);
    Bat.get_level();
    Lcd.setTextSize(1);
    sprintf(string_buf, "BAT:% 3.1f%%", Bat.level);
    Lcd.setCursor(250,0);
    Lcd.print_5x8(string_buf);

    Key.get_btn_data();

    if (Key.key_pressed & CODE_KEY_START) {
      break; // go to main loop
    } else if (Key.key_pressed & CODE_KEY_ZL) {
      if (Lcd.get_bright() < 50) {
        Lcd.set_bright(0);
      } else if (Lcd.get_bright() > 50) {
        Lcd.set_bright(Lcd.get_bright() - 50);
      }
      LOG_PRINTF("bright = %d\n", Lcd.get_bright());
    } else if (Key.key_pressed & CODE_KEY_ZR) {
      if (Lcd.get_bright() < 0) {
        Lcd.set_bright(0);
      } else if (Lcd.get_bright() < LCD_BACKLIGHT_MAX) {
        Lcd.set_bright(Lcd.get_bright() + 50);
      }
      LOG_PRINTF("bright = %d\n", Lcd.get_bright());
    }
  }
  

  LOG_PRINTF("go to main loop\n");
  multicore_fifo_push_blocking(1);

  sleep_ms(100);
  Key.get_btn_data();
  if(~Key.key_pressed & CODE_KEY_SELECT) {
    Sound.set_waning(32);
    Sound.play_music_ex(Music_Boot_ex, 10, 100);
  }
  Sound.set_mute(true);
  // boot sequence end
  
  while (1) {
    led_task();
    battery_task();
    button_task();
  }

  return 0;
}

void core1_entry() { // uses core 1 to main core
  
  multicore_fifo_pop_blocking(); // wait until boot process is done

  for(int i=0; i<100; i++) {
    float b = (float)(99-i)/100;
    Led.set_bright_float(1, b);
    Led.set_bright_float(2, b);
    Led.set_bright_float(3, b);
    Led.set_bright_float(4, b);
    sleep_ms(10);
  }

  char string_buf[32];

  Lcd.fillScreen(0x0000);
  Lcd.setCursor(80,120);
  Lcd.setTextColor(0xFFFF, 0x0000);
  Lcd.setTextSize(2);
  Lcd.print_5x8("Now Loading...");

  while (1) {
    sleep_ms(100);
    Bat.get_level();
    Lcd.setTextSize(1);
    sprintf(string_buf, "BAT:% 3.1f%%", Bat.level);
    Lcd.setCursor(250,0);
    Lcd.print_5x8(string_buf);
  }
}

void battery_task(void) {
  static uint32_t last_bat_check = 0;
  if (time_us_64() - last_bat_check > 1000000) { // check every second
    last_bat_check = time_us_64();
    Bat.get_level();
  }
}

void button_task(void) {
  static uint32_t last_btn_check = 0;
  if (time_us_64() - last_btn_check > 10000) { // check every 10ms
    last_btn_check = time_us_64();
    Key.get_btn_data();
  }
}

void led_task(void) {
  static uint32_t last_led_check = 0;
  if (time_us_64() - last_led_check > 1000) { // check every 1ms
    last_led_check = time_us_64();
  }
}