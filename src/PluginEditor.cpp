#include "PluginProcessor.h"
#include "PluginEditor.h"

void Lnf::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider)
{
    using namespace juce;

    auto bounds = Rectangle<float>(x, y, width, height);

    g.setColour(Colour(10u, 10u, 10u));
    g.fillEllipse(bounds);

    g.setColour(Colour(255u, 255u, 255u));
    g.drawEllipse(bounds, 3.f);

    if (auto* rswl = dynamic_cast<RotarySliderWithLabels*>(&slider)) {
        auto center = bounds.getCentre();
        Path p;
        Rectangle<float> r;
        r.setLeft(center.getX() - 3);
        r.setRight(center.getX() + 3);
        r.setTop(bounds.getY());
        r.setBottom(center.getY() - rswl->getTextHeight() * 1.5);

        p.addRoundedRectangle(r, 2.f);

        jassert(rotaryStartAngle < rotaryEndAngle);

        auto sliderAngRad = jmap(sliderPosProportional, 0.f, 1.f, rotaryStartAngle, rotaryEndAngle);

        p.applyTransform(AffineTransform().rotated(sliderAngRad, center.getX(), center.getY()));

        g.fillPath(p);

        g.setFont(rswl->getTextHeight());
        auto text = rswl->getDisplayString();
        auto strWidth = g.getCurrentFont().getStringWidth(text);

        r.setSize(strWidth + 4, rswl->getTextHeight() + 2);
        r.setCentre(bounds.getCentre());

        g.setColour(Colours::black);
        g.fillRect(r);

        g.setColour(Colours::white);
        g.drawFittedText(text, r.toNearestInt(), juce::Justification::centred, 1);
    }
}

void Lnf::drawToggleButton(juce::Graphics& g, juce::ToggleButton& toggleButton, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    using namespace juce;

    Path powerButton;

    auto bounds = toggleButton.getLocalBounds();
    auto size = jmin(bounds.getWidth(), bounds.getHeight()) - 6;
    auto r = bounds.withSizeKeepingCentre(size, size).toFloat();

    float ang = 30.f;
    size -= 6;
    powerButton.addCentredArc(r.getCentreX(), r.getCentreY(), size * 0.5, size * 0.5, 0.f, degreesToRadians(ang), degreesToRadians(360.f - ang), true);
    powerButton.startNewSubPath(r.getCentreX(), r.getY());
    powerButton.lineTo(r.getCentre());

    PathStrokeType pst(2.f, PathStrokeType::JointStyle::curved);
    auto colour = toggleButton.getToggleState() ? Colours::dimgrey : Colour(255u, 255u, 255u);

    g.setColour(colour);
    g.strokePath(powerButton, pst);

    g.drawEllipse(r, 2);
}

void RotarySliderWithLabels::paint(juce::Graphics& g)
{
    using namespace juce;

    // These are to find the angles were to add the labels, which are at 7 o'clock and 5 o'clock
    auto startAng = degreesToRadians(180.f + 45.f);
    auto endAng = degreesToRadians(180.f - 45.f) + MathConstants<float>::twoPi;

    auto range = getRange();

    auto sliderBounds = getSliderBounds();

    //getLookAndFeel().drawRotarySlider(g, sliderBounds.getX(), sliderBounds.getY(), sliderBounds.getWidth(), sliderBounds.getHeight(), jmap(getValue(), range.getStart(), range.getEnd(), 0.0, 1.0), startAng, endAng, *this);
    // The code below is to keep the skew factor from the plugin processor file
    getLookAndFeel().drawRotarySlider(g, sliderBounds.getX(), sliderBounds.getY(), sliderBounds.getWidth(), sliderBounds.getHeight(), valueToProportionOfLength(getValue()), startAng, endAng, *this);

    auto center = sliderBounds.toFloat().getCentre();
    auto radius = sliderBounds.getWidth() * 0.5f;

    g.setColour(Colour(255u, 255u, 255u));
    g.setFont(getTextHeight());

    auto numChoices = labels.size();
    for (int i = 0; i < numChoices; ++i) {
        auto pos = labels[i].pos;
        jassert(0.f <= pos);
        jassert(pos <= 1.f);

        auto ang = jmap(pos, 0.f, 1.f, startAng, endAng);

        auto c = center.getPointOnCircumference(radius + getTextHeight() * 0.5f + 1, ang);

        Rectangle<float> r;
        auto str = labels[i].label;
        r.setSize(g.getCurrentFont().getStringWidth(str), getTextHeight());
        r.setCentre(c);
        r.setY(r.getY() + getTextHeight());

        g.drawFittedText(str, r.toNearestInt(), juce::Justification::centred, 1);
    }
}

juce::Rectangle<int> RotarySliderWithLabels::getSliderBounds() const
{
    auto bounds = getLocalBounds();

    auto size = juce::jmin(bounds.getWidth(), bounds.getHeight());
    size -= getTextHeight() * 2;
    juce::Rectangle<int> r;
    r.setSize(size, size);
    r.setCentre(bounds.getCentreX(), 0);
    r.setY(2);

    return r;
}

juce::String RotarySliderWithLabels::getDisplayString() const
{
    if (auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(param)) {
        return choiceParam->getCurrentChoiceName();
    }

    juce::String str;
    bool addK = false;

    if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(param)) {
        float val = getValue();
        if (val > 999.f) {
            val /= 1000.f;
            addK = true;
        }

        str = juce::String(val, (addK ? 2 : 0));
    }
    else {
        jassertfalse;
    }

    if (suffix.isNotEmpty()) {
        str << " ";
        if (addK) {
            str << "k";
        }

        str << suffix;
    }

    return str;
}

MaeqAudioProcessorEditor::MaeqAudioProcessorEditor(MaeqAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p),
    inputGainSlider(*audioProcessor.apvts.getParameter("Input Gain"), "dB"),
    highPassFilterSlider(*audioProcessor.apvts.getParameter("HighPass Freq"), "Hz"),
    lowShelfFreqSlider(*audioProcessor.apvts.getParameter("LowShelf Freq"), "Hz"),
    lowShelfGainSlider(*audioProcessor.apvts.getParameter("LowShelf Gain"), "dB"),
    highShelfGainSlider(*audioProcessor.apvts.getParameter("HighShelf Gain"), "dB"),
    highShelfFreqSlider(*audioProcessor.apvts.getParameter("HighShelf Freq"), "Hz"),
    lowPassFilterSlider(*audioProcessor.apvts.getParameter("LowPass Freq"), "Hz"),
    outputGainSlider(*audioProcessor.apvts.getParameter("Output Gain"), "dB"),
    inputGainSliderAttachment(audioProcessor.apvts, "Input Gain", inputGainSlider),
    highPassFilterSliderAttachment(audioProcessor.apvts, "HighPass Freq", highPassFilterSlider),
    lowShelfFreqSliderAttachment(audioProcessor.apvts, "LowShelf Freq", lowShelfFreqSlider),
    lowShelfGainSliderAttachment(audioProcessor.apvts, "LowShelf Gain", lowShelfGainSlider),
    highShelfGainSliderAttachment(audioProcessor.apvts, "HighShelf Gain", highShelfGainSlider),
    highShelfFreqSliderAttachment(audioProcessor.apvts, "HighShelf Freq", highShelfFreqSlider),
    lowPassFilterSliderAttachment(audioProcessor.apvts, "LowPass Freq", lowPassFilterSlider),
    outputGainSliderAttachment(audioProcessor.apvts, "Output Gain", outputGainSlider),
    oversampleActivateAttachment(audioProcessor.apvts, "Oversample", oversampleActivate)
{
    inputGainSlider.labels.add({0.f, "-18.0dB"});
    inputGainSlider.labels.add({1.f, "18.0dB"});
    highPassFilterSlider.labels.add({0.f, "5Hz"});
    highPassFilterSlider.labels.add({1.f, "200Hz"});
    lowShelfFreqSlider.labels.add({0.f, "32Hz"});
    lowShelfFreqSlider.labels.add({0.5f, "64Hz"});
    lowShelfFreqSlider.labels.add({1.f, "128Hz"});
    lowShelfGainSlider.labels.add({0.f, "-10dB"});
    lowShelfGainSlider.labels.add({1.f, "10dB"});
    highShelfGainSlider.labels.add({0.f, "-10dB"});
    highShelfGainSlider.labels.add({1.f, "10dB"});
    highShelfFreqSlider.labels.add({0.f, "5.8kHz"});
    highShelfFreqSlider.labels.add({0.25f, "8.2kHz"});
    highShelfFreqSlider.labels.add({0.5f, "11.6kHz"});
    highShelfFreqSlider.labels.add({0.75f, "16.4kHz"});
    highShelfFreqSlider.labels.add({1.f, "21kHz"});
    lowPassFilterSlider.labels.add({0.f, "8kHz"});
    lowPassFilterSlider.labels.add({1.f, "21kHz"});
    outputGainSlider.labels.add({0.f, "-18.0dB"});
    outputGainSlider.labels.add({1.f, "18.0dB"});

    for (auto* comp : getComps()) {
        addAndMakeVisible(comp);
    }

    oversampleActivate.setLookAndFeel(&lnf);

    setSize(1200, 109);
}

MaeqAudioProcessorEditor::~MaeqAudioProcessorEditor()
{
    oversampleActivate.setLookAndFeel(nullptr);
}

void MaeqAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
}

void MaeqAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    float hRatio = 0.25f;
    
    auto inputGainArea = bounds.removeFromLeft(bounds.getWidth() * 0.125);
    auto highPassFilterArea = bounds.removeFromLeft(bounds.getWidth() * (1.f / 7));
    auto lowShelfFreqArea = bounds.removeFromLeft(bounds.getWidth() * (1.f / 6));
    auto lowShelfGainArea = bounds.removeFromLeft(bounds.getWidth() * 0.2f);
    auto highShelfGainArea = bounds.removeFromLeft(bounds.getWidth() * 0.25f);
    auto highShelfFreqArea = bounds.removeFromLeft(bounds.getWidth() * 0.33f);
    auto lowPassFilterArea = bounds.removeFromLeft(bounds.getWidth() * 0.5f);
    auto outputGainArea = bounds.removeFromLeft(bounds.getWidth());

    inputGainSlider.setBounds(inputGainArea.removeFromTop(inputGainArea.getHeight() * 0.5f));
    highPassFilterSlider.setBounds(highPassFilterArea.removeFromTop(highPassFilterArea.getHeight() * 0.5f));
    lowShelfFreqSlider.setBounds(lowShelfFreqArea.removeFromTop(lowShelfFreqSlider.getHeight() * 0.5f));
    lowShelfGainSlider.setBounds(lowShelfGainArea.removeFromTop(lowShelfGainArea.getHeight() * 0.5f));
    highShelfGainSlider.setBounds(highShelfGainArea.removeFromTop(highShelfGainArea.getHeight() * 0.5f));
    highShelfFreqSlider.setBounds(highShelfFreqArea.removeFromTop(highShelfFreqSlider.getHeight() * 0.5f));
    lowPassFilterSlider.setBounds(lowPassFilterArea.removeFromTop(lowPassFilterArea.getHeight() * 0.5f));
    outputGainSlider.setBounds(outputGainArea.removeFromTop(outputGainArea.getHeight() * 0.5f));
}

std::vector<juce::Component*> MaeqAudioProcessorEditor::getComps()
{
    return {
        &inputGainSlider,
        &highPassFilterSlider,
        &lowShelfFreqSlider,
        &lowShelfGainSlider,
        &highShelfGainSlider,
        &highShelfFreqSlider,
        &lowPassFilterSlider,
        &outputGainSlider,
        &oversampleActivate
    };
}