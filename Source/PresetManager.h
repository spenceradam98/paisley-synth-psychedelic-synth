#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "FactoryPresets.h"

// Preset browser model. Factory presets are compiled in; user presets are
// XML files (.paisley) in  <Documents>/Paisley/Presets/<Category>/<Name>.paisley
class PresetManager
{
public:
    struct Entry { juce::String name, category; int factoryIndex = -1; juce::File file; };

    explicit PresetManager (juce::AudioProcessorValueTreeState&);

    void rescan();
    const juce::Array<Entry>& getEntries() const { return entries; }
    juce::StringArray getCategories() const;
    juce::Array<int> getIndicesInCategory (const juce::String& category) const;

    void load (int entryIndex);
    void loadInit();
    bool saveUser (const juce::String& name, const juce::String& category);
    void step (int delta); // next/previous within the whole list

    int getCurrentIndex() const { return current; }
    juce::String getCurrentName() const;
    void setCurrentByName (const juce::String& name);

    static juce::File getUserDir();
    static constexpr const char* fileExt = ".paisley";

    std::function<void()> onPresetChanged;

private:
    void resetToDefaults();
    juce::AudioProcessorValueTreeState& apvts;
    juce::Array<Entry> entries;
    int current = -1;
    juce::String currentName { "Init" };
};
