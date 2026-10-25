# Parts list

## The tube

| Item | Quantity | Notes |
|---|---|---|
| Second-hand bed sheets | as many as you need | From charity shops. Mixing colours gives the striped look. |
| Very large crochet hook | 1 | Sized to suit your sheets (10–25 mm is common for rag crochet) |
| Fabric scissors or rotary cutter + mat | 1 | For cutting sheets into strips |
| Large-eye needle / safety pins | a few | For joining and finishing |

## Sensor and wireless link

| Item | Quantity | Notes |
|---|---|---|
| Piezoelectric coaxial cable | ~30 m | Sold by the metre online. Same length as the tube, plus a little extra for the connection. |
| 6.35 mm (¼") mono jack plug | 1 | On the end of the piezo cable |
| Plug-in wireless guitar system (transmitter + receiver) | 1 set | Any make. See [choosing a wireless system](03-piezo-cable.md#choosing-a-wireless-system). |
| Cable ties / tape / heat-shrink | a few | Strain relief, and to fix the transmitter to the tube |

## Electronics box

| Item | Quantity | Notes |
|---|---|---|
| Teensy 4.1 | 1 | [pjrc.com](https://www.pjrc.com/store/teensy41.html) |
| Teensy Audio Shield, **Rev D** | 1 | [pjrc.com](https://www.pjrc.com/store/teensy3_audio.html) |
| Header pins / stacking headers | 1 set | To join the Teensy and Audio Shield |
| microSD card | 1 | 8–32 GB, FAT32 |
| 6.35 mm (¼") mono jack socket | 1 | Input from the wireless receiver |
| 10 kΩ linear (B10k) potentiometer + knob | 1 | Volume |
| 3.5 mm (mini jack) panel-mount socket | 1 | Audio out |
| Power switch | 1 | On the front of the box |
| USB power bank | 1 | Fits inside the box |
| 3D-printed enclosure | 1 | [`hardware/enclosure/`](../hardware/enclosure/) |
| Hook-up wire | some | |
| 3.5 mm to mixer cable or adapter | 1 | For example 3.5 mm to two ¼" jacks, to suit your mixer |

## Only for the wired option (no wireless system)

| Item | Quantity | Notes |
|---|---|---|
| 1 MΩ resistor | 2 | Bias |
| 10 kΩ resistor | 1 | Current limit |
| Schottky diode (BAT85 / 1N5817) | 2 | Input protection |

See [wired option](03-piezo-cable.md#wired-option-no-wireless-system).

## Tools

- Soldering iron and solder
- Wire strippers
- Multimeter (for checking the piezo cable isn't shorted)
- 3D printer (or a print service)
- Computer with Arduino IDE and Teensy support installed
