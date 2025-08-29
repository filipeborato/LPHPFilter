/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "BinaryData.h"

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
    // Tamanho inicial e limites de redimensionamento
    constexpr int INITIAL_WIDTH = 360;
    constexpr int INITIAL_HEIGHT = 260;
    constexpr int MIN_WIDTH = 300;
    constexpr int MIN_HEIGHT = 200;
    constexpr int MAX_WIDTH = 800;
    constexpr int MAX_HEIGHT = 600;

    // Configurações de redimensionamento
    setResizable (true, true); // Permite redimensionamento e adiciona o corner resizer
    setResizeLimits (MIN_WIDTH, MIN_HEIGHT, MAX_WIDTH, MAX_HEIGHT);
    getConstrainer()->setFixedAspectRatio (INITIAL_WIDTH / (float) INITIAL_HEIGHT); // Mantém proporção 

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

    setSize (INITIAL_WIDTH, INITIAL_HEIGHT);
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
    
    // Fonte escalável baseada no tamanho da janela
    auto fontSize = getHeight() * 0.06f; // 6% da altura da janela
    g.setFont (fontSize);
}

void LPHPFilterAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    
    // Calcula proporções baseadas no tamanho atual
    auto windowWidth = (float) bounds.getWidth();
    auto windowHeight = (float) bounds.getHeight();
    
    // Margem proporcional (4% da largura - reduzida para dar mais espaço aos knobs)
    auto margin = windowWidth * 0.04f;
    bounds = bounds.reduced (juce::roundToInt (margin));

    // Área dos knobs (75% da altura disponível - aumentada)
    auto knobAreaHeight = (float) bounds.getHeight() * 0.75f;
    auto knobsRow = bounds.removeFromTop (juce::roundToInt (knobAreaHeight));
    
    // Divide em duas colunas para os knobs
    auto leftKnob = knobsRow.removeFromLeft (knobsRow.getWidth() / 2).reduced (juce::roundToInt (margin * 0.3f));
    auto rightKnob = knobsRow.reduced (juce::roundToInt (margin * 0.3f));

    // Altura do label proporcional (10% da altura da área de knob - reduzida)
    auto labelHeight = knobAreaHeight * 0.1f;
    
    // Tamanho do knob aumentado para 95% do espaço disponível
    auto availableKnobSize = juce::jmin ((float) leftKnob.getWidth(), (float) leftKnob.getHeight() - labelHeight);
    auto knobSize = availableKnobSize * 0.95f; // Aumentado de 85% para 95%
    
    // Configura cutoff knob
    cutoffFrequencyLabel.setBounds (leftKnob.removeFromTop (juce::roundToInt (labelHeight)));
    cutoffFrequencySlider.setBounds (leftKnob.withSizeKeepingCentre (juce::roundToInt (knobSize), 
                                                                     juce::roundToInt (knobSize)));
    
    // Configura gain knob
    gainLabel.setBounds (rightKnob.removeFromTop (juce::roundToInt (labelHeight)));
    gainSlider.setBounds (rightKnob.withSizeKeepingCentre (juce::roundToInt (knobSize), 
                                                           juce::roundToInt (knobSize)));

    // Atualiza tamanho das textboxes dos sliders proporcionalmente
    auto textBoxWidth = knobSize * 0.6f; // Aumentado de 55% para 60%
    auto textBoxHeight = windowHeight * 0.08f;
    
    cutoffFrequencySlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 
                                           juce::roundToInt (textBoxWidth), 
                                           juce::roundToInt (textBoxHeight));
    gainSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 
                                 juce::roundToInt (textBoxWidth), 
                                 juce::roundToInt (textBoxHeight));

    // Área do toggle button (restante da altura)
    bounds.removeFromTop (juce::roundToInt (margin * 0.4f)); // Espaçamento reduzido
    auto toggleRow = bounds.removeFromTop (juce::roundToInt (windowHeight * 0.1f)); // Reduzido de 12% para 10%
    
    // Tamanho do toggle button proporcional
    auto toggleSize = juce::jmin (toggleRow.getHeight() * 0.8f, windowWidth * 0.08f);
    
    highpassButton.setBounds (toggleRow.removeFromLeft (juce::roundToInt (toggleSize)));
    toggleRow.removeFromLeft (juce::roundToInt (margin * 0.3f)); // Espaçamento pequeno
    highpassButtonLabel.setBounds (toggleRow);
}

