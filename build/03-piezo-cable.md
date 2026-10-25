# The piezo cable

## What it is

Most people know piezos as small brass discs (the "buzzers" or contact mics found in kits). A **piezoelectric cable** works the same way, but the sensing material runs along the whole length of a cable. This means the full 30 m of the tube is the sensor, not just one point on it.

It looks like ordinary thin coaxial cable:

```
   outer jacket (plastic)
   ┌───────────────────────────────────┐
   │  braided shield / outer conductor │  ← ground
   │  ┌─────────────────────────────┐  │
   │  │ piezo polymer layer (PVDF)  │  │  ← the sensing material
   │  │  ┌───────────────────────┐  │  │
   │  │  │ centre core           │  │  │  ← signal
   │  │  └───────────────────────┘  │  │
   │  └─────────────────────────────┘  │
   └───────────────────────────────────┘
```

In a normal audio cable, the layer between the core and the braid is just insulation. In piezo cable, that layer is a **piezoelectric polymer** (PVDF). When the cable is squeezed, bent, rubbed or shaken, the polymer produces a small electric charge between the core and the braid. More energetic movement gives a bigger signal.

Because the braid surrounds the signal, the cable is shielded against electrical hum along its whole length. This matters for a cable this long.

Piezo cable is sold by the metre online. Search for "piezo cable" or "piezoelectric coaxial cable".

## The signal chain: going wireless

In the original version, the tube is **wireless**. A 30 m tube being passed around by an audience would tangle or yank any cable running to the electronics, so the piezo cable plugs into a **wireless guitar system** instead:

```
 piezo cable inside the tube
          │
   6.35 mm (¼") mono jack plug
          │
 wireless guitar TRANSMITTER  (plugs straight into the jack, rides on the end of the tube)
          ┊
          ┊   radio
          ┊
 wireless guitar RECEIVER  ──►  6.35 mm jack socket on the electronics box
                                         │
                                Teensy 4.1 + Audio Shield
                                         │
                                  audio out ──► PA / speakers
```

This works because, electrically, a piezo cable behaves a lot like a guitar pickup: a high-impedance source with a small, AC signal. Guitar wireless systems are designed to take exactly that kind of signal.

Wiring the plug:

- **Core** → tip of the jack plug
- **Braid** → sleeve of the jack plug

### Choosing a wireless system

Any plug-in guitar wireless set (transmitter and receiver that each plug directly into a ¼" jack, charged by USB) works. Things to look for:

- **Rechargeable, with a long battery life.** Check it lasts longer than a performance plus soundcheck.
- **A low-latency digital system**, so sounds trigger the moment the tube moves.
- **No built-in noise gate**, or one you can switch off. A gate can swallow gentle movements.

### What the wireless link changes

- **It protects the Teensy.** A hard yank on a piezo cable can produce voltage spikes of many volts. The wireless receiver outputs a normal instrument-level signal instead, so the Teensy never sees those spikes.
- **It responds to vibration more than slow sway.** Guitar wireless systems pass audio frequencies (roughly 20 Hz upwards). Slow, smooth swinging of the tube is mostly below that, so the tube responds best to **shaking, rustling, knocking and jolts**. This is worth knowing when writing instructions for the audience.

## Wired option (no wireless system)

If you'd rather plug the cable straight into the electronics, the piezo cable needs a protection circuit. The Teensy 4.1 analogue pins read **0 to 3.3 V** and are **not 5 V tolerant**, while the cable's signal goes negative and can spike far above 3.3 V. This circuit:

1. **Biases** the pin to the middle of the range (about 1.65 V) so swings in both directions can be read.
2. **Limits** current with a series resistor during big spikes.
3. **Clamps** the pin between 0 and 3.3 V with two diodes.

```
                         3.3V
                          │
                         [R1] 1 MΩ
                          │
 piezo core ───[R3 10 kΩ]─┼──────────────┬──────────► Teensy A0 (pin 14)
                          │              │
                         [R2] 1 MΩ      ─┴─ D1 (cathode to 3.3V, anode to A0)
                          │             ─┬─ D2 (cathode to A0, anode to GND)
                          │              │
 piezo braid ─────────────┴──────────────┴──────────► Teensy GND
```

| Part | Value | Job |
|---|---|---|
| R1, R2 | 1 MΩ each | Bias A0 to ~1.65 V and drain charge from the cable |
| R3 | 10 kΩ | Limits current into the pin during spikes |
| D1, D2 | small Schottky diodes (e.g. BAT85 or 1N5817) | Clamp the pin between 0 V and 3.3 V |

**Why 1 MΩ?** A long piezo cable acts like a capacitor, roughly 1 nF per metre (check your cable's datasheet), so about 30 nF for 30 m. Together with the bias resistors, it forms a filter that cuts low frequencies. With 1 MΩ resistors the cut-off is about 10 Hz. Smaller resistors (for example 100 kΩ) push it up to around 100 Hz and filter out most of the shaking. For a much shorter cable, smaller values are fine.

This wired option hasn't been tested with the full 30 m tube. The original uses the wireless system.

## Soldering the piezo cable

The piezo layer is **heat-sensitive** and can lose its piezo properties or melt if overheated.

- Strip carefully: the PVDF layer is thin and easy to nick.
- Solder quickly with a hot iron rather than holding a cooler iron on for a long time.
- After soldering, check with a multimeter that the core and braid are **not** shorted together. A short means no signal.
- **Strain relief:** the plug end will get pulled by the audience. Tie the cable to the tube near the end (a cable tie or a few stitches) so pulls go into the crochet, not into the solder joint. Tape or strap the transmitter to the tube so it can't swing on the plug.

## Testing

The [piezo test sketch](../tools/piezo_test/piezo_test.ino) reads the cable on pin A0, so it's for the **wired option**. In the wireless setup the signal comes in through the Audio Shield's LINE IN instead, so test with the performance firmware: see [calibrating](04-electronics-and-firmware.md#calibrating).

With the test sketch:

1. Upload it and open the Serial Monitor at 115200 baud.
2. Leave the tube still. `raw` should settle and `delta` should stay small (under ~20).
3. Shake the tube. `delta` should jump. Note the size of a gentle shake and a strong shake.
4. Set `TRIGGER_DELTA` between "noise when still" and "a gentle shake".

If `raw` jumps around when nothing is moving: check the transmitter and receiver are charged and paired, the plug is fully in, and (for the wired option) the braid is connected to GND.
