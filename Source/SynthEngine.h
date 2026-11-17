#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <array>
#include <random>
#include "Params.h"

// ============================================================================
//  Paisley synth engine: 12-voice poly / mono, two multi-model oscillators
//  with unison, sub, noise, ring mod, multimode TPT filter, two ADSRs,
//  two global LFOs, analog drift and tape warble on pitch.
// ============================================================================

namespace dsp
{
    constexpr float twoPi = juce::MathConstants<float>::twoPi;
    constexpr float pi    = juce::MathConstants<float>::pi;

    inline float polyBlep (float t, float dt)
    {
        if (t < dt)        { t /= dt;            return t + t - t * t - 1.0f; }
        if (t > 1.0f - dt) { t = (t - 1.0f) / dt; return t * t + t + t + 1.0f; }
        return 0.0f;
    }

    inline float fold (float x, float amt)
    {
        if (amt <= 0.001f) return x;
        return std::sin (x * (1.0f + amt * 5.0f) * pi * 0.5f);
    }

    struct Rng
    {
        uint32_t s = 0x12345678u;
        void seed (uint32_t v) { s = v ? v : 1u; }
        float next() // -1..1
        {
            s ^= s << 13; s ^= s >> 17; s ^= s << 5;
            return (float) (s & 0xffffff) / (float) 0x7fffff - 1.0f;
        }
    };

    // Zero-delay-feedback state variable filter (Simper / Zavalishin)
    struct SVF
    {
        float ic1 = 0, ic2 = 0;
        void reset() { ic1 = ic2 = 0; }
        struct Out { float lp, bp, hp; };
        Out process (float v0, float g, float k)
        {
            const float a1 = 1.0f / (1.0f + g * (g + k));
            const float a2 = g * a1, a3 = g * a2;
            const float v3 = v0 - ic2;
            const float v1 = a1 * ic1 + a2 * v3;
            const float v2 = ic2 + a2 * ic1 + a3 * v3;
            ic1 = 2.0f * v1 - ic1;
            ic2 = 2.0f * v2 - ic2;
            return { v2, v1, v0 - k * v1 - v2 };
        }
    };
}

// Snapshot of all synth (non-FX) parameters, taken once per block.
struct SynthParams
{
    struct Osc { int wave; float pitchOffset; float shape, level; int unison; float detune; } osc[2];
    float sub, noise, ring;
    int filtMode; float cutoff, reso, filtDrive, filtEnv, keyTrack, filtVel;
    float fA, fD, fS, fR, aA, aD, aS, aR, ampVel;
    float lfo1Delay, lfo1Pitch, lfo1Cutoff, lfo2Shape, lfo2Pan, lfo2Amp;
    int voiceMode; float glide, drift, warble, spread; int bendRange;
};

class Voice
{
public:
    static constexpr int maxUnison = 5;

    void prepare (double sr)
    {
        sampleRate = (float) sr;
        ampEnv.setSampleRate (sr);
        filtEnv.setSampleRate (sr);
    }

    bool isActive() const  { return ampEnv.isActive(); }
    bool isGateOn() const  { return gate; }
    int  getNote() const   { return note; }

    void start (int midiNote, float vel, const SynthParams& p, float glideFromPitch, bool retrigger, uint32_t seed)
    {
        const bool wasActive = isActive();
        note = midiNote;
        velocity = vel;
        gate = true;
        sustained = false;
        targetPitch = (float) midiNote;
        if (! wasActive || p.glide <= 0.0005f)
            pitch = (p.glide > 0.0005f && glideFromPitch >= 0.0f) ? glideFromPitch : targetPitch;

        if (! wasActive)
        {
            rng.seed (seed);
            driftVal = rng.next() * 0.5f;
            for (auto& o : phase) for (auto& ph : o) ph = (rng.next() * 0.5f + 0.5f);
            subPhase = 0;
            for (auto& f : filt) f.reset();
            for (auto& f : formant) f.reset();
            lfoFadeSamples = 0;
        }
        if (retrigger || ! wasActive)
        {
            setEnvParams (p);
            ampEnv.noteOn();
            filtEnv.noteOn();
            lfoFadeSamples = 0;
        }
        age = 0;
    }

    void setTarget (int midiNote) { note = midiNote; targetPitch = (float) midiNote; }

    void release()
    {
        gate = false;
        ampEnv.noteOff();
        filtEnv.noteOff();
    }

    void kill() { ampEnv.reset(); filtEnv.reset(); gate = false; }

    void setEnvParams (const SynthParams& p)
    {
        ampEnv.setParameters ({ p.aA, p.aD, p.aS, p.aR });
        filtEnv.setParameters ({ p.fA, p.fD, p.fS, p.fR });
    }

    bool sustained = false;
    uint64_t age = 0;

    // Render and add into out L/R. lfo1/lfo2/wow are per-sample global modulators.
    void render (float* outL, float* outR, int start, int num, const SynthParams& p,
                 const float* lfo1, const float* lfo2, const float* wow, float bend, float modWheel)
    {
        if (! isActive()) return;
        setEnvParams (p);

        const float nyq = sampleRate * 0.5f;
        const float glideCoef = p.glide > 0.0005f ? 1.0f - std::exp (-1.0f / (p.glide * 0.35f * sampleRate)) : 1.0f;
        const float velAmp = 1.0f - p.ampVel + p.ampVel * velocity;
        const float lfoFadeLen = p.lfo1Delay * sampleRate;
        const float vibDepth = juce::jlimit (0.0f, 1.0f, p.lfo1Pitch + modWheel * 0.5f);
        const float vibSemis = vibDepth * vibDepth * 2.0f;

        // Unison detune ratios + pans
        float uniRatio[2][maxUnison], uniPanL[2][maxUnison], uniPanR[2][maxUnison], uniNorm[2];
        for (int o = 0; o < 2; ++o)
        {
            const int n = p.osc[o].unison;
            uniNorm[o] = 1.0f / std::sqrt ((float) n);
            for (int u = 0; u < n; ++u)
            {
                const float pos = n == 1 ? 0.0f : (2.0f * u / (n - 1) - 1.0f); // -1..1
                const float cents = pos * p.osc[o].detune * p.osc[o].detune * 50.0f
                                  + (u == 0 ? 0.0f : 1.3f * (float) u * p.osc[o].detune); // de-correlate
                uniRatio[o][u] = std::exp2 (cents / 1200.0f);
                const float pan = 0.5f + 0.5f * pos * p.spread;
                uniPanL[o][u] = std::cos (pan * dsp::pi * 0.5f) * 1.41421f;
                uniPanR[o][u] = std::sin (pan * dsp::pi * 0.5f) * 1.41421f;
            }
        }

        // Organ drawbar registrations (16' 5 1/3' 8' 4' 2 2/3' 2' 1 3/5' 1 1/3' 1')
        static const float regs[4][9] = { { 8, 8, 8, 0, 0, 0, 0, 0, 0 },
                                          { 0, 0, 8, 4, 0, 0, 0, 0, 0 },
                                          { 6, 8, 8, 6, 0, 0, 0, 0, 4 },
                                          { 0, 0, 8, 8, 6, 8, 4, 4, 8 } };
        static const float organHarm[9] = { 1, 3, 2, 4, 6, 8, 10, 12, 16 }; // relative to 16'

        // Vowel formants A E I O U (F1, F2)
        static const float vowels[5][2] = { { 800, 1150 }, { 400, 1600 }, { 350, 2300 }, { 450, 800 }, { 325, 700 } };

        for (int i = start; i < start + num; ++i)
        {
            pitch += (targetPitch - pitch) * glideCoef;

            // analog drift random walk
            driftVal = juce::jlimit (-1.0f, 1.0f, driftVal + rng.next() * 0.0004f);

            const float lfoFade = lfoFadeLen > 1.0f ? juce::jmin (1.0f, (float) lfoFadeSamples / lfoFadeLen) : 1.0f;
            ++lfoFadeSamples;

            const float pitchMod = bend * (float) p.bendRange
                                 + lfo1[i] * vibSemis * lfoFade
                                 + driftVal * p.drift * 0.15f
                                 + wow[i] * p.warble * p.warble * 0.6f;

            float sumL = 0, sumR = 0;
            float raw[2] = { 0, 0 };

            for (int o = 0; o < 2; ++o)
            {
                const auto& op = p.osc[o];
                if (op.level <= 0.0001f && ! (o == 0 && (p.sub > 0 || p.ring > 0)) && ! (o == 1 && p.ring > 0))
                    continue;

                const float basePitch = pitch + op.pitchOffset + pitchMod;
                float freq = 440.0f * std::exp2 ((basePitch - 69.0f) / 12.0f);
                const float shape = juce::jlimit (0.0f, 1.0f, op.shape + lfo2[i] * p.lfo2Shape * 0.5f);
                const bool organ = op.wave == 4;
                if (organ) freq *= 0.5f; // phase runs at 16' footage

                float oL = 0, oR = 0;
                for (int u = 0; u < op.unison; ++u)
                {
                    const float f = freq * uniRatio[o][u];
                    const float dt = juce::jmin (0.45f, f / sampleRate);
                    float& ph = phase[o][u];
                    float s = 0;
                    switch (op.wave)
                    {
                        case 0: s = dsp::fold (std::sin (dsp::twoPi * ph), shape); break;
                        case 1: s = dsp::fold (4.0f * std::abs (ph - 0.5f) - 1.0f, shape); break;
                        case 2: s = dsp::fold (2.0f * ph - 1.0f - dsp::polyBlep (ph, dt), shape * 0.6f); break;
                        case 3:
                        {
                            const float pw = 0.5f - shape * 0.45f;
                            s = (ph < pw ? 1.0f : -1.0f) + dsp::polyBlep (ph, dt);
                            float t2 = ph - pw; if (t2 < 0) t2 += 1.0f;
                            s -= dsp::polyBlep (t2, dt);
                            break;
                        }
                        case 4:
                        {
                            const float rpos = shape * 2.999f;
                            const int r0 = (int) rpos; const float rf = rpos - (float) r0;
                            float acc = 0, norm = 0;
                            for (int h = 0; h < 9; ++h)
                            {
                                const float a = regs[r0][h] + (regs[r0 + 1][h] - regs[r0][h]) * rf;
                                if (a <= 0 || f * organHarm[h] > nyq * 0.9f) continue;
                                float hp = ph * organHarm[h]; hp -= std::floor (hp);
                                acc += a * std::sin (dsp::twoPi * hp);
                                norm += a;
                            }
                            s = norm > 0 ? acc / (norm * 0.55f + 2.0f) * 1.6f : 0.0f;
                            break;
                        }
                        case 5: s = 2.0f * ph - 1.0f - dsp::polyBlep (ph, dt); break; // formant-filtered below
                        case 6:
                        {
                            float h2 = ph * 2.0f; h2 -= std::floor (h2);
                            float h3 = ph * 3.0f; h3 -= std::floor (h3);
                            s = std::sin (dsp::twoPi * ph) + 0.25f * std::sin (dsp::twoPi * h2) + 0.08f * std::sin (dsp::twoPi * h3)
                              + rng.next() * shape * 0.35f;
                            break;
                        }
                        default: s = rng.next(); break;
                    }
                    ph += dt;
                    if (ph >= 1.0f) ph -= 1.0f;
                    if (u == 0) raw[o] = s;
                    oL += s * uniPanL[o][u];
                    oR += s * uniPanR[o][u];
                }
                oL *= uniNorm[o]; oR *= uniNorm[o];

                if (op.wave == 5) // vox: two parallel formant band-passes per channel
                {
                    const float vpos = shape * 3.999f;
                    const int v0 = (int) vpos; const float vf = vpos - (float) v0;
                    const float f1 = vowels[v0][0] + (vowels[v0 + 1][0] - vowels[v0][0]) * vf;
                    const float f2 = vowels[v0][1] + (vowels[v0 + 1][1] - vowels[v0][1]) * vf;
                    const float g1 = std::tan (dsp::pi * juce::jmin (f1, nyq * 0.9f) / sampleRate);
                    const float g2 = std::tan (dsp::pi * juce::jmin (f2, nyq * 0.9f) / sampleRate);
                    const float k = 0.18f;
                    float l = formant[o * 4 + 0].process (oL, g1, k).bp + 0.6f * formant[o * 4 + 1].process (oL, g2, k).bp;
                    float r = formant[o * 4 + 2].process (oR, g1, k).bp + 0.6f * formant[o * 4 + 3].process (oR, g2, k).bp;
                    oL = l * 0.5f; oR = r * 0.5f;
                }

                sumL += oL * op.level;
                sumR += oR * op.level;
            }

            // Sub oscillator (sine, an octave under osc 1)
            if (p.sub > 0.0001f)
            {
                const float f = 440.0f * std::exp2 ((pitch + p.osc[0].pitchOffset + pitchMod - 12.0f - 69.0f) / 12.0f);
                const float s = std::sin (dsp::twoPi * subPhase) * p.sub;
                subPhase += f / sampleRate; if (subPhase >= 1.0f) subPhase -= 1.0f;
                sumL += s; sumR += s;
            }
            if (p.noise > 0.0001f)
            {
                sumL += rng.next() * p.noise * 0.5f;
                sumR += rng.next() * p.noise * 0.5f;
            }
            if (p.ring > 0.0001f)
            {
                const float r = raw[0] * raw[1] * p.ring;
                sumL += r; sumR += r;
            }

            // ---- Filter
            const float fe = filtEnv.getNextSample();
            const float octs = p.filtEnv * 6.0f * fe
                             + lfo1[i] * p.lfo1Cutoff * 3.0f * lfoFade
                             + p.keyTrack * (pitch - 60.0f) / 12.0f
                             + p.filtVel * (velocity - 0.7f) * 3.0f;
            const float fc = juce::jlimit (16.0f, nyq * 0.92f, p.cutoff * std::exp2 (octs));
            const float g = std::tan (dsp::pi * fc / sampleRate);
            const float k = 2.0f - 1.96f * p.reso;

            if (p.filtDrive > 0.001f)
            {
                const float d = 1.0f + p.filtDrive * 5.0f;
                const float n = 1.0f / std::tanh (d);
                sumL = std::tanh (sumL * d) * n;
                sumR = std::tanh (sumR * d) * n;
            }

            float yL, yR;
            switch (p.filtMode)
            {
                case 0: // LP24
                    yL = filt[1].process (filt[0].process (sumL, g, 1.414f).lp, g, k).lp;
                    yR = filt[3].process (filt[2].process (sumR, g, 1.414f).lp, g, k).lp;
                    break;
                case 1: yL = filt[0].process (sumL, g, k).lp; yR = filt[2].process (sumR, g, k).lp; break;
                case 2: yL = filt[0].process (sumL, g, k).bp * 1.5f; yR = filt[2].process (sumR, g, k).bp * 1.5f; break;
                case 3: yL = filt[0].process (sumL, g, k).hp; yR = filt[2].process (sumR, g, k).hp; break;
                default:
                {
                    auto a = filt[0].process (sumL, g, k); auto b = filt[2].process (sumR, g, k);
                    yL = a.lp + a.hp; yR = b.lp + b.hp; break;
                }
            }

            // ---- Amp
            const float env = ampEnv.getNextSample();
            float amp = env * velAmp * (1.0f - p.lfo2Amp * 0.5f * (1.0f - lfo2[i]));
            float panL = 1.0f, panR = 1.0f;
            if (p.lfo2Pan > 0.0001f)
            {
                const float pan = 0.5f + 0.5f * lfo2[i] * p.lfo2Pan;
                panL = std::cos (pan * dsp::pi * 0.5f) * 1.41421f;
                panR = std::sin (pan * dsp::pi * 0.5f) * 1.41421f;
            }
            outL[i] += yL * amp * panL * 0.3f;
            outR[i] += yR * amp * panR * 0.3f;

            if (! ampEnv.isActive()) break;
        }
        age += (uint64_t) num;
    }

private:
    float sampleRate = 44100.0f;
    int note = -1;
    float velocity = 1.0f;
    bool gate = false;
    float pitch = 60.0f, targetPitch = 60.0f;
    float driftVal = 0.0f;
    float phase[2][maxUnison] {};
    float subPhase = 0.0f;
    int64_t lfoFadeSamples = 0;
    dsp::SVF filt[4];
    dsp::SVF formant[8];
    dsp::Rng rng;
    juce::ADSR ampEnv, filtEnv;
};

// ---------------------------------------------------------------------------
class GlobalLfo
{
public:
    void prepare (double sr) { sampleRate = (float) sr; }
    void reset() { phase = 0; }
    float next (int wave, float rate)
    {
        phase += rate / sampleRate;
        bool wrapped = false;
        if (phase >= 1.0f) { phase -= 1.0f; wrapped = true; }
        if (wrapped) { prevSh = curSh; curSh = rng.next(); }
        switch (wave)
        {
            case 0: return std::sin (dsp::twoPi * phase);
            case 1: return 4.0f * std::abs (phase - 0.5f) - 1.0f;
            case 2: return 1.0f - 2.0f * phase;
            case 3: return phase < 0.5f ? 1.0f : -1.0f;
            case 4: return curSh;
            default:
            {
                const float t = phase * phase * (3.0f - 2.0f * phase);
                return prevSh + (curSh - prevSh) * t;
            }
        }
    }
private:
    float sampleRate = 44100.0f, phase = 0.0f, curSh = 0.0f, prevSh = 0.0f;
    dsp::Rng rng;
};

// ---------------------------------------------------------------------------
class SynthEngine
{
public:
    static constexpr int numVoices = 12;

    explicit SynthEngine (juce::AudioProcessorValueTreeState& s) : apvts (s)
    {
        auto get = [&] (const juce::String& id) { return apvts.getRawParameterValue (id); };
        for (int o = 0; o < 2; ++o)
        {
            auto pre = "osc" + juce::String (o + 1);
            op[o] = { get (pre + "Wave"), get (pre + "Oct"), get (pre + "Semi"), get (pre + "Fine"),
                      get (pre + "Shape"), get (pre + "Level"), get (pre + "Unison"), get (pre + "Detune") };
        }
        sub = get ("subLevel"); noise = get ("noiseLevel"); ring = get ("ringLevel");
        filtMode = get ("filtMode"); cutoff = get ("cutoff"); reso = get ("reso"); filtDrive = get ("filtDrive");
        filtEnvAmt = get ("filtEnv"); keyTrack = get ("keyTrack"); filtVel = get ("filtVel");
        fA = get ("fA"); fD = get ("fD"); fS = get ("fS"); fR = get ("fR");
        aA = get ("aA"); aD = get ("aD"); aS = get ("aS"); aR = get ("aR"); ampVel = get ("ampVel");
        lfo1Wave = get ("lfo1Wave"); lfo1Rate = get ("lfo1Rate"); lfo1Delay = get ("lfo1Delay");
        lfo1Pitch = get ("lfo1Pitch"); lfo1Cutoff = get ("lfo1Cutoff");
        lfo2Wave = get ("lfo2Wave"); lfo2Rate = get ("lfo2Rate"); lfo2Shape = get ("lfo2Shape");
        lfo2Pan = get ("lfo2Pan"); lfo2Amp = get ("lfo2Amp");
        voiceMode = get ("voiceMode"); glide = get ("glide"); drift = get ("drift"); warble = get ("warble");
        spread = get ("spread"); bendRange = get ("bendRange");
    }

    void prepare (double sr, int maxBlock)
    {
        sampleRate = sr;
        for (auto& v : voices) v.prepare (sr);
        lfo1.prepare (sr); lfo2.prepare (sr); wowLfo.prepare (sr); flutLfo.prepare (sr);
        lfo1Buf.assign ((size_t) maxBlock, 0.0f);
        lfo2Buf.assign ((size_t) maxBlock, 0.0f);
        wowBuf.assign ((size_t) maxBlock, 0.0f);
    }

    void allNotesOff() { for (auto& v : voices) v.kill(); heldNotes.clear(); }

    void process (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
    {
        const int n = buffer.getNumSamples();
        if ((int) lfo1Buf.size() < n) { lfo1Buf.resize ((size_t) n); lfo2Buf.resize ((size_t) n); wowBuf.resize ((size_t) n); }
        snapshot();

        const int w1 = (int) lfo1Wave->load(), w2 = (int) lfo2Wave->load();
        const float r1 = lfo1Rate->load(), r2 = lfo2Rate->load();
        for (int i = 0; i < n; ++i)
        {
            lfo1Buf[(size_t) i] = lfo1.next (w1, r1);
            lfo2Buf[(size_t) i] = lfo2.next (w2, r2);
            wowBuf[(size_t) i] = 0.7f * wowLfo.next (5, 0.9f) + 0.15f * flutLfo.next (0, 6.5f);
        }

        auto* L = buffer.getWritePointer (0);
        auto* R = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : nullptr;
        buffer.clear();
        std::vector<float>& dummy = scratch; if (! R) { dummy.assign ((size_t) n, 0.0f); R = dummy.data(); }

        int pos = 0;
        for (const auto meta : midi)
        {
            const int t = juce::jlimit (0, n, meta.samplePosition);
            renderVoices (L, R, pos, t - pos);
            pos = t;
            handleMidi (meta.getMessage());
        }
        renderVoices (L, R, pos, n - pos);
    }

private:
    void snapshot()
    {
        for (int o = 0; o < 2; ++o)
        {
            auto& d = sp.osc[o]; auto& s = op[o];
            d.wave = (int) s.wave->load();
            d.pitchOffset = s.oct->load() * 12.0f + s.semi->load() + s.fine->load() / 100.0f;
            d.shape = s.shape->load(); d.level = s.level->load();
            d.unison = juce::jlimit (1, Voice::maxUnison, (int) s.unison->load());
            d.detune = s.detune->load();
        }
        sp.sub = sub->load(); sp.noise = noise->load(); sp.ring = ring->load();
        sp.filtMode = (int) filtMode->load(); sp.cutoff = cutoff->load(); sp.reso = reso->load();
        sp.filtDrive = filtDrive->load(); sp.filtEnv = filtEnvAmt->load(); sp.keyTrack = keyTrack->load(); sp.filtVel = filtVel->load();
        sp.fA = fA->load(); sp.fD = fD->load(); sp.fS = fS->load(); sp.fR = fR->load();
        sp.aA = aA->load(); sp.aD = aD->load(); sp.aS = aS->load(); sp.aR = aR->load(); sp.ampVel = ampVel->load();
        sp.lfo1Delay = lfo1Delay->load(); sp.lfo1Pitch = lfo1Pitch->load(); sp.lfo1Cutoff = lfo1Cutoff->load();
        sp.lfo2Shape = lfo2Shape->load(); sp.lfo2Pan = lfo2Pan->load(); sp.lfo2Amp = lfo2Amp->load();
        sp.voiceMode = (int) voiceMode->load(); sp.glide = glide->load(); sp.drift = drift->load();
        sp.warble = warble->load(); sp.spread = spread->load(); sp.bendRange = (int) bendRange->load();
    }

    void renderVoices (float* L, float* R, int start, int num)
    {
        if (num <= 0) return;
        for (auto& v : voices)
            v.render (L, R, start, num, sp, lfo1Buf.data(), lfo2Buf.data(), wowBuf.data(), bend, modWheel);
    }

    void handleMidi (const juce::MidiMessage& m)
    {
        if (m.isNoteOn())            noteOn (m.getNoteNumber(), m.getFloatVelocity());
        else if (m.isNoteOff())      noteOff (m.getNoteNumber());
        else if (m.isPitchWheel())   bend = (float) (m.getPitchWheelValue() - 8192) / 8192.0f;
        else if (m.isController())
        {
            if (m.getControllerNumber() == 1) modWheel = m.getControllerValue() / 127.0f;
            else if (m.getControllerNumber() == 64)
            {
                sustainPedal = m.getControllerValue() >= 64;
                if (! sustainPedal)
                    for (auto& v : voices) if (v.sustained) { v.sustained = false; v.release(); }
            }
        }
        else if (m.isAllNotesOff() || m.isAllSoundOff()) allNotesOff();
    }

    void noteOn (int note, float vel)
    {
        const bool mono = sp.voiceMode != 0;
        if (mono)
        {
            heldNotes.removeFirstMatchingValue (note);
            heldNotes.add (note);
            auto& v = voices[0];
            const bool legato = sp.voiceMode == 2 && v.isGateOn();
            v.start (note, vel, sp, lastPitch, ! legato, ++seedCounter);
            if (legato) v.setTarget (note);
            lastPitch = (float) note;
            return;
        }

        Voice* target = nullptr;
        for (auto& v : voices) if (v.isActive() && v.getNote() == note) { target = &v; break; }
        if (! target) for (auto& v : voices) if (! v.isActive()) { target = &v; break; }
        if (! target)
        {   // steal: oldest released voice first, then oldest overall
            uint64_t best = 0;
            for (auto& v : voices) if (! v.isGateOn() && v.age >= best) { best = v.age; target = &v; }
            if (! target) for (auto& v : voices) if (v.age >= best) { best = v.age; target = &v; }
        }
        target->start (note, vel, sp, lastPitch, true, ++seedCounter);
        lastPitch = (float) note;
    }

    void noteOff (int note)
    {
        if (sp.voiceMode != 0)
        {
            heldNotes.removeFirstMatchingValue (note);
            auto& v = voices[0];
            if (v.getNote() != note) return;
            if (heldNotes.isEmpty())
            {
                if (sustainPedal) v.sustained = true; else v.release();
            }
            else
            {   // fall back to previous held note, legato glide
                const int prev = heldNotes.getLast();
                v.setTarget (prev);
                lastPitch = (float) prev;
            }
            return;
        }
        for (auto& v : voices)
            if (v.isGateOn() && v.getNote() == note)
            {
                if (sustainPedal) v.sustained = true; else v.release();
            }
    }

    struct OscPtrs { std::atomic<float> *wave, *oct, *semi, *fine, *shape, *level, *unison, *detune; } op[2];
    std::atomic<float> *sub, *noise, *ring, *filtMode, *cutoff, *reso, *filtDrive, *filtEnvAmt, *keyTrack, *filtVel,
        *fA, *fD, *fS, *fR, *aA, *aD, *aS, *aR, *ampVel, *lfo1Wave, *lfo1Rate, *lfo1Delay, *lfo1Pitch, *lfo1Cutoff,
        *lfo2Wave, *lfo2Rate, *lfo2Shape, *lfo2Pan, *lfo2Amp, *voiceMode, *glide, *drift, *warble, *spread, *bendRange;

    juce::AudioProcessorValueTreeState& apvts;
    double sampleRate = 44100.0;
    std::array<Voice, numVoices> voices;
    SynthParams sp {};
    GlobalLfo lfo1, lfo2, wowLfo, flutLfo;
    std::vector<float> lfo1Buf, lfo2Buf, wowBuf, scratch;
    juce::Array<int> heldNotes;
    float bend = 0, modWheel = 0, lastPitch = -1.0f;
    bool sustainPedal = false;
    uint32_t seedCounter = 1;
};
