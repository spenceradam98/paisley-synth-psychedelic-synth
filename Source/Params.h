#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

// All parameter IDs and the parameter layout live here so the engine,
// editor and preset bank agree on names.
namespace Params
{
    inline const juce::StringArray oscWaves  { "Sine", "Triangle", "Saw", "Pulse", "Organ", "Vox", "Flute", "Noise" };
    inline const juce::StringArray filtModes { "LP 24", "LP 12", "Band", "High", "Notch" };
    inline const juce::StringArray lfoWaves  { "Sine", "Triangle", "Saw", "Square", "S&H", "Drift" };
    inline const juce::StringArray voiceModes{ "Poly", "Mono", "Legato" };
    inline const juce::StringArray driveTypes{ "Warm", "Fuzz", "Crunch" };
    inline const juce::StringArray chorusModes{ "Chorus", "Flanger", "Ensemble" };
    inline const juce::StringArray tremShapes{ "Sine", "Square", "Ramp" };
    inline const juce::StringArray syncDivs  { "1/32", "1/16T", "1/16", "1/16D", "1/8T", "1/8", "1/8D",
                                               "1/4T", "1/4", "1/4D", "1/2", "1/2D", "1/1" };
    inline const float syncBeats[] = { 0.125f, 1.0f / 6.0f, 0.25f, 0.375f, 1.0f / 3.0f, 0.5f, 0.75f,
                                       2.0f / 3.0f, 1.0f, 1.5f, 2.0f, 3.0f, 4.0f };

    using Layout = juce::AudioProcessorValueTreeState::ParameterLayout;
    using NR = juce::NormalisableRange<float>;

    inline NR skewed (float lo, float hi, float centre)
    {
        NR r (lo, hi);
        r.setSkewForCentre (centre);
        return r;
    }

