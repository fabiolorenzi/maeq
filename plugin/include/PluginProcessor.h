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

enum ShelvesGain
{
    Gain_Neg_10_0,
    Gain_Neg_9_5,
    Gain_Neg_9_0,
    Gain_Neg_8_5,
    Gain_Neg_8_0,
    Gain_Neg_7_5,
    Gain_Neg_7_0,
    Gain_Neg_6_5,
    Gain_Neg_6_0,
    Gain_Neg_5_5,
    Gain_Neg_5_0,
    Gain_Neg_4_5,
    Gain_Neg_4_0,
    Gain_Neg_3_5,
    Gain_Neg_3_0,
    Gain_Neg_2_5,
    Gain_Neg_2_0,
    Gain_Neg_1_5,
    Gain_Neg_1_0,
    Gain_Neg_0_5,
    Gain_0_0,
    Gain_0_5,
    Gain_1_0,
    Gain_1_5,
    Gain_2_0,
    Gain_2_5,
    Gain_3_0,
    Gain_3_5,
    Gain_4_0,
    Gain_4_5,
    Gain_5_0,
    Gain_5_5,
    Gain_6_0,
    Gain_6_5,
    Gain_7_0,
    Gain_7_5,
    Gain_8_0,
    Gain_8_5,
    Gain_9_0,
    Gain_9_5,
    Gain_10_0
};

enum LowFreq
{
    Freq_32,
    Freq_64,
    Freq_128,
    Freq_256,
    Freq_512
};

enum HighFreq
{
    Freq_5800,
    Freq_8192,
    Freq_11600,
    Freq_16400,
    Freq_22100
};

struct ChainSettings
{
    float inputGain { 0 }, outputGain { 0 };
    float highPassFreq { 0 }, lowPassFreq { 0 };
    LowFreq lowShelfFreq { LowFreq::Freq_32 };
    HighFreq highShelfFreq { HighFreq::Freq_5800 };
    ShelvesGain lowShelfGain { ShelvesGain::Gain_0_0 }, highShelfGain { ShelvesGain::Gain_0_0 };
};

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts);

using Filter = juce::dsp::IIR::Filter<float>;
using CutFilter = juce::dsp::ProcessorChain<Filter>;
using MonoChain = juce::dsp::ProcessorChain<CutFilter, Filter, Filter, CutFilter>;
using Coefficients = Filter::CoefficientsPtr;

void updateCoefficients(Coefficients& old, const Coefficients& replacements);
Coefficients makeLowShelfFilter(const ChainSettings& chainSettings, double sampleRate);
Coefficients makeHighShelfFilter(const ChainSettings& chainSettings, double sampleRate, bool isLeft);

template<int Index, typename ChainType, typename CoefficientType>
void update(ChainType& chain, const CoefficientType& coefficients);

template<typename ChainType, typename CoefficientType>
void updateCutFilter(ChainType& chain, const CoefficientType& cutCoefficients);

inline auto makeHighPassFilter(const ChainSettings& chainSettings, double sampleRate);
inline auto makeLowPassFilter(const ChainSettings& chainSettings, double sampleRate);

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