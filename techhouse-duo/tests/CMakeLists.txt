cmake_minimum_required(VERSION 3.22)
project(TechHouseDuoTests VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Reuses the JUCE checkout the main build already fetched when present, so
# running the tests doesn't trigger a second clone.
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        7.0.12
    GIT_SHALLOW    TRUE
    SOURCE_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/../build/_deps/juce-src
)
FetchContent_MakeAvailable(JUCE)

juce_add_console_app(DspTests PRODUCT_NAME "DspTests")

target_sources(DspTests PRIVATE DspTests.cpp)

target_compile_definitions(DspTests PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_STANDALONE_APPLICATION=1
    JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(DspTests PRIVATE juce::juce_dsp juce::juce_core)

# Renders the visualisers offscreen with data from the real DSP modules. Needs
# juce_gui_basics for the paint routines, but no display: JUCE's software
# renderer draws straight into a juce::Image.
juce_add_console_app(UiPreview PRODUCT_NAME "UiPreview")

target_sources(UiPreview PRIVATE UiPreview.cpp)

target_compile_definitions(UiPreview PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_STANDALONE_APPLICATION=1
    JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(UiPreview PRIVATE juce::juce_dsp juce::juce_gui_basics)

# One-off measurement backing the switch from direct-form filters to the book's
# TPT structure. Kept in the tree so the claim can be re-checked rather than
# taken on trust.
juce_add_console_app(ModulationExperiment PRODUCT_NAME "ModulationExperiment")
target_sources(ModulationExperiment PRIVATE ModulationExperiment.cpp)
target_compile_definitions(ModulationExperiment PUBLIC
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_STANDALONE_APPLICATION=1 JUCE_DISPLAY_SPLASH_SCREEN=0)
target_link_libraries(ModulationExperiment PRIVATE juce::juce_dsp juce::juce_core)
