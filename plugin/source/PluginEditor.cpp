#include "PluginProcessor.h"
#include "PluginEditor.h"

MaeqAudioProcessorEditor::MaeqAudioProcessorEditor(MaeqAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    juce::ignoreUnused(processorRef);
    setSize (400, 300);
}

MaeqAudioProcessorEditor::~MaeqAudioProcessorEditor()
{
}

void MaeqAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    g.setColour(juce::Colours::white);
    g.setFont(15.0f);
    g.drawFittedText("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void MaeqAudioProcessorEditor::resized()
{
}