#include "Filter.h"

FilterCoefficients makeHighPassFilter(const ChainSettings& chainSettings, double sampleRate)
{
    return juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(chainSettings.highPassFreq, sampleRate, 6);
}

FilterCoefficients makeLowPassFilter(const ChainSettings& chainSettings, double sampleRate)
{
    return juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(chainSettings.lowPassFreq, sampleRate, 6);
}