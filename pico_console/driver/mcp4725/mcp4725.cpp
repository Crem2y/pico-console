#include "mcp4725.hpp"

repeating_timer_t timer_dac;
struct mcp4725_data_t dac_data;

bool timer_callback_dac(struct repeating_timer *t) {
  if(dac_data.sound_data.mode == SND_OUTPUT_MODE_MUTE) {
    uint8_t output_buf[2] = {0x03, 0x00}; // Power-down mode

    i2c_write_blocking(dac_data.i2c, DAC_L, output_buf, 2, false);
    i2c_write_blocking(dac_data.i2c, DAC_R, output_buf, 2, false);

  } else if (dac_data.sound_data.mode == SND_OUTPUT_MODE_MONO) {
    uint8_t output_buf[2];
    uint16_t sound_data = dac_data.sound_data.buf[dac_data.sound_data.buf_read_pos];
    output_buf[0] = sound_data >> 8;
    output_buf[1] = sound_data & 0xFF;

    dac_data.sound_data.buf_read_pos += 1;

    if (dac_data.sound_data.buf_read_pos >= dac_data.sound_data.buf_size) {
      if (dac_data.sound_data.buf_mode == SND_BUF_MODE_LINEAR) {
        dac_data.sound_data.mode = SND_OUTPUT_MODE_MUTE; // Switch to mute mode if linear buffer is exhausted
      }
      dac_data.sound_data.buf_read_pos = 0; // Reset read position if it exceeds buffer size
    }

    i2c_write_blocking(dac_data.i2c, DAC_L, output_buf, 2, false);
    i2c_write_blocking(dac_data.i2c, DAC_R, output_buf, 2, false);

  } else if (dac_data.sound_data.mode == SND_OUTPUT_MODE_STEREO) {
    uint8_t output_buf[4];
    uint16_t sound_data = dac_data.sound_data.buf[dac_data.sound_data.buf_read_pos];
    output_buf[0] = sound_data >> 8;
    output_buf[1] = sound_data & 0xFF;
    sound_data = dac_data.sound_data.buf[dac_data.sound_data.buf_read_pos + 1];
    output_buf[2] = sound_data >> 8;
    output_buf[3] = sound_data & 0xFF;

    dac_data.sound_data.buf_read_pos += 2;

    if (dac_data.sound_data.buf_read_pos >= dac_data.sound_data.buf_size) {
      if (dac_data.sound_data.buf_mode == SND_BUF_MODE_LINEAR) {
        dac_data.sound_data.mode = SND_OUTPUT_MODE_MUTE; // Switch to mute mode if linear buffer is exhausted
      }
      dac_data.sound_data.buf_read_pos = 0; // Reset read position if it exceeds buffer size
    }

    i2c_write_blocking(dac_data.i2c, DAC_L, &output_buf[0], 2, false);
    i2c_write_blocking(dac_data.i2c, DAC_R, &output_buf[2], 2, false);
  }

  return true;
}

uint16_t sound_buffer[1024]= {0,};

mcp4725::mcp4725(i2c_inst_t* i2c, int pin_scl, int pin_sda, struct mcp4725_data_t* dac_data) {
  _i2c = i2c;
  _pin_scl = pin_scl;
  _pin_sda = pin_sda;

  dac_data->i2c = i2c;
  dac_data->sound_data.buf = sound_buffer;
  dac_data->sound_data.buf_size = sizeof(sound_buffer) / 2;
  dac_data->sound_data.buf_read_pos = 0;
  dac_data->sound_data.buf_write_pos = 0;
  dac_data->sound_data.buf_mode = SND_BUF_MODE_LINEAR;
  dac_data->sound_data.mode = SND_OUTPUT_MODE_MUTE;
  dac_data->sound_data.dac_max_value = DAC_HW_MAX; // 12-bit DAC max value
  dac_data->sound_data.sampling_freq = DAC_SAMPLING_FREQ;
}

void mcp4725::init(void) {

  i2c_init(_i2c, 1000 * 1000);             //Initialize I2C on i2c0 port with 1000kHz
  gpio_set_function(_pin_scl, GPIO_FUNC_I2C); //Use GPIO as I2C
  gpio_set_function(_pin_sda, GPIO_FUNC_I2C);
  gpio_pull_up(_pin_scl);                     //Pull up GPIO
  gpio_pull_up(_pin_sda);

  dac_data.sound_data.mode = SND_OUTPUT_MODE_MUTE;

  uint8_t output_buf[2] = {0x03, 0x00}; // Power-down mode
  i2c_write_blocking(dac_data.i2c, DAC_L, output_buf, 2, false);
  i2c_write_blocking(dac_data.i2c, DAC_R, output_buf, 2, false);

  add_repeating_timer_us(-(1000000/DAC_SAMPLING_FREQ), timer_callback_dac, NULL, &timer_dac);
}