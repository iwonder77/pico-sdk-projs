#include "pico/stdlib.h"

#define LED_DELAY_MS 100

// Initialize the GPIO for the LED
static void init_led(void) {
  gpio_init(PICO_DEFAULT_LED_PIN);
  gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
}

// Turn the LED on or off
static void set_led(bool led_on) {
  // Just set the GPIO on or off
  gpio_put(PICO_DEFAULT_LED_PIN, led_on);
}

int main(void) {
  init_led();
  while (true) {
    set_led(true);
    sleep_ms(LED_DELAY_MS);
    set_led(false);
    sleep_ms(LED_DELAY_MS);
  }
}
