#include "pico/stdlib.h"
#include <hardware/gpio.h>
#include <stdio.h>

#define STATUS_LED_PIN 25u
#define BUTTON_INPUT_PIN 22u
#define BUTTON_DEBOUNCE_MS 20u
#define LED_TURN_ON_MS 5000u

enum class PlaybackState : uint8_t { IDLE, PLAYING };

PlaybackState state = PlaybackState::IDLE;
unsigned int press_count = 0;
bool last_raw_pressed = false;
bool stable_pressed = false;
uint32_t raw_changed_at_ms = 0;
uint32_t led_turn_on_at_ms = 0;

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
static void set_led(bool led_on) { gpio_put(STATUS_LED_PIN, led_on); }

// returns true if our active-low button is pressed (reads low)
static bool button_is_pressed(void) { return !gpio_get(BUTTON_INPUT_PIN); }

static bool debounce_button(uint32_t now) {
  bool raw_pressed = button_is_pressed();

  if (raw_pressed != last_raw_pressed) {
    // raw reading changed (possibly a bounce), restart stopwatch here
    raw_changed_at_ms = now;
    last_raw_pressed = raw_pressed;
    return false;
  }

  if (raw_pressed != stable_pressed &&
      now - raw_changed_at_ms > BUTTON_DEBOUNCE_MS) {
    // enough time has passed since button changed state, update stable
    // reading variable and proceed to detecting event (press or release)
    stable_pressed = raw_pressed;
    if (stable_pressed) {
      // press event was detected, update count and print it
      press_count++;
      printf("PRESSED! count: %u\n", press_count);
    } else {
      // otherwise button was released
      printf("RELEASED!\n");
    }
  }
  return stable_pressed;
}

int main(void) {
  stdio_init_all();
  init_pins();

  sleep_ms(10000);
  printf("---PICO BLINKY STARTUP LINE---\n");
  while (true) {
    uint32_t now = to_ms_since_boot(get_absolute_time());
    switch (state) {
    case PlaybackState::IDLE:
      if (debounce_button(now)) {
        state = PlaybackState::PLAYING;
        led_turn_on_at_ms = now;
        set_led(true);
      }
    case PlaybackState::PLAYING:
      // debounce just for logging
      debounce_button(now);
      if (now - led_turn_on_at_ms > LED_TURN_ON_MS) {
        set_led(false);
        state = PlaybackState::IDLE;
      }
    }
  }
}
