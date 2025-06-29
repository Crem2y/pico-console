#pragma once
#include <stdint.h>
#include "pico/stdlib.h"

#define SND_CH_MAX 4
#define SND_VOL_MAX 255

#define SND_WAVE_RES 10000    // 계산 정확도

#define SND_USING_CH 4

volatile typedef struct _dac_set_t {
  float         volume;     // dac volume (0~1)
  unsigned int  wave_count; // dac wave count (0~127)
  unsigned int  count;      // dac count (x1000)
  unsigned int  count_up;   // dac count up value (x1000)
  const float*  wave;       // dac wave (pointer)
} dac_set_t;

enum wave {
  W_NONE = 0,
  W_PS25,
  W_PS50,
  W_PS75,
  W_TRIN,
  W_SAWT,
  W_SINE,
  W_NOIZ,
};

enum note {
  S_NN = 0,
  S_C, S_Cs,
  S_D, S_Ds,
  S_E,
  S_F, S_Fs,
  S_G, S_Gs,
  S_A, S_As,
  S_B
};

enum buffer_mode {
  SND_BUF_MODE_CIRCULAR = 0, // circular buffer mode
  SND_BUF_MODE_LINEAR, // linear buffer mode
};

enum output_mode {
  SND_OUTPUT_MODE_MUTE = 0,
  SND_OUTPUT_MODE_MONO,
  SND_OUTPUT_MODE_STEREO,
};

struct sound_system_data_t  // sound system data structure
{
  uint16_t *buf;     // sound buffer pointer
  uint32_t buf_size; // sound buffer size
  uint32_t buf_read_pos;
  uint32_t buf_write_pos;
  uint8_t buf_mode;
  uint32_t dac_max_value;
  uint32_t sampling_freq;
  uint8_t mode;
};

class soundSystem {
  public:
    soundSystem(void);

    void init(struct sound_system_data_t *sound_data);

    void beep(uint32_t ms);

    void channel_set(uint8_t channel, float freq, uint8_t volume);
    void channel_wave(uint8_t channel, float freq, uint8_t volume, const float* wave);
    void channel_stop(uint8_t channel);

    //void play_sound(uint16_t (*music_table)[USING_CH]);
    //void play_music(uint16_t (*music_table)[USING_CH], uint32_t music_length, uint32_t music_delay);
};