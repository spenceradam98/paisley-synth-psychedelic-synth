#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include "SynthEngine.h"

// ============================================================================
//  FX rack, fixed order:
//  Drive -> Phaser -> Chorus/Flanger/Ensemble -> Tremolo -> Tape
//        -> Delay A -> Delay B (the delay chain) -> Reverb
// ============================================================================

struct DelayLine
{
    std::vector<float> buf;
    int write = 0;
    void prepare (int size) { buf.assign ((size_t) juce::jmax (4, size), 0.0f); write = 0; }
    void clear() { std::fill (buf.begin(), buf.end(), 0.0f); }
    void push (float x) { buf[(size_t) write] = x; if (++write >= (int) buf.size()) write = 0; }
    // delay in samples (fractional), measured from most recent pushed sample
    float read (float d) const
    {
        const int size = (int) buf.size();
        d = juce::jlimit (1.0f, (float) size - 3.0f, d);
        float rp = (float) write - d;
        while (rp < 0) rp += (float) size;
        const int i0 = (int) rp; const float fr = rp - (float) i0;
        const int i1 = (i0 + 1) % size;
        return buf[(size_t) i0] + (buf[(size_t) i1] - buf[(size_t) i0]) * fr;
    }
};

struct OnePole
{
    float z = 0;
    float lp (float x, float coef) { z += coef * (x - z); return z; }
    static float coefFor (float hz, float sr) { return 1.0f - std::exp (-dsp::twoPi * hz / sr); }
};

class FxRack
{
public:
    explicit FxRack (juce::AudioProcessorValueTreeState& s) : apvts (s) {}

    float p (const char* id) const { return apvts.getRawParameterValue (id)->load(); }
    bool on (const char* id) const { return p (id) > 0.5f; }

    void prepare (double sampleRate, int maxBlock)
    {
        sr = (float) sampleRate;
        for (auto& d : choLines) d.prepare ((int) (sr * 0.06f));
        for (auto& d : tapeLines) d.prepare ((int) (sr * 0.05f));
        for (auto& d : dlyLines) d.prepare ((int) (sr * 4.2f));
        reverb.setSampleRate (sampleRate);
        juce::ignoreUnused (maxBlock);
        tapeWowLfo.prepare (sampleRate);
        for (auto& w : dlyWobble) w.prepare (sampleRate);
        reset();
    }

    void reset()
    {
        for (auto& d : choLines) d.clear();
        for (auto& d : tapeLines) d.clear();
        for (auto& d : dlyLines) d.clear();
        reverb.reset();
        for (auto& a : apState) for (auto& z : a) z = 0;
        phsFbState[0] = phsFbState[1] = 0;
        for (auto& t : dlyTime) t = -1.0f;
    }

    void setBpm (double b) { bpm = b > 0 ? (float) b : 120.0f; }

    void process (juce::AudioBuffer<float>& buffer)
    {
        const int n = buffer.getNumSamples();
        if (buffer.getNumChannels() < 2) return;
        float* L = buffer.getWritePointer (0);
        float* R = buffer.getWritePointer (1);

        if (on ("drvOn"))  drive (L, R, n);
        if (on ("phsOn"))  phaser (L, R, n);
        if (on ("choOn"))  chorus (L, R, n);
        if (on ("trmOn"))  tremolo (L, R, n);
        if (on ("tapOn"))  tape (L, R, n);
        if (on ("dlyAOn")) delay (0, "dlyA", L, R, n);
        if (on ("dlyBOn")) delay (1, "dlyB", L, R, n);
        if (on ("revOn"))  reverbFx (L, R, n);
    }

private:
    // ---------------------------------------------------------------- drive
    void drive (float* L, float* R, int n)
    {
        const int type = (int) p ("drvType");
        const float amt = p ("drvAmt"), mix = p ("drvMix");
        const float tc = OnePole::coefFor (800.0f * std::exp2 (p ("drvTone") * 4.5f), sr);
        const float gain = 1.0f + amt * (type == 1 ? 40.0f : 10.0f);
        const float outGain = 1.0f / std::sqrt (gain) * (type == 1 ? 1.6f : 1.2f);
        float* ch[2] = { L, R };
        for (int c = 0; c < 2; ++c)
            for (int i = 0; i < n; ++i)
            {
                const float x = ch[c][i];
                float y = x * gain;
                if (type == 0)      y = std::tanh (y);
                else if (type == 1) { y = y > 0 ? juce::jmin (y, 0.8f) : juce::jmax (y * 1.3f, -1.0f); y = std::tanh (y * 2.0f); }
                else                y = y / (1.0f + std::abs (y)) + 0.15f * std::tanh (y * y * 0.3f);
                y = drvTone[c].lp (y, tc) * outGain;
                ch[c][i] = x + (y - x) * mix;
            }
    }

    // --------------------------------------------------------------- phaser
    void phaser (float* L, float* R, int n)
    {
        const float rate = p ("phsRate"), depth = p ("phsDepth"), fb = p ("phsFb"), mix = p ("phsMix");
        for (int i = 0; i < n; ++i)
        {
            phsPhase += rate / sr; if (phsPhase >= 1.0f) phsPhase -= 1.0f;
            float* ch[2] = { L, R };
            for (int c = 0; c < 2; ++c)
            {
                const float lfo = 0.5f + 0.5f * std::sin (dsp::twoPi * (phsPhase + c * 0.25f));
                const float f = 180.0f * std::exp2 (lfo * depth * 6.0f);
                const float t = std::tan (dsp::pi * juce::jmin (f, sr * 0.45f) / sr);
                const float a = (t - 1.0f) / (t + 1.0f);
                const float x = ch[c][i];
                float y = x + phsFbState[c] * fb;
                for (int s = 0; s < 6; ++s)
                {
                    const float o = a * y + apState[c][s];
                    apState[c][s] = y - a * o;
                    y = o;
                }
                phsFbState[c] = y;
                ch[c][i] = x * (1.0f - mix * 0.5f) + y * mix * 0.5f * 1.4f;
            }
        }
    }

    // -------------------------------------------------- chorus / flanger / ensemble
    void chorus (float* L, float* R, int n)
    {
        const int mode = (int) p ("choMode");
        const float rate = p ("choRate"), depth = p ("choDepth"), fb = p ("choFb"), mix = p ("choMix");
        const float ms = sr * 0.001f;
        const float centre = mode == 1 ? 2.5f : (mode == 2 ? 9.0f : 14.0f);
        const float range  = mode == 1 ? 2.2f : (mode == 2 ? 3.5f : 7.0f);
        float* ch[2] = { L, R };
        for (int i = 0; i < n; ++i)
        {
            choPhase += rate / sr; if (choPhase >= 1.0f) choPhase -= 1.0f;
            choPhase2 += 6.3f / sr; if (choPhase2 >= 1.0f) choPhase2 -= 1.0f;
            for (int c = 0; c < 2; ++c)
            {
                const float x = ch[c][i];
                choLines[c].push (x + choFbState[c] * fb);
                float wet = 0;
                if (mode == 2) // ensemble: three taps 120 deg apart + fast vibrato layer
                {
                    for (int t = 0; t < 3; ++t)
                    {
                        const float ph = choPhase + t / 3.0f + c * 0.17f;
                        const float m = std::sin (dsp::twoPi * ph) * depth + 0.15f * std::sin (dsp::twoPi * (choPhase2 + t / 3.0f));
                        wet += choLines[c].read ((centre + range * m) * ms);
                    }
                    wet *= 0.45f;
                }
                else
                {
                    const float m = std::sin (dsp::twoPi * (choPhase + c * 0.25f));
                    wet = choLines[c].read ((centre + range * depth * m) * ms);
                }
                choFbState[c] = wet;
                ch[c][i] = x * (1.0f - mix * 0.5f) + wet * mix;
            }
        }
    }

    // --------------------------------------------------------------- tremolo
    void tremolo (float* L, float* R, int n)
    {
        const int shape = (int) p ("trmShape");
        const float rate = p ("trmRate"), depth = p ("trmDepth");
        const bool pan = on ("trmPan");
        for (int i = 0; i < n; ++i)
        {
            trmPhase += rate / sr; if (trmPhase >= 1.0f) trmPhase -= 1.0f;
            auto wave = [shape] (float ph)
            {
                if (shape == 0) return 0.5f + 0.5f * std::sin (dsp::twoPi * ph);
                if (shape == 1) return 0.5f + 0.5f * std::tanh (6.0f * std::sin (dsp::twoPi * ph));
                return 1.0f - ph;
            };
            const float wl = wave (trmPhase);
            float ph2 = trmPhase + (pan ? 0.5f : 0.0f); if (ph2 >= 1.0f) ph2 -= 1.0f;
            const float wr = wave (ph2);
            L[i] *= 1.0f - depth * (1.0f - wl);
            R[i] *= 1.0f - depth * (1.0f - wr);
        }
    }

    // ------------------------------------------------------------------ tape
    void tape (float* L, float* R, int n)
    {
        const float wow = p ("tapWow"), flut = p ("tapFlutter"), sat = p ("tapSat"), ageAmt = p ("tapAge");
        const float ms = sr * 0.001f;
        const float lpc = OnePole::coefFor (18000.0f * std::exp2 (-ageAmt * 2.8f), sr);
        const float drive = 1.0f + sat * 4.0f, norm = 1.0f / std::tanh (drive);
        float* ch[2] = { L, R };
        for (int i = 0; i < n; ++i)
        {
            const float w = 0.7f * tapeWowLfo.next (5, 0.6f) + 0.3f * std::sin (dsp::twoPi * (tapePhase));
            tapePhase += 0.45f / sr; if (tapePhase >= 1.0f) tapePhase -= 1.0f;
            flutPhase += 7.3f / sr;  if (flutPhase >= 1.0f) flutPhase -= 1.0f;
            const float f = std::sin (dsp::twoPi * flutPhase) + 0.3f * rng.next();
            const float d = (6.0f + w * wow * 4.0f + f * flut * 0.25f) * ms;
            for (int c = 0; c < 2; ++c)
            {
                tapeLines[c].push (ch[c][i]);
                float y = tapeLines[c].read (d);
                y = std::tanh (y * drive) * norm;
                y = tapeLp[c].lp (y, lpc);
                y += rng.next() * ageAmt * ageAmt * 0.004f;
                ch[c][i] = y;
            }
        }
    }

    // ----------------------------------------------------------------- delay
    void delay (int idx, const char* pre, float* L, float* R, int n)
    {
        auto id = [pre] (const char* s) { return juce::String (pre) + s; };
        auto par = [&] (const char* s) { return apvts.getRawParameterValue (id (s))->load(); };
        float timeMs;
        if (par ("Sync") > 0.5f)
        {
            const int div = juce::jlimit (0, Params::syncDivs.size() - 1, (int) par ("Div"));
            timeMs = Params::syncBeats[div] * 60000.0f / bpm;
        }
        else timeMs = par ("Ms");
        timeMs = juce::jlimit (5.0f, 4000.0f, timeMs);
        const float target = timeMs * sr * 0.001f;
        if (dlyTime[idx] < 0) dlyTime[idx] = target;

        const float fb = par ("Fb"), mix = par ("Mix"), mod = par ("Mod");
        const bool ping = par ("Ping") > 0.5f;
        const float lpc = OnePole::coefFor (1200.0f * std::exp2 (par ("Tone") * 4.0f), sr);
        const float hpc = OnePole::coefFor (90.0f, sr);
        const float smooth = 1.0f - std::exp (-1.0f / (0.25f * sr)); // tape-style glide on time change
        auto& lines = dlyLines;
        const int l0 = idx * 2, l1 = idx * 2 + 1;

        for (int i = 0; i < n; ++i)
        {
            dlyTime[idx] += (target - dlyTime[idx]) * smooth;
            dlyModPhase[idx] += 0.7f / sr; if (dlyModPhase[idx] >= 1.0f) dlyModPhase[idx] -= 1.0f;
            const float m = (std::sin (dsp::twoPi * dlyModPhase[idx]) + 0.4f * dlyWobble[idx].next (5, 2.3f)) * mod * mod * 0.012f;
            const float dL = dlyTime[idx] * (1.0f + m);
            const float dR = dlyTime[idx] * (1.0f - m * 0.8f);

            float wl = lines[(size_t) l0].read (dL);
            float wr = lines[(size_t) l1].read (dR);

            // feedback path: tone, low cut, soft saturation (lets it self-oscillate musically)
            auto shapeFb = [&] (float x, int c)
            {
                x = dlyLp[idx * 2 + c].lp (x, lpc);
                x -= dlyHp[idx * 2 + c].lp (x, hpc);
                return std::tanh (x * fb * 1.1f) / 1.1f;
            };
            const float fl = shapeFb (wl, 0), fr = shapeFb (wr, 1);
            if (ping)
            {
                lines[(size_t) l0].push (0.5f * (L[i] + R[i]) + fr);
                lines[(size_t) l1].push (fl);
            }
            else
            {
                lines[(size_t) l0].push (L[i] + fl);
                lines[(size_t) l1].push (R[i] + fr);
            }
            L[i] = L[i] * (1.0f - mix * 0.4f) + wl * mix;
            R[i] = R[i] * (1.0f - mix * 0.4f) + wr * mix;
        }
    }

    // ---------------------------------------------------------------- reverb
    void reverbFx (float* L, float* R, int n)
    {
        juce::Reverb::Parameters rp;
        rp.roomSize = 0.3f + 0.69f * p ("revSize");
        rp.damping = p ("revDamp");
        rp.width = p ("revWidth");
        const float mix = p ("revMix");
        rp.wetLevel = mix * 0.6f;
        rp.dryLevel = 1.0f - mix * 0.5f;
        reverb.setParameters (rp);
        reverb.processStereo (L, R, n);
    }

    juce::AudioProcessorValueTreeState& apvts;
    float sr = 44100.0f, bpm = 120.0f;
    dsp::Rng rng;

    OnePole drvTone[2];
    float phsPhase = 0, apState[2][6] {}, phsFbState[2] {};
    DelayLine choLines[2]; float choPhase = 0, choPhase2 = 0, choFbState[2] {};
    float trmPhase = 0;
    DelayLine tapeLines[2]; OnePole tapeLp[2]; GlobalLfo tapeWowLfo; float tapePhase = 0, flutPhase = 0;
    std::array<DelayLine, 4> dlyLines; OnePole dlyLp[4], dlyHp[4]; float dlyTime[2] { -1, -1 }, dlyModPhase[2] {};
    GlobalLfo dlyWobble[2];
    juce::Reverb reverb;

public:
    void prepareLfos (double s) { tapeWowLfo.prepare (s); for (auto& w : dlyWobble) w.prepare (s); }
};
