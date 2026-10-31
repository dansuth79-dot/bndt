cmake_minimum_required(VERSION 3.22)
project(BNDT VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BNDT_HEADLESS_TEST "Build the offline render test (Linux CI only)" OFF)

# ---- JUCE: use a local copy if there is one, otherwise download it ----
if(DEFINED JUCE_DIR AND EXISTS "${JUCE_DIR}/CMakeLists.txt")
    add_subdirectory("${JUCE_DIR}" JUCE)
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/JUCE/CMakeLists.txt")
    add_subdirectory(JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.4
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(JUCE)
endif()

# ---- Embedded art + fonts (drop FancyFootwork.ttf into assets/ to use it) ----
file(GLOB BNDT_ASSETS
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/*.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/*.jpg"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/*.ttf"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/*.otf")
list(REMOVE_DUPLICATES BNDT_ASSETS)
juce_add_binary_data(BndtAssets SOURCES ${BNDT_ASSETS})

set(BNDT_FORMATS VST3 Standalone)
if(APPLE)
    list(APPEND BNDT_FORMATS AU)
endif()

juce_add_plugin(bndt
    COMPANY_NAME "Switchkicker"
    PRODUCT_NAME "bndt"
    BUNDLE_ID "com.switchkicker.bndt"
    PLUGIN_MANUFACTURER_CODE Swkk
    PLUGIN_CODE Bndt
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES Instrument Synth
    AU_MAIN_TYPE kAudioUnitType_MusicDevice
    ICON_BIG "${CMAKE_CURRENT_SOURCE_DIR}/assets/branding/icon.png"
    MICROPHONE_PERMISSION_ENABLED TRUE
    MICROPHONE_PERMISSION_TEXT "bndt listens to your audio input as the vocoder source."
    HARDENED_RUNTIME_ENABLED TRUE
    HARDENED_RUNTIME_OPTIONS com.apple.security.device.audio-input
    FORMATS ${BNDT_FORMATS})

set(BNDT_SOURCES
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/SynthEngine.cpp
    Source/Presets.cpp
    Source/FX.cpp
    Source/Vocoder.cpp
    Source/UI/Widgets.cpp)

target_sources(bndt PRIVATE ${BNDT_SOURCES})
target_include_directories(bndt PRIVATE Source)

set(BNDT_DEFS
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0)

if(BNDT_HEADLESS_TEST)
    list(APPEND BNDT_DEFS JUCE_USE_XRANDR=0 JUCE_USE_XINERAMA=0 JUCE_USE_XCURSOR=0 JUCE_ALSA=0 JUCE_JACK=0)
endif()

target_compile_definitions(bndt PUBLIC ${BNDT_DEFS})

target_link_libraries(bndt
    PRIVATE
        BndtAssets
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

if(MSVC)
    target_compile_options(bndt PRIVATE /fp:fast)
endif()

# ---- Offline test: renders every preset to WAV and the UI to PNG ----
if(BNDT_HEADLESS_TEST)
    juce_add_console_app(bndt_test PRODUCT_NAME "bndt_test")
    target_sources(bndt_test PRIVATE tools/RenderTest.cpp ${BNDT_SOURCES})
    target_include_directories(bndt_test PRIVATE Source)
    target_compile_definitions(bndt_test PRIVATE ${BNDT_DEFS} JucePlugin_Name="bndt" JucePlugin_IsSynth=1
        JucePlugin_WantsMidiInput=1 JucePlugin_ProducesMidiOutput=0 JucePlugin_IsMidiEffect=0
        JucePlugin_VersionString="1.0.0" BNDT_TEST=1)
    target_link_libraries(bndt_test PRIVATE BndtAssets juce::juce_audio_utils juce::juce_dsp
        juce::juce_recommended_config_flags)
endif()
