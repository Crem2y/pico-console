#include "pico/stdlib.h"
#ifdef PICO_W
#include "pico/cyw43_arch.h"
#endif

int built_in_led_init(void);
void built_in_led_on(void);
void built_in_led_off(void);

int built_in_led_init(void) {
  #ifdef PICO_W
  if (cyw43_arch_init()) {
    wprintf(L"Wi-Fi init failed");
    return -1;
  }
  #else
  gpio_init(PICO_DEFAULT_LED_PIN);
  gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
  #endif
  return 0;
}

void built_in_led_on(void) {
  #ifdef PICO_W
  cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
  #else
  gpio_put(LED_PIN, 1);
  #endif
}

void built_in_led_off(void) {
  #ifdef PICO_W
  cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
  #else
  gpio_put(LED_PIN, 0);
  #endif
}