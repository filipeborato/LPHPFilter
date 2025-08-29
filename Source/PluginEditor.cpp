/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
// RotarySliderLookAndFeel implementation
RotarySliderLookAndFeel::RotarySliderLookAndFeel()
{
    // Carrega o SVG do arquivo local
    auto svgFile = juce::File (juce::File::getSpecialLocation (juce::File::currentExecutableFile)
                                    .getParentDirectory()
                                    .getChildFile ("Resources")
                                    .getChildFile ("noun-knob.svg"));
    
    // Fallback: tenta carregar do BinaryData se arquivo não existir
    if (svgFile.existsAsFile())
    {
        auto svgText = svgFile.loadFileAsString();
        if (auto xml = juce::XmlDocument::parse (svgText))
        {
            knobFace = juce::Drawable::createFromSVG (*xml);
        }
    }
    else
    {
        // Fallback para o BinaryData existente
        if (auto xml = juce::XmlDocument::parse (juce::String::fromUTF8 (
                (const char*) BinaryData::nounknob_svg,
                BinaryData::nounknob_svgSize)))
        {
            // Remove nós de texto
            std::function<void(juce::XmlElement&)> removeTextNodes = [&](juce::XmlElement& element)
            {
                for (int i = element.getNumChildElements(); --i >= 0; )
                {
                    if (auto* child = element.getChildElement(i))
                    {
                        if (child->hasTagName ("text"))
                        {
                            element.removeChildElement (child, true);
                        }
                        else
                        {
                            removeTextNodes (*child);
                        }
                    }
                }
            };
            
            removeTextNodes (*xml);
            knobFace = juce::Drawable::createFromSVG (*xml);
        }
    }
    
    // Ajusta cor para melhor contraste
    if (knobFace != nullptr)
        knobFace->replaceColour (juce::Colours::black, juce::Colour::fromRGB (80, 80, 80));
}

void RotarySliderLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                                float sliderPosProportional, float rotaryStartAngle,
                                                float rotaryEndAngle, juce::Slider& slider)
{
    auto area = juce::Rectangle<float> ((float) x, (float) y, (float) width, (float) height).reduced (4.0f);
    const auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

    if (knobFace != nullptr)
    {
        // Rotaciona TODO o SVG do knob (incluindo a bolinha indicadora)
        auto centre = area.getCentre();
        auto bounds = knobFace->getDrawableBounds().toFloat();
        
        // Calcula escala para caber na área
        const float scale = std::min (area.getWidth() / bounds.getWidth(),
                                      area.getHeight() / bounds.getHeight()) * 0.8f;
        
        // Transform completa: centraliza, escala e rotaciona todo o SVG
        auto transform = juce::AffineTransform::translation (-bounds.getCentreX(), -bounds.getCentreY())
                                              .scaled (scale)
                                              .rotated (angle)
                                              .translated (centre.x, centre.y);
        
        knobFace->draw (g, 1.0f, transform);
    }
    else
    {
        // Fallback: círculo + ponteiro
        const auto radius = std::min (area.getWidth(), area.getHeight()) * 0.5f;
        auto centre = area.getCentre();
        auto circle = juce::Rectangle<float> (centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f);

        g.setColour (juce::Colours::darkgrey);
        g.fillEllipse (circle);
        g.setColour (juce::Colours::white);
        g.drawEllipse (circle, 2.0f);

        // Desenha uma bolinha indicadora (similar ao SVG)
        auto dotRadius = radius * 0.08f;
        auto dotDistance = radius * 0.7f;
        auto dotX = centre.x + std::cos (angle - juce::MathConstants<float>::halfPi) * dotDistance;
        auto dotY = centre.y + std::sin (angle - juce::MathConstants<float>::halfPi) * dotDistance;
        
        g.setColour (juce::Colours::orange);
        g.fillEllipse (dotX - dotRadius, dotY - dotRadius, dotRadius * 2.0f, dotRadius * 2.0f);
    }
}

//==============================================================================

LPHPFilterAudioProcessorEditor::LPHPFilterAudioProcessorEditor (LPHPFilterAudioProcessor& p,
                                                                juce::AudioProcessorValueTreeState& vts)
    : juce::AudioProcessorEditor (&p),
      audioProcessor (p),
      apvts (vts)
{
    constexpr int WIDTH = 360;
    constexpr int HEIGHT = 260;

    // Cutoff
    cutoffFrequencySlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    cutoffFrequencySlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 20);
    cutoffFrequencySlider.setLookAndFeel (&rotaryLook);
    addAndMakeVisible (cutoffFrequencySlider);
    jassert (apvts.getParameter ("cutoff_frequency") != nullptr);
    cutoffFrequencyAttachment = std::make_unique<SliderAttachment> (apvts, "cutoff_frequency", cutoffFrequencySlider);
    cutoffFrequencyLabel.setText ("Cutoff", juce::dontSendNotification);
    cutoffFrequencyLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (cutoffFrequencyLabel);

    // Gain
    gainSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    gainSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 20);
    gainSlider.setLookAndFeel (&rotaryLook);
    addAndMakeVisible (gainSlider);
    jassert (apvts.getParameter ("gain") != nullptr);
    gainAttachment = std::make_unique<SliderAttachment> (apvts, "gain", gainSlider);
    gainLabel.setText ("Gain", juce::dontSendNotification);
    gainLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (gainLabel);

    // Highpass
    addAndMakeVisible (highpassButton);
    jassert (apvts.getParameter ("highpass") != nullptr);
    highpassAttachment = std::make_unique<ButtonAttachment> (apvts, "highpass", highpassButton);
    highpassButtonLabel.setText ("Highpass", juce::dontSendNotification);
    highpassButtonLabel.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (highpassButtonLabel);

    setSize (WIDTH, HEIGHT);
}

LPHPFilterAudioProcessorEditor::~LPHPFilterAudioProcessorEditor()
{
    cutoffFrequencySlider.setLookAndFeel (nullptr);
    gainSlider.setLookAndFeel (nullptr);
}

//==============================================================================

void LPHPFilterAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Fundo roxo escuro
    g.fillAll (juce::Colour::fromRGB (25, 10, 40));
    g.setColour (juce::Colours::whitesmoke);
    g.setFont (15.0f);
}

void LPHPFilterAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced (16);

    auto knobsRow = bounds.removeFromTop (180);
    auto left = knobsRow.removeFromLeft (knobsRow.getWidth() / 2).reduced (8);
    auto right = knobsRow.reduced (8);

    cutoffFrequencyLabel.setBounds (left.removeFromTop (20));
    cutoffFrequencySlider.setBounds (left.withSizeKeepingCentre (150, 150));

    gainLabel.setBounds (right.removeFromTop (20));
    gainSlider.setBounds (right.withSizeKeepingCentre (150, 150));

    auto row = bounds.removeFromTop (30);
    highpassButton.setBounds (row.removeFromLeft (30));
    row.removeFromLeft (10);
    highpassButtonLabel.setBounds (row);
}

