/*
 * SparkFun Pro Micro RP2040: onboard WS2812-2020 rainbow cycle.
 *
 * Requires only pico_stdlib and hardware_pio from the Raspberry Pi Pico SDK.
 * The PIO instruction sequence is based on Raspberry Pi's BSD-3-Clause
 * licensed pico-examples/pio/ws2812/ws2812.pio.
 */

#include <stdint.h>

#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "pico/stdlib.h"

#define LED_PIN 25u
#define WS2812_FREQ_HZ 800000u
#define FRAME_DELAY_MS 20u
#define BRIGHTNESS_SHIFT 2u // Divide channel values by 4 (maximum 63).

/* Four PIO instructions; 10 PIO clock cycles per transmitted bit.
 *   out x, 1          side 0 [3]
 *   jmp !x, do_zero   side 1 [2]
 *   jmp bitloop       side 1 [2]
 *   nop               side 0 [2]  ; do_zero
 *
 * Encodings taken from the official pico-examples WS2812 PIO program.
 */
static const uint16_t ws2812_instructions[] = {0x6321, 0x1223, 0x1200, 0xa242};

static const struct pio_program ws2812_program = {
    .instructions = ws2812_instructions,
    .length = 4,
    .origin = -1,
};

static void ws2812_init(PIO pio, uint sm) {
  const uint offset = pio_add_program(pio, &ws2812_program);

  pio_gpio_init(pio, LED_PIN);
  pio_sm_set_consecutive_pindirs(pio, sm, LED_PIN, 1, true);

  pio_sm_config config = pio_get_default_sm_config();
  sm_config_set_wrap(&config, offset, offset + 3);
  sm_config_set_sideset(&config, 1, false, false);
  sm_config_set_sideset_pins(&config, LED_PIN);
  sm_config_set_out_shift(&config, false, true,
                          24); // MSB first, autopull 24 bits.
  sm_config_set_fifo_join(&config, PIO_FIFO_JOIN_TX);
  sm_config_set_clkdiv(&config,
                       (float)clock_get_hz(clk_sys) / (WS2812_FREQ_HZ * 10u));

  pio_sm_init(pio, sm, offset, &config);
  pio_sm_set_enabled(pio, sm, true);
}

/* 8-bit color wheel: red -> green -> blue -> red.
 * Returns the packed 24-bit color in GRB order, as WS2812 expects.
 */
static uint32_t rainbow_grb(uint8_t hue) {
  uint8_t r = 0, g = 0, b = 0;

  if (hue < 85) {
    r = 255 - hue * 3;
    g = hue * 3;
  } else if (hue < 170) {
    hue -= 85;
    g = 255 - hue * 3;
    b = hue * 3;
  } else {
    hue -= 170;
    b = 255 - hue * 3;
    r = hue * 3;
  }

  return ((uint32_t)(g >> BRIGHTNESS_SHIFT) << 16) |
         ((uint32_t)(r >> BRIGHTNESS_SHIFT) << 8) |
         (uint32_t)(b >> BRIGHTNESS_SHIFT);
}

int main(void) {
  const PIO pio = pio0;
  const uint sm = 0;

  ws2812_init(pio, sm);

  uint8_t hue = 0;
  while (true) {
    // Left-align the 24 GRB bits in the 32-bit PIO FIFO entry.
    pio_sm_put_blocking(pio, sm, rainbow_grb(hue) << 8);
    ++hue; // uint8_t wraps automatically from 255 to 0.

    // Also leaves far more than the WS2812 reset/latch interval.
    sleep_ms(FRAME_DELAY_MS);
  }
}
