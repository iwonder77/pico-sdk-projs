#include "pico/stdlib.h"
#include <stdio.h>

#define LED_DELAY_MS 2000u
#define STATUS_LED_PIN 25u

// Initialize the GPIO for the LED
static void init_led(void) {
  gpio_init(STATUS_LED_PIN);
  gpio_set_dir(STATUS_LED_PIN, GPIO_OUT);
}

// Turn the LED on or off
static void set_led(bool led_on) {
  // Just set the GPIO on or off
  gpio_put(STATUS_LED_PIN, led_on);
}

int main(void) {
  int count = 0;
  stdio_init_all();
  sleep_ms(10000);
  printf("---PICO BLINKY STARTUP LINE---\n");
  init_led();
  while (true) {
    printf("count: %d\n", count);
    set_led(true);
    sleep_ms(LED_DELAY_MS);
    set_led(false);
    sleep_ms(LED_DELAY_MS);
    count++;
  }
}
