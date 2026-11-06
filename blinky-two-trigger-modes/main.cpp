/**
 * Button-triggered playback
 *
 * A debounced button press starts a timed playback. The status LED stands in
 * for the real trigger outputs. TRIGGER_MODE selects what a held button does.
 */
#include <cinttypes>
#include <cstdint>
#include <cstdio>

#include "pico/stdlib.h"

// ----- Configuration ---------------------------------------------------------

enum class TriggerMode {
  // One playback per press. When playback ends, the button must be released
  // and pressed again to start another. A held button does nothing more.
  ReleaseToRearm,

  // Playback restarts every time it ends, for as long as the button is held.
  // A button held at power-up starts playback as soon as the board boots, and
  // a stuck or shorted button makes this mode loop forever.
  RepeatWhileHeld,
};

constexpr TriggerMode TRIGGER_MODE = TriggerMode::ReleaseToRearm;

constexpr uint STATUS_LED_PIN = 25;
constexpr uint BUTTON_PIN = 22; // active low, external pull-up

constexpr uint32_t DEBOUNCE_MS = 20;
constexpr uint32_t PLAYBACK_DURATION_MS = 5000;

// ----- Hardware --------------------------------------------------------------

static void init_pins() {
  gpio_init(STATUS_LED_PIN);
  gpio_set_dir(STATUS_LED_PIN, GPIO_OUT);

  gpio_init(BUTTON_PIN);
  gpio_set_dir(BUTTON_PIN, GPIO_IN);
  gpio_disable_pulls(BUTTON_PIN); // external pull-up; the RP2040 resets with
                                  // its internal pull-down enabled
}

static void set_led(bool on) { gpio_put(STATUS_LED_PIN, on); }

static bool button_is_pressed() { return !gpio_get(BUTTON_PIN); }

static uint32_t now_ms() { return to_ms_since_boot(get_absolute_time()); }

// ----- Button debouncing -----------------------------------------------------

// Accepts a new button state only after the raw reading has stayed unchanged
// for DEBOUNCE_MS. Call update() once per loop pass.
struct Debouncer {
  bool last_raw = false;          // raw reading from the previous pass
  bool stable_pressed = false;    // accepted (debounced) state
  uint32_t raw_changed_at_ms = 0; // when the raw reading last changed

  // Start from a real reading so a button held at power-up is not a press.
  void reset(bool raw, uint32_t now) {
    last_raw = raw;
    stable_pressed = raw;
    raw_changed_at_ms = now;
  }

  // Returns true only on the pass where the button becomes pressed.
  bool update(bool raw, uint32_t now) {
    if (raw != last_raw) { // possibly a bounce: restart the stopwatch
      last_raw = raw;
      raw_changed_at_ms = now;
      return false;
    }
    if (raw == stable_pressed || now - raw_changed_at_ms < DEBOUNCE_MS) {
      return false;
    }
    stable_pressed = raw;
    return stable_pressed;
  }
};

// ----- Trigger policy --------------------------------------------------------

// Called only while idle. TRIGGER_MODE is a compile-time constant, so the
// compiler keeps just the branch in use.
static bool should_start_playback(bool press_event, bool button_held) {
  switch (TRIGGER_MODE) {
  case TriggerMode::ReleaseToRearm:
    return press_event; // the moment of pressing
  case TriggerMode::RepeatWhileHeld:
    return button_held; // any time the button is down
  }
  return false;
}

// ----- Main ------------------------------------------------------------------

enum class PlaybackState { Idle, Playing };

int main() {
  stdio_init_all();
  init_pins();

  Debouncer button;
  button.reset(button_is_pressed(), now_ms());

  PlaybackState state = PlaybackState::Idle;
  uint32_t playback_started_at_ms = 0;
  uint32_t playback_count = 0;

  printf("button playback ready, mode: %s\n",
         TRIGGER_MODE == TriggerMode::ReleaseToRearm ? "release to re-arm"
                                                     : "repeat while held");

  while (true) {
    const uint32_t now = now_ms();

    // Runs every pass in every state, so presses during playback are
    // consumed here instead of queuing up for later.
    const bool press_event = button.update(button_is_pressed(), now);
    if (press_event) {
      printf("button pressed\n");
    }

    switch (state) {
    case PlaybackState::Idle:
      if (should_start_playback(press_event, button.stable_pressed)) {
        state = PlaybackState::Playing;
        playback_started_at_ms = now;
        set_led(true);
        printf("playback %" PRIu32 " started\n", ++playback_count);
      }
      break;

    case PlaybackState::Playing:
      if (now - playback_started_at_ms >= PLAYBACK_DURATION_MS) {
        state = PlaybackState::Idle;
        set_led(false);
        printf("playback ended\n");
      }
      break;
    }
  }
}
