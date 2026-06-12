# Obsidian v0.2 — Wavetable Synthesizer (VST3)

A JUCE-based wavetable synth built around the same architecture as Serum.

## Features

**Oscillators**

- Two wavetable oscillators with frame morphing and FFT-built anti-aliasing
  mipmaps (9 band-limited levels per frame)
- Three tables: Basic (sine→tri→saw→square→growl), Pulse (PWM sweep), and
  **User** — load any WAV and it’s sliced into 2048-sample frames, Serum-style
- **Warp modes**: Sync, Bend, Mirror, and FM-from-Osc-B (osc A)
- Semi + fine tuning per oscillator
- Sub oscillator (-1/-2 oct sine) and white noise
- Unison up to 7 voices with detune and stereo width

**Filter**

- SVF lowpass / bandpass / highpass, or 24 dB ladder
- Pre-filter drive (tanh), resonance, Env 2 amount, LFO 1 amount

**Modulation**

- Env 1 (amp) + Env 2 (assignable), full ADSR each
- 2 LFOs: sine / triangle / saw / square / S&H, free Hz or host-tempo-synced
  (1/1 … 1/16, triplets, dotted)
- **8-slot mod matrix**: Env 2, LFO 1/2, velocity, mod wheel, note, random →
  morphs, levels, pitch, cutoff, resonance, pan, unison detune, warp amounts
- Glide/portamento, pitch bend with adjustable range

**FX rack** (in series): distortion → chorus → phaser → stereo delay →
reverb → compressor, each with its own on/off and controls

**Other**

- 8-voice polyphony, full DAW state recall
- Preset save/load (.obsn files) and Init patch
- Resizable tabbed UI: SYNTH / FX / MATRIX

## Building

### Prerequisites

- CMake 3.22+, Git (JUCE is fetched automatically on first configure)
- **Windows:** Visual Studio 2022 · **macOS:** Xcode ·
  **Linux:** GCC/Clang plus:
  `sudo apt install libasound2-dev libx11-dev libxext-dev libxrandr-dev libxinerama-dev libxcursor-dev libfreetype6-dev libwebkit2gtk-4.1-dev libcurl4-openssl-dev`

### Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

The VST3 auto-installs to your system plugin folder
(`C:\Program Files\Common Files\VST3\`, `~/Library/Audio/Plug-Ins/VST3/`,
or `~/.vst3/`). A Standalone app is built too, under
`build/Obsidian_artefacts/Release/Standalone/`.

## Project layout

```
CMakeLists.txt              Build config (fetches JUCE 8)
Source/
  Wavetable.h               Wavetable + FFT mipmap builder + WAV import + oscillator
  Modulation.h              LFO + mod matrix source/destination definitions
  SynthVoice.h              Voice: oscs, warps, FM, sub/noise, glide, filters, matrix
  FXChain.h                 Global FX rack
  PluginProcessor.h/.cpp    ~90 parameters, synth engine, audio callback
  PluginEditor.h/.cpp       Tabbed UI, preset + wavetable file handling
```

## Honest gap analysis vs Serum

Still missing, roughly in order of effort:

1. **Visual wavetable editor** — drawing waveforms, FFT bin editing, the 3D view
1. **Drawable LFO curves** with grid snapping (current LFOs are fixed shapes)
1. **Drag-and-drop mod assignment** from source to knob
1. **More warp modes** (Serum has ~14), FM/AM/RM between oscillators as a mode set
1. **Per-FX modulation** and reorderable FX chain
1. **Factory content** — Serum ships ~150 wavetables and hundreds of presets
1. **Oversampling** options (2x–8x) for the warp/FM paths
1. Granular extras: chaos oscillators, alternate unison stack modes, MPE

The engine architecture (mipmapped tables, control-rate matrix, per-voice DSP)
is built so each of these can be added without rewrites.

## License note

JUCE is dual-licensed (GPLv3 or commercial). GPL release = free; closed-source
distribution requires a JUCE license. “Serum” is a trademark of Xfer Records —
don’t ship under a confusingly similar name.