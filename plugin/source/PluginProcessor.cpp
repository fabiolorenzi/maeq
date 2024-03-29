#include "PluginProcessor.h"
#include "PluginEditor.h"

//======================================================GLOBAL_PROCESSES======================================================

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts)
{
    ChainSettings settings;

    settings.inputGain = apvts.getRawParameterValue("Input Gain")->load();
    settings.highPassFreq = apvts.getRawParameterValue("HighPass Freq")->load();
    settings.lowShelfFreq = static_cast<LowFreq>(apvts.getRawParameterValue("LowShelf Freq")->load());
    settings.lowShelfGain = static_cast<ShelvesGain>(apvts.getRawParameterValue("LowShelf Gain")->load());
    settings.highShelfGain = static_cast<ShelvesGain>(apvts.getRawParameterValue("HighShelf Gain")->load());
    settings.highShelfFreq = static_cast<HighFreq>(apvts.getRawParameterValue("HighShelf Freq")->load());
    settings.lowPassFreq = apvts.getRawParameterValue("LowPass Freq")->load();
    settings.outputGain = apvts.getRawParameterValue("Output Gain")->load();

    return settings;
}

void updateCoefficients(Coefficients& old, const Coefficients& replacements)
{
    *old = *replacements;
}

Coefficients makeLowShelfFilter(const ChainSettings &chainSettings, double sampleRate)
{
    float freq { 32.f };
    freq *= chainSettings.lowShelfFreq == 0 ? 1 : 2;
    float gain = -10.f + (0.5f * chainSettings.lowShelfGain);
    return juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, freq, 0.15, juce::Decibels::decibelsToGain(gain));
}

Coefficients makeHighShelfFilter(const ChainSettings &chainSettings, double sampleRate, bool isLeft)
{
    float freq;
    float gain = -10.f + (0.5f * chainSettings.highShelfGain);

    if (chainSettings.highShelfFreq == 0) {
        freq = 5800.f;
    } else if (chainSettings.highShelfFreq == 1) {
        freq = 8192.f;
    } else if (chainSettings.highShelfFreq == 2) {
        freq = 11600.f;
    } else if (chainSettings.highShelfFreq == 3) {
        freq = 16400.f;
    } else {
        freq = 22100.f;
    }

    if (isLeft) {
        return juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, (freq - (chainSettings.inputGain * 2) - 50), 0.1, juce::Decibels::decibelsToGain((gain + ((chainSettings.inputGain == 0.f ? 0.1f : chainSettings.inputGain) / 36.f) + 0.1)));
    }
    return juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, freq, 0.1, juce::Decibels::decibelsToGain(gain));
}

template<int Index, typename ChainType, typename CoefficientType>
inline void update(ChainType &chain, const CoefficientType &coefficients)
{
    updateCoefficients(chain.template get<Index>().coefficients, coefficients[Index]);
}

template<typename ChainType, typename CoefficientType>
void updateCutFilter(ChainType &chain, const CoefficientType &cutCoefficients)
{
    update<0>(chain, cutCoefficients);
}

inline auto makeHighPassFilter(const ChainSettings &chainSettings, double sampleRate)
{
    return juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(chainSettings.highPassFreq, sampleRate, 12);
}

inline auto makeLowPassFilter(const ChainSettings &chainSettings, double sampleRate)
{
    return juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(chainSettings.lowPassFreq, sampleRate, 12);
}

//======================================================MAEQ_AUDIO_PROCESSOR======================================================

MaeqAudioProcessor::MaeqAudioProcessor() 
    : AudioProcessor(BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
{
}

MaeqAudioProcessor::~MaeqAudioProcessor()
{
}

const juce::String MaeqAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MaeqAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MaeqAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MaeqAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MaeqAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MaeqAudioProcessor::getNumPrograms()
{
    return 1;
}

int MaeqAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MaeqAudioProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String MaeqAudioProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
}

void MaeqAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

void MaeqAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 1;
    spec.sampleRate = sampleRate;

    leftChain.prepare(spec);
    rightChain.prepare(spec);

    auto chainSettings = getChainSettings(apvts);

    updateFilters();
}

void MaeqAudioProcessor::releaseResources()
{
}

bool MaeqAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
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

void MaeqAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i) {
        buffer.clear(i, 0, buffer.getNumSamples());
    }

    auto chainSettings = getChainSettings(apvts);

    updateFilters();

    juce::dsp::AudioBlock<float> block(buffer);

    auto leftBlock = block.getSingleChannelBlock(0);
    auto rightBlock = block.getSingleChannelBlock(1);

    juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
    juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);

    leftChain.process(leftContext);
    rightChain.process(rightContext);
}

bool MaeqAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* MaeqAudioProcessor::createEditor()
{
    // return new MaeqAudioProcessorEditor(*this);
    return new juce::GenericAudioProcessorEditor(*this);
}

void MaeqAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream mos(destData, true);
    apvts.state.writeToStream(mos);
}

void MaeqAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if (tree.isValid()) {
        apvts.replaceState(tree);
        updateFilters();
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout MaeqAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    juce::StringArray lowFreqArray;
    int tempFreq { 32 };
    for (int i = 0; i < 3; ++i) {
        juce::String str;
        tempFreq *= i == 0 ? 1 : 2;
        str << std::to_string(tempFreq) << "Hz";
        lowFreqArray.add(str);
    }

    juce::StringArray highFreqArray;
    for (int i = 0; i < 5; ++i) {
        juce::String str;

        if (i == 0) {
            str << "5800Hz";
        } else if (i == 1) {
            str << "8192Hz";
        } else if (i == 2) {
            str << "11600Hz";
        } else if (i == 3) {
            str << "16400Hz";
        } else {
            str << "22100Hz";
        }

        highFreqArray.add(str);
    }

    juce::StringArray lowGainArray;
    for (int i = 0; i < 41; ++i) {
        juce::String str;
        str << std::to_string(-10.f + (0.5 * i)) << "dB";
        lowGainArray.add(str);
    }
    juce::StringArray highGainArray = lowGainArray;

    layout.add(std::make_unique<juce::AudioParameterFloat>("Input Gain", "Input Gain", juce::NormalisableRange<float>(-18.f, 18.f, 0.1, 1.f), 0.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("HighPass Freq", "HighPass Freq", juce::NormalisableRange<float>(10.f, 200.f, 1.f, 0.5f), 10.f));
    layout.add(std::make_unique<juce::AudioParameterChoice>("LowShelf Freq", "LowShelf Freq", lowFreqArray, 0));
    layout.add(std::make_unique<juce::AudioParameterChoice>("LowShelf Gain", "LowShelf Gain", lowGainArray, 20));
    layout.add(std::make_unique<juce::AudioParameterChoice>("HighShelf Gain", "HighShelf Gain", highGainArray, 20));
    layout.add(std::make_unique<juce::AudioParameterChoice>("HighShelf Freq", "HighShelf, Freq", highFreqArray, 0));
    layout.add(std::make_unique<juce::AudioParameterFloat>("LowPass Freq", "LowPass Freq", juce::NormalisableRange<float>(8000.f, 20000.f, 1.f, 0.5f), 20000.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("Output Gain", "Output Gain", juce::NormalisableRange<float>(-18.f, 18.f, 0.1, 1.f), 0.f));

    return layout;
}

void MaeqAudioProcessor::updateLowShelfFilter(const ChainSettings& chainSettings)
{
    auto shelfCoefficients = makeLowShelfFilter(chainSettings, getSampleRate());

    updateCoefficients(leftChain.get<ChainPositions::LowShelf>().coefficients, shelfCoefficients);
    updateCoefficients(rightChain.get<ChainPositions::LowShelf>().coefficients, shelfCoefficients);
}

void MaeqAudioProcessor::updateHighShelfFilter(const ChainSettings& chainSettings)
{
    auto shelfCoefficientsLeft = makeHighShelfFilter(chainSettings, getSampleRate(), true);
    auto shelfCoefficientsRight = makeHighShelfFilter(chainSettings, getSampleRate(), false);

    updateCoefficients(leftChain.get<ChainPositions::HighShelf>().coefficients, shelfCoefficientsLeft);
    updateCoefficients(rightChain.get<ChainPositions::HighShelf>().coefficients, shelfCoefficientsRight);
}

void MaeqAudioProcessor::updateHighPassFilter(const ChainSettings& chainSettings)
{
    auto cutCoefficients = makeHighPassFilter(chainSettings, getSampleRate());
    auto& leftHighPass = leftChain.get<ChainPositions::HighPass>();
    auto& rightHighPass = rightChain.get<ChainPositions::HighPass>();

    updateCutFilter(leftHighPass, cutCoefficients);
    updateCutFilter(rightHighPass, cutCoefficients);
}

void MaeqAudioProcessor::updateLowPassFilter(const ChainSettings& chainSettings)
{
    auto cutCoefficients = makeLowPassFilter(chainSettings, getSampleRate());
    auto& leftLowPass = leftChain.get<ChainPositions::LowPass>();
    auto& rightLowPass = rightChain.get<ChainPositions::LowPass>();

    updateCutFilter(leftLowPass, cutCoefficients);
    updateCutFilter(rightLowPass, cutCoefficients);
}

void MaeqAudioProcessor::updateFilters()
{
    auto chainSettings = getChainSettings(apvts);

    updateLowShelfFilter(chainSettings);
    updateHighShelfFilter(chainSettings);
    updateHighPassFilter(chainSettings);
    updateLowPassFilter(chainSettings);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MaeqAudioProcessor();
}