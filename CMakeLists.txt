cmake_minimum_required(VERSION 3.15)
project(LPHPFilter VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)
FetchContent_Declare(juce
  GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
  GIT_TAG 7.0.11
)
FetchContent_MakeAvailable(juce)

juce_add_plugin(LPHPFilter
  COMPANY_NAME "LPHP"
  IS_SYNTH FALSE
  NEEDS_MIDI_INPUT FALSE
  NEEDS_MIDI_OUTPUT FALSE
  IS_MIDI_EFFECT FALSE
  EDITOR_WANTS_KEYBOARD_FOCUS FALSE
  COPY_PLUGIN_AFTER_BUILD TRUE
  PLUGIN_MANUFACTURER_CODE Juce
  PLUGIN_CODE Lpf1
  FORMATS VST3
  PRODUCT_NAME "LPHPFilter"
)

juce_generate_juce_header(LPHPFilter)

target_sources(LPHPFilter PRIVATE
  Source/PluginProcessor.cpp
  Source/PluginEditor.cpp
  Source/LowpassHighpassFilter.cpp
)

target_compile_definitions(LPHPFilter PUBLIC
  JUCE_WEB_BROWSER=0
  JUCE_USE_CURL=0
  JUCE_VST3_CAN_REPLACE_VST2=0
)add_subdirectory(chowdsp_utils)


target_link_libraries(LPHPFilter PRIVATE
  juce::juce_audio_utils
  juce::juce_dsp
  chowdsp::chowdsp_gui
  chowdsp::chowdsp_plugin_base
  juce::juce_recommended_config_flags
  juce::juce_recommended_lto_flags
  juce::juce_recommended_warning_flags
)
