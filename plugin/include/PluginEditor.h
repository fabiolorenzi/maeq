#pragma once

#include "PluginProcessor.h"

class MaeqAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit MaeqAudioProcessorEditor(MaeqAudioProcessor&);
    ~MaeqAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    MaeqAudioProcessor& processorRef;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MaeqAudioProcessorEditor)
};