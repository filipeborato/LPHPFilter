#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
// Construtor: inicializa o APVTS
LPHPFilterAudioProcessor::LPHPFilterAudioProcessor()
#if !defined(JucePlugin_PreferredChannelConfigurations)
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    )
#else
    : AudioProcessor()
#endif
    , parameters(*this, nullptr, juce::Identifier("LowpassAndHighpassPlugin"),
        {
            std::make_unique<juce::AudioParameterFloat>(
                "cutoff_frequency", "Cutoff Frequency",
                juce::NormalisableRange<float>(20.f, 20000.f, 0.1f, 0.2f, false), 500.f
            ),
            std::make_unique<juce::AudioParameterBool>("highpass", "Highpass", false),

                // MODIFICADO: parâmetro de ganho agora vai de 0.0 a 2.0, default 1.0 (unity gain)
                std::make_unique<juce::AudioParameterFloat>(
                    "gain", "Gain",
                    juce::NormalisableRange<float>(0.0f, 2.0f, 0.001f), 1.0f
                )
        })
{
    cutoffFrequencyParameter = parameters.getRawParameterValue("cutoff_frequency");
    highpassParameter = parameters.getRawParameterValue("highpass");
    gainParameter = parameters.getRawParameterValue("gain");
}

LPHPFilterAudioProcessor::~LPHPFilterAudioProcessor() = default;

//==============================================================================
const juce::String LPHPFilterAudioProcessor::getName() const { return JucePlugin_Name; }

bool LPHPFilterAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool LPHPFilterAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool LPHPFilterAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double LPHPFilterAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int LPHPFilterAudioProcessor::getNumPrograms() { return 1; }
int LPHPFilterAudioProcessor::getCurrentProgram() { return 0; }
void LPHPFilterAudioProcessor::setCurrentProgram(int) {}
const juce::String LPHPFilterAudioProcessor::getProgramName(int) { return {}; }
void LPHPFilterAudioProcessor::changeProgramName(int, const juce::String&) {}

//==============================================================================
void LPHPFilterAudioProcessor::prepareToPlay(double sampleRate, int /*samplesPerBlock*/)
{
    filter.setSamplingRate(static_cast<float> (sampleRate));
}

void LPHPFilterAudioProcessor::releaseResources() {}

#if !defined(JucePlugin_PreferredChannelConfigurations)
bool LPHPFilterAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

void LPHPFilterAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    for (int ch = getTotalNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear(ch, 0, buffer.getNumSamples());

    const float cutoff = cutoffFrequencyParameter->load();
    const bool  highpass = highpassParameter->load() >= 0.5f;
    const float gain = gainParameter->load();

    filter.setCutoffFrequency(cutoff);
    filter.setHighpass(highpass);
    filter.processBlock(buffer, midiMessages);

    // aplica ganho global após o filtro (agora pode amplificar até 2x)
    buffer.applyGain(gain);
}

//==============================================================================
bool LPHPFilterAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* LPHPFilterAudioProcessor::createEditor()
{
    return new LPHPFilterAudioProcessorEditor(*this, parameters);
}

//==============================================================================
// Persistência do estado - CONFIRMADO: função implementada corretamente
void LPHPFilterAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // Salva o estado completo do APVTS (todos os parâmetros)
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void LPHPFilterAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // Restaura o estado completo do APVTS (todos os parâmetros)
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName(parameters.state.getType()))
    {
        juce::ValueTree vt = juce::ValueTree::fromXml(*xml);
        parameters.replaceState(vt);
    }
}

//==============================================================================
// Factory
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LPHPFilterAudioProcessor();
}
