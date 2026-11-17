#include "PluginProcessor.h"
#include "PluginEditor.h"

static juce::PropertiesFile::Options settingsOptions()
{
    juce::PropertiesFile::Options o;
    o.applicationName = "Paisley";
    o.filenameSuffix = ".settings";
    o.folderName = "Paisley";
    o.osxLibrarySubFolder = "Application Support";
    return o;
}

PaisleyProcessor::PaisleyProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PAISLEY", Params::createLayout()),
      presets (apvts),
      engine (apvts),
      fx (apvts)
{
    master = apvts.getRawParameterValue ("master");
    settings = std::make_unique<juce::PropertiesFile> (settingsOptions());
    themeIndex = settings->getIntValue ("theme", 0);
}

void PaisleyProcessor::setThemeIndex (int i)
{
    themeIndex = i;
    settings->setValue ("theme", i);
    settings->saveIfNeeded();
}

bool PaisleyProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void PaisleyProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    engine.prepare (sampleRate, samplesPerBlock);
    fx.prepare (sampleRate, samplesPerBlock);
    masterGain.reset (sampleRate, 0.05);
    masterGain.setCurrentAndTargetValue (master->load());
}

void PaisleyProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();
    keyboardState.processNextMidiBuffer (midi, 0, n, true);

    if (auto* ph = getPlayHead())
        if (auto pos = ph->getPosition())
            if (auto bpm = pos->getBpm())
                fx.setBpm (*bpm);

    engine.process (buffer, midi);
    fx.process (buffer);

    masterGain.setTargetValue (master->load());
    for (int i = 0; i < n; ++i)
    {
        const float gain = masterGain.getNextValue();
        for (int c = 0; c < buffer.getNumChannels(); ++c)
        {
            auto* d = buffer.getWritePointer (c);
            float x = d[i] * gain;
            if (! std::isfinite (x)) x = 0.0f;
            // gentle safety clipper: transparent below ~0.8, never exceeds 1.0
            d[i] = std::abs (x) < 0.8f ? x : std::copysign (0.8f + 0.2f * std::tanh ((std::abs (x) - 0.8f) / 0.2f), x);
        }
    }
}

int PaisleyProcessor::getNumPrograms() { return juce::jmax (1, (int) getFactoryPresets().size()); }
int PaisleyProcessor::getCurrentProgram() { return juce::jmax (0, presets.getCurrentIndex()); }
void PaisleyProcessor::setCurrentProgram (int i) { if (i < (int) getFactoryPresets().size()) presets.load (i); }
const juce::String PaisleyProcessor::getProgramName (int i)
{
    const auto& fp = getFactoryPresets();
    return juce::isPositiveAndBelow (i, (int) fp.size()) ? juce::String (fp[(size_t) i].name) : juce::String ("Init");
}

void PaisleyProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    auto state = apvts.copyState();
    state.setProperty ("presetName", presets.getCurrentName(), nullptr);
    state.setProperty ("theme", themeIndex, nullptr);
    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, dest);
}

void PaisleyProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
    {
        auto state = juce::ValueTree::fromXml (*xml);
        if (state.hasType (apvts.state.getType()))
        {
            apvts.replaceState (state);
            presets.setCurrentByName (state.getProperty ("presetName").toString());
            if (state.hasProperty ("theme")) themeIndex = (int) state.getProperty ("theme");
        }
    }
}

juce::AudioProcessorEditor* PaisleyProcessor::createEditor() { return new PaisleyEditor (*this); }

#if ! PAISLEY_NO_PLUGIN_ENTRY
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new PaisleyProcessor(); }
#endif
