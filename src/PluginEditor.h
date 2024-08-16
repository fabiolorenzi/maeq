#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

struct Lnf : juce::LookAndFeel_V4
{
    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height, float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle, juce::Slider&) override;
    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& toggleButton, bool shouldDrawButtonAsHighlighted, bool shoudDrawButtonAsDown) override;
};

struct RotarySliderWithLabels : juce::Slider
{
    RotarySliderWithLabels(juce::RangedAudioParameter& rap, const juce::String& unitSuffix) : 
    juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::NoTextBox),
    param(&rap), suffix(unitSuffix) {
        setLookAndFeel(&lnf);
    }

    ~RotarySliderWithLabels() {
        setLookAndFeel(nullptr);
    }

    struct LabelPos {
        float pos;
        juce::String label;
    };

    juce::Array<LabelPos> labels;

    void paint(juce::Graphics& g) override;
    juce::Rectangle<int> getSliderBounds() const;
    int getTextHeight() const { return 14; }
    juce::String getDisplayString() const;
private:
    Lnf lnf;

    juce::RangedAudioParameter* param;
    juce::String suffix;
};

class MaeqAudioProcessorEditor  : public juce::AudioProcessorEditor
{
    public:
        MaeqAudioProcessorEditor(MaeqAudioProcessor&);
        ~MaeqAudioProcessorEditor() override;

        void paint(juce::Graphics&) override;
        void resized() override;

    private:
        MaeqAudioProcessor& audioProcessor;

        RotarySliderWithLabels inputGainSlider,
            highPassFilterSlider,   
            lowShelfFreqSlider,
            lowShelfGainSlider,
            highShelfGainSlider,
            highShelfFreqSlider,
            lowPassFilterSlider,
            outputGainSlider;

        using APVTS = juce::AudioProcessorValueTreeState;
        using Attachment = APVTS::SliderAttachment;

        Attachment inputGainSliderAttachment,
            highPassFilterSliderAttachment,   
            lowShelfFreqSliderAttachment,
            lowShelfGainSliderAttachment,
            highShelfGainSliderAttachment,
            highShelfFreqSliderAttachment,
            lowPassFilterSliderAttachment,
            outputGainSliderAttachment;

        juce::ToggleButton oversampleActivate;

        using ButtonAttachment = APVTS::ButtonAttachment;
        ButtonAttachment oversampleActivateAttachment;

        std::vector<juce::Component*> getComps();

        Lnf lnf;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MaeqAudioProcessorEditor);
};