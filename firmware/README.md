# Firmware

[`charity_shop_sheets/charity_shop_sheets.ino`](charity_shop_sheets/charity_shop_sheets.ino) is the performance sketch for the Teensy 4.1 + Audio Shield.

For setup and uploading, see [electronics and firmware](../build/04-electronics-and-firmware.md).

## What it does

1. **Listens** to the tube through the Audio Shield's **LINE IN** (left channel), where the wireless receiver is plugged in. It only measures the signal level and never plays the input itself.
2. **Detects hits.** When the level jumps above `HIT_THRESHOLD`, that counts as one hit. It won't trigger again until the level has dropped back down, so one jolt gives one sound.
3. **Tracks how busy the tube is.** Every movement adds to a "motion energy" value, which slowly fades when the tube is still.
4. **Chooses a sound:**
   - **Calm** (single jolts, low energy) → a random sound from **`/LONG`**: long, resonant gongs, bells and tam-tam
   - **Busy** (lots of shaking, high energy) → a random sound from **`/SHORT`**: short strikes
5. **Plays a tail.** When the audience stops and the tube has been quiet for about 1.2 seconds, it plays one last long sound to close the phrase.
6. **Plays up to three sounds at once**, taking turns between three players, so sounds can overlap and ring into each other.
7. **Reads the volume knob** on A0 and sets the output volume.

## Settings to adjust

These are near the top of the sketch.

| Setting | What it does | Default |
|---|---|---|
| `HIT_THRESHOLD` | How hard the tube has to move to trigger a sound. Lower = more sensitive. | `0.008` (try 0.004–0.008) |
| `ENERGY_INPUT_THRESHOLD` | How much movement counts towards "busy". | `0.0015` |
| `HIGH_ENERGY` | Where "calm" ends and "busy" starts. Lower = more short sounds. | `0.35` |
| `ENERGY_DECAY_PER_SEC` | How fast the instrument forgets movement. Higher = calms down faster. | `0.5` |
| `DEBOUNCE_MS` | Minimum time between hits, in milliseconds. | `180` |
| `QUIET_FOR_TAIL_MS` | How long the tube must be still before the closing long sound. | `1200` |
| `sgtl5000_1.lineInLevel(12)` | Input gain on LINE IN (0–15). Raise it if the wireless signal is weak. | `12` |

## Watching it work

Open the Serial Monitor at **9600 baud**. At startup it lists the files it found in `/LONG` and `/SHORT`. While playing, it prints every hit with its level and motion energy, and the file it chose. Use this to set `HIT_THRESHOLD`: shake gently and read the levels, then set the threshold just below them.

## Known limits

- **Up to 32 files are loaded from each folder** (`MAX_FILES = 32`). The code is kept exactly as performed. The full library has 72 short and 44 long sounds, so **choose 32 of each** for your card. This choice is part of each performance. If you want more, raise `MAX_FILES`.
- `random()` isn't seeded, so the order of random choices is the same each time the box is switched on.

## Test sketch

[`tools/piezo_test`](../tools/piezo_test/piezo_test.ino) is a simpler sketch for the [wired option](../build/03-piezo-cable.md#wired-option-no-wireless-system). It reads the piezo on pin A0 and plays `TEST.WAV` from the root of the SD card on every hit.
