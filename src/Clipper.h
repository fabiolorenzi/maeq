#pragma once;

#include <JuceHeader.h>

using Context = juce::dsp::ProcessContextReplacing<float>;
using Oversampler = juce::dsp::Oversampling<float>;
using AudioBlock = juce::dsp::AudioBlock<float>;

class Clipper
{
    public:
        Clipper();
        ~Clipper();
        void prepare(double inputSampleRate, int maxBlockSize);
        void process(const Context& context);
    private:
        double sampleRate;
        int bufferSize;
        int oversampledBufferSize;
        float threshold;
        float ceiling;
        std::array<float, 2> normalGainReduction;
        std::array<float, 2> oversampledGainReduction;
        Oversampler oversampler {2, 2, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, false, true};
        void clipBuffer(AudioBlock& block, int blockSize, std::array<float, 2>& outputGR);
        void applyGain(AudioBlock& block, int blockSize);
};