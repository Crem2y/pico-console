#pragma once
// PICO W WIFI lib

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "hardware/vreg.h"
#include "hardware/clocks.h"

// for using unicode
#include <wchar.h>
#include <locale.h>

typedef struct {
  char    ssid[32];
  int     rssi;
  int     chan;
  uint8_t bssid[6];
  int     sec;
} wifi_ap_t;

static wifi_ap_t ap_list[10];
static int ap_num = 0;

static int scan_result(void *env, const cyw43_ev_scan_result_t *result);

class PICO_W_WIFI {
  public:
  PICO_W_WIFI(void);

  void init(void);
  void ap_scan(absolute_time_t scan_time);

  private:
};