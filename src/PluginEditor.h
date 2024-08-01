#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class MaeqAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    MaeqAudioProcessorEditor(MaeqAudioProcessor&);
    ~MaeqAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    MaeqAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MaeqAudioProcessorEditor);
};