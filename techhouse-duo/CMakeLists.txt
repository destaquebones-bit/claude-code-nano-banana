cmake_minimum_required(VERSION 3.22)

project(TechHouseDuo VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# On a developer machine the built plugin should land straight in the system
# plugin folder. On CI there is no DAW to install into and the copy just adds a
# failure mode, so the workflow turns this off.
option(PLUGIN_COPY_AFTER_BUILD "Copy the built plugin into the system plugin folder" ON)

# JUCE is fetched rather than vendored, so this repo stays free of a
# multi-hundred-MB framework checkout. Pinned to the same tag the
# mastering-chain-vst project uses.
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        7.0.12
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
)
FetchContent_MakeAvailable(JUCE)

# One target on purpose: the Kick and Bass instances share a process-wide
# analysis bus (Source/dsp/LinkBus.h), and two separate plugin binaries would
# each get their own copy of that state instead of seeing each other.
juce_add_plugin(TechHouseDuo
    COMPANY_NAME                "DestaqueBones"
    PLUGIN_MANUFACTURER_CODE    Dbtb
    PLUGIN_CODE                 Thdu
    FORMATS                     VST3 Standalone
    PRODUCT_NAME                "TechHouse Duo"
    IS_SYNTH                    FALSE
    NEEDS_MIDI_INPUT            FALSE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    VST3_CATEGORIES             Fx Dynamics EQ
    COPY_PLUGIN_AFTER_BUILD     ${PLUGIN_COPY_AFTER_BUILD}
)

juce_generate_juce_header(TechHouseDuo)

target_sources(TechHouseDuo PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
)

target_compile_definitions(TechHouseDuo PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(TechHouseDuo
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
