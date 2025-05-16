// PICO W WIFI lib

#include "pico_w_wifi.hpp"

static int scan_result(void *env, const cyw43_ev_scan_result_t *result) {

  if (ap_num > 9) {
    return 0;
  }

  if (result) {
    wprintf(L"ssid: %-32s rssi: %4d chan: %3d mac: %02x:%02x:%02x:%02x:%02x:%02x sec: %u\n",
      result->ssid, result->rssi, result->channel,
      result->bssid[0], result->bssid[1], result->bssid[2], result->bssid[3], result->bssid[4], result->bssid[5],
      result->auth_mode);

    char string_buf[32];
    sprintf(string_buf, "%s", result->ssid);
    wprintf(L"string_buf = %s\n", string_buf);

    for(int i=0; i<32; i++) {
      ap_list[ap_num].ssid[i] = string_buf[i];
    }
    wprintf(L"ap_list[%d].ssid = %s\n", ap_num, ap_list[ap_num].ssid);

    ap_list[ap_num].rssi = result->rssi;
    ap_list[ap_num].chan = result->channel;
    ap_list[ap_num].bssid[0] = result->bssid[0];
    ap_list[ap_num].bssid[1] = result->bssid[1];
    ap_list[ap_num].bssid[2] = result->bssid[2];
    ap_list[ap_num].bssid[3] = result->bssid[3];
    ap_list[ap_num].bssid[4] = result->bssid[4];
    ap_list[ap_num].bssid[5] = result->bssid[5];
    //ap_list[ap_num].auth_mode = result->auth_mode;

    ap_num++;
  }
  return 0;
}

picoWWifi::picoWWifi(void) {

}

void picoWWifi::init(void) {
  //cyw43_arch_enable_sta_mode();
}

void picoWWifi::ap_scan(absolute_time_t scan_time) {
  bool scan_in_progress = false;
  ap_num = 0;

  if (absolute_time_diff_us(get_absolute_time(), scan_time) < 0) {
    if (!scan_in_progress) {
      cyw43_wifi_scan_options_t scan_options = {0};
      int err = cyw43_wifi_scan(&cyw43_state, &scan_options, NULL, scan_result);
      if (err == 0) {
        wprintf(L"\nPerforming wifi scan\n");
        scan_in_progress = true;
      } else {
        wprintf(L"Failed to start scan: %d\n", err);
        scan_time = make_timeout_time_ms(10000); // wait 10s and scan again
      }
    } else if (!cyw43_wifi_scan_active(&cyw43_state)) {
      scan_time = make_timeout_time_ms(10000); // wait 10s and scan again
      scan_in_progress = false; 
    }
  }

  cyw43_arch_poll();
  cyw43_arch_wait_for_work_until(scan_time);
}
