#pragma once

#include <JuceHeader.h>

//======================================================GLOBAL_PROCESSES======================================================

enum ChainPositions
{
    HighPass,
    LowShelf,
    HighShelf,
    LowPass
};

enum LowFreq
{
    Freq_32 = 32,
    Freq_64 = 64,
    Freq_128 = 128,
    Freq_256 = 256
};

enum HighFreq
{
    Freq_5800 = 5800,
    Freq_8192 = 8192,
    Freq_11600 = 11600,
    Freq_16400 = 16400
};

struct ChainSettings
{
    float inputGainInDecibels { 0 }, outputGainInDecibels { 0 };
    float highPassFreq { 0 }, lowPassFreq { 0 };
    LowFreq lowShelfFreq { LowFreq::Freq_32 };
    HighFreq highShelfFreq { HighFreq::Freq_5800 };
    float lowShelfGainInDecibels { 0 }, highShelfGainInDecibels { 0 };
};

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts);

using Filter = juce::dsp::IIR::Filter<float>;
using CutFilter = juce::dsp::ProcessorChain<Filter>;
using MonoChain = juce::dsp::ProcessorChain<CutFilter, Filter, Filter, CutFilter>;
using Coefficients = Filter::CoefficientsPtr;

void updateCoefficients(Coefficients& old, const Coefficients& replacements);
Coefficients makeShelfFilter(const ChainSettings& chainSettings, double sampleRate);

template<int Index, typename ChainType, typename CoefficientType>
void update(ChainType& chain, const CoefficientType& coefficients);

template<typename ChainType, typename CoefficientType>
void updateCutFilter(ChainType& chain, const CoefficientType& cutCoefficients);

inline auto makeHighPassFilter(const ChainSettings& chainSettings, double sampleRate);
inline auto makeLowPassfilter(const ChainSettings& chainSettings, double sampleRate);

//======================================================MAEQ_AUDIO_PROCESSOR======================================================

class MaeqAudioProcessor final : public juce::AudioProcessor
{
public:
    MaeqAudioProcessor();
    ~MaeqAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState apvts {*this, nullptr, "Parameters", createParameterLayout()};
private:
    MonoChain leftChain, rightChain;

    void updateLowShelfFilter(const ChainSettings& chainSettings);
    void updateHighShelfFilter(const ChainSettings& chainSettings);
    void updateHighPassFilter(const ChainSettings& chainSettings);
    void updateLowPassFilter(const ChainSettings& chainSettings);
    void updateFilters();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MaeqAudioProcessor)
};