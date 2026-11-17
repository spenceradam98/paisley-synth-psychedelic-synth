cmake_minimum_required(VERSION 3.22)

# macOS: universal binary (Intel + Apple Silicon), runs on 10.13+ incl. Big Sur on 2015 Intel Macs.
# No CPU-specific flags anywhere (plain SSE2 baseline) so old Intel/AMD machines are fine.
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum macOS version")
# Windows: static MSVC runtime so friends don't need the Visual C++ redistributable.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "macOS architectures")
project(Paisley VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# JUCE: use a local checkout if present, otherwise fetch 8.0.9
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/JUCE/CMakeLists.txt)
    add_subdirectory(JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.9 GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(JUCE)
endif()

juce_add_plugin(Paisley
    COMPANY_NAME "Paisley Audio"
    BUNDLE_ID com.paisleyaudio.paisley
    PLUGIN_MANUFACTURER_CODE Psly
    PLUGIN_CODE Psy1
    FORMATS VST3 AU Standalone
    AU_MAIN_TYPE kAudioUnitType_MusicDevice
    PRODUCT_NAME "Paisley"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    VST3_CATEGORIES Instrument Synth
    COPY_PLUGIN_AFTER_BUILD FALSE)

target_sources(Paisley PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/FactoryPresets.cpp
    Source/PresetManager.cpp)

target_compile_definitions(Paisley PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0)

target_link_libraries(Paisley PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)

# ---- Offline render / screenshot tool (dev only): renders every preset to WAV,
# checks for silence/NaN/clipping, and snapshots the editor in every theme.
option(PAISLEY_BUILD_TOOLS "Build the offline render/snapshot tool" OFF)
if(PAISLEY_BUILD_TOOLS)
    juce_add_console_app(PaisleyRender PRODUCT_NAME "PaisleyRender")
    target_sources(PaisleyRender PRIVATE tools/Render.cpp
        Source/PluginProcessor.cpp Source/PluginEditor.cpp Source/FactoryPresets.cpp Source/PresetManager.cpp)
    target_compile_definitions(PaisleyRender PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 PAISLEY_NO_PLUGIN_ENTRY=1)
    target_link_libraries(PaisleyRender PRIVATE juce::juce_audio_utils juce::juce_dsp juce::juce_recommended_config_flags)
endif()
