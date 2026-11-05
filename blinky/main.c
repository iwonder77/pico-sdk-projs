#include "pico/stdlib.h"

#ifndef LED_DELAY_MS
#define LED_DELAY_MS 500
#endif

#ifndef PICO_DEFAULT_LED_PIN
#warning blink_simple example requires a board with a regular LED
#endif

// Initialize the GPIO for the LED
static void init_led(void) {
#ifdef PICO_DEFAULT_LED_PIN
  // A device like Pico that uses a GPIO for the LED will define
  // PICO_DEFAULT_LED_PIN so we can use normal GPIO functionality to turn the
  // led on and off
  gpio_init(PICO_DEFAULT_LED_PIN);
  gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
#endif
}

// Turn the LED on or off
static void set_led(bool led_on) {
#if defined(PICO_DEFAULT_LED_PIN)
  // Just set the GPIO on or off
  gpio_put(PICO_DEFAULT_LED_PIN, led_on);
#endif
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
