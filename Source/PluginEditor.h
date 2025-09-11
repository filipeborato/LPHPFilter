/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

// LookAndFeel para os knobs
class RotarySliderLookAndFeel : public juce::LookAndFeel_V4
{
public:
    RotarySliderLookAndFeel();
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
        float sliderPosProportional, float rotaryStartAngle,
        float rotaryEndAngle, juce::Slider& slider) override;
private:
    std::unique_ptr<juce::Drawable> knobFace; // store SVG
};

//==============================================================================

class LPHPFilterAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    LPHPFilterAudioProcessorEditor(LPHPFilterAudioProcessor& p,
        juce::AudioProcessorValueTreeState& vts);
    ~LPHPFilterAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    LPHPFilterAudioProcessor& audioProcessor;
    juce::AudioProcessorValueTreeState& apvts; // referência ao APVTS

    RotarySliderLookAndFeel rotaryLook;

    // Knob de cutoff
    juce::Slider cutoffFrequencySlider;
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> cutoffFrequencyAttachment;
    juce::Label cutoffFrequencyLabel;

    // NOVO: Knob de gain
    juce::Slider gainSlider;
    std::unique_ptr<SliderAttachment> gainAttachment;
    juce::Label gainLabel;

    // Toggle Highpass
    juce::ToggleButton highpassButton;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    std::unique_ptr<ButtonAttachment> highpassAttachment;
    juce::Label highpassButtonLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LPHPFilterAudioProcessorEditor)
};
