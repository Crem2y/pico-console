#include "pca9554.hpp"

uint8_t conami_code[10] = {
  KEY_S1_UP,
  KEY_S1_UP,
  KEY_S1_DOWN,
  KEY_S1_DOWN,
  KEY_S1_LEFT,
  KEY_S1_RIGHT,
  KEY_S1_LEFT,
  KEY_S1_RIGHT,
  KEY_B,
  KEY_A
};

enum menu {
  MENU_MAIN,
  MENU_BTN_TEST,

};

enum menu_main {
  MM_BTN_TEST,
  MM_LED_TEST,
  MM_LCD_TEST,
  MM_DAC_TEST,
  MM_BAT_TEST,
  MM_IR_TEST,
  MM_SD_TEST,
  MM_USB_TEST,
  MM_BT_TEST,
  MM_WIFI_TEST
};

void menu_btn_test();
void menu_led_test();
void menu_lcd_test();
void menu_dac_test();
void menu_bat_test();
void menu_ir_test();
void menu_sd_test();
void menu_usb_test();
void menu_bt_test();
void menu_wifi_test();