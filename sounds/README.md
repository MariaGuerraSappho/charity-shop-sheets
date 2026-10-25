# Sound library

All percussion sounds were **performed by Colin Frank** and recorded and edited by Maria Sappho in the percussion studio at the University of Huddersfield in 2025.

The sounds are licensed [CC BY-NC 4.0](https://creativecommons.org/licenses/by-nc/4.0/): free for non-commercial use, as long as you credit **Colin Frank** (performer) and **Maria Sappho**.

## Contents

116 WAV files, 16-bit, 44.1 kHz, about 254 MB, split into two groups:

| Folder | Files | Description |
|---|---|---|
| `short/` | 72 | Short, dry strikes |
| `long/` | 44 | Long, resonant sounds that ring out |

The instruments are tuned gongs (chromatic, C4 to E5), tam-tam (soft strikes and scrapes), tubular bells (C to C, one octave), bell tree (low, mid, high and sweeps), temple blocks, bass drum, tabla, vibraslap and cymbals (Istanbul Agop and Paiste Formula 602).

The full list of files is in [file-list.md](file-list.md).

## Download

The sound files are too large to keep in the git history comfortably, so they are attached as a zip to the repository's [**Releases**](https://github.com/MariaGuerraSappho/charity-shop-sheets/releases) page.

## Putting them on the SD card

The firmware looks for two folders in the root of the SD card:

```
SD card
├── LONG/    ← long, resonant sounds (played when the tube is calm, and as the closing "tail")
└── SHORT/   ← short strikes (played when the tube is busy)
```

It loads **up to 32 files from each folder**. The library has 72 short and 44 long sounds, so pick 32 of each for your card. Which sounds you choose is part of your version of the piece.

Keep filenames to lowercase letters, numbers, `_` and `-`. Some current names contain `#` (for example `Gong_D#5`) and spaces in folder names. Renaming these (for example `gong_ds5_short.wav`) avoids problems on some systems.

## Using your own sounds

The instrument works with any sounds. Export them as **16-bit, 44.1 kHz WAV** files and follow the same folder layout.
