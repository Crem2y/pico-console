// PCM2554 lib

#include <stdio.h>

// for using unicode
#include <wchar.h>
#include <locale.h>

#include "pico/stdlib.h"
#include "pca9554.hpp"

pca9554::pca9554(int pin_scl, int pin_sda) {
  _pin_scl = pin_scl;
  _pin_sda = pin_sda;
}

void pca9554::init(void) {
  i2c_init(KEY_I2C_CH, 1000 * 1000);          //Initialize I2C on i2c1 port with 1000kHz
  gpio_set_function(_pin_scl, GPIO_FUNC_I2C); //Use GPIO as I2C
  gpio_set_function(_pin_sda, GPIO_FUNC_I2C);
  gpio_pull_up(_pin_scl);                     //Pull up GPIO pin
  gpio_pull_up(_pin_sda);

  uint8_t buf[2] = {0x02, 0xFF};              // set all ports of pca9554 to input  
  wprintf(L"KEY CHIP");
  i2c_write_blocking(KEY_I2C_CH, KEY_IC_A, buf, 2, false);
  wprintf(L"A");
  i2c_write_blocking(KEY_I2C_CH, KEY_IC_B, buf, 2, false);
  wprintf(L"B");
  i2c_write_blocking(KEY_I2C_CH, KEY_IC_C, buf, 2, false);
  wprintf(L"C OK\n");

  log_reset();
  key_pressed = 0x00000000;
}

void pca9554::get_btn_data(void) {
  uint8_t buffer[1] = {0};
  uint8_t buf[1] = {0x00}; // input register address of pca9554

  i2c_write_blocking(KEY_I2C_CH, KEY_IC_A, buf, 1, true);
  i2c_read_blocking(KEY_I2C_CH, KEY_IC_A, buffer, 1, false); // [S1_U, S1_D, S1_L, S1_R, KEY_U, KEY_D, KEY_L, KEY_R] 
  key_flags.chip_a = buffer[0];
  i2c_write_blocking(KEY_I2C_CH, KEY_IC_B, buf, 1, true);
  i2c_read_blocking(KEY_I2C_CH, KEY_IC_B, buffer, 1, false); // [S2_U, S2_D, S2_L, S2_R, KEY_A, KEY_B, KEY_X, KEY_Y]
  key_flags.chip_b = buffer[0];
  i2c_write_blocking(KEY_I2C_CH, KEY_IC_C, buf, 1, true);
  i2c_read_blocking(KEY_I2C_CH, KEY_IC_C, buffer, 1, false); // [S2_C, KEY_SE, KEY_SR, KEY_ZR, S1_C, KEY_ST, KEY_SL, KEY_ZL]
  key_flags.chip_c = buffer[0];

  uint32_t key_pressed_old = key_pressed;
  key_pressed = key_flags.flag_int;

  uint8_t mask = 0x01;
  for(int i=0; i<8; i++) {
    if(key_flags.chip_a & mask) {
      if(!(key_pressed_old & (0x00000001 << i))) {
        log_add(i);
      }
    }
    if(key_flags.chip_b & mask) {
      if(!(key_pressed_old & (0x00000100 << i))) {
        log_add(i+8);
      }
    }
    if(key_flags.chip_c & mask) {
      if(!(key_pressed_old & (0x00010000 << i))) {
        log_add(i+16);
      }
    }
    
    mask <<= 1;
  }

  //printf("%02x, %02x, %02x\n", buffer_a, buffer_b, buffer_c);
}

void pca9554::wait_until(uint8_t key, bool pressed) {
  while(true) {
    get_btn_data();
    if(pressed) {
      if(key_pressed & (0x00000001 << key)) {
        break;
      }
    } else {
      if(~key_pressed & (0x00000001 << key)) {
        break;
      }
    }
    sleep_ms(10);
  }
}

void pca9554::log_add(uint8_t key) {
  for(int i=0; i<KEY_LOG_MAX-1; i++) {
    key_log[KEY_LOG_MAX-1-i] = key_log[KEY_LOG_MAX-2-i];
  }
  key_log[0] = key;
}

void pca9554::log_reset(void) {
  for(int i=0; i<KEY_LOG_MAX; i++) {
    key_log[i] = KEY_NONE;
  }
}