#include "sound_system.hpp"
#include "sound_freq.h"
#include "sound_waves.h"

volatile dac_set_t dac_ch[SND_CH_MAX] = {
  {0, 0, 0, 0, SND_wave_none},
  {0, 0, 0, 0, SND_wave_none},
  {0, 0, 0, 0, SND_wave_none},
  {0, 0, 0, 0, SND_wave_none}
};

struct sound_system_data_t* _sound_data;
repeating_timer_t timer_sound;
volatile bool delay_ended = false;

bool timer_callback_sound(struct repeating_timer *t) {
  unsigned int SND_buffer;  // sound buffer (0~4095)
  float SND_buffer_f;       // sound buffer (0~4095)

  for(int i=0; i < (_sound_data->buf_size/2); i++) {
    if(_sound_data->buf_write_pos >= _sound_data->buf_read_pos) {
      break;
    }

    SND_buffer = 0;
    SND_buffer_f = 0;

    for(int j=0; j<SND_CH_MAX; j++) {
      
      SND_buffer_f += 
        (dac_ch[j].wave[dac_ch[j].wave_count] * dac_ch[j].volume);
      
      dac_ch[j].count += dac_ch[j].count_up;
      
      dac_ch[j].wave_count += (dac_ch[j].count / SND_WAVE_RES);
      dac_ch[j].wave_count %= SND_WAVE_LENGTH;
      
      dac_ch[j].count = dac_ch[j].count % SND_WAVE_RES;
    }
    
    SND_buffer = (int)SND_buffer_f;
    if(SND_buffer > _sound_data->dac_max_value)
      SND_buffer = _sound_data->dac_max_value;

    _sound_data->buf[_sound_data->buf_write_pos] = SND_buffer;
    _sound_data->buf_write_pos += 1;
    if (_sound_data->buf_write_pos >= _sound_data->buf_size) {
      _sound_data->buf_write_pos = 0; // Reset write position if it exceeds buffer size
    }

  }
  return true;
}

soundSystem::soundSystem(void) {

}

void soundSystem::init(struct sound_system_data_t *sound_data) {
  _sound_data = sound_data;

  for(int i=0; i<SND_CH_MAX; i++) {
    dac_ch[i].volume      = 0;
    dac_ch[i].wave_count  = 0;
    dac_ch[i].count       = 0;
    dac_ch[i].count_up    = 0;
    dac_ch[i].wave        = SND_wave_none;
  }
  
  add_repeating_timer_us(-(1000000/1000), timer_callback_sound, NULL, &timer_sound); // 1ms sound timer
}

void soundSystem::beep(uint32_t ms) {
  //test beep
  /*
  for(int i=0; i<_sound_data->buf_size; i++) {
    if(i & 0x04)
      _sound_data->buf[i] = 256;
  }
  _sound_data->buf_read_pos = 0;
  _sound_data->mode = SND_OUTPUT_MODE_MONO;
  */
  channel_wave(0, 130.813, 15, SND_wave_pulse_50);
  _sound_data->mode = SND_OUTPUT_MODE_MONO;
  sleep_ms(ms);
}

void soundSystem::channel_set(uint8_t channel, float freq, uint8_t volume) {
  
  float count_calc = (freq * SND_WAVE_LENGTH) / _sound_data->dac_max_value * SND_WAVE_RES;
  if(volume > SND_VOL_MAX) volume = SND_VOL_MAX;
  
  dac_ch[channel].count_up = (int)count_calc;
  dac_ch[channel].volume = (float)volume / (SND_VOL_MAX*2);
}

void soundSystem::channel_wave(uint8_t channel, float freq, uint8_t volume, const float* wave) {
  
  channel_set(channel, freq, volume);
  dac_ch[channel].wave = wave;
}

void soundSystem::channel_stop(uint8_t channel) {
  
  dac_ch[channel].wave_count = 0;
  dac_ch[channel].count = 0;
  dac_ch[channel].count_up = 0;
  dac_ch[channel].volume = 0;
  dac_ch[channel].wave = SND_wave_none;
}

/*
void soundSystem::play_sound(uint16_t (*music_table)[USING_CH]) {
    
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

void soundSystem::play_music(uint16_t (*music_table)[USING_CH], uint32_t music_length, uint32_t music_delay) {

  for(int i=0; i<music_length; i++) {
    music_table_pos = i;
    play_sound(music_table);
    delay(music_delay);
  }
  music_table_pos = 0;
}
*/