#include "Clipper.h"

Clipper::Clipper()
{
    sampleRate = 0.0;
    bufferSize = 0;
    oversampledBufferSize = 0;
    threshold = -4.f;
    ceiling = -4.f;
    normalGainReduction = {0.f, 0.f};
    oversampledGainReduction = {0.f, 0.f};
    oversample = false;
    oversampleValue = 1;
}

Clipper::~Clipper() {}

void Clipper::prepare(double inputSampleRate, int maxBlockSize)
{
    sampleRate = inputSampleRate * oversampleValue;
    bufferSize = maxBlockSize;
    oversampledBufferSize = bufferSize * oversampleValue;
    oversampler.reset();
    oversampler.initProcessing(oversampledBufferSize);
}

void Clipper::process(const Context& context)
{
    auto& outputBuffer = context.getOutputBlock();
    auto upsampledBlock = oversampler.processSamplesUp(context.getInputBlock());
    // clip with 4x oversampling
    clipBuffer(upsampledBlock, oversampledBufferSize, oversampledGainReduction);
    oversampler.processSamplesDown(context.getOutputBlock());
    // clip with no oversampling
    clipBuffer(outputBuffer, bufferSize, normalGainReduction);
    applyGain(outputBuffer, bufferSize);
}

void Clipper::clipBuffer(AudioBlock& block, int blockSize, std::array<float, 2>& outputGR)
{
    outputGR = {0.f, 0.f};
    std::array<float, 2> tempGR = {0.f, 0.f};
    const float thresholdHigh = Decibels::decibelsToGain(threshold);
    const float thresholdLow = thresholdHigh * -1.f;

    for (int sample = 0; sample < blockSize; sample++) {
        for (int channel = 0; channel < 2; channel++) {
            const float inputSample = block.getSample(channel, sample);
            const float outputSample = jmax(jmin(inputSample, thresholdHigh), thresholdLow);
            if (inputSample != outputSample) {
                tempGR[channel] = Decibels::gainToDecibels(inputSample) - threshold;
            }
            outputGR[channel] = jmax(tempGR[channel], outputGR[channel]);
            block.setSample(channel, sample, outputSample);
        }
    }
}

void Clipper::applyGain(AudioBlock& block, int blockSize)
{
    for (int sample = 0; sample < blockSize; sample++) {
        for (int channel = 0; channel < 2; channel++) {
            float outputSample = block.getSample(channel, sample);
            outputSample *= Decibels::decibelsToGain(threshold * -1.f);
            outputSample *= Decibels::decibelsToGain(ceiling);
            block.setSample(channel, sample, outputSample);
        }
    }
}

void Clipper::updateThr(float vol)
{
    threshold = - vol - 4.f;
    ceiling = - vol - 4.f;
}

void Clipper::updateOversample(bool oversampleNewValue, int inputSampleRate, int maxBlockSize)
{
    if (oversampleNewValue != oversample) {
        oversample = oversampleNewValue;
        oversampleValue = oversample ? 4 : 1;
        prepare(inputSampleRate, maxBlockSize);
    }
}