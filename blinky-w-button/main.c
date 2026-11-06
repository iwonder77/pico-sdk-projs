#include "pico/stdlib.h"
#include "pico/time.h"
#include <hardware/gpio.h>
#include <stdio.h>

#define LED_DELAY_MS 2000u
#define STATUS_LED_PIN 25u
#define BUTTON_INPUT_PIN 22u

const long BUTTON_DEBOUNCE_MS = 500;

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
  stdio_init_all();
  init_pins();

  unsigned int press_count = 0;
  bool last_raw_pressed = gpio_get(BUTTON_INPUT_PIN);
  bool stable_pressed = gpio_get(BUTTON_INPUT_PIN);
  long raw_changed_at_ms = 0;

  sleep_ms(10000);
  printf("---PICO BLINKY STARTUP LINE---\n");
  while (true) {
    unsigned long now = to_ms_since_boot(get_absolute_time());

    bool raw_pressed = gpio_get(BUTTON_INPUT_PIN);

    if (raw_pressed != last_raw_pressed) {
      // button press or release event occured, record timestamp here to
      // begin countdown and update previous stored reading
      raw_changed_at_ms = now;
      last_raw_pressed = raw_pressed;
    }

    if (raw_pressed != stable_pressed &&
        now - last_raw_pressed > BUTTON_DEBOUNCE_MS) {
      // enough time has passed since button changed state, update stable
      // reading variable and proceed to detecting event (falling or rising
      // edge)
      stable_pressed = raw_pressed;
      if (stable_pressed == 0) {
        // if falling edge was detected, update count and print it
        press_count++;
        printf("PRESSED! count: %u\n", press_count);
      } else {
        // otherwise button was released (rising edge)
        printf("RELEASED!\n");
      }
      // don't forget to turn on LED
      set_led(!stable_pressed);
    }
  }
}
