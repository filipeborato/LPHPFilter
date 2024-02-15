/*
  ==============================================================================

    LowpassHighpassFilter.cpp
    Created: 14 Feb 2024 3:06:54pm
    Author:  Filipe Borato

  ==============================================================================
*/

#include "LowpassHighpassFilter.h"

void LowpassHighpassFilter::setHighpass(bool highpass)
{
	this->highpass = highpass;
}

void LowpassHighpassFilter::setCutoffFrequency(float cutoffFrequency)
{
	this->cutoffFrequency = cutoffFrequency;
}

void LowpassHighpassFilter::setSamplingRate(float samplingRate)
{
	this->samplingRate = samplingRate;
}

void LowpassHighpassFilter::processBlock(juce::AudioBuffer<float>& buffer,
											juce::MidiBuffer& midiMessages)
{
	// Process audio buffer
}