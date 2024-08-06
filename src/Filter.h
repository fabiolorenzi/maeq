#pragma once

#include <JuceHeader.h>
#include "ChainSettings.h"

using Filter = juce::dsp::IIR::Filter<float>;
using CutFilter = juce::dsp::ProcessorChain<Filter>;
using FilterCoefficients = juce::ReferenceCountedArray<juce::dsp::IIR::Coefficients<float>>;

FilterCoefficients makeHighPassFilter(const ChainSettings& chainSettings, double sampleRate);
FilterCoefficients makeLowPassFilter(const ChainSettings& chainSettings, double sampleRate);