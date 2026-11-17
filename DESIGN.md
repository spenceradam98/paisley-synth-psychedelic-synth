# Paisley: psychedelic synthesiser (design plan, v0.1)

Touchstones: Melody's Echo Chamber, Broadcast, Scenic, Tame Impala, MGMT, Pond, Gum.
The sound is warm, wobbly, a bit broken: combo organs, tape-replay flutes and strings,
warbly electric keys, phased pads, fuzz leads, and lots of echo.

Stack: JUCE 8.0.9, C++17, CMake. Builds VST3 + Standalone (AU can be added for macOS
with one word in `FORMATS`). Same stack as the scope plugin so both can live in one repo.

## Synth architecture (`Source/SynthEngine.h`)

- 12 voices, Poly / Mono / Legato, glide, sustain pedal, mod wheel adds vibrato, pitch bend.
- 2 oscillators, each: wave, octave, semi, fine, shape, level, unison (1-5) and detune
  (unison voices are spread across the stereo field by **Spread**).

  | Wave | What *Shape* does |
  |---|---|
  | Sine, Triangle, Saw | wavefolder amount |
  | Pulse | pulse width (PWM via LFO 2) |
  | Organ | morphs drawbar registrations: 888 gospel > 8' 4' flutes > full > bright combo |
  | Vox | morphs vowel formants A-E-I-O-U |
  | Flute | breath noise (tape-replay flute) |
  | Noise | - |

- Mixer: sub oscillator (sine, octave down), noise, ring mod (osc1 x osc2).
- Filter: zero-delay-feedback SVF, LP24 / LP12 / Band / High / Notch, resonance, pre-drive,
  envelope amount (bipolar), key tracking, velocity.
- Two ADSRs (filter, amp) + amp velocity.
- LFO 1 (sine/tri/saw/square/S&H/drift): fade-in delay, to pitch (vibrato) and cutoff.
- LFO 2: to oscillator shape (PWM / formant / drawbar motion), auto-pan, amplitude.
- Character: **Drift** (per-voice analogue pitch wander) and **Warble** (shared tape wow and
  flutter on pitch, the "warbly keys" control).

## FX chain (`Source/Effects.h`), fixed order

Drive (Warm / Fuzz / Crunch) > Phaser (6-stage, feedback) > Chorus / Flanger / Ensemble
(3-tap string-machine ensemble) > Tremolo (sine / square / ramp, auto-pan) > Tape (wow,
flutter, saturation, age = darker + hiss) > **Delay A > Delay B** (the delay chain: each has
tempo sync or ms, feedback past 100% with saturation so it self-oscillates musically,
tone, wobble, ping-pong) > Reverb.

Every block has a power switch; off blocks cost nothing.

## Presets (`Source/FactoryPresets.cpp`, `Source/PresetManager.*`)

- Factory presets are compiled in as sparse overrides on top of the Init patch, so they stay
  readable and survive new parameters being added. Shared "pedalboard" snippets (tape echo,
  ensemble, hall, worn tape...) are reused across presets.
- Every factory preset is loudness-matched: `PaisleyRender --calibrate` renders each one and
  writes `FactoryPresetLevels.h` (master level per preset, -15 dB RMS chord / -13 dB mono line).
- User presets are XML files (`.paisley`) in `Documents/Paisley/Presets/<folder>/`, shown in
  the browser under "User" categories. Host state stores the full parameter tree, preset
  name and theme.
- v0.1 bank: 66 presets in 9 categories: Organs, Keys, Strings, Tape, Pads, Leads, Basses,
  Plucks, Textures.

## Theme system (`Source/Themes.h`)

A theme = palette + procedural background pattern + knob style, rendered by one LookAndFeel.
The whole UI scales (50-150%) with a fixed aspect ratio. Last-used theme is remembered.

| Theme | Vibe | Pattern | Knobs |
|---|---|---|---|
| Sunburst '73 | warm 70s orange/brown | sun rays | chicken-head |
| Kaleido Neon | purple/pink/cyan | kaleidoscope rings | neon ring |
| Space Age Library | cream, black, red, 60s library-music sleeve | dots and circles | bakelite |
| Dream Pop Pastel | lilac to peach | soft waves | soft dot |
| Oil Light Show | black with liquid colour blobs | blobs | neon ring |
| Desert Fuzz | teal and mustard garage | wavy stripes | chicken-head |

## Dev tool (`tools/Render.cpp`, `-DPAISLEY_BUILD_TOOLS=ON`)

Renders every preset to WAV, flags NaN / silence / clipping, snapshots the UI in every theme,
and (with `--calibrate`) regenerates the preset loudness table.

## Next steps (not in v0.1)

- Drag-to-reorder FX chain; reverb upgrade (FDN with shimmer); tempo-synced LFOs.
- Proper mod matrix and macro knobs; arpeggiator.
- Sample-based tape-replay layer (real recorded sources would beat synthesis for flutes/choirs).
- macOS AU + signed builds via CI.
