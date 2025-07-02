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

#define TABLE_LENGTH 128
const uint16_t Music_Table[4 * TABLE_LENGTH] = {
  0x0000, 0x0001, 0x0002, 0x0003,
  0x0004, 0x0005, 0x0006, 0x0007,
  0x0008, 0x0009, 0x000A, 0x000B,
  0x000C, 0x000D, 0x000E, 0x000F,
  0x0010, 0x0011, 0x0012, 0x0013,
  0x0014, 0x0015, 0x0016, 0x0017,
  0x0018, 0x0019, 0x001A, 0x001B,
  0x001C, 0x001D, 0x001E, 0x001F,
  0x0020, 0x0021, 0x0022, 0x0023,
  0x0024, 0x0025, 0x0026, 0x0027,
  0x0028, 0x0029, 0x002A, 0x002B,
  0x002C, 0x002D, 0x002E, 0x002F,
  0x0030, 0x0031, 0x0032, 0x0033,
  0x0034, 0x0035, 0x0036, 0x0037,
  0x0038, 0x0039, 0x003A, 0x003B,
  0x003C, 0x003D, 0x003E, 0x003F,
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

void update_screen(uint32_t start_pos) {
  Lcd.setTextColor(0xFFFF, 0x0000);

  for(int i=0; i<13; i++) {
    char string_buf[64];
    int pos = start_pos + i;

    if(pos < TABLE_LENGTH) {
      sprintf(string_buf, "%04d  0x%04X 0x%04X 0x%04X 0x%04X",
        pos,
        Music_Table[pos * 4],
        Music_Table[(pos * 4) + 1],
        Music_Table[(pos * 4) + 2],
        Music_Table[(pos * 4) + 3]);
    } else {
      sprintf(string_buf, "                                 ");
    }
    Lcd.setCursor(0, 32 + (i * 16));
    Lcd.print_16(string_buf);
  }
}

void update_cursor(uint32_t cursor_x, uint32_t cursor_y, uint32_t cursor_x_prv, uint32_t cursor_y_prv) {
  uint32_t x_pos = (cursor_x_prv / 4) * 7 + 8 + (cursor_x_prv % 4);
  Lcd.setTextColor(0xFFFF, 0x0000);
  Lcd.setCursor(x_pos * 8, 32);
  Lcd.print_16("-");
  x_pos = (cursor_x / 4) * 7 + 8 + (cursor_x % 4);
  Lcd.setTextColor(0x0000, 0xFFFF);
  Lcd.setCursor(x_pos * 8, 32);
  Lcd.print_16("-");
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
  uint32_t cursor_x = 0;
  uint32_t cursor_y = 0;
  uint32_t cursor_x_prv = 0;
  uint32_t cursor_y_prv = 0;

  Lcd.fillScreen(0x0000);
  Lcd.setTextColor(0xFFFF, 0x0000);
  Lcd.setTextSize(1);
  //Lcd.setCursor(100,120);
  //Lcd.print_16("Now Loading...");
  
  Lcd.setCursor(0,0);
  Lcd.print_16("d:1000ms");

  Lcd.setCursor(140,0);
  Lcd.print_16("temp");

  Lcd.setCursor(0,16);
  Lcd.print_16("count   ch1    ch2    ch3    ch4");

  update_screen(cursor_y);
  update_cursor(cursor_x, cursor_y, cursor_x_prv, cursor_y_prv);

  while (1) {
    sleep_ms(100);
    
    // battery check
    Lcd.setTextColor(0xFFFF, 0x0000);
    sprintf(string_buf, "B:%3d%%", (int)Bat.level);
    Lcd.setCursor(270,0);
    Lcd.print_16(string_buf);

    if (Key.key_pressed & CODE_KEY_START) {
      Sound.play_music_ex(Music_Boot_ex, 10, 100);
    } else if (Key.key_pressed & CODE_KEY_SELECT) {
    } else if (Key.key_pressed & CODE_KEY_S1_UP) {
      if(cursor_y > 0) {
        cursor_y--;
      }
      update_screen(cursor_y);
      update_cursor(cursor_x, cursor_y, cursor_x_prv, cursor_y_prv);
      cursor_y_prv = cursor_y;
    } else if (Key.key_pressed & CODE_KEY_S1_DOWN) {
      if(cursor_y < TABLE_LENGTH - 1) {
        cursor_y++;
      }
      update_screen(cursor_y);
      update_cursor(cursor_x, cursor_y, cursor_x_prv, cursor_y_prv);
      cursor_y_prv = cursor_y;
    } else if (Key.key_pressed & CODE_KEY_S1_LEFT) {
      if(cursor_x > 0) {
        cursor_x--;
      }
      update_cursor(cursor_x, cursor_y, cursor_x_prv, cursor_y_prv);
      cursor_x_prv = cursor_x;
    } else if (Key.key_pressed & CODE_KEY_S1_RIGHT) {
      if(cursor_x < 16) {
        cursor_x++;
      }
      update_cursor(cursor_x, cursor_y, cursor_x_prv, cursor_y_prv);
      cursor_x_prv = cursor_x;
    } else if (Key.key_pressed & CODE_KEY_UP) {
      if(cursor_y > 8) {
        cursor_y -= 8;
      } else if (cursor_y > 0) {
        cursor_y = 0;
      }
      update_screen(cursor_y);
      update_cursor(cursor_x, cursor_y, cursor_x_prv, cursor_y_prv);
      cursor_y_prv = cursor_y;
    } else if (Key.key_pressed & CODE_KEY_DOWN) {
      if(cursor_y < TABLE_LENGTH - 9) {
        cursor_y += 8;
      } else if (cursor_y < TABLE_LENGTH) {
        cursor_y = TABLE_LENGTH - 1;
      }
      update_screen(cursor_y);
      update_cursor(cursor_x, cursor_y, cursor_x_prv, cursor_y_prv);
    } else if (Key.key_pressed & CODE_KEY_LEFT) {
    } else if (Key.key_pressed & CODE_KEY_RIGHT) {
    } else if (Key.key_pressed & CODE_KEY_S2_UP) {
    } else if (Key.key_pressed & CODE_KEY_S2_DOWN) {
    } else if (Key.key_pressed & CODE_KEY_S2_LEFT) {
    } else if (Key.key_pressed & CODE_KEY_S2_RIGHT) {
    } else if (Key.key_pressed & CODE_KEY_A) {
    } else if (Key.key_pressed & CODE_KEY_B) {
    } else if (Key.key_pressed & CODE_KEY_X) {
    } else if (Key.key_pressed & CODE_KEY_Y) {
    }
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