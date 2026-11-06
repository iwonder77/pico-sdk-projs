# Button debounce

Mechanical button contacts bounce for a few milliseconds when pressed or released, so the raw pin reading flickers between pressed and released before it settles. Without debouncing, one press can look like several.

This firmware checks the button on every pass of the main loop. It only accepts a new button state after the raw reading has stayed unchanged for longer than `BUTTON_DEBOUNCE_MS` (20 ms). The input PCB's RC filter removes most bounce in hardware; the software debounce handles whatever gets through, including noise on long cable runs.

## Variables

| Variable            | Meaning                                             |
| ------------------- | --------------------------------------------------- |
| `raw_pressed`       | What the pin says on this pass (`true` = pressed)   |
| `last_raw_pressed`  | What the pin said on the previous pass              |
| `raw_changed_at_ms` | When the raw reading last changed (the "stopwatch") |
| `stable_pressed`    | The accepted, debounced button state                |

## Each pass of the loop

```
raw_pressed = button_is_pressed()

if raw_pressed != last_raw_pressed:          # any change, possibly a bounce
    raw_changed_at_ms = now                  # restart the stopwatch
    last_raw_pressed = raw_pressed

if raw_pressed != stable_pressed and now - raw_changed_at_ms > BUTTON_DEBOUNCE_MS:
    stable_pressed = raw_pressed             # accept the new state
    report PRESSED or RELEASED               # fires once per real change
```

Every bounce restarts the stopwatch, so only a reading that holds steady for the full debounce time is accepted. The `raw_pressed != stable_pressed` check is what makes each press produce a single event rather than a stream of them while the button is held.

## Design notes

`button_is_pressed()` hides the active-low wiring, so `true` always means pressed. Both `last_raw_pressed` and `stable_pressed` start from one reading taken after the pins are configured, so a button held at power-up is not counted as a press. Time math uses unsigned `now - then`, which stays correct when the 32-bit millisecond counter wraps around after about 49.7 days. The button is polled rather than using GPIO interrupts, because a bouncing or noisy line would produce bursts of interrupts.

Raising `BUTTON_DEBOUNCE_MS` improves noise immunity, but it adds latency and causes taps shorter than the window to be ignored.

## Verification

Tested on a Raspberry Pi Pico with four checks, all passing. A bare jumper wire tapped to ground gave exactly one PRESSED and one RELEASED per tap. A quick tap of about 100 ms registered. A 5-second hold gave one PRESSED, then one RELEASED on letting go. Powering up with the button held produced no event until the button was released and pressed again.