    inline Layout createLayout()
    {
        Layout layout;
        auto pid = [] (const char* id) { return juce::ParameterID { id, 1 }; };
        auto addF = [&] (const char* id, const char* name, NR range, float def, const char* suffix = "")
        {
            layout.add (std::make_unique<juce::AudioParameterFloat> (pid (id), name, range, def,
                juce::AudioParameterFloatAttributes().withLabel (suffix)));
        };
        auto addI = [&] (const char* id, const char* name, int lo, int hi, int def)
        {
            layout.add (std::make_unique<juce::AudioParameterInt> (pid (id), name, lo, hi, def));
        };
        auto addC = [&] (const char* id, const char* name, const juce::StringArray& items, int def)
        {
            layout.add (std::make_unique<juce::AudioParameterChoice> (pid (id), name, items, def));
        };
        auto addB = [&] (const char* id, const char* name, bool def)
        {
            layout.add (std::make_unique<juce::AudioParameterBool> (pid (id), name, def));
        };

        // ---- Oscillators
        for (int o = 1; o <= 2; ++o)
        {
            auto p = juce::String ("osc") + juce::String (o);
            auto n = juce::String ("Osc ") + juce::String (o) + " ";
            auto s = [] (const juce::String& x) { return x.toRawUTF8(); };
            addC (s (p + "Wave"),    s (n + "Wave"), oscWaves, 2);
            addI (s (p + "Oct"),     s (n + "Octave"), -3, 3, 0);
            addI (s (p + "Semi"),    s (n + "Semi"), -12, 12, 0);
            addF (s (p + "Fine"),    s (n + "Fine"), NR (-100.0f, 100.0f), 0.0f, "ct");
            addF (s (p + "Shape"),   s (n + "Shape"), NR (0.0f, 1.0f), 0.0f);
            addF (s (p + "Level"),   s (n + "Level"), NR (0.0f, 1.0f), o == 1 ? 0.8f : 0.0f);
            addI (s (p + "Unison"),  s (n + "Unison"), 1, 5, 1);
            addF (s (p + "Detune"),  s (n + "Detune"), NR (0.0f, 1.0f), 0.2f);
        }

        // ---- Mixer
        addF ("subLevel",   "Sub Level",   NR (0.0f, 1.0f), 0.0f);
        addF ("noiseLevel", "Noise Level", NR (0.0f, 1.0f), 0.0f);
        addF ("ringLevel",  "Ring Mod",    NR (0.0f, 1.0f), 0.0f);

        // ---- Filter
        addC ("filtMode",  "Filter Mode", filtModes, 0);
        addF ("cutoff",    "Cutoff", skewed (20.0f, 20000.0f, 1000.0f), 12000.0f, "Hz");
        addF ("reso",      "Resonance", NR (0.0f, 1.0f), 0.1f);
        addF ("filtDrive", "Filter Drive", NR (0.0f, 1.0f), 0.0f);
        addF ("filtEnv",   "Filter Env Amt", NR (-1.0f, 1.0f), 0.0f);
        addF ("keyTrack",  "Key Track", NR (0.0f, 1.0f), 0.3f);
        addF ("filtVel",   "Filter Velocity", NR (0.0f, 1.0f), 0.0f);

        // ---- Envelopes
        auto timeRange = skewed (0.001f, 10.0f, 0.5f);
        addF ("fA", "Filter Attack",  timeRange, 0.005f, "s");
        addF ("fD", "Filter Decay",   timeRange, 0.4f, "s");
        addF ("fS", "Filter Sustain", NR (0.0f, 1.0f), 0.5f);
        addF ("fR", "Filter Release", timeRange, 0.4f, "s");
        addF ("aA", "Amp Attack",  timeRange, 0.005f, "s");
        addF ("aD", "Amp Decay",   timeRange, 0.3f, "s");
        addF ("aS", "Amp Sustain", NR (0.0f, 1.0f), 0.8f);
        addF ("aR", "Amp Release", timeRange, 0.3f, "s");
        addF ("ampVel", "Amp Velocity", NR (0.0f, 1.0f), 0.5f);

        // ---- LFOs
        addC ("lfo1Wave",  "LFO1 Wave", lfoWaves, 0);
        addF ("lfo1Rate",  "LFO1 Rate", skewed (0.02f, 20.0f, 2.0f), 5.0f, "Hz");
        addF ("lfo1Delay", "LFO1 Fade In", NR (0.0f, 3.0f), 0.0f, "s");
        addF ("lfo1Pitch", "LFO1 > Pitch", NR (0.0f, 1.0f), 0.0f);
        addF ("lfo1Cutoff","LFO1 > Cutoff", NR (0.0f, 1.0f), 0.0f);
        addC ("lfo2Wave",  "LFO2 Wave", lfoWaves, 1);
        addF ("lfo2Rate",  "LFO2 Rate", skewed (0.02f, 20.0f, 2.0f), 0.3f, "Hz");
        addF ("lfo2Shape", "LFO2 > Shape", NR (0.0f, 1.0f), 0.0f);
        addF ("lfo2Pan",   "LFO2 > Pan", NR (0.0f, 1.0f), 0.0f);
        addF ("lfo2Amp",   "LFO2 > Amp", NR (0.0f, 1.0f), 0.0f);

        // ---- Voice
        addC ("voiceMode", "Voice Mode", voiceModes, 0);
        addF ("glide",     "Glide", skewed (0.0f, 2.0f, 0.3f), 0.0f, "s");
        addF ("drift",     "Analog Drift", NR (0.0f, 1.0f), 0.15f);
        addF ("warble",    "Tape Warble", NR (0.0f, 1.0f), 0.0f);
        addF ("spread",    "Stereo Spread", NR (0.0f, 1.0f), 0.5f);
        addI ("bendRange", "Bend Range", 0, 12, 2);
        addF ("master",    "Master", skewed (0.0f, 2.0f, 0.7f), 0.7f);

        // ---- FX: Drive
        addB ("drvOn",  "Drive On", false);
        addC ("drvType","Drive Type", driveTypes, 0);
        addF ("drvAmt", "Drive Amount", NR (0.0f, 1.0f), 0.3f);
        addF ("drvTone","Drive Tone", NR (0.0f, 1.0f), 0.6f);
        addF ("drvMix", "Drive Mix", NR (0.0f, 1.0f), 1.0f);
        // Phaser
        addB ("phsOn",   "Phaser On", false);
        addF ("phsRate", "Phaser Rate", skewed (0.02f, 10.0f, 0.8f), 0.3f, "Hz");
        addF ("phsDepth","Phaser Depth", NR (0.0f, 1.0f), 0.7f);
        addF ("phsFb",   "Phaser Feedback", NR (0.0f, 0.95f), 0.5f);
        addF ("phsMix",  "Phaser Mix", NR (0.0f, 1.0f), 0.5f);
        // Chorus / flanger / ensemble
        addB ("choOn",   "Chorus On", false);
        addC ("choMode", "Chorus Mode", chorusModes, 0);
        addF ("choRate", "Chorus Rate", skewed (0.02f, 10.0f, 0.8f), 0.5f, "Hz");
        addF ("choDepth","Chorus Depth", NR (0.0f, 1.0f), 0.5f);
        addF ("choFb",   "Chorus Feedback", NR (0.0f, 0.95f), 0.0f);
        addF ("choMix",  "Chorus Mix", NR (0.0f, 1.0f), 0.5f);
        // Tremolo
        addB ("trmOn",    "Tremolo On", false);
        addC ("trmShape", "Tremolo Shape", tremShapes, 0);
        addF ("trmRate",  "Tremolo Rate", skewed (0.1f, 20.0f, 4.0f), 5.0f, "Hz");
        addF ("trmDepth", "Tremolo Depth", NR (0.0f, 1.0f), 0.5f);
        addB ("trmPan",   "Tremolo Auto-Pan", false);
        // Tape
        addB ("tapOn",     "Tape On", false);
        addF ("tapWow",    "Tape Wow", NR (0.0f, 1.0f), 0.3f);
        addF ("tapFlutter","Tape Flutter", NR (0.0f, 1.0f), 0.2f);
        addF ("tapSat",    "Tape Saturation", NR (0.0f, 1.0f), 0.3f);
        addF ("tapAge",    "Tape Age", NR (0.0f, 1.0f), 0.2f);
        // Delay A + B (in series = the delay chain)
        for (auto d : { "A", "B" })
        {
            auto p = juce::String ("dly") + d;
            auto n = juce::String ("Delay ") + d + " ";
            auto s = [] (const juce::String& x) { return x.toRawUTF8(); };
            addB (s (p + "On"),   s (n + "On"), false);
            addB (s (p + "Sync"), s (n + "Sync"), true);
            addC (s (p + "Div"),  s (n + "Division"), syncDivs, juce::String (d) == "A" ? 6 : 9);
            addF (s (p + "Ms"),   s (n + "Time"), skewed (10.0f, 2000.0f, 400.0f), 350.0f, "ms");
            addF (s (p + "Fb"),   s (n + "Feedback"), NR (0.0f, 1.05f), 0.4f);
            addF (s (p + "Tone"), s (n + "Tone"), NR (0.0f, 1.0f), 0.6f);
            addF (s (p + "Mod"),  s (n + "Wobble"), NR (0.0f, 1.0f), 0.2f);
            addB (s (p + "Ping"), s (n + "Ping-Pong"), false);
            addF (s (p + "Mix"),  s (n + "Mix"), NR (0.0f, 1.0f), 0.3f);
        }
        // Reverb
        addB ("revOn",   "Reverb On", false);
        addF ("revSize", "Reverb Size", NR (0.0f, 1.0f), 0.6f);
        addF ("revDamp", "Reverb Damping", NR (0.0f, 1.0f), 0.5f);
        addF ("revWidth","Reverb Width", NR (0.0f, 1.0f), 1.0f);
        addF ("revMix",  "Reverb Mix", NR (0.0f, 1.0f), 0.25f);

        return layout;
    }
}
