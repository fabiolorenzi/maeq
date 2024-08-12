#pragma once

#include <JuceHeader.h>
#include "EQValues.h"

class ChainSettings
{
    public:
        ChainSettings();
        ~ChainSettings();

        float inputGain;
        float clipperThr;
        bool oversample;
        float outputGain;
        float highPassFreq;
        float lowPassFreq;
        float ghostPeakFreq;
        float ghostPeakGain;
        LowFreq lowShelfFreq;
        HighFreq highShelfFreq;
        ShelvesGain lowShelfGain;
        ShelvesGain highShelfGain;

        ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts);
};