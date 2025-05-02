// sound frequency header

#define	FREQ_C1       32.704
#define	FREQ_C1X      34.648
#define	FREQ_D1       36.709
#define	FREQ_D1X      38.891
#define	FREQ_E1       41.204
#define	FREQ_F1       43.654
#define	FREQ_F1X      46.250
#define	FREQ_G1       49.000
#define	FREQ_G1X      51.913
#define	FREQ_A1       55.000
#define	FREQ_A1X      58.271
#define	FREQ_B1       61.734

#define	FREQ_C2       65.407
#define	FREQ_C2X      69.296
#define	FREQ_D2       73.417
#define	FREQ_D2X      77.782
#define	FREQ_E2       82.407
#define	FREQ_F2       87.307
#define	FREQ_F2X      92.499
#define	FREQ_G2       97.999
#define	FREQ_G2X     103.826
#define	FREQ_A2      110.000
#define	FREQ_A2X     116.541
#define	FREQ_B2      123.471

#define	FREQ_C3      130.813
#define	FREQ_C3X     138.592
#define	FREQ_D3      146.833
#define	FREQ_D3X     155.564
#define	FREQ_E3      164.814
#define	FREQ_F3      174.614
#define	FREQ_F3X     184.997
#define	FREQ_G3      195.998
#define	FREQ_G3X     207.652
#define	FREQ_A3      220.000
#define	FREQ_A3X     233.082
#define	FREQ_B3      246.942

#define	FREQ_C4      261.626
#define	FREQ_C4X     277.183
#define	FREQ_D4      293.665
#define	FREQ_D4X     311.127
#define	FREQ_E4      329.628
#define	FREQ_F4      349.228
#define	FREQ_F4X     369.994
#define	FREQ_G4      391.995
#define	FREQ_G4X     415.305
#define	FREQ_A4      440.000
#define	FREQ_A4X     466.164
#define	FREQ_B4      493.883

#define	FREQ_C5      523.251
#define	FREQ_C5X     554.365
#define	FREQ_D5      587.330
#define	FREQ_D5X     622.254
#define	FREQ_E5      659.255
#define	FREQ_F5      698.456
#define	FREQ_F5X     739.989
#define	FREQ_G5      783.991
#define	FREQ_G5X     830.609
#define	FREQ_A5      880.000
#define	FREQ_A5X     932.328
#define	FREQ_B5      987.766

#define	FREQ_C6     1046.502
#define	FREQ_C6X    1108.730
#define	FREQ_D6     1174.660
#define	FREQ_D6X    1244.508
#define	FREQ_E6     1318.510
#define	FREQ_F6     1396.912
#define	FREQ_F6X    1479.978
#define	FREQ_G6     1567.982
#define	FREQ_G6X    1661.218
#define	FREQ_A6     1760.000
#define	FREQ_A6X    1864.656
#define	FREQ_B6     1975.532

#define	FREQ_C7     2093.004
#define	FREQ_C7X    2217.460
#define	FREQ_D7     2349.320
#define	FREQ_D7X    2489.016
#define	FREQ_E7     2637.020
#define	FREQ_F7     2793.824
#define	FREQ_F7X    2959.956
#define	FREQ_G7     3135.964
#define	FREQ_G7X    3322.436
#define	FREQ_A7     3520.000
#define	FREQ_A7X    3729.312
#define	FREQ_B7     3951.064

const float sound_freq_table[7][12] = {
  {FREQ_C1, FREQ_C1X, FREQ_D1, FREQ_D1X, FREQ_E1, FREQ_F1, FREQ_F1X, FREQ_G1, FREQ_G1X, FREQ_A1, FREQ_A1X, FREQ_B1},
  {FREQ_C2, FREQ_C2X, FREQ_D2, FREQ_D2X, FREQ_E2, FREQ_F2, FREQ_F2X, FREQ_G2, FREQ_G2X, FREQ_A2, FREQ_A2X, FREQ_B2},
  {FREQ_C3, FREQ_C3X, FREQ_D3, FREQ_D3X, FREQ_E3, FREQ_F3, FREQ_F3X, FREQ_G3, FREQ_G3X, FREQ_A3, FREQ_A3X, FREQ_B3},
  {FREQ_C4, FREQ_C4X, FREQ_D4, FREQ_D4X, FREQ_E4, FREQ_F4, FREQ_F4X, FREQ_G4, FREQ_G4X, FREQ_A4, FREQ_A4X, FREQ_B4},
  {FREQ_C5, FREQ_C5X, FREQ_D5, FREQ_D5X, FREQ_E5, FREQ_F5, FREQ_F5X, FREQ_G5, FREQ_G5X, FREQ_A5, FREQ_A5X, FREQ_B5},
  {FREQ_C6, FREQ_C6X, FREQ_D6, FREQ_D6X, FREQ_E6, FREQ_F6, FREQ_F6X, FREQ_G6, FREQ_G6X, FREQ_A6, FREQ_A6X, FREQ_B6},
  {FREQ_C7, FREQ_C7X, FREQ_D7, FREQ_D7X, FREQ_E7, FREQ_F7, FREQ_F7X, FREQ_G7, FREQ_G7X, FREQ_A7, FREQ_A7X, FREQ_B7}
};

