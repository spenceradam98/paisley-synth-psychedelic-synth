#pragma once
#include <vector>
#include <utility>

// A factory preset is a sparse list of parameter overrides on top of the
// init patch (every parameter's default). Values are in real units:
// Hz, seconds, choice index, 0/1 for switches.
struct FactoryPreset
{
    const char* name;
    const char* category;
    std::vector<std::pair<const char*, float>> values;
};

const std::vector<FactoryPreset>& getFactoryPresets();
