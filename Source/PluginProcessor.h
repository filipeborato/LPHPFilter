/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "LowpassHighpassFilter.h"

//==============================================================================
// Processador do plugin
class LPHPFilterAudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    LPHPFilterAudioProcessor();
    ~LPHPFilterAudioProcessor() override;

    // Ciclo de vida DSP
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    // Editor
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    // Metadados
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    // Programs (mantidos no padrão JUCE)
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    // Estado (persistência)
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Acesso ao APVTS, se algum componente externo precisar
    juce::AudioProcessorValueTreeState& getAPVTS() noexcept { return parameters; }

private:
    // Parâmetros (APVTS) e ponteiros brutos para leitura thread-safe no audio thread
    juce::AudioProcessorValueTreeState parameters;
    std::atomic<float>* cutoffFrequencyParameter = nullptr; // "cutoff_frequency"
    std::atomic<float>* highpassParameter = nullptr; // "highpass" (bool como float)
    std::atomic<float>* gainParameter = nullptr; // "gain" (0..1)

    // Seu DSP
    LowpassHighpassFilter filter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LPHPFilterAudioProcessor)
};
