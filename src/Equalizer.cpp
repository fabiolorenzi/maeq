#include "Equalizer.h"

void updateCoefficients(Coefficients& old, const Coefficients& replacements)
{
    *old = *replacements;
}

Coefficients makeLowShelfFilter(const ChainSettings& chainSettings, double sampleRate)
{
    float freq { 32.f };
    int index = chainSettings.lowShelfFreq;
    freq *= index == 0 ? 1 : index == 1 ? 2 : 4;
    float gain = -10.f + (0.5f * chainSettings.lowShelfGain);
    return juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, freq, 0.20, juce::Decibels::decibelsToGain(gain));
}

Coefficients makeGhostPeakFilter(const ChainSettings& chainSettings, double sampleRate)
{
    float freq { 32.f };
    int index = chainSettings.lowShelfFreq;
    freq *= index == 0 ? 1 : index == 1 ? 2 : 4;
    float gain = chainSettings.lowShelfGain > 20 ? -10.f + (0.5f * chainSettings.lowShelfGain) : 0.2;
    std::cout << chainSettings.lowShelfGain << std::endl;
    return juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        sampleRate,
        (250.f + freq + gain),
        1.7,
        juce::Decibels::decibelsToGain(gain * -0.5)
    );
}

Coefficients makeHighShelfFilter(const ChainSettings& chainSettings, double sampleRate, bool isLeft)
{
    float freq;
    float gain = -10.f + (0.5f * chainSettings.highShelfGain);

    switch (chainSettings.highShelfFreq) {
        case 0: freq = 5800.f; break;
        case 1: freq = 8192.f; break;
        case 2: freq = 11600.f; break;
        case 3: freq = 16400.f; break;
        case 4: freq = 21000.f; break;
    }

    if (isLeft) {
        return juce::dsp::IIR::Coefficients<float>::makePeakFilter(
            sampleRate,
            (freq - chainSettings.inputGain * 2) - 87,
            0.1,
            juce::Decibels::decibelsToGain((gain + ((chainSettings.inputGain == 0.f ? 0.1f : chainSettings.inputGain) / 36.f) + 0.1f))
        );
    }
    return juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, freq, 0.1, juce::Decibels::decibelsToGain(gain));
}