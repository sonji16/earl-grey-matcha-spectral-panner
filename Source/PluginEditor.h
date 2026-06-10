/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

    Used Claude.Ai Sonnet 4.6 and Opus 4.8 for the initial coding and debugging.
 
     Author: Jiwoo Son
     Program: Spectral Panner
     Course: MUS 177 (Professor Tom Erbe) at UCSD
     Date Created: Jun 8th, 2026
 
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "PannerlookAndFeel.h"

//==============================================================================
/**
*/
class JiwooSonSpectralPannerAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    JiwooSonSpectralPannerAudioProcessorEditor (JiwooSonSpectralPannerAudioProcessor&);
    ~JiwooSonSpectralPannerAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    JiwooSonSpectralPannerAudioProcessor& audioProcessor;
    PannerLookAndFeel laf;

    // lfo frequency display is drawn in paint()
    
    // lfo rate, depth slider and shape buttons
    juce::Slider rateSlider;
    juce::Slider depthSlider;
    juce::Slider widthSlider;
    juce::ComboBox lfoShapeBox; // dropdown menu
    juce::Label rateLabel, depthLabel, widthLabel, lfoShapeLabel;
 
    
    // center freq slider and filter selection (12, 24, 36, 48)
    juce::Slider centerFreqSlider;
    juce::Slider filterSlopeKnob; // snapping rotary wheel
    juce::Label centerFreqLabel, filterSlopeLabel;

    
    //mix, gain
    juce::Slider mixSlider;
    juce::Slider gainSlider;
    juce::Label mixLabel, gainLabel;
    
    // parameter attachments
    using SliderAttachment  = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment   = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
     
    std::unique_ptr<SliderAttachment> rateAttachment;
    std::unique_ptr<SliderAttachment> depthAttachment;
    std::unique_ptr<SliderAttachment> widthAttachment;
    std::unique_ptr<SliderAttachment> centerFreqAttachment;
    std::unique_ptr<SliderAttachment> filterSlopeAttachment;
    std::unique_ptr<SliderAttachment> mixAttachment;
    std::unique_ptr<SliderAttachment> gainAttachment;
    std::unique_ptr<ComboAttachment>  lfoShapeAttachment;
    
    float lfoDisplayPhase = 0.0f;
    void styleRotaryKnob(juce::Slider& s);
    void styleSnapKnob(juce::Slider& s, int numSteps,
                            const juce::StringArray& labels);
    void styleLabel(juce::Label& l,  const juce::String& text);
    void styleButton(juce::TextButton& b, bool active);
    void drawSectionCard(juce::Graphics& g, juce::Rectangle<int> bounds,
                            const juce::String& title);
    void drawFrequencyDisplay(juce::Graphics& g, juce::Rectangle<int> bounds);
    void drawLFOWaveform(juce::Graphics& g, juce::Rectangle<int>bounds);
    void drawMeters(juce::Graphics& g, juce::Rectangle<int> bounds);
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JiwooSonSpectralPannerAudioProcessorEditor)
};
