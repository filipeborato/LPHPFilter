/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
void RotarySliderLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                                float sliderPosProportional, float rotaryStartAngle,
                                                float rotaryEndAngle, juce::Slider& slider)
{
    auto radius = juce::jmin (width / 2, height / 2) - 4.0f;
    auto centreX = x + width * 0.5f;
    auto centreY = y + height * 0.5f;
    auto rx = centreX - radius;
    auto ry = centreY - radius;
    auto rw = radius * 2.0f;
    auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

    g.setColour (juce::Colours::darkgrey);
    g.fillEllipse (rx, ry, rw, rw);

    g.setColour (juce::Colours::black);
    g.drawEllipse (rx, ry, rw, rw, 1.0f);

    juce::Path p;
    auto pointerLength = radius * 0.6f;
    auto pointerThickness = 2.0f;
    p.addRectangle (-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);

    g.setColour (juce::Colours::orange);
    g.fillPath (p, juce::AffineTransform::rotation (angle).translated (centreX, centreY));
}

//==============================================================================
LPHPFilterAudioProcessorEditor::LPHPFilterAudioProcessorEditor (LPHPFilterAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    constexpr auto HEIGHT = 400;
    constexpr auto WIDTH = 200;

    addAndMakeVisible (cutoffFrequencySlider);
    cutoffFrequencySlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    cutoffFrequencySlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 20);
    cutoffFrequencySlider.setLookAndFeel (&rotaryLook);
    cutoffFrequencyAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (vts, "cutoff_frequency", cutoffFrequencySlider));

    addAndMakeVisible (cutoffFrequencyLabel);
    cutoffFrequencyLabel.setText ("Cutoff Frequency", juce::dontSendNotification);

    addAndMakeVisible (highpassButton);
    highpassAttachment.reset (new juce::AudioProcessorValueTreeState::ButtonAttachment (vts, "highpass", highpassButton));

    addAndMakeVisible (highpassButtonLabel);
    highpassButtonLabel.setText ("Highpass", juce::dontSendNotification);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize(WIDTH, HEIGHT);
}

LPHPFilterAudioProcessorEditor::~LPHPFilterAudioProcessorEditor()
{
    cutoffFrequencySlider.setLookAndFeel (nullptr);
}

//==============================================================================
void LPHPFilterAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);

    g.setColour (juce::Colours::whitesmoke);
    g.setFont (15.0f);
}

void LPHPFilterAudioProcessorEditor::resized()
{
    cutoffFrequencySlider.setBounds ({ 25, 40, 150, 150 });
    cutoffFrequencyLabel.setBounds ({ cutoffFrequencySlider.getX(), cutoffFrequencySlider.getY() - 20, 150, 20 });
    highpassButton.setBounds ({ cutoffFrequencySlider.getX(), cutoffFrequencySlider.getBottom() + 15, 30, 30 });
    highpassButtonLabel.setBounds ({ highpassButton.getRight() + 10, highpassButton.getY(), cutoffFrequencySlider.getWidth() - highpassButton.getWidth(), highpassButton.getHeight() });
}
