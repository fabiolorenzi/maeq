cmake_minimum_required(VERSION 3.22)

project(Maeq VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 23)

set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs)
include(cmake/cpm.cmake)

CPMAddPackage(
    NAME JUCE
    GITHUB_REPOSITORY juce-framework/JUCE
    GIT_TAG 7.0.5
    VERSION 7.0.5
    SOURCE_DIR ${LIB_DIR}/juce
)

if (MSVC)
    add_compile_options(/Wall)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

juce_add_plugin(${PROJECT_NAME}
    COMPANY_NAME "F. Lorenzi"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    PLUGIN_MANUFACTURER_CODE FLSD
    PLUGIN_CODE AUPL
    FORMATS VST3 Standalone
    PRODUCT_NAME "Maeq"
)

juce_generate_juce_header(${PROJECT_NAME})

target_sources(${PROJECT_NAME}
    PRIVATE
        src/ChainSettings.cpp
        src/Equalizer.cpp
        src/Filter.cpp
        src/PluginEditor.cpp
        src/PluginProcessor.cpp
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        src/ChainSettings.h
        src/Equalizer.h
        src/EQValues.h
        src/Filter.h
        src/PluginEditor.h
        src/PluginProcessor.h
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

target_compile_definitions(${PROJECT_NAME}
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

if(MSVC)
target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        _SILENCE_CXX23_ALIGNED_STORAGE_DEPRECATION_WARNING
)
endif()