/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.
    Used Claude.Ai Sonnet 4.6 and Opus 4.8 for the instructions and debugging.
 
     Author: Jiwoo Son
     Program: Spectral Panner
     Course: MUS 177 (Professor Tom Erbe) at UCSD
     Date Created: Jun 8th, 2026
 
  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

//==============================================================================
/**
*/
class JiwooSonSpectralPannerAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    JiwooSonSpectralPannerAudioProcessor();
    ~JiwooSonSpectralPannerAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;


    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;


    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;
    

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;
    float getLevelL() const { return levelL.load(); }
    float getLevelR() const { return levelR.load(); }

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    
    juce::AudioProcessorValueTreeState parameters;
private:
    //==============================================================================

    
    float lfoPhase = 0.0f;
    float lfoRate = 0.5f; // LFO rate (0-)
    float lfoDepth = 0.7f; // 0-1 (determines the amplitude of the LFO shape)
    float lfoWidth = 1.0f; // 0-1
    float centerFreq = 1000.0f; // user can set it. the center of the crossover point
    float sweepOctaves = 2.0f; // 0.5 = half octave sweep, 1 = 1 octave sweep,
                               // 2 = 2 octave sweep,.. i.e. the range of crossover
                               // point moving
    float mix      = 1.0f; //0-1. 1 is 100% wet, 0 is % wet
    float gainDb   = 0.0f; // outpuf gain
    
    float sampleRate = 48000.0f;
    float s_r = 48000.0f; // a variable for current sample rate when going through each
    int filterStages = 2; // 12, 24, 32, 48 db 
    int lfoShape = 0; // 0, 1, 2, 3 = sine, saw, sq, tri respectively
    
    // filters
    static constexpr int maxStages = 4; // (1,2,3,4)
    juce::IIRFilter lpfChainL[maxStages];
    juce::IIRFilter lpfChainR[maxStages];
    
    std::atomic<float>* pRate = nullptr;
    std::atomic<float>* pDepth = nullptr;
    std::atomic<float>* pWidth = nullptr;
    std::atomic<float>* pCenterFreq = nullptr;
    std::atomic<float>* pSweepOctaves = nullptr;
    std::atomic<float>* pFilterSlope = nullptr;
    std::atomic<float>* pLfoShape = nullptr;
    std::atomic<float>* pMix  = nullptr;
    std::atomic<float>* pGain = nullptr;
    std::atomic<float> levelL { 0.0f };
    std::atomic<float> levelR { 0.0f };
    
    // function for choosing which shape (sin, saw, wq, tri)
    float chooseLFO(float phase, int shape);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JiwooSonSpectralPannerAudioProcessor)
};
