#include "MCP4725.hpp"
#include "Sound_Freq.h"
#include "Sound_Waves.h"

volatile dac_set_t dac_ch[DAC_CH_MAX] = {
  {0, 0, 0, 0, DAC_wave_none},
  {0, 0, 0, 0, DAC_wave_none},
  {0, 0, 0, 0, DAC_wave_none},
  {0, 0, 0, 0, DAC_wave_none}
};

volatile bool delay_ended = false;
volatile bool muted = false;
repeating_timer_t timer_delay;

bool timer_callback_mcp4725(struct repeating_timer *t) {
  unsigned int DAC_buffer = 0;                  // dac buffer (0~4095)
  float DAC_buffer_f = 0;                       // dac buffer (0~4095)
  uint8_t buf[2] = {0x03, 0};

  if(muted) {
    i2c_write_blocking(DAC_I2C_CH, DAC_L, buf, 2, false);
    i2c_write_blocking(DAC_I2C_CH, DAC_R, buf, 2, false);
    return true;
  }

  for(int i=0; i<DAC_CH_MAX; i++) {
    
    DAC_buffer_f += 
      (dac_ch[i].wave[dac_ch[i].wave_count] * dac_ch[i].volume);
    
    dac_ch[i].count += dac_ch[i].count_up;
    
    dac_ch[i].wave_count += (dac_ch[i].count / WAVE_RES);
    dac_ch[i].wave_count %= WAVE_LENGTH;
    
    dac_ch[i].count = dac_ch[i].count % WAVE_RES;
  }
  
  DAC_buffer = (int)DAC_buffer_f;
  if(DAC_buffer > DAC_HW_MAX) DAC_buffer = DAC_HW_MAX;

  buf[0] = DAC_buffer >> 8;
  buf[1] = DAC_buffer;

  i2c_write_blocking(DAC_I2C_CH, DAC_L, buf, 2, false);
  i2c_write_blocking(DAC_I2C_CH, DAC_R, buf, 2, false);
  return true;
}

int64_t delay_callback(alarm_id_t id, void *user_data) {
    delay_ended = true;
    return 0;
}

MCP4725::MCP4725(int pin_scl, int pin_sda) {
  _pin_scl = pin_scl;
  _pin_sda = pin_sda;
}

void MCP4725::init(void) {

  i2c_init(DAC_I2C_CH, 1000 * 1000);             //Initialize I2C on i2c0 port with 1000kHz
  gpio_set_function(_pin_scl, GPIO_FUNC_I2C); //Use GPIO as I2C
  gpio_set_function(_pin_sda, GPIO_FUNC_I2C);
  gpio_pull_up(_pin_scl);                     //Pull up GPIO
  gpio_pull_up(_pin_sda);

  add_repeating_timer_us(-(1000000/WAVE_FREQ), timer_callback_mcp4725, NULL, &timer_delay);
}

void MCP4725::data(uint8_t addr, uint16_t data) {
  uint8_t buf[2] = {0, 0};

  buf[0] = data >> 8;
  buf[1] = data;
  i2c_write_blocking(DAC_I2C_CH, addr, buf, 2, false);
}

void MCP4725::menual_beep(uint32_t ms) {

  for(int i=0; i<(ms); i++) {
    data(DAC_L, 0);
    data(DAC_R, 0);
    sleep_us(500);
    data(DAC_L, 4095);
    data(DAC_R, 4095);
    sleep_us(500);
  }
}

void MCP4725::menual_mute(void) {
  uint8_t buf[2] = {0x03, 0}; // power-down mode

  i2c_write_blocking(DAC_I2C_CH, DAC_L, buf, 2, false);
  i2c_write_blocking(DAC_I2C_CH, DAC_R, buf, 2, false);
}

void MCP4725::beep(uint32_t ms) {

  set_mute(false);
  channel_wave(0, 1000, 15, DAC_wave_pulse_50);
  delay(ms);
  set_mute(true);
}

void MCP4725::set_mute(bool mute) {
  muted = mute;
}

void MCP4725::set_waning(uint32_t waning) {
  _sound_waning = ((float)waning / (DAC_VOL_MAX*2));
}

void MCP4725::sound_waning(void) {
  for(int i=0; i<DAC_CH_MAX; i++) {
    if(dac_ch[i].volume > _sound_waning) {
      dac_ch[i].volume -= _sound_waning;
    } else {
      dac_ch[i].volume = 0;
    }
  }
}

void MCP4725::delay(uint32_t ms) {
  add_alarm_in_ms(ms, delay_callback, NULL, false);
  delay_ended = false;

  sound_waning();
  // Wait for alarm callback to set timer_fired
  while (!delay_ended) {
    sleep_us(10);
  }
}

void MCP4725::channel_set(uint8_t channel, float freq, uint8_t volume) {
  
  float count_calc = (freq * WAVE_LENGTH) / WAVE_FREQ * WAVE_RES;
  if(volume > DAC_VOL_MAX) volume = DAC_VOL_MAX;
  
  dac_ch[channel].count_up = (int)count_calc;
  dac_ch[channel].volume = (float)volume / (DAC_VOL_MAX*2);
}

void MCP4725::channel_wave(uint8_t channel, float freq, uint8_t volume, const float* wave) {
  
  channel_set(channel, freq, volume);
  dac_ch[channel].wave = wave;
}

void MCP4725::channel_stop(uint8_t channel) {
  
  dac_ch[channel].wave_count = 0;
  dac_ch[channel].count = 0;
  dac_ch[channel].count_up = 0;
  dac_ch[channel].volume = 0;
  dac_ch[channel].wave = DAC_wave_none;
}

void MCP4725::play_sound(uint16_t (*music_table)[USING_CH]) {
    
  unsigned int music_temp;
  unsigned int music_wav;
  unsigned int music_vol;
  unsigned int music_freq1;
  unsigned int music_freq2;
  const float* sound_wave;
  
  unsigned int ch_start;
  unsigned int ch_end;

  ch_start = 0;
  ch_end = 4;
  
  for(int i=ch_start; i<ch_end; i++) {
    music_temp = music_table[music_table_pos][i];
    
    music_wav   = music_temp >> 12;
    music_vol   = (music_temp >> 8) & 0x000F;
    music_freq1 = (music_temp >> 4) & 0x000F;
    music_freq2 = music_temp & 0x000F;
    
    switch(music_wav) {
      case W_NONE:
        sound_wave = DAC_wave_none;
        break;
      case W_PS25:
        sound_wave = DAC_wave_pulse_25;
        break;
      case W_PS50:
        sound_wave = DAC_wave_pulse_50;
        break;
      case W_PS75:
        sound_wave = DAC_wave_pulse_75;
        break;
      case W_TRIN:
        sound_wave = DAC_wave_triangle;
        break;
      case W_SAWT:
        sound_wave = DAC_wave_sawtooth;
        break;
      case W_SINE:
        sound_wave = DAC_wave_sine;
        break;
      case W_NOIZ:
        sound_wave = DAC_wave_noise;
        break;
    }
    channel_wave(i, sound_freq_table[music_freq1 - 1][music_freq2 - 1], music_vol, sound_wave);
  }
}

void MCP4725::play_music(uint16_t (*music_table)[USING_CH], uint32_t music_length, uint32_t music_delay) {

  for(int i=0; i<music_length; i++) {
    music_table_pos = i;
    play_sound(music_table);
    delay(music_delay);
  }
  music_table_pos = 0;
}

void MCP4725::play_sound_ex(const uint32_t music_table) {
  unsigned int music_ch;
  unsigned int music_wav;
  unsigned int music_vol;
  unsigned int music_freq1;
  unsigned int music_freq2;
  const float* sound_wave;

  music_ch     = (music_table >> 20) & 0x0000000F;
  music_wav    = (music_table >> 16) & 0x0000000F;
  music_vol    = (music_table >> 8)  & 0x000000FF;
  music_freq1  = (music_table >> 4)  & 0x0000000F;
  music_freq2  = music_table         & 0x0000000F;
  
  switch(music_wav) {
    case W_NONE:
      sound_wave = DAC_wave_none;
      break;
    case W_PS25:
      sound_wave = DAC_wave_pulse_25;
      break;
    case W_PS50:
      sound_wave = DAC_wave_pulse_50;
      break;
    case W_PS75:
      sound_wave = DAC_wave_pulse_75;
      break;
    case W_TRIN:
      sound_wave = DAC_wave_triangle;
      break;
    case W_SAWT:
      sound_wave = DAC_wave_sawtooth;
      break;
    case W_SINE:
      sound_wave = DAC_wave_sine;
      break;
    case W_NOIZ:
      sound_wave = DAC_wave_noise;
      break;
  }
  channel_wave(music_ch, sound_freq_table[music_freq1 - 1][music_freq2 - 1], music_vol, sound_wave);
}

void MCP4725::play_music_ex(const uint32_t *music_table, uint32_t music_length, uint32_t music_delay) {
  unsigned int music_opcode;

  set_mute(false);

  for(uint32_t i=0; i<music_length; i++) {
    music_opcode = (music_table[i] >> 24);
    switch(music_opcode) {
      case OP_NONE:
        play_sound_ex(music_table[i]);
        break;
      case OP_DELAY4:
        delay(music_delay);
      case OP_DELAY3:
        delay(music_delay);
      case OP_DELAY2:
        delay(music_delay);
      case OP_DELAY1:
        delay(music_delay);
        break;
      case OP_MUTE:
        set_mute(true);
        break;
      case OP_STOP:
        i = music_length;
        for(int c=0; c<USING_CH; c++) {
          channel_stop(c);
        }
        break;
    }
  }
}