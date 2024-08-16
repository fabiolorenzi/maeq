#include "PluginProcessor.h"
#include "PluginEditor.h"

//=========================================================GLOBAL_PROCESSES=========================================================

template<int Index, typename ChainType, typename CoefficientType>
inline void update(ChainType& chain, const CoefficientType& coefficients)
{
    updateCoefficients(chain.template get<Index>().coefficients, coefficients[Index]);
}

template<typename ChainType, typename CoefficientType>
void updateCutFilter(ChainType& chain, const CoefficientType& cutCoefficients)
{
    update<0>(chain, cutCoefficients);
}

//=======================================================MAEQ_AUDIO_PROCESSOR=======================================================

MaeqAudioProcessor::MaeqAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor(BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    chainSettings = ChainSettings();
    clipper = new Clipper();
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
    samplesPB = samplesPerBlock;

    leftChain.prepare(spec);
    rightChain.prepare(spec);

    chainSettings.getChainSettings(apvts);
    updateFilters();

    clipper->prepare(sampleRate, samplesPerBlock);
}

void MaeqAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MaeqAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    #if JucePlugin_IsMidiEffect
        juce::ignoreUnused (layouts);
        return true;
    #else
        if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
            return false;

        // This checks if the input layout matches the output layout
    #if ! JucePlugin_IsSynth
        if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
            return false;
    #endif

        return true;
    #endif
}
#endif

void MaeqAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    chainSettings.getChainSettings(apvts);

    AudioBlock block(buffer);
    Context context(block);

    clipper->updateOversample(chainSettings.oversample, getSampleRate(), samplesPB);
    clipper->updateThr(chainSettings.inputGain);
    clipper->process(context);

    for (int channel = 0; channel < totalNumInputChannels; ++channel) {
        auto* channelData = buffer.getWritePointer(channel);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            channelData[sample] = channelData[sample] * juce::Decibels::decibelsToGain(chainSettings.inputGain);
            channelData[sample] = channelData[sample] * juce::Decibels::decibelsToGain(chainSettings.outputGain);
        }
    }

    updateFilters();

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
    return new MaeqAudioProcessorEditor(*this);
    // return new juce::GenericAudioProcessorEditor(*this);
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
            str << "21000Hz";
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
    layout.add(std::make_unique<juce::AudioParameterFloat>("Clipper Thr", "Clipper Thr", juce::NormalisableRange<float>(10.f, 12.f, 0.1, 1.f), 0.f));
    layout.add(std::make_unique<juce::AudioParameterBool>("Oversample", "Oversample", false));
    layout.add(std::make_unique<juce::AudioParameterFloat>("HighPass Freq", "HighPass Freq", juce::NormalisableRange<float>(5.f, 200.f, 1.f, 0.5f), 5.f));
    layout.add(std::make_unique<juce::AudioParameterChoice>("LowShelf Freq", "LowShelf Freq", lowFreqArray, 0));
    layout.add(std::make_unique<juce::AudioParameterChoice>("LowShelf Gain", "LowShelf Gain", lowGainArray, 20));
    layout.add(std::make_unique<juce::AudioParameterFloat>("GhostPeak Freq", "GhostPeak Freq", juce::NormalisableRange<float>(250.f, 500.f, 0.1, 1.f), 250.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("GhostPeak Gain", "GhostPeak Gain", juce::NormalisableRange<float>(-3.5, 0.f, 0.1, 1.f), 0.f));
    layout.add(std::make_unique<juce::AudioParameterChoice>("HighShelf Gain", "HighShelf Gain", highGainArray, 20));
    layout.add(std::make_unique<juce::AudioParameterChoice>("HighShelf Freq", "HighShelf, Freq", highFreqArray, 0));
    layout.add(std::make_unique<juce::AudioParameterFloat>("LowPass Freq", "LowPass Freq", juce::NormalisableRange<float>(8000.f, 21000.f, 1.f, 0.5f), 21000.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("Output Gain", "Output Gain", juce::NormalisableRange<float>(-18.f, 18.f, 0.1, 1.f), 0.f));

    return layout;
}

void MaeqAudioProcessor::updateLowShelfFilter(const ChainSettings& chainSettings)
{
    auto shelfCoefficients = makeLowShelfFilter(chainSettings, getSampleRate());

    updateCoefficients(leftChain.get<ChainPositions::LowShelf>().coefficients, shelfCoefficients);
    updateCoefficients(rightChain.get<ChainPositions::LowShelf>().coefficients, shelfCoefficients);
}

void MaeqAudioProcessor::updateGhostPeakFilter(const ChainSettings& chainSettings)
{
    auto peakCoefficients = makeGhostPeakFilter(chainSettings, getSampleRate());

    updateCoefficients(leftChain.get<ChainPositions::GhostPeak>().coefficients, peakCoefficients);
    updateCoefficients(rightChain.get<ChainPositions::GhostPeak>().coefficients, peakCoefficients);
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
    chainSettings.getChainSettings(apvts);

    updateLowShelfFilter(chainSettings);
    updateGhostPeakFilter(chainSettings);
    updateHighShelfFilter(chainSettings);
    updateHighPassFilter(chainSettings);
    updateLowPassFilter(chainSettings);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MaeqAudioProcessor();
}