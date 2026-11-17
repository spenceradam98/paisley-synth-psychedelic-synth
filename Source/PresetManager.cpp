#include "PresetManager.h"

PresetManager::PresetManager (juce::AudioProcessorValueTreeState& s) : apvts (s) { rescan(); }

juce::File PresetManager::getUserDir()
{
    return juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
        .getChildFile ("Paisley").getChildFile ("Presets");
}

void PresetManager::rescan()
{
    entries.clear();
    const auto& fp = getFactoryPresets();
    for (int i = 0; i < (int) fp.size(); ++i)
        entries.add ({ fp[(size_t) i].name, fp[(size_t) i].category, i, {} });

    auto dir = getUserDir();
    if (dir.isDirectory())
    {
        auto files = dir.findChildFiles (juce::File::findFiles, true, juce::String ("*") + fileExt);
        files.sort();
        for (auto& f : files)
        {
            auto cat = f.getParentDirectory() == dir ? juce::String ("User") : "User: " + f.getParentDirectory().getFileName();
            entries.add ({ f.getFileNameWithoutExtension(), cat, -1, f });
        }
    }
}

juce::StringArray PresetManager::getCategories() const
{
    juce::StringArray cats;
    for (auto& e : entries) cats.addIfNotAlreadyThere (e.category);
    return cats;
}

juce::Array<int> PresetManager::getIndicesInCategory (const juce::String& category) const
{
    juce::Array<int> r;
    for (int i = 0; i < entries.size(); ++i) if (entries[i].category == category) r.add (i);
    return r;
}

void PresetManager::resetToDefaults()
{
    for (auto* p : apvts.processor.getParameters())
        if (auto* rp = dynamic_cast<juce::RangedAudioParameter*> (p))
            rp->setValueNotifyingHost (rp->getDefaultValue());
}

void PresetManager::loadInit()
{
    resetToDefaults();
    current = -1;
    currentName = "Init";
    if (onPresetChanged) onPresetChanged();
}

void PresetManager::load (int idx)
{
    if (! juce::isPositiveAndBelow (idx, entries.size())) return;
    const auto& e = entries.getReference (idx);
    if (e.factoryIndex >= 0)
    {
        resetToDefaults();
        for (auto& [id, value] : getFactoryPresets()[(size_t) e.factoryIndex].values)
        {
            if (auto* p = apvts.getParameter (id))
                p->setValueNotifyingHost (p->convertTo0to1 (value));
            else
                jassertfalse; // typo in a factory preset parameter id
        }
    }
    else
    {
        auto xml = juce::XmlDocument::parse (e.file);
        if (xml == nullptr) return;
        auto state = juce::ValueTree::fromXml (*xml);
        resetToDefaults();
        for (int i = 0; i < state.getNumChildren(); ++i)
        {
            auto c = state.getChild (i);
            if (auto* p = apvts.getParameter (c.getProperty ("id").toString()))
                p->setValueNotifyingHost (p->convertTo0to1 ((float) c.getProperty ("value")));
        }
    }
    current = idx;
    currentName = e.name;
    if (onPresetChanged) onPresetChanged();
}

bool PresetManager::saveUser (const juce::String& name, const juce::String& category)
{
    auto safe = juce::File::createLegalFileName (name.trim());
    if (safe.isEmpty()) return false;
    auto dir = getUserDir();
    if (category.isNotEmpty() && category != "User")
        dir = dir.getChildFile (juce::File::createLegalFileName (category));
    dir.createDirectory();
    auto state = apvts.copyState();
    state.setProperty ("presetName", name, nullptr);
    auto xml = state.createXml();
    auto file = dir.getChildFile (safe + fileExt);
    if (! xml->writeTo (file)) return false;
    rescan();
    for (int i = 0; i < entries.size(); ++i)
        if (entries[i].file == file) { current = i; currentName = entries[i].name; }
    if (onPresetChanged) onPresetChanged();
    return true;
}

void PresetManager::step (int delta)
{
    if (entries.isEmpty()) return;
    int n = current < 0 ? (delta > 0 ? 0 : entries.size() - 1)
                        : (current + delta + entries.size()) % entries.size();
    load (n);
}

juce::String PresetManager::getCurrentName() const { return currentName; }

void PresetManager::setCurrentByName (const juce::String& name)
{
    currentName = name.isEmpty() ? juce::String ("Init") : name;
    current = -1;
    for (int i = 0; i < entries.size(); ++i)
        if (entries[i].name == currentName) { current = i; break; }
}
