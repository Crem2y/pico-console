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
#include "hw_test.hpp"
#include "raspberry_logo.h"

// drivers
#include "led_status.hpp"
#include "li_battery.hpp"
#include "ir_remote.hpp"
#include "mcp4725.hpp"
#include "pca9554.hpp"
#include "tm022hdh26.hpp"
#include "sd_spi.hpp"
#include "usb_sw.hpp"
#include "pico_w_bt.hpp"
#include "pico_w_wifi.hpp"

// middlewares

// hw lib init
LED_STATUS Led = LED_STATUS(8,9,10,11);
MCP4725 Dac = MCP4725(5,4);
TM022HDH26 Lcd = TM022HDH26(13,14,15,12);
PCA9554 Key = PCA9554(3,2);
LI_BATTERY Bat = LI_BATTERY(28, ((double)1/2));
IR_REMOTE Ir = IR_REMOTE(21);
SD_SPI Sdcard = SD_SPI(17,20);
USB_SW Swusb = USB_SW(6,7);
//PICO_W_BT Bt = PICO_W_BT();
PICO_W_WIFI Wifi = PICO_W_WIFI();

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

//////// function ////////

int main() { // uses core 0 to sub core
  stdio_init_all();
  built_in_led_init();

  sleep_ms(100);
  built_in_led_on();

  multicore_launch_core1(core1_entry);
  setlocale(LC_ALL,"");

  // initalizing hardwares
  Led.init();
  wprintf(L"LED ok\n");
  Lcd.begin();
  Lcd.fillScreen(0x0000);
  Lcd.drawBitmap(128, 60, raspberry64x80, 64, 80, 0xFFFF, 0x0000);
  Lcd.set_bright(500);
  Lcd.setCursor(90,180);
  Lcd.setTextColor(0xFFFF, 0x0000);
  Lcd.setTextSize(2);
  Lcd.print_5x8("PICO CONSOLE");
  wprintf(L"LCD ok\n");
  Dac.init();
  wprintf(L"DAC ok\n");
  Key.init();
  wprintf(L"KEY ok\n");
  Bat.init();
  wprintf(L"BAT ok\n");
  Ir.init();
  wprintf(L"IR ok\n");
  Sdcard.init();
  wprintf(L"SD card ok\n");
  Swusb.init();
  wprintf(L"SW USB ok\n");
  wprintf(L"all HWs ok!\n");
  wprintf(L"core freq = %ld hz\n", SYS_CLK_KHZ * 1000);
  // hardware initalized

  // boot sequence start
  Lcd.setTextSize(1);
  Lcd.setCursor(128,200);
  Lcd.print_5x8("press START");
  Lcd.setCursor(64,210);
  Lcd.print_5x8("press SELECT+START to quiet boot");
  Key.wait_until(KEY_START, true);

  wprintf(L"code check..\n");
  for(int i=0; i<10; i++) {
    //printf("%d %d...\n", Key.key_log[i+2], conami_code[9-i]);
    if(Key.key_log[i+1] != conami_code[9-i]) break;
    if(i == 9) {
      wprintf(L"code activated!\n");
      Dac.set_waning(32);
      Dac.play_music_ex(Music_Test, 10, 100);
    }
  }

  wprintf(L"go to main loop\n");
  multicore_fifo_push_blocking(1);

  sleep_ms(100);
  Key.get_btn_data();
  if(~Key.key_pressed & (1 << KEY_SELECT)) {
    Dac.set_waning(32);
    Dac.play_music_ex(Music_Boot_ex, 10, 100);
  }
  Dac.set_mute(true);
  // boot sequence end
  
  while (true) {
    sleep_ms(10);
    Key.get_btn_data();
    Bat.get_level();
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
  uint8_t cursor_x = 0;
  uint8_t cursor_x_old = 0;

main_menu_loop:
  Lcd.fillScreen(0x0000);
  Lcd.setTextColor(0xFFFF, 0x0000);

  while (1) {
    Lcd.setTextSize(2);
    Lcd.setCursor(16,0);
    Lcd.print_5x8("Button test");
    Lcd.setCursor(16,16);
    Lcd.print_5x8("LED test");
    Lcd.setCursor(16,32);
    Lcd.print_5x8("LCD test");
    Lcd.setCursor(16,48);
    Lcd.print_5x8("DAC test");
    Lcd.setCursor(16,64);
    Lcd.print_5x8("Battery test");
    Lcd.setCursor(16,80);
    Lcd.print_5x8("IR LED test");
    Lcd.setCursor(16,96);
    Lcd.print_5x8("SD card test");
    Lcd.setCursor(16,112);
    Lcd.print_5x8("USB(bottom) test");
    Lcd.setCursor(16,128);
    Lcd.print_5x8("Bluetooth test");
    Lcd.setCursor(16,144);
    Lcd.print_5x8("Wi-Fi test");

    Lcd.setTextSize(1);
    Lcd.setCursor(0,224);
    Lcd.print_5x8("press up/down to move cursor");
    Lcd.setCursor(0,232);
    Lcd.print_5x8("press A or START to select");

    while(1) {
      sleep_ms(100);

      Lcd.setTextSize(1);
      sprintf(string_buf, "BAT:% 3.1f%%", Bat.level);
      Lcd.setCursor(260,0);
      Lcd.print_5x8(string_buf);

      Lcd.setTextSize(2);
      if(cursor_x_old != cursor_x) {
        Lcd.setCursor(0,cursor_x_old * 16);
        Lcd.print_5x8(" ");
      }
      Lcd.setCursor(0,cursor_x * 16);
      Lcd.print_5x8("-");

      cursor_x_old = cursor_x;

      if(Key.key_flags.s1_up || Key.key_flags.up) {
        if(cursor_x > 0) cursor_x--;
      }
      if(Key.key_flags.s1_down || Key.key_flags.down) {
        if(cursor_x < MM_WIFI_TEST) cursor_x++;
      }

      if(Key.key_flags.a || Key.key_flags.start) {
        Lcd.fillScreen(0x0000);
        switch (cursor_x)
        {
        case MM_BTN_TEST:
          menu_btn_test();
          break;
        case MM_LED_TEST:
          menu_led_test();
          break;
        case MM_LCD_TEST:
          menu_lcd_test();
          break;
        case MM_DAC_TEST:
          menu_dac_test();
          break;
        case MM_BAT_TEST:
          menu_bat_test();
          break;
        case MM_IR_TEST:
          menu_ir_test();
          break;
        case MM_SD_TEST:
          menu_sd_test();
          break;
        case MM_USB_TEST:
          menu_usb_test();
          break;
        case MM_BT_TEST:
          menu_bt_test();
          break;
        case MM_WIFI_TEST:
          menu_wifi_test();
          break;
        }
        goto main_menu_loop;
      }
    }
  }
}

//////// test menus ////////

void menu_btn_test(void) {
  Lcd.setTextSize(1);
  Lcd.setCursor(0,232);
  Lcd.print_5x8("press SELECT & START to exit menu");
  Lcd.setTextSize(2);
  Lcd.setCursor(0,0);
  Lcd.print_5x8("Button test");
  
  Lcd.drawRect(20,64,280,156,0xFFFF);
  Lcd.drawRect(152,200,16,16,0xF800);

  while(1) {
    sleep_ms(10);
    char string_buf[32];
    Lcd.setTextSize(2);
    sprintf(string_buf, "KEY : 0x%08X", Key.key_flags.flag_int);
    Lcd.setCursor(0,16);
    Lcd.print_5x8(string_buf);
    
    if(Key.key_flags.sl) {
      Lcd.fillRect(20,46,16,16,0xFFFF);
    } else {
      Lcd.fillRect(21,47,14,14,0x0000);
      Lcd.drawRect(20,46,16,16,0xFFFF);
    }
    if(Key.key_flags.sr) {
      Lcd.fillRect(284,46,16,16,0xFFFF);
    } else {
      Lcd.fillRect(285,47,14,14,0x0000);
      Lcd.drawRect(284,46,16,16,0xFFFF);
    }
    if(Key.key_flags.zl) {
      Lcd.fillRect(60,46,16,16,0xFFFF);
    } else {
      Lcd.fillRect(61,47,14,14,0x0000);
      Lcd.drawRect(60,46,16,16,0xFFFF);
    }
    if(Key.key_flags.zr) {
      Lcd.fillRect(244,46,16,16,0xFFFF);
    } else {
      Lcd.fillRect(245,47,14,14,0x0000);
      Lcd.drawRect(244,46,16,16,0xFFFF);
    }
    if(Key.key_flags.select) {
      Lcd.fillRect(100,74,16,16,0xFFFF);
    } else {
      Lcd.fillRect(101,75,14,14,0x0000);
      Lcd.drawRect(100,74,16,16,0xFFFF);
    }
    if(Key.key_flags.start) {
      Lcd.fillRect(205,74,16,16,0xFFFF);
    } else {
      Lcd.fillRect(206,75,14,14,0x0000);
      Lcd.drawRect(205,74,16,16,0xFFFF);
    }

    if(Key.key_flags.s1_center) {
      Lcd.fillRect(50,94,16,16,0xFFFF);
      Lcd.fillRect(50,174,16,16,0xFFFF);
    } else {
      Lcd.fillRect(51,95,14,14,0x0000);
      Lcd.drawRect(50,94,16,16,0xFFFF);
      Lcd.fillRect(51,175,14,14,0x0000);
      Lcd.drawRect(50,174,16,16,0xFFFF);
    }
    if(Key.key_flags.s1_up) {
      Lcd.fillRect(50,74,16,16,0xFFFF);
    } else {
      Lcd.fillRect(51,75,14,14,0x0000);
      Lcd.drawRect(50,74,16,16,0xFFFF);
    }
    if(Key.key_flags.s1_down) {
      Lcd.fillRect(50,114,16,16,0xFFFF);
    } else {
      Lcd.fillRect(51,115,14,14,0x0000);
      Lcd.drawRect(50,114,16,16,0xFFFF);
    }
    if(Key.key_flags.s1_left) {
      Lcd.fillRect(30,94,16,16,0xFFFF);
    } else {
      Lcd.fillRect(31,95,14,14,0x0000);
      Lcd.drawRect(30,94,16,16,0xFFFF);
    }
    if(Key.key_flags.s1_right) {
      Lcd.fillRect(70,94,16,16,0xFFFF);
    } else {
      Lcd.fillRect(71,95,14,14,0x0000);
      Lcd.drawRect(70,94,16,16,0xFFFF);
    }

    if(Key.key_flags.up) {
      Lcd.fillRect(50,154,16,16,0xFFFF);
    } else {
      Lcd.fillRect(51,155,14,14,0x0000);
      Lcd.drawRect(50,154,16,16,0xFFFF);
    }
    if(Key.key_flags.down) {
      Lcd.fillRect(50,194,16,16,0xFFFF);
    } else {
      Lcd.fillRect(51,195,14,14,0x0000);
      Lcd.drawRect(50,194,16,16,0xFFFF);
    }
    if(Key.key_flags.left) {
      Lcd.fillRect(30,174,16,16,0xFFFF);
    } else {
      Lcd.fillRect(31,175,14,14,0x0000);
      Lcd.drawRect(30,174,16,16,0xFFFF);
    }
    if(Key.key_flags.right) {
      Lcd.fillRect(70,174,16,16,0xFFFF);
    } else {
      Lcd.fillRect(71,175,14,14,0x0000);
      Lcd.drawRect(70,174,16,16,0xFFFF);
    }
   
    if(Key.key_flags.s2_center) {
      Lcd.fillRect(255,94,16,16,0xFFFF);
      Lcd.fillRect(255,174,16,16,0xFFFF);
    } else {
      Lcd.fillRect(256,95,14,14,0x0000);
      Lcd.drawRect(255,94,16,16,0xFFFF);
      Lcd.fillRect(256,175,14,14,0x0000);
      Lcd.drawRect(255,174,16,16,0xFFFF);
    }
    if(Key.key_flags.x) {
      Lcd.fillRect(255,74,16,16,0xFFFF);
    } else {
      Lcd.fillRect(256,75,14,14,0x0000);
      Lcd.drawRect(255,74,16,16,0xFFFF);
    }
    if(Key.key_flags.b) {
      Lcd.fillRect(255,114,16,16,0xFFFF);
    } else {
      Lcd.fillRect(256,115,14,14,0x0000);
      Lcd.drawRect(255,114,16,16,0xFFFF);
    }
    if(Key.key_flags.y) {
      Lcd.fillRect(235,94,16,16,0xFFFF);
    } else {
      Lcd.fillRect(236,95,14,14,0x0000);
      Lcd.drawRect(235,94,16,16,0xFFFF);
    }
    if(Key.key_flags.a) {
      Lcd.fillRect(274,94,16,16,0xFFFF);
    } else {
      Lcd.fillRect(275,95,14,14,0x0000);
      Lcd.drawRect(274,94,16,16,0xFFFF);
    }

    if(Key.key_flags.s2_up) {
      Lcd.fillRect(255,154,16,16,0xFFFF);
    } else {
      Lcd.fillRect(256,155,14,14,0x0000);
      Lcd.drawRect(255,154,16,16,0xFFFF);
    }
    if(Key.key_flags.s2_down) {
      Lcd.fillRect(255,194,16,16,0xFFFF);
    } else {
      Lcd.fillRect(256,195,14,14,0x0000);
      Lcd.drawRect(255,194,16,16,0xFFFF);
    }
    if(Key.key_flags.s2_left) {
      Lcd.fillRect(235,174,16,16,0xFFFF);
    } else {
      Lcd.fillRect(236,175,14,14,0x0000);
      Lcd.drawRect(235,174,16,16,0xFFFF);
    }
    if(Key.key_flags.s2_right) {
      Lcd.fillRect(274,174,16,16,0xFFFF);
    } else {
      Lcd.fillRect(275,175,14,14,0x0000);
      Lcd.drawRect(274,174,16,16,0xFFFF);
    }


    if(Key.key_flags.select && Key.key_flags.start) {
      return;
    }
  }
}

void menu_led_test(void) {
  Lcd.setTextSize(1);
  Lcd.setCursor(0,232);
  Lcd.print_5x8("press SELECT & START to exit menu");
  Lcd.setTextSize(2);
  Lcd.setCursor(0,0);
  Lcd.print_5x8("LED test");

  while(1) {
    sleep_ms(100);

    for(int i=0; i<100; i++) {
      float b = (float)(i)/100;
      Led.set_bright_float(1, b);
      Led.set_bright_float(2, b);
      Led.set_bright_float(3, b);
      Led.set_bright_float(4, b);
      sleep_ms(10);

      if(Key.key_flags.select && Key.key_flags.start) {
        Led.set_bright(1, 0);
        Led.set_bright(2, 0);
        Led.set_bright(3, 0);
        Led.set_bright(4, 0);
        return;
      }
    }
    for(int i=0; i<100; i++) {
      float b = (float)(99-i)/100;
      Led.set_bright_float(1, b);
      Led.set_bright_float(2, b);
      Led.set_bright_float(3, b);
      Led.set_bright_float(4, b);
      sleep_ms(10);

      if(Key.key_flags.select && Key.key_flags.start) {
        Led.set_bright(1, 0);
        Led.set_bright(2, 0);
        Led.set_bright(3, 0);
        Led.set_bright(4, 0);
        return;
      }
    }
  }
}

void menu_lcd_test(void) {
  Lcd.setTextSize(1);
  Lcd.setCursor(0,232);
  Lcd.print_5x8("press SELECT & START to exit menu");
  Lcd.setTextSize(2);
  Lcd.setCursor(0,0);
  Lcd.print_5x8("LCD test");

  while(1) {
    sleep_ms(10);

    if(Key.key_flags.select && Key.key_flags.start) {
      return;
    }
  }
}

void menu_dac_test(void) {
  Lcd.setTextSize(1);
  Lcd.setCursor(0,232);
  Lcd.print_5x8("press SELECT & START to exit menu");
  Lcd.setTextSize(2);
  Lcd.setCursor(0,0);
  Lcd.print_5x8("DAC test");

  Lcd.setCursor(0,16);
  Lcd.print_5x8("press A to play sound");

  while(1) {
    sleep_ms(100);
    
    if(Key.key_flags.a) {
      Dac.play_music_ex(Music_Test, 10, 100);
      Dac.set_mute(true);
    }

    if(Key.key_flags.select && Key.key_flags.start) {
      return;
    }
  }
}

void menu_bat_test(void) {
  Lcd.setTextSize(1);
  Lcd.setCursor(0,232);
  Lcd.print_5x8("press SELECT & START to exit menu");
  Lcd.setTextSize(2);
  Lcd.setCursor(0,0);
  Lcd.print_5x8("Battery test");

  while(1) {
    sleep_ms(100);
    char string_buf[32];
    sprintf(string_buf, "bat level   : % 3.1f%%", Bat.level);
    Lcd.setCursor(0,16);
    Lcd.print_5x8(string_buf);
    sprintf(string_buf, "bat voltage : %01.3fV", Bat.voltage);
    Lcd.setCursor(0,32);
    Lcd.print_5x8(string_buf);

    if(Key.key_flags.select && Key.key_flags.start) {
      return;
    }
  }
}

void menu_ir_test(void) {
  Lcd.setTextSize(1);
  Lcd.setCursor(0,232);
  Lcd.print_5x8("press SELECT & START to exit menu");
  Lcd.setTextSize(2);
  Lcd.setCursor(0,0);
  Lcd.print_5x8("IR LED test");

  Lcd.setCursor(0,16);
  Lcd.print_5x8("press A to turn on IR LED");
  
  while(1) {
    sleep_ms(10);

    if(Key.key_flags.start) {
      Ir.send_nec_format(true, 0x00, 0xff, 0x09); // 전원
      sleep_ms(500);
    } else if(Key.key_flags.select) {
      Ir.send_nec_format(true, 0x00, 0xff, 0x40); // 외부입력
      sleep_ms(500);
    } else if(Key.key_flags.a) {
      Ir.send_nec_format(true, 0x00, 0xff, 0x16); // 선택
      sleep_ms(500);
    } else if(Key.key_flags.b) {
      Ir.send_nec_format(true, 0x00, 0xff, 0x10); // 나가기
      sleep_ms(500);
    } else if(Key.key_flags.s1_up) {
      Ir.send_nec_format(true, 0x00, 0xff, 0x11); // 상
      sleep_ms(500);
    } else if(Key.key_flags.s1_down) {
      Ir.send_nec_format(true, 0x00, 0xff, 0x04); // 하
      sleep_ms(500);
    } else if(Key.key_flags.s1_left) {
      Ir.send_nec_format(true, 0x00, 0xff, 0x12); // 좌
      sleep_ms(500);
    } else if(Key.key_flags.s1_right) {
      Ir.send_nec_format(true, 0x00, 0xff, 0x4d); // 우
      sleep_ms(500);
    } else {
      Ir.menual_off();
    } 

    if(Key.key_flags.select && Key.key_flags.start) {
      return;
    }
  }
}

void menu_sd_test(void) {
  Lcd.setTextSize(1);
  Lcd.setCursor(0,232);
  Lcd.print_5x8("press SELECT & START to exit menu");
  Lcd.setTextSize(2);
  Lcd.setCursor(0,0);
  Lcd.print_5x8("SD card test");

  sleep_ms(100);
  char string_buf[32];
  sprintf(string_buf, "SD card : %s", Sdcard.card_check() ? "inserted    " : "not inserted");
  Lcd.setCursor(0,16);
  Lcd.print_5x8(string_buf);

  int ret = Sdcard.card_init();
  if(ret < 0) {
    Sdcard.card_deinit();
  }

  sprintf(string_buf, "inited : %s", Sdcard.info.is_inited ? "yes" : "error");
  Lcd.setCursor(0,32);
  Lcd.print_5x8(string_buf);

  if(!Sdcard.info.is_inited) {
    sprintf(string_buf, "(%d)", ret);
    Lcd.print_5x8(string_buf);

  } else {
    Lcd.setCursor(0,48);
    Lcd.print_5x8("SD card type : ");
  
    switch (Sdcard.info.type)
    {
    case SD_TYPE_SDSC:
      Lcd.print_5x8("SDSC");
      break;
    case SD_TYPE_SDHC:
      if(Sdcard.info.size > 34359738368) { // 34359738368 = 1024^3
        Lcd.print_5x8("SDXC");
      } else {
        Lcd.print_5x8("SDHC");
      }
      break;
    default:
      Lcd.print_5x8("UNKNOWN");
      break;
    }

    Lcd.setCursor(0,64);
    Lcd.print_5x8("size : ");
    sprintf(string_buf, "%llu MB", (Sdcard.info.size / 1000000));
    Lcd.print_5x8(string_buf);

    int ret;
    uint8_t buf[512] = {0,};
    ret = Sdcard.sector_read(0, buf);
    if(ret < 0) {
      wprintf(L"sector_read error(%d)\n", ret);
    }
    wprintf(L"SD SECTOR[0] : ");
    for(int i=0; i<512; i++)
    {
      wprintf(L"%02X ", buf[i]);
    }
    wprintf(L"\n");

    ret = Sdcard.sector_read(1, buf);
    if(ret < 0) {
      wprintf(L"sector_read error(%d)\n", ret);
    }
    wprintf(L"SD SECTOR[1] : ");
    for(int i=0; i<512; i++)
    {
      wprintf(L"%02X ", buf[i]);
    }
    wprintf(L"\n");
  }

  while(1) {
    sleep_ms(100);

    if(Key.key_flags.select && Key.key_flags.start) {
      return;
    }
  }
}

void menu_usb_test(void) {
  Lcd.setTextSize(1);
  Lcd.setCursor(0,232);
  Lcd.print_5x8("press SELECT & START to exit menu");
  Lcd.setTextSize(2);
  Lcd.setCursor(0,0);
  Lcd.print_5x8("USB(bottom) test");

  while(1) {
    sleep_ms(100);
  
    if(Key.key_flags.select && Key.key_flags.start) {
      return;
    }
  }
}

void menu_bt_test(void) {
  Lcd.setTextSize(1);
  Lcd.setCursor(0,232);
  Lcd.print_5x8("press SELECT & START to exit menu");
  Lcd.setTextSize(2);
  Lcd.setCursor(0,0);
  Lcd.print_5x8("Bluetooth test");

  while(1) {
    sleep_ms(100);

    if(Key.key_flags.select && Key.key_flags.start) {
      return;
    }
  }
}

void menu_wifi_test(void) {
  Lcd.setTextSize(1);
  Lcd.setCursor(0,232);
  Lcd.print_5x8("press SELECT & START to exit menu");
  Lcd.setTextSize(2);
  Lcd.setCursor(0,0);
  Lcd.print_5x8("Wi-Fi test");

  cyw43_arch_enable_sta_mode();

  absolute_time_t scan_time = nil_time;
  Wifi.ap_scan(scan_time);

  Lcd.setCursor(0,16);
  Lcd.print_5x8("Wi-Fi ap, rssi, channel");

  char string_buf[32];

  for(int i=0; i<ap_num; i++) {
    sprintf(string_buf, "%20s, %4d, %3d\n", ap_list[i].ssid, ap_list[i].rssi, ap_list[i].chan);
    Lcd.setCursor(0, 32 + (16 * ap_num));
    Lcd.print_5x8(string_buf);
  }
  
  while(1) {
    sleep_ms(100);
  
    if(Key.key_flags.select && Key.key_flags.start) {
      return;
    }
  }
}