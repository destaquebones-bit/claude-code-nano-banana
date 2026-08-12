cmake_minimum_required(VERSION 3.22)

project(MasteringChain VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# See techhouse-duo/CMakeLists.txt: CI has no DAW to install into, so it turns
# the post-build copy off.
option(PLUGIN_COPY_AFTER_BUILD "Copy the built plugin into the system plugin folder" ON)

# JUCE is fetched from source rather than vendored -- keeps this repo free
# of a multi-hundred-MB framework checkout. First configure will clone it
# (shallow, pinned tag below); subsequent configures reuse the cached
# checkout under the build directory.
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        7.0.12
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(MasteringChain
    COMPANY_NAME                "DestaqueBones"
    PLUGIN_MANUFACTURER_CODE    Dbtb
    PLUGIN_CODE                 Mstc
    FORMATS                     VST3 Standalone
    PRODUCT_NAME                "Mastering Chain"
    IS_SYNTH                    FALSE
    NEEDS_MIDI_INPUT            FALSE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    VST3_CATEGORIES             Fx Mastering
    COPY_PLUGIN_AFTER_BUILD     ${PLUGIN_COPY_AFTER_BUILD}
)

juce_generate_juce_header(MasteringChain)

target_sources(MasteringChain PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
)

target_compile_definitions(MasteringChain PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(MasteringChain
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
