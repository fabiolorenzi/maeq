#include "ChainSettings.h"

ChainSettings::ChainSettings()
{
    inputGain = 0.f;
    clipperThr = 12.f;
    outputGain = 0.f;
    highPassFreq = 0.f;
    lowPassFreq = 0.f;
    ghostPeakFreq = 0.f;
    ghostPeakGain = 0.f;
    lowShelfFreq = LowFreq::Freq_32;
    highShelfFreq = HighFreq::Freq_5800;
    lowShelfGain = ShelvesGain::Gain_0_0;
    highShelfGain = ShelvesGain::Gain_0_0;
}

ChainSettings::~ChainSettings()
{
}

ChainSettings ChainSettings::getChainSettings(juce::AudioProcessorValueTreeState& apvts)
{
    inputGain = apvts.getRawParameterValue("Input Gain")->load();
    clipperThr = apvts.getRawParameterValue("Clipper Thr")->load();
    highPassFreq = apvts.getRawParameterValue("HighPass Freq")->load();
    lowShelfFreq = static_cast<LowFreq>(apvts.getRawParameterValue("LowShelf Freq")->load());
    lowShelfGain = static_cast<ShelvesGain>(apvts.getRawParameterValue("LowShelf Gain")->load());
    ghostPeakFreq = apvts.getRawParameterValue("GhostPeak Freq")->load();
    ghostPeakGain = apvts.getRawParameterValue("GhostPeak Gain")->load();
    highShelfGain = static_cast<ShelvesGain>(apvts.getRawParameterValue("HighShelf Gain")->load());
    highShelfFreq = static_cast<HighFreq>(apvts.getRawParameterValue("HighShelf Freq")->load());
    lowPassFreq = apvts.getRawParameterValue("LowPass Freq")->load();
    outputGain = apvts.getRawParameterValue("Output Gain")->load();

    return *this;
}