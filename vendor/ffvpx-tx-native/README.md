# ffvpx-tx-native

Frozen copies of the FFmpeg `av_tx` sources used by Firefox 159's bundled
ffvpx (`media/ffvpx`), together with the aarch64 NEON assembly transform.
Files are byte-identical to `vendor/firefox-source/media/ffvpx` (verified by
`manifest.json` SHA-256 digests); `config/config.h` is that tree's
`config_darwin_aarch64.h`.

## Why this exists

On macOS arm64, Firefox dispatches the split-radix inverse FFT that blink's
PeriodicWave runs (through Gecko's FFTBlock) to the hand-written NEON
assembly in `libavutil/aarch64/tx_float_neon.S`. The rounding of that code
cannot be reproduced bit-exactly by a portable reimplementation, and an
oscillator wave table that differs by even one ulp is observable to audio
fingerprinting. Linking the real transform is the only faithful option for
this platform.

## License

The FFmpeg sources in this directory are LGPL-2.1-or-later
(see `COPYING.LGPLv2.1`). They are built as **standalone relocatable object
files** and linked into the runtime separately from the AGPL engine sources —
the same arrangement Mozilla uses for ffvpx in Firefox — so that a recipient
of the runtime can relink or replace this component. The engine's sources
include none of these headers; the only coupling is the `av_tx_init` /
`av_tx_uninit` C interface re-declared in
`src/domain/audio/periodic_wave_table.h`.

Only the inverse real DFT (AV_TX_FLOAT_RDFT) is exercised, on macOS arm64.
Other platforms use the engine's portable emulation in
`src/domain/audio/split_radix_fft.h`.
