#pragma once

#include <JuceHeader.h>
#include "ChainSettings.h"
#include "EQValues.h"

#define JucePlugin_Name "Maeq"

//=========================================================GLOBAL_PROCESSES=========================================================

using Filter = juce::dsp::IIR::Filter<float>;
using CutFilter = juce::dsp::ProcessorChain<Filter>;
using MonoChain = juce::dsp::ProcessorChain<CutFilter, Filter, Filter, Filter, CutFilter>;
using Coefficients = Filter::CoefficientsPtr;

void updateCoefficients(Coefficients& old, const Coefficients& replacements);
Coefficients makeLowShelfFilter(const ChainSettings& chainSettings, double sampleRate);
Coefficients makeGhostPeakFilter(const ChainSettings& chainSettings, double sampleRate);
Coefficients makeHighShelfFilter(const ChainSettings& chainSettings, double sampleRate, bool isLeft);

template<int Index, typename ChainType, typename CoefficientType>
void update(ChainType& chain, const CoefficientType& cutCoefficients);

template<typename ChainType, typename CoefficientType>
void updateCutFilter(ChainType& chain, const CoefficientType& cutCoefficients);

inline auto makeHighPassFilter(const ChainSettings& chainSettings, double sampleRate);
inline auto makeLowPassFilter(const ChainSettings& chainSettings, double sampleRate);

//=======================================================MAEQ_AUDIO_PROCESSOR=======================================================

class MaeqAudioProcessor  : public juce::AudioProcessor
{
	public:
		using AudioProcessor::processBlock;

		MaeqAudioProcessor();
		~MaeqAudioProcessor() override;

		void prepareToPlay(double sampleRate, int samplesPerBlock) override;
		void releaseResources() override;

		bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

		void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

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

		ChainSettings chainSettings;

	private:
		MonoChain leftChain, rightChain;

		void updateLowShelfFilter(const ChainSettings& chainSettings);
		void updateGhostPeakFilter(const ChainSettings& chainSettings);
		void updateHighShelfFilter(const ChainSettings& chainSettings);
		void updateHighPassFilter(const ChainSettings& chainSettings);
		void updateLowPassFilter(const ChainSettings& chainSettings);
		void updateFilters();

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MaeqAudioProcessor);
};