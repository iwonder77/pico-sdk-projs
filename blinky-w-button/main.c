#include "pico/stdlib.h"
#include <hardware/gpio.h>
#include <stdio.h>

#define LED_DELAY_MS 2000u
#define STATUS_LED_PIN 25u
#define BUTTON_INPUT_PIN 22u

// Initialize the GPIO for the LED and button
static void init_pins(void) {
  gpio_init(STATUS_LED_PIN);
  gpio_set_dir(STATUS_LED_PIN, GPIO_OUT);
  gpio_init(BUTTON_INPUT_PIN);
  gpio_set_dir(BUTTON_INPUT_PIN, GPIO_IN);
  // I'm using an external pull up, so disable all pull resistors (down and up)
  // on this pin with the following:
  gpio_disable_pulls(BUTTON_INPUT_PIN);
  // P.S. if you wanted to, this is how you'd enable the internal pull up:
  // gpio_pull_up(BUTTON_INPUT_PIN);
}

// Turn the LED on or off
static void set_led(bool led_on) {
  // Just set the GPIO on or off
  gpio_put(STATUS_LED_PIN, led_on);
}

int main(void) {
  int press_count = 0;
  stdio_init_all();
  sleep_ms(10000);
  bool previous_reading, current_reading = gpio_get(BUTTON_INPUT_PIN);
  printf("---PICO BLINKY STARTUP LINE---\n");
  init_pins();
  while (true) {
    current_reading = gpio_get(BUTTON_INPUT_PIN);
    if (current_reading != previous_reading && current_reading == 0) {
      press_count++;
      printf("PRESSED! count: %d\n", press_count);
      set_led(true);
      previous_reading = current_reading;
    } else if (current_reading == previous_reading && current_reading == 0) {
      // press already detected and updated, still light it up
      set_led(true);
    } else {
      set_led(false);
      previous_reading = current_reading;
    }
  }
}
