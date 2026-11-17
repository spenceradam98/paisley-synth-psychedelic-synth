#include "FactoryPresets.h"
#if __has_include("FactoryPresetLevels.h")
 #include "FactoryPresetLevels.h"
#else
 #include <map>
 #include <string>
 static const std::map<std::string, float> factoryPresetLevels;
#endif

// ============================================================================
//  Paisley factory bank.
//  Wave index:   0 Sine 1 Triangle 2 Saw 3 Pulse 4 Organ 5 Vox 6 Flute 7 Noise
//  Filter mode:  0 LP24 1 LP12 2 Band 3 High 4 Notch
//  LFO wave:     0 Sine 1 Tri 2 Saw 3 Square 4 S&H 5 Drift
//  Voice mode:   0 Poly 1 Mono 2 Legato
//  Drive type:   0 Warm 1 Fuzz 2 Crunch      Chorus mode: 0 Chorus 1 Flanger 2 Ensemble
//  Delay div:    5 = 1/8, 6 = 1/8D, 7 = 1/4T, 8 = 1/4, 9 = 1/4D, 10 = 1/2
// ============================================================================

namespace
{
using V = std::vector<std::pair<const char*, float>>;

// Reusable FX "pedalboard" snippets, appended to presets
V operator+ (V a, const V& b) { a.insert (a.end(), b.begin(), b.end()); return a; }

const V tapeEcho   { { "dlyAOn", 1 }, { "dlyADiv", 6 }, { "dlyAFb", 0.45f }, { "dlyATone", 0.45f }, { "dlyAMod", 0.4f }, { "dlyAMix", 0.3f } };
const V pingEcho   { { "dlyAOn", 1 }, { "dlyADiv", 8 }, { "dlyAFb", 0.4f }, { "dlyAPing", 1 }, { "dlyATone", 0.6f }, { "dlyAMix", 0.28f } };
const V echoChain  { { "dlyAOn", 1 }, { "dlyADiv", 6 }, { "dlyAFb", 0.35f }, { "dlyAMod", 0.3f }, { "dlyAMix", 0.25f },
                     { "dlyBOn", 1 }, { "dlyBDiv", 10 }, { "dlyBFb", 0.5f }, { "dlyBPing", 1 }, { "dlyBTone", 0.4f }, { "dlyBMix", 0.25f } };
const V room       { { "revOn", 1 }, { "revSize", 0.45f }, { "revMix", 0.2f } };
const V hall       { { "revOn", 1 }, { "revSize", 0.8f }, { "revDamp", 0.4f }, { "revMix", 0.32f } };
const V cathedral  { { "revOn", 1 }, { "revSize", 1.0f }, { "revDamp", 0.25f }, { "revMix", 0.45f } };
const V oldTape    { { "tapOn", 1 }, { "tapWow", 0.35f }, { "tapFlutter", 0.3f }, { "tapSat", 0.35f }, { "tapAge", 0.4f } };
const V wornTape   { { "tapOn", 1 }, { "tapWow", 0.7f }, { "tapFlutter", 0.5f }, { "tapSat", 0.5f }, { "tapAge", 0.7f } };
const V lesliePh   { { "phsOn", 1 }, { "phsRate", 0.6f }, { "phsDepth", 0.6f }, { "phsFb", 0.3f }, { "phsMix", 0.45f } };
const V slowPhase  { { "phsOn", 1 }, { "phsRate", 0.12f }, { "phsDepth", 0.85f }, { "phsFb", 0.6f }, { "phsMix", 0.6f } };
const V chorusLush { { "choOn", 1 }, { "choMode", 0 }, { "choRate", 0.45f }, { "choDepth", 0.6f }, { "choMix", 0.5f } };
const V ensemble   { { "choOn", 1 }, { "choMode", 2 }, { "choRate", 0.6f }, { "choDepth", 0.7f }, { "choMix", 0.6f } };
const V jetFlange  { { "choOn", 1 }, { "choMode", 1 }, { "choRate", 0.15f }, { "choDepth", 0.9f }, { "choFb", 0.7f }, { "choMix", 0.55f } };
const V vibrato    { { "lfo1Rate", 5.5f }, { "lfo1Delay", 0.6f }, { "lfo1Pitch", 0.22f } };

const std::vector<FactoryPreset> bank =
{
    // ======================================================== ORGANS
    { "Sunday Combo", "Organs", V {
        { "osc1Wave", 4 }, { "osc1Shape", 1.0f }, { "osc1Level", 0.8f }, { "cutoff", 9000 }, { "keyTrack", 0.5f },
        { "aA", 0.003f }, { "aD", 0.1f }, { "aS", 1.0f }, { "aR", 0.06f }, { "ampVel", 0.1f } } + lesliePh + room },
    { "Basement Gospel", "Organs", V {
        { "osc1Wave", 4 }, { "osc1Shape", 0.0f }, { "osc1Level", 0.85f }, { "cutoff", 7000 },
        { "aA", 0.003f }, { "aS", 1.0f }, { "aR", 0.08f }, { "ampVel", 0.0f }, { "drvOn", 1 }, { "drvAmt", 0.35f } }
        + V { { "choOn", 1 }, { "choMode", 0 }, { "choRate", 6.2f }, { "choDepth", 0.25f }, { "choMix", 0.45f } } + room },
    { "Flute Stops", "Organs", V {
        { "osc1Wave", 4 }, { "osc1Shape", 0.33f }, { "osc1Level", 0.85f }, { "cutoff", 6000 },
        { "aA", 0.01f }, { "aS", 1.0f }, { "aR", 0.15f }, { "ampVel", 0.0f } } + vibrato + oldTape + hall },
    { "Spinning Cabinet", "Organs", V {
        { "osc1Wave", 4 }, { "osc1Shape", 0.66f }, { "osc1Level", 0.8f }, { "cutoff", 8000 },
        { "aA", 0.003f }, { "aS", 1.0f }, { "aR", 0.08f }, { "ampVel", 0.0f },
        { "trmOn", 1 }, { "trmRate", 6.5f }, { "trmDepth", 0.35f }, { "trmPan", 1 }, { "drvOn", 1 }, { "drvAmt", 0.2f } } + room },
    { "Fuzz Combo Organ", "Organs", V {
        { "osc1Wave", 3 }, { "osc1Shape", 0.15f }, { "osc1Level", 0.6f }, { "osc2Wave", 4 }, { "osc2Shape", 1.0f },
        { "osc2Oct", 1 }, { "osc2Level", 0.5f }, { "cutoff", 5500 }, { "reso", 0.2f },
        { "aA", 0.002f }, { "aS", 1.0f }, { "aR", 0.05f }, { "ampVel", 0.0f },
        { "lfo1Rate", 6.0f }, { "lfo1Pitch", 0.12f }, { "drvOn", 1 }, { "drvType", 1 }, { "drvAmt", 0.35f }, { "drvTone", 0.5f } } + tapeEcho + room },
    { "Chapel In Space", "Organs", V {
        { "osc1Wave", 4 }, { "osc1Shape", 0.2f }, { "osc1Level", 0.7f }, { "osc2Wave", 4 }, { "osc2Shape", 0.5f },
        { "osc2Oct", 1 }, { "osc2Fine", 7 }, { "osc2Level", 0.35f }, { "cutoff", 5000 },
        { "aA", 0.3f }, { "aS", 1.0f }, { "aR", 1.2f }, { "ampVel", 0.0f } } + slowPhase + echoChain + cathedral },
    { "Melting Fairground", "Organs", V {
        { "osc1Wave", 4 }, { "osc1Shape", 0.45f }, { "osc1Level", 0.8f }, { "cutoff", 6500 }, { "warble", 0.55f },
        { "aA", 0.01f }, { "aS", 1.0f }, { "aR", 0.2f }, { "ampVel", 0.0f } } + wornTape + chorusLush + hall },
    { "Continental Dream", "Organs", V {
        { "osc1Wave", 4 }, { "osc1Shape", 0.95f }, { "osc1Level", 0.6f }, { "osc2Wave", 2 }, { "osc2Level", 0.25f },
        { "cutoff", 4200 }, { "reso", 0.25f }, { "filtMode", 1 }, { "aA", 0.002f }, { "aS", 1.0f }, { "aR", 0.07f }, { "ampVel", 0.0f },
        { "lfo1Rate", 6.8f }, { "lfo1Pitch", 0.15f } } + pingEcho + room },

    // ======================================================== KEYS
    { "Warbly Electric", "Keys", V {
        { "osc1Wave", 0 }, { "osc1Shape", 0.25f }, { "osc1Level", 0.8f }, { "osc2Wave", 0 }, { "osc2Oct", 2 }, { "osc2Level", 0.18f },
        { "cutoff", 4000 }, { "filtEnv", 0.25f }, { "fA", 0.001f }, { "fD", 0.6f }, { "fS", 0.2f }, { "filtVel", 0.6f },
        { "aA", 0.002f }, { "aD", 2.5f }, { "aS", 0.25f }, { "aR", 0.4f }, { "ampVel", 0.7f }, { "warble", 0.45f },
        { "trmOn", 1 }, { "trmRate", 4.5f }, { "trmDepth", 0.3f }, { "trmPan", 1 } } + oldTape + room },
    { "Faded Tines", "Keys", V {
        { "osc1Wave", 0 }, { "osc1Shape", 0.15f }, { "osc1Level", 0.8f }, { "osc2Wave", 1 }, { "osc2Oct", 1 }, { "osc2Level", 0.15f },
        { "cutoff", 3200 }, { "filtEnv", 0.2f }, { "fD", 0.8f }, { "fS", 0.3f }, { "filtVel", 0.5f },
        { "aA", 0.002f }, { "aD", 3.0f }, { "aS", 0.2f }, { "aR", 0.5f }, { "ampVel", 0.7f } } + chorusLush + tapeEcho + room },
    { "Toy Piano Haze", "Keys", V {
        { "osc1Wave", 1 }, { "osc1Shape", 0.4f }, { "osc1Oct", 1 }, { "osc1Level", 0.7f }, { "osc2Wave", 0 }, { "osc2Oct", 2 }, { "osc2Semi", 7 }, { "osc2Level", 0.25f },
        { "cutoff", 5000 }, { "aA", 0.001f }, { "aD", 0.9f }, { "aS", 0.0f }, { "aR", 0.6f }, { "ampVel", 0.6f }, { "warble", 0.3f } } + oldTape + pingEcho + hall },
    { "Clav Crunch", "Keys", V {
        { "osc1Wave", 3 }, { "osc1Shape", 0.7f }, { "osc1Level", 0.8f }, { "filtMode", 2 }, { "cutoff", 900 }, { "reso", 0.45f },
        { "filtEnv", 0.45f }, { "fA", 0.001f }, { "fD", 0.25f }, { "fS", 0.1f }, { "filtVel", 0.7f },
        { "aA", 0.001f }, { "aD", 0.5f }, { "aS", 0.4f }, { "aR", 0.1f }, { "ampVel", 0.8f }, { "drvOn", 1 }, { "drvType", 2 }, { "drvAmt", 0.3f } } + lesliePh + room },
    { "Celeste Lullaby", "Keys", V {
        { "osc1Wave", 0 }, { "osc1Oct", 1 }, { "osc1Level", 0.6f }, { "osc2Wave", 0 }, { "osc2Oct", 2 }, { "osc2Level", 0.35f }, { "ringLevel", 0.25f },
        { "cutoff", 8000 }, { "aA", 0.001f }, { "aD", 1.6f }, { "aS", 0.0f }, { "aR", 1.0f }, { "ampVel", 0.5f } } + echoChain + hall },
    { "Wobbly Upright", "Keys", V {
        { "osc1Wave", 2 }, { "osc1Level", 0.6f }, { "osc2Wave", 3 }, { "osc2Fine", 6 }, { "osc2Level", 0.4f }, { "cutoff", 1600 },
        { "filtEnv", 0.45f }, { "fA", 0.001f }, { "fD", 0.7f }, { "fS", 0.0f }, { "filtVel", 0.6f },
        { "aA", 0.001f }, { "aD", 2.2f }, { "aS", 0.0f }, { "aR", 0.5f }, { "ampVel", 0.75f }, { "warble", 0.6f } } + wornTape + room },
    { "Lounge Vibes", "Keys", V {
        { "osc1Wave", 0 }, { "osc1Level", 0.8f }, { "osc2Wave", 0 }, { "osc2Oct", 2 }, { "osc2Level", 0.12f },
        { "aA", 0.001f }, { "aD", 2.8f }, { "aS", 0.0f }, { "aR", 1.2f }, { "ampVel", 0.6f },
        { "trmOn", 1 }, { "trmRate", 5.5f }, { "trmDepth", 0.55f } } + hall },
    { "Harpsichord Trip", "Keys", V {
        { "osc1Wave", 2 }, { "osc1Unison", 2 }, { "osc1Detune", 0.15f }, { "osc1Level", 0.7f }, { "osc2Wave", 3 }, { "osc2Oct", 1 }, { "osc2Shape", 0.6f }, { "osc2Level", 0.3f },
        { "filtMode", 3 }, { "cutoff", 300 }, { "aA", 0.001f }, { "aD", 1.1f }, { "aS", 0.0f }, { "aR", 0.4f }, { "ampVel", 0.4f } } + jetFlange + room },

    // ======================================================== STRINGS
    { "String Machine", "Strings", V {
        { "osc1Wave", 2 }, { "osc1Level", 0.6f }, { "osc2Wave", 2 }, { "osc2Oct", 1 }, { "osc2Level", 0.4f },
        { "cutoff", 5000 }, { "filtMode", 1 }, { "aA", 0.25f }, { "aS", 1.0f }, { "aR", 0.9f }, { "ampVel", 0.2f } } + ensemble + hall },
    { "Velvet Ensemble", "Strings", V {
        { "osc1Wave", 2 }, { "osc1Unison", 4 }, { "osc1Detune", 0.35f }, { "osc1Level", 0.7f }, { "osc2Wave", 3 }, { "osc2Shape", 0.4f }, { "osc2Oct", 1 }, { "osc2Level", 0.25f },
        { "cutoff", 3500 }, { "aA", 0.5f }, { "aS", 1.0f }, { "aR", 1.4f }, { "ampVel", 0.2f }, { "lfo2Shape", 0.5f } } + ensemble + slowPhase + hall },
    { "Tape Violins", "Strings", V {
        { "osc1Wave", 2 }, { "osc1Unison", 3 }, { "osc1Detune", 0.25f }, { "osc1Level", 0.75f }, { "cutoff", 2800 }, { "filtMode", 1 }, { "reso", 0.15f },
        { "aA", 0.18f }, { "aS", 0.9f }, { "aR", 0.6f }, { "warble", 0.5f } } + vibrato + oldTape + room },
    { "Sunset Strings", "Strings", V {
        { "osc1Wave", 2 }, { "osc1Level", 0.55f }, { "osc2Wave", 2 }, { "osc2Oct", -1 }, { "osc2Level", 0.35f },
        { "cutoff", 4000 }, { "aA", 0.35f }, { "aS", 1.0f }, { "aR", 1.0f } } + ensemble + echoChain + hall },
    { "Cello Fog", "Strings", V {
        { "osc1Wave", 2 }, { "osc1Oct", -1 }, { "osc1Unison", 2 }, { "osc1Detune", 0.2f }, { "osc1Level", 0.75f }, { "cutoff", 1500 }, { "reso", 0.2f },
        { "filtEnv", 0.15f }, { "fA", 0.4f }, { "aA", 0.3f }, { "aS", 0.9f }, { "aR", 0.8f } } + vibrato + chorusLush + hall },
    { "Pizzicato Garden", "Strings", V {
        { "osc1Wave", 2 }, { "osc1Unison", 3 }, { "osc1Detune", 0.2f }, { "osc1Level", 0.75f }, { "cutoff", 1200 },
        { "filtEnv", 0.5f }, { "fA", 0.001f }, { "fD", 0.25f }, { "fS", 0.0f }, { "aA", 0.002f }, { "aD", 0.4f }, { "aS", 0.0f }, { "aR", 0.3f } } + pingEcho + hall },

    // ======================================================== TAPE (Mellotron-style)
    { "Tape Flutes", "Tape", V {
        { "osc1Wave", 6 }, { "osc1Shape", 0.35f }, { "osc1Level", 0.85f }, { "cutoff", 4500 }, { "filtMode", 1 },
        { "aA", 0.06f }, { "aD", 1.0f }, { "aS", 0.85f }, { "aR", 0.35f }, { "ampVel", 0.3f }, { "warble", 0.5f } } + oldTape + room },
    { "Meadow Flutes", "Tape", V {
        { "osc1Wave", 6 }, { "osc1Shape", 0.5f }, { "osc1Level", 0.8f }, { "osc2Wave", 6 }, { "osc2Fine", 9 }, { "osc2Level", 0.3f },
        { "cutoff", 3800 }, { "filtMode", 1 }, { "aA", 0.08f }, { "aS", 0.9f }, { "aR", 0.4f }, { "warble", 0.65f } } + wornTape + hall },
    { "Tape Strings 3-Violin", "Tape", V {
        { "osc1Wave", 2 }, { "osc1Unison", 3 }, { "osc1Detune", 0.3f }, { "osc1Level", 0.75f }, { "cutoff", 2400 }, { "filtMode", 1 },
        { "aA", 0.1f }, { "aD", 4.0f }, { "aS", 0.6f }, { "aR", 0.35f }, { "warble", 0.55f } } + vibrato + oldTape + room },
    { "Tape Choir", "Tape", V {
        { "osc1Wave", 5 }, { "osc1Shape", 0.15f }, { "osc1Unison", 3 }, { "osc1Detune", 0.25f }, { "osc1Level", 0.9f },
        { "osc2Wave", 5 }, { "osc2Shape", 0.7f }, { "osc2Oct", 1 }, { "osc2Level", 0.3f }, { "cutoff", 5000 },
        { "aA", 0.2f }, { "aS", 0.9f }, { "aR", 0.6f }, { "warble", 0.5f } } + vibrato + oldTape + hall },
    { "Tape Brass", "Tape", V {
        { "osc1Wave", 2 }, { "osc1Unison", 2 }, { "osc1Detune", 0.15f }, { "osc1Level", 0.8f }, { "cutoff", 900 }, { "reso", 0.15f },
        { "filtEnv", 0.45f }, { "fA", 0.06f }, { "fD", 0.5f }, { "fS", 0.5f }, { "aA", 0.04f }, { "aS", 0.85f }, { "aR", 0.3f }, { "warble", 0.45f } } + oldTape + room },
    { "Attic Cassette", "Tape", V {
        { "osc1Wave", 6 }, { "osc1Shape", 0.2f }, { "osc1Level", 0.6f }, { "osc2Wave", 2 }, { "osc2Level", 0.3f }, { "cutoff", 2200 }, { "filtMode", 1 },
        { "aA", 0.05f }, { "aS", 0.85f }, { "aR", 0.4f }, { "warble", 0.85f }, { "noiseLevel", 0.04f } }
        + V { { "tapOn", 1 }, { "tapWow", 0.9f }, { "tapFlutter", 0.7f }, { "tapSat", 0.6f }, { "tapAge", 0.9f } } + room },
    { "Recorder Consort", "Tape", V {
        { "osc1Wave", 6 }, { "osc1Shape", 0.7f }, { "osc1Oct", 1 }, { "osc1Level", 0.75f }, { "cutoff", 6000 },
        { "aA", 0.04f }, { "aS", 0.9f }, { "aR", 0.2f }, { "warble", 0.35f } } + vibrato + oldTape + pingEcho + room },
    { "Library Music Cue", "Tape", V {
        { "osc1Wave", 6 }, { "osc1Shape", 0.3f }, { "osc1Level", 0.6f }, { "osc2Wave", 5 }, { "osc2Shape", 0.4f }, { "osc2Level", 0.4f },
        { "cutoff", 3000 }, { "aA", 0.1f }, { "aS", 0.8f }, { "aR", 0.6f }, { "warble", 0.5f } } + ensemble + oldTape + tapeEcho + hall },

    // ======================================================== PADS
    { "Lilac Haze", "Pads", V {
        { "osc1Wave", 2 }, { "osc1Unison", 5 }, { "osc1Detune", 0.4f }, { "osc1Level", 0.6f }, { "osc2Wave", 1 }, { "osc2Oct", 1 }, { "osc2Shape", 0.3f }, { "osc2Level", 0.35f },
        { "cutoff", 1800 }, { "reso", 0.2f }, { "filtEnv", 0.2f }, { "fA", 1.5f }, { "fD", 2.0f }, { "aA", 1.2f }, { "aS", 1.0f }, { "aR", 2.5f },
        { "lfo1Wave", 5 }, { "lfo1Rate", 0.2f }, { "lfo1Cutoff", 0.25f } } + chorusLush + echoChain + cathedral },
    { "Glass Cathedral", "Pads", V {
        { "osc1Wave", 0 }, { "osc1Shape", 0.5f }, { "osc1Level", 0.6f }, { "osc2Wave", 0 }, { "osc2Oct", 1 }, { "osc2Semi", 7 }, { "osc2Shape", 0.3f }, { "osc2Level", 0.35f },
        { "aA", 1.5f }, { "aS", 1.0f }, { "aR", 3.0f }, { "lfo2Rate", 0.15f }, { "lfo2Shape", 0.6f }, { "lfo2Pan", 0.5f } } + slowPhase + echoChain + cathedral },
    { "Endless Summer", "Pads", V {
        { "osc1Wave", 3 }, { "osc1Shape", 0.3f }, { "osc1Unison", 3 }, { "osc1Detune", 0.3f }, { "osc1Level", 0.6f }, { "osc2Wave", 2 }, { "osc2Fine", -8 }, { "osc2Level", 0.4f },
        { "cutoff", 2400 }, { "aA", 0.8f }, { "aS", 1.0f }, { "aR", 2.0f }, { "lfo2Rate", 0.25f }, { "lfo2Shape", 0.7f }, { "warble", 0.3f } } + ensemble + oldTape + hall },
    { "Kaleidoscope Bloom", "Pads", V {
        { "osc1Wave", 2 }, { "osc1Unison", 4 }, { "osc1Detune", 0.5f }, { "osc1Level", 0.55f }, { "osc2Wave", 5 }, { "osc2Shape", 0.5f }, { "osc2Oct", 1 }, { "osc2Level", 0.4f },
        { "cutoff", 900 }, { "reso", 0.35f }, { "filtEnv", 0.55f }, { "fA", 3.0f }, { "fD", 4.0f }, { "fS", 0.3f },
        { "aA", 2.0f }, { "aS", 1.0f }, { "aR", 3.0f }, { "lfo2Rate", 0.08f }, { "lfo2Shape", 1.0f } } + jetFlange + echoChain + cathedral },
    { "Dusty Choir", "Pads", V {
        { "osc1Wave", 5 }, { "osc1Shape", 0.0f }, { "osc1Unison", 4 }, { "osc1Detune", 0.3f }, { "osc1Level", 0.8f }, { "cutoff", 6000 },
        { "aA", 1.0f }, { "aS", 1.0f }, { "aR", 2.0f }, { "lfo2Rate", 0.1f }, { "lfo2Shape", 0.8f } } + oldTape + hall },
    { "Sunlit Fog", "Pads", V {
        { "osc1Wave", 1 }, { "osc1Unison", 3 }, { "osc1Detune", 0.25f }, { "osc1Level", 0.7f }, { "noiseLevel", 0.08f }, { "cutoff", 1500 },
        { "aA", 2.5f }, { "aS", 1.0f }, { "aR", 3.5f }, { "lfo1Wave", 5 }, { "lfo1Rate", 0.3f }, { "lfo1Cutoff", 0.4f } } + chorusLush + cathedral },
    { "Floating Pillow", "Pads", V {
        { "osc1Wave", 0 }, { "osc1Shape", 0.15f }, { "osc1Level", 0.7f }, { "osc2Wave", 1 }, { "osc2Oct", -1 }, { "osc2Level", 0.4f },
        { "aA", 1.2f }, { "aS", 1.0f }, { "aR", 2.5f }, { "warble", 0.25f } } + ensemble + pingEcho + hall },
    { "Moonbeam Phase", "Pads", V {
        { "osc1Wave", 2 }, { "osc1Unison", 3 }, { "osc1Detune", 0.2f }, { "osc1Level", 0.6f }, { "osc2Wave", 3 }, { "osc2Shape", 0.2f }, { "osc2Fine", 5 }, { "osc2Level", 0.4f },
        { "cutoff", 2200 }, { "aA", 0.9f }, { "aS", 1.0f }, { "aR", 2.2f } }
        + V { { "phsOn", 1 }, { "phsRate", 0.05f }, { "phsDepth", 1.0f }, { "phsFb", 0.8f }, { "phsMix", 0.7f } } + hall },

    // ======================================================== LEADS
    { "Fuzz Sun Lead", "Leads", V {
        { "voiceMode", 2 }, { "glide", 0.06f }, { "osc1Wave", 2 }, { "osc1Level", 0.7f }, { "osc2Wave", 3 }, { "osc2Fine", 8 }, { "osc2Level", 0.4f },
        { "cutoff", 2500 }, { "reso", 0.3f }, { "filtEnv", 0.3f }, { "fD", 0.5f }, { "aA", 0.005f }, { "aS", 0.9f }, { "aR", 0.25f },
        { "drvOn", 1 }, { "drvType", 1 }, { "drvAmt", 0.55f }, { "drvTone", 0.55f } } + vibrato + tapeEcho + room },
    { "Gliding Sine", "Leads", V {
        { "voiceMode", 2 }, { "glide", 0.15f }, { "osc1Wave", 0 }, { "osc1Shape", 0.2f }, { "osc1Level", 0.85f },
        { "aA", 0.02f }, { "aS", 1.0f }, { "aR", 0.4f } } + vibrato + echoChain + hall },
    { "Wah Lead", "Leads", V {
        { "voiceMode", 1 }, { "glide", 0.03f }, { "osc1Wave", 2 }, { "osc1Unison", 2 }, { "osc1Detune", 0.12f }, { "osc1Level", 0.8f },
        { "filtMode", 2 }, { "cutoff", 700 }, { "reso", 0.65f }, { "lfo1Rate", 1.6f }, { "lfo1Cutoff", 0.55f },
        { "aA", 0.005f }, { "aS", 1.0f }, { "aR", 0.2f }, { "drvOn", 1 }, { "drvType", 0 }, { "drvAmt", 0.45f } } + tapeEcho + room },
    { "Theremin Ghost", "Leads", V {
        { "voiceMode", 2 }, { "glide", 0.35f }, { "osc1Wave", 0 }, { "osc1Level", 0.8f }, { "osc2Wave", 1 }, { "osc2Oct", 1 }, { "osc2Level", 0.1f },
        { "aA", 0.15f }, { "aS", 1.0f }, { "aR", 0.6f }, { "lfo1Rate", 6.5f }, { "lfo1Delay", 0.3f }, { "lfo1Pitch", 0.35f } } + hall },
    { "Synth Sitar", "Leads", V {
        { "osc1Wave", 3 }, { "osc1Shape", 0.85f }, { "osc1Level", 0.6f }, { "osc2Wave", 2 }, { "osc2Oct", 1 }, { "osc2Fine", 12 }, { "osc2Level", 0.35f },
        { "ringLevel", 0.3f }, { "filtMode", 2 }, { "cutoff", 2400 }, { "reso", 0.55f }, { "filtEnv", 0.4f }, { "fD", 0.6f }, { "fS", 0.1f },
        { "aA", 0.001f }, { "aD", 2.0f }, { "aS", 0.2f }, { "aR", 0.6f } } + jetFlange + tapeEcho + hall },
    { "Sky Whistle", "Leads", V {
        { "voiceMode", 2 }, { "glide", 0.1f }, { "osc1Wave", 6 }, { "osc1Shape", 0.25f }, { "osc1Oct", 1 }, { "osc1Level", 0.8f },
        { "aA", 0.03f }, { "aS", 1.0f }, { "aR", 0.3f } } + vibrato + echoChain + hall },
    { "Daydream Lead", "Leads", V {
        { "voiceMode", 2 }, { "glide", 0.05f }, { "osc1Wave", 3 }, { "osc1Shape", 0.5f }, { "osc1Level", 0.65f }, { "osc2Wave", 3 }, { "osc2Shape", 0.2f }, { "osc2Oct", 1 }, { "osc2Level", 0.3f },
        { "cutoff", 3500 }, { "filtEnv", 0.2f }, { "aA", 0.005f }, { "aS", 0.9f }, { "aR", 0.3f }, { "lfo2Rate", 0.4f }, { "lfo2Shape", 0.6f } } + chorusLush + pingEcho + room },
    { "Psych Guitar Synth", "Leads", V {
        { "voiceMode", 1 }, { "osc1Wave", 2 }, { "osc1Level", 0.7f }, { "osc2Wave", 2 }, { "osc2Oct", -1 }, { "osc2Fine", 5 }, { "osc2Level", 0.5f },
        { "cutoff", 3000 }, { "filtEnv", 0.25f }, { "fD", 0.3f }, { "aA", 0.002f }, { "aD", 1.5f }, { "aS", 0.6f }, { "aR", 0.3f },
        { "drvOn", 1 }, { "drvType", 1 }, { "drvAmt", 0.75f }, { "drvTone", 0.45f } } + lesliePh + tapeEcho + room },

    // ======================================================== BASSES
    { "Round Psych Bass", "Basses", V {
        { "voiceMode", 1 }, { "osc1Wave", 2 }, { "osc1Oct", -1 }, { "osc1Level", 0.7f }, { "subLevel", 0.5f },
        { "cutoff", 400 }, { "reso", 0.2f }, { "filtEnv", 0.35f }, { "fA", 0.001f }, { "fD", 0.3f }, { "fS", 0.2f }, { "keyTrack", 0.4f },
        { "aA", 0.002f }, { "aD", 0.6f }, { "aS", 0.7f }, { "aR", 0.12f }, { "drift", 0.05f } } + V { { "drvOn", 1 }, { "drvAmt", 0.2f } } },
    { "Fuzz Bass", "Basses", V {
        { "voiceMode", 1 }, { "osc1Wave", 3 }, { "osc1Oct", -1 }, { "osc1Shape", 0.3f }, { "osc1Level", 0.7f }, { "subLevel", 0.4f },
        { "cutoff", 900 }, { "filtEnv", 0.2f }, { "fD", 0.2f }, { "aA", 0.002f }, { "aS", 0.9f }, { "aR", 0.1f },
        { "drvOn", 1 }, { "drvType", 1 }, { "drvAmt", 0.6f }, { "drvTone", 0.4f } } },
    { "Rubber Band", "Basses", V {
        { "voiceMode", 2 }, { "glide", 0.07f }, { "osc1Wave", 1 }, { "osc1Oct", -1 }, { "osc1Shape", 0.35f }, { "osc1Level", 0.8f }, { "subLevel", 0.3f },
        { "cutoff", 600 }, { "reso", 0.45f }, { "filtEnv", 0.4f }, { "fD", 0.25f }, { "fS", 0.1f }, { "aA", 0.002f }, { "aS", 0.8f }, { "aR", 0.1f } } },
    { "Hollow Body Bass", "Basses", V {
        { "voiceMode", 1 }, { "osc1Wave", 0 }, { "osc1Oct", -1 }, { "osc1Shape", 0.45f }, { "osc1Level", 0.8f },
        { "aA", 0.002f }, { "aD", 1.4f }, { "aS", 0.3f }, { "aR", 0.15f }, { "ampVel", 0.6f } } + V { { "tapOn", 1 }, { "tapWow", 0.1f }, { "tapFlutter", 0.1f }, { "tapSat", 0.5f }, { "tapAge", 0.35f } } },
    { "Ladder Drop", "Basses", V {
        { "voiceMode", 2 }, { "glide", 0.04f }, { "osc1Wave", 2 }, { "osc1Oct", -1 }, { "osc1Level", 0.6f }, { "osc2Wave", 3 }, { "osc2Oct", -2 }, { "osc2Level", 0.5f },
        { "cutoff", 250 }, { "reso", 0.35f }, { "filtEnv", 0.55f }, { "fA", 0.001f }, { "fD", 0.35f }, { "fS", 0.0f }, { "filtDrive", 0.4f },
        { "aA", 0.002f }, { "aS", 0.85f }, { "aR", 0.12f } } },
    { "Wobble Organ Bass", "Basses", V {
        { "voiceMode", 1 }, { "osc1Wave", 4 }, { "osc1Shape", 0.0f }, { "osc1Oct", -1 }, { "osc1Level", 0.85f }, { "cutoff", 1400 },
        { "aA", 0.002f }, { "aS", 1.0f }, { "aR", 0.08f }, { "ampVel", 0.0f } } + V { { "trmOn", 1 }, { "trmRate", 4.0f }, { "trmDepth", 0.3f } } },

    // ======================================================== PLUCKS & ARPS
    { "Mellow Pluck", "Plucks", V {
        { "osc1Wave", 2 }, { "osc1Level", 0.6f }, { "osc2Wave", 3 }, { "osc2Fine", 6 }, { "osc2Level", 0.4f },
        { "cutoff", 500 }, { "reso", 0.25f }, { "filtEnv", 0.5f }, { "fA", 0.001f }, { "fD", 0.3f }, { "fS", 0.0f },
        { "aA", 0.001f }, { "aD", 0.7f }, { "aS", 0.0f }, { "aR", 0.4f } } + chorusLush + pingEcho + room },
    { "Marimba Mirage", "Plucks", V {
        { "osc1Wave", 0 }, { "osc1Level", 0.7f }, { "osc2Wave", 0 }, { "osc2Oct", 2 }, { "osc2Level", 0.2f }, { "ringLevel", 0.2f },
        { "aA", 0.001f }, { "aD", 0.45f }, { "aS", 0.0f }, { "aR", 0.4f }, { "ampVel", 0.6f } } + echoChain + hall },
    { "Kalimba Drip", "Plucks", V {
        { "osc1Wave", 1 }, { "osc1Shape", 0.25f }, { "osc1Oct", 1 }, { "osc1Level", 0.7f }, { "osc2Wave", 0 }, { "osc2Oct", 3 }, { "osc2Level", 0.15f },
        { "aA", 0.001f }, { "aD", 0.6f }, { "aS", 0.0f }, { "aR", 0.5f }, { "warble", 0.25f } } + pingEcho + hall },
    { "Harp Of Dreams", "Plucks", V {
        { "osc1Wave", 1 }, { "osc1Level", 0.6f }, { "osc2Wave", 2 }, { "osc2Level", 0.3f }, { "cutoff", 1800 },
        { "filtEnv", 0.35f }, { "fA", 0.001f }, { "fD", 0.5f }, { "fS", 0.0f }, { "aA", 0.001f }, { "aD", 2.0f }, { "aS", 0.0f }, { "aR", 1.5f } } + chorusLush + echoChain + cathedral },
    { "Space Echo Blip", "Plucks", V {
        { "osc1Wave", 3 }, { "osc1Shape", 0.5f }, { "osc1Level", 0.7f }, { "cutoff", 1200 }, { "reso", 0.5f }, { "filtEnv", 0.6f }, { "fD", 0.12f }, { "fS", 0.0f },
        { "aA", 0.001f }, { "aD", 0.2f }, { "aS", 0.0f }, { "aR", 0.15f } }
        + V { { "dlyAOn", 1 }, { "dlyADiv", 6 }, { "dlyAFb", 0.75f }, { "dlyAMod", 0.5f }, { "dlyATone", 0.4f }, { "dlyAMix", 0.45f } } + oldTape + room },
    { "Arp Fountain", "Plucks", V {
        { "osc1Wave", 2 }, { "osc1Unison", 2 }, { "osc1Detune", 0.2f }, { "osc1Level", 0.7f }, { "cutoff", 900 }, { "reso", 0.4f },
        { "filtEnv", 0.5f }, { "fD", 0.18f }, { "fS", 0.0f }, { "aA", 0.001f }, { "aD", 0.3f }, { "aS", 0.0f }, { "aR", 0.2f },
        { "lfo1Wave", 1 }, { "lfo1Rate", 0.1f }, { "lfo1Cutoff", 0.5f } } + echoChain + hall },

    // ======================================================== TEXTURES & FX
    { "Jet Plane Drone", "Textures", V {
        { "osc1Wave", 2 }, { "osc1Unison", 5 }, { "osc1Detune", 0.6f }, { "osc1Oct", -1 }, { "osc1Level", 0.6f }, { "noiseLevel", 0.25f },
        { "cutoff", 2500 }, { "aA", 2.0f }, { "aS", 1.0f }, { "aR", 4.0f } }
        + V { { "choOn", 1 }, { "choMode", 1 }, { "choRate", 0.07f }, { "choDepth", 1.0f }, { "choFb", 0.88f }, { "choMix", 0.7f } } + cathedral },
    { "Radiophonic Wobble", "Textures", V {
        { "osc1Wave", 0 }, { "osc1Shape", 0.7f }, { "osc1Level", 0.6f }, { "osc2Wave", 1 }, { "osc2Semi", 7 }, { "osc2Level", 0.4f }, { "ringLevel", 0.6f },
        { "lfo1Wave", 4 }, { "lfo1Rate", 6.0f }, { "lfo1Pitch", 0.3f }, { "aA", 0.01f }, { "aS", 1.0f }, { "aR", 0.8f } } + echoChain + hall },
    { "Ring Bell Dimension", "Textures", V {
        { "osc1Wave", 0 }, { "osc1Level", 0.2f }, { "osc2Wave", 0 }, { "osc2Semi", 11 }, { "osc2Oct", 1 }, { "osc2Fine", 30 }, { "osc2Level", 0.1f }, { "ringLevel", 0.9f },
        { "aA", 0.001f }, { "aD", 3.0f }, { "aS", 0.0f }, { "aR", 3.0f } } + slowPhase + echoChain + cathedral },
    { "Ocean Of Static", "Textures", V {
        { "osc1Wave", 7 }, { "osc1Level", 0.5f }, { "filtMode", 2 }, { "cutoff", 900 }, { "reso", 0.75f }, { "keyTrack", 1.0f },
        { "lfo1Wave", 5 }, { "lfo1Rate", 0.15f }, { "lfo1Cutoff", 0.7f }, { "aA", 1.5f }, { "aS", 1.0f }, { "aR", 3.0f } } + pingEcho + cathedral },
    { "Melting Clock", "Textures", V {
        { "osc1Wave", 6 }, { "osc1Shape", 0.4f }, { "osc1Level", 0.7f }, { "osc2Wave", 0 }, { "osc2Oct", 1 }, { "osc2Level", 0.3f },
        { "aA", 0.2f }, { "aS", 0.9f }, { "aR", 1.5f }, { "warble", 1.0f } }
        + V { { "tapOn", 1 }, { "tapWow", 1.0f }, { "tapFlutter", 0.8f }, { "tapSat", 0.4f }, { "tapAge", 0.6f } } + echoChain + hall },
    { "Third Eye Swell", "Textures", V {
        { "osc1Wave", 5 }, { "osc1Shape", 0.0f }, { "osc1Unison", 5 }, { "osc1Detune", 0.45f }, { "osc1Level", 0.7f }, { "subLevel", 0.3f },
        { "lfo2Rate", 0.05f }, { "lfo2Shape", 1.0f }, { "aA", 4.0f }, { "aS", 1.0f }, { "aR", 5.0f } } + slowPhase + echoChain + cathedral },
    { "Self Oscillating Echo", "Textures", V {
        { "osc1Wave", 1 }, { "osc1Level", 0.7f }, { "aA", 0.001f }, { "aD", 0.3f }, { "aS", 0.0f }, { "aR", 0.3f } }
        + V { { "dlyAOn", 1 }, { "dlyADiv", 9 }, { "dlyAFb", 1.02f }, { "dlyATone", 0.35f }, { "dlyAMod", 0.6f }, { "dlyAMix", 0.5f } } + oldTape + hall },
    { "Sitar Drone", "Textures", V {
        { "osc1Wave", 3 }, { "osc1Shape", 0.9f }, { "osc1Oct", -1 }, { "osc1Level", 0.5f }, { "osc2Wave", 2 }, { "osc2Semi", 7 }, { "osc2Oct", -1 }, { "osc2Level", 0.35f },
        { "filtMode", 2 }, { "cutoff", 1600 }, { "reso", 0.6f }, { "lfo1Wave", 1 }, { "lfo1Rate", 0.2f }, { "lfo1Cutoff", 0.4f },
        { "aA", 0.5f }, { "aS", 1.0f }, { "aR", 2.0f } } + jetFlange + hall },
};
} // namespace

const std::vector<FactoryPreset>& getFactoryPresets()
{
    // Append the loudness-matched master level to every factory preset.
    static const std::vector<FactoryPreset> levelled = []
    {
        auto b = bank;
        for (auto& p : b)
        {
            auto it = factoryPresetLevels.find (p.name);
            if (it != factoryPresetLevels.end()) p.values.push_back ({ "master", it->second });
        }
        return b;
    }();
    return levelled;
}
