#include "PluginProcessor.h"
#include "PluginEditor.h"

//======================================================GLOBAL_PROCESSES======================================================

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState &apvts)
{
    return ChainSettings();
}

void updateCoefficients(Coefficients& old, const Coefficients& replacements)
{
}

Coefficients makeShelfFilter(const ChainSettings &chainSettings, double sampleRate)
{
    return Coefficients();
}

template<int Index, typename ChainType, typename CoefficientType>
inline void update(ChainType &chain, const CoefficientType &coefficients)
{
}

template<typename ChainType, typename CoefficientType>
void updateCutFilter(ChainType &chain, const CoefficientType &cutCoefficients)
{
}

inline auto makeHighPassFilter(const ChainSettings &chainSettings, double sampleRate)
{
    return juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(chainSettings.highPassFreq, sampleRate, 12);
}

inline auto makeLowPassfilter(const ChainSettings &chainSettings, double sampleRate)
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
    juce::ignoreUnused(sampleRate, samplesPerBlock);
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

void MaeqAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        juce::ignoreUnused(channelData);
    }
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
    juce::ignoreUnused(destData);
}

void MaeqAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    juce::ignoreUnused(data, sizeInBytes);
}

juce::AudioProcessorValueTreeState::ParameterLayout MaeqAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    juce::StringArray lowFreqArray;
    int tempFreq { 32 };
    for (int i = 0; i < 5; ++i) {
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

    layout.add(std::make_unique<juce::AudioParameterFloat>("Input Gain", "Input Gain", juce::NormalisableRange<float>(-18.f, 18.f, 0.1, 1.f), 0.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("HighPass Freq", "HighPass Freq", juce::NormalisableRange<float>(10.f, 200.f, 1.f, 0.5f), 10.f));
    layout.add(std::make_unique<juce::AudioParameterChoice>("LowShelf Freq", "LowShelf Freq", lowFreqArray, 0));
    layout.add(std::make_unique<juce::AudioParameterFloat>("LowShelf Gain", "LowShelf Gain", juce::NormalisableRange<float>(-10.f, 10.f, 0.5, 1.f), 0.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("HighShelf Gain", "HighShelf Gain", juce::NormalisableRange<float>(-10.f, 10.f, 0.5, 1.f), 0.f));
    layout.add(std::make_unique<juce::AudioParameterChoice>("HighShelf Freq", "HighShelf, Freq", highFreqArray, 0));
    layout.add(std::make_unique<juce::AudioParameterFloat>("LowPass Freq", "LowPass Freq", juce::NormalisableRange<float>(10000.f, 20000.f, 1.f, 0.5f), 20000.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("Output Gain", "Output Gain", juce::NormalisableRange<float>(-18.f, 18.f, 0.1, 1.f), 0.f));

    return layout;
}

void MaeqAudioProcessor::updateLowShelfFilter(const ChainSettings& chainSettings)
{
}

void MaeqAudioProcessor::updateHighShelfFilter(const ChainSettings& chainSettings)
{
}

void MaeqAudioProcessor::updateHighPassFilter(const ChainSettings& chainSettings)
{
}

void MaeqAudioProcessor::updateLowPassFilter(const ChainSettings& chainSettings)
{
}

void MaeqAudioProcessor::updateFilters()
{
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MaeqAudioProcessor();
}