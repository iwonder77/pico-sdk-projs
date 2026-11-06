# Button-triggered playback

Firmware for a Raspberry Pi Pico or SparkFun Pro Micro RP2040 that starts a timed playback when a visitor presses a button. For now the status LED (GPIO 25) stands in for the BrightSign and sound-module triggers, and playback lasts `PLAYBACK_DURATION_MS` (5 s).

## Trigger modes

Set `TRIGGER_MODE` at the top of `src/main.cpp` and rebuild. It is a compile-time constant, so the unused mode is compiled out.

| Mode                       | Playback starts when idle and the button… | Holding the button                                 |
| -------------------------- | ----------------------------------------- | -------------------------------------------------- |
| `ReleaseToRearm` (default) | has just been pressed                     | One playback; release and press again for the next |
| `RepeatWhileHeld`          | is down                                   | Playback restarts every time it ends               |

Both modes ignore presses during playback. `RepeatWhileHeld` also starts playback at boot if the button is held at power-up, and a stuck or shorted button keeps it looping forever. That is why `ReleaseToRearm` is the default for unattended exhibits.

## How it works

Each pass of the main loop reads the button once and passes the reading to a `Debouncer`, which accepts a new state only after the raw reading has been steady for 20 ms. The debouncer reports two things: `press_event`, true only on the pass where the button becomes pressed, and `stable_pressed`, true for as long as it is held. `should_start_playback()` is the only place the modes differ: `ReleaseToRearm` uses the event and `RepeatWhileHeld` uses the held state. A two-state machine (`Idle`, `Playing`) then times the playback with timestamps, so the loop never blocks.

## How we got here

The project grew from a blinky in small steps: UART and USB logging, edge detection, a timestamp-based debouncer, then a state machine. The first state machine started playback whenever the debounced button was held, so a held button retriggered as soon as each playback ended, and a press made during playback carried over into the next one. A missing `break` also let `Idle` fall through into `Playing`. Comparing it with an earlier Arduino sketch showed the fix: report a press event ("was just pressed") instead of the held state. Rather than discard the original behavior, both are kept as selectable modes.

## Checked behavior

| Scenario                | `ReleaseToRearm`           | `RepeatWhileHeld`        |
| ----------------------- | -------------------------- | ------------------------ |
| Bouncy press, held 12 s | 1 playback                 | 3 playbacks back to back |
| Quick 100 ms tap        | 1 playback                 | 1 playback               |
| Tap during playback     | Ignored                    | Ignored                  |
| Button held at power-up | Nothing until a real press | Starts at boot           |

These results come from running `main.cpp` against scripted button timelines on a desktop with the SDK calls stubbed out. Playback always began 20 ms after the last bounce. Confirm on hardware with the 12-second hold test before relying on it.
