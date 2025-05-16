#pragma once
// PCM2554 lib

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"

#define KEY_LOG_MAX 32 // set key log length
#define KEY_I2C_CH i2c1
#define KEY_IC_A 0x38 // PCM2554 (A) address
#define KEY_IC_B 0x39 // PCM2554 (B) address
#define KEY_IC_C 0x3A // PCM2554 (C) address

enum key_code {
  KEY_S1_UP,
  KEY_S1_DOWN,
  KEY_S1_LEFT,
  KEY_S1_RIGHT,
  KEY_RIGHT,
  KEY_LEFT,
  KEY_DOWN,
  KEY_UP,
  KEY_S2_UP,
  KEY_S2_DOWN,
  KEY_S2_LEFT,
  KEY_S2_RIGHT,
  KEY_Y,
  KEY_X,
  KEY_B,
  KEY_A,
  KEY_S2_CENTER,
  KEY_START,
  KEY_SR,
  KEY_ZR,
  KEY_ZL,
  KEY_SL,
  KEY_SELECT,
  KEY_S1_CENTER,
  KEY_NONE = 0xff
};

enum key_code_hex {
  CODE_KEY_S1_UP      = 1 << KEY_S1_UP,
  CODE_KEY_S1_DOWN    = 1 << KEY_S1_DOWN,
  CODE_KEY_S1_LEFT    = 1 << KEY_S1_LEFT,
  CODE_KEY_S1_RIGHT   = 1 << KEY_S1_RIGHT,
  CODE_KEY_RIGHT      = 1 << KEY_RIGHT,
  CODE_KEY_LEFT       = 1 << KEY_LEFT,
  CODE_KEY_DOWN       = 1 << KEY_DOWN,
  CODE_KEY_UP         = 1 << KEY_UP,
  CODE_KEY_S2_UP      = 1 << KEY_S2_UP,
  CODE_KEY_S2_DOWN    = 1 << KEY_S2_DOWN,
  CODE_KEY_S2_LEFT    = 1 << KEY_S2_LEFT,
  CODE_KEY_S2_RIGHT   = 1 << KEY_S2_RIGHT,
  CODE_KEY_Y          = 1 << KEY_Y,
  CODE_KEY_X          = 1 << KEY_X,
  CODE_KEY_B          = 1 << KEY_B,
  CODE_KEY_A          = 1 << KEY_A,
  CODE_KEY_S2_CENTER  = 1 << KEY_S2_CENTER,
  CODE_KEY_START      = 1 << KEY_START,
  CODE_KEY_SR         = 1 << KEY_SR,
  CODE_KEY_ZR         = 1 << KEY_ZR,
  CODE_KEY_ZL         = 1 << KEY_ZL,
  CODE_KEY_SL         = 1 << KEY_SL,
  CODE_KEY_SELECT     = 1 << KEY_SELECT,
  CODE_KEY_S1_CENTER  = 1 << KEY_S1_CENTER,
  CODE_KEY_NONE       = 0x00000000
};

typedef struct _key_flags_t{
  union {
    struct  {
      uint32_t s1_up      : 1;
      uint32_t s1_down    : 1;
      uint32_t s1_left    : 1;
      uint32_t s1_right   : 1;
      uint32_t right      : 1;
      uint32_t left       : 1;
      uint32_t down       : 1;
      uint32_t up         : 1;
      
      uint32_t s2_up      : 1;
      uint32_t s2_down    : 1;
      uint32_t s2_left    : 1;
      uint32_t s2_right   : 1;
      uint32_t y          : 1;
      uint32_t x          : 1;
      uint32_t b          : 1;
      uint32_t a          : 1;
      
      uint32_t s2_center  : 1;
      uint32_t start      : 1;
      uint32_t sr         : 1;
      uint32_t zr         : 1;
      uint32_t zl         : 1;
      uint32_t sl         : 1;
      uint32_t select     : 1;
      uint32_t s1_center  : 1;
    };
    struct {
      uint8_t chip_a;
      uint8_t chip_b;
      uint8_t chip_c;
    };
    uint32_t flag_int;
  };
} key_flags_t;

class pca9554 {
  public:
    pca9554(int pin_scl, int pin_sda);

    uint8_t key_log[KEY_LOG_MAX] = {KEY_NONE,};
    uint32_t key_pressed = 0x00000000;
    key_flags_t key_flags;

    void init(void);
    void get_btn_data(void);
    void wait_until(uint8_t key, bool pressed);

    void log_add(uint8_t key);
    void log_reset(void);

  private:
    int _pin_scl, _pin_sda;
};