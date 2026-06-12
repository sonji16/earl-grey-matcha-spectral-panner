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

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
JiwooSonSpectralPannerAudioProcessor::JiwooSonSpectralPannerAudioProcessor()
     : AudioProcessor (BusesProperties()
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                       ),
parameters(*this, nullptr, "Parameters",{
    std::make_unique<juce::AudioParameterFloat>(
        "rate", "LFO Rate",
        juce::NormalisableRange<float>(0.05f, 8.0f, 0.01f, 0.5f),
        0.5f),

    std::make_unique<juce::AudioParameterFloat>(
        "depth", "Depth",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.7f),

    std::make_unique<juce::AudioParameterFloat>(
        "width", "Width",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        1.0f),

    std::make_unique<juce::AudioParameterFloat>(
        "centerFreq", "Center Freq",
        juce::NormalisableRange<float>(100.0f, 8000.0f, 1.0f, 0.3f),
        1000.0f),

    std::make_unique<juce::AudioParameterFloat>(
        "sweepOctaves", "Sweep Octaves",
        juce::NormalisableRange<float>(0.5f, 4.0f, 0.1f),
        2.0f),

    std::make_unique<juce::AudioParameterChoice>(
        "filterSlope", "Filter Slope",
        juce::StringArray{"12 dB", "24 dB", "36 dB", "48 dB"},
                                                 1),
    std::make_unique<juce::AudioParameterChoice>(
        "lfoShape", "LFO Shape",
        juce::StringArray{"Sine", "Saw", "Square", "Triangle"},
        0),  // default index 0 = sine
    std::make_unique<juce::AudioParameterFloat>(
        "mix", "Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        1.0f),

    std::make_unique<juce::AudioParameterFloat>(
        "gain", "Gain",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f),
                                                0.0f),

})
{
    pRate = parameters.getRawParameterValue("rate");
    pDepth = parameters.getRawParameterValue("depth");
    pWidth = parameters.getRawParameterValue("width");
    pCenterFreq = parameters.getRawParameterValue("centerFreq");
    pSweepOctaves = parameters.getRawParameterValue("sweepOctaves");
    pFilterSlope = parameters.getRawParameterValue("filterSlope");
    pLfoShape = parameters.getRawParameterValue("lfoShape");
    pMix  = parameters.getRawParameterValue("mix");
    pGain = parameters.getRawParameterValue("gain");
}


JiwooSonSpectralPannerAudioProcessor::~JiwooSonSpectralPannerAudioProcessor()
{
}

//==============================================================================
const juce::String JiwooSonSpectralPannerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool JiwooSonSpectralPannerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool JiwooSonSpectralPannerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool JiwooSonSpectralPannerAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double JiwooSonSpectralPannerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int JiwooSonSpectralPannerAudioProcessor::getNumPrograms()
{
    return 1;
}

int JiwooSonSpectralPannerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void JiwooSonSpectralPannerAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String JiwooSonSpectralPannerAudioProcessor::getProgramName (int index)
{
    return {};
}

void JiwooSonSpectralPannerAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void JiwooSonSpectralPannerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // resetting sample rate
    this->sampleRate = sampleRate;
    s_r = (float)sampleRate;
    
    // resetting all the lfo filters
    for (int s=0; s<maxStages; s++){
        lpfChainL[s].reset();
        lpfChainR[s].reset();
    }
    
}

void JiwooSonSpectralPannerAudioProcessor::releaseResources()
{

}

bool JiwooSonSpectralPannerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    auto out = layouts.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void JiwooSonSpectralPannerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    
    //overwriting default values with the inputs
    lfoRate = pRate->load();
    lfoDepth = pDepth->load();
    lfoWidth = pWidth->load();
    centerFreq = pCenterFreq->load();
    sweepOctaves = pSweepOctaves->load();
    filterStages = (int)pFilterSlope->load() + 1;
    lfoShape = (int)pLfoShape->load();
    mix    = pMix->load();
    gainDb = pGain->load();
    float peakL = 0.0f;
    float peakR = 0.0f;
    
    // stereo writing
    int numChannels = buffer.getNumChannels();
    int numSamples  = buffer.getNumSamples();

    // need at least 2 channels for stereo panning
    if (numChannels < 2)
        return;
    
    // buffer setup
    auto* leftChannel  = buffer.getWritePointer(0); // left
    auto* rightChannel = buffer.getWritePointer(1); // right

    
    // run for every sample
    for (int i = 0; i < numSamples; i++)
    {
        //LFO phase
        lfoPhase += lfoRate / s_r;
        if (lfoPhase >= 1.0f)
            lfoPhase -= 1.0f;

        // raw LFO shape (-1 to 1)
        float lfoRaw = chooseLFO(lfoPhase, lfoShape);

        // depth controls filter sweep — convert to 0..1
        float lfo = (lfoRaw * lfoDepth + 1.0f) * 0.5f;

        // width controls panning spread only
        float lfoShaped = (lfo - 0.5f) * lfoWidth + 0.5f;

        // FILTER CUTOFFS using LFO (depth)
        float lfoCutoff_l = centerFreq * std::pow(2.0f, (lfo - 0.5f) * sweepOctaves * 2.0f);
        float lfoCutoff_h = centerFreq * std::pow(2.0f, (0.5f - lfo) * sweepOctaves * 2.0f);

        //setting the frequency range for LFO cutoff
        lfoCutoff_l = juce::jlimit(20.0f, 18000.0f, lfoCutoff_l);
        lfoCutoff_h = juce::jlimit(20.0f, 18000.0f, lfoCutoff_h);

        // coefficients (lowpass and highpass filters)
        auto lpfCoefficients = juce::IIRCoefficients::makeLowPass(s_r, lfoCutoff_l);
        auto hpfCoefficients = juce::IIRCoefficients::makeLowPass(s_r, lfoCutoff_h);

        // assign coefficients to the respective filter chains
        for (int s = 0; s < filterStages; s++)
        {
            lpfChainL[s].setCoefficients(lpfCoefficients);
            lpfChainR[s].setCoefficients(hpfCoefficients);
        }

        // mono sum input
        float input = (leftChannel[i] + rightChannel[i]) * 0.5f;

        // filter chains
        float lpfOut   = input;
        float hpfChain = input;
        for (int s = 0; s < filterStages; s++)
        {
            lpfOut   = lpfChainL[s].processSingleSampleRaw(lpfOut);
            hpfChain = lpfChainR[s].processSingleSampleRaw(hpfChain);
        }
        float hpfOut = input - hpfChain;

        // PANNING using LFO Shaped (width)
        float lpfGainL = std::sqrt(1.0f - lfoShaped);
        float lpfGainR = std::sqrt(lfoShaped);
        float hpfGainL = std::sqrt(lfoShaped);
        float hpfGainR = std::sqrt(1.0f - lfoShaped);

        
        // OUTPUT //
        
        // MIX
        // keep the dry signal before effect
        float dryL = leftChannel[i];
        float dryR = rightChannel[i];

        // wet signal from the panner
        float wetL = lpfOut * lpfGainL + hpfOut * hpfGainL;
        float wetR = lpfOut * lpfGainR + hpfOut * hpfGainR;

        // dry/wet mix
        float outL = dryL * (1.0f - mix) + wetL * mix;
        float outR = dryR * (1.0f - mix) + wetR * mix;

        // GAIN
        // gain — convert dB to linear
        float gainLin = std::pow(10.0f, gainDb / 20.0f);
        outL *= gainLin;
        outR *= gainLin;

        leftChannel[i]  = outL;
        rightChannel[i] = outR;
        
        peakL = std::max(peakL, std::abs(outL));
        peakR = std::max(peakR, std::abs(outR));
    }
    levelL.store(peakL);
    levelR.store(peakR);
}

//==============================================================================
bool JiwooSonSpectralPannerAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* JiwooSonSpectralPannerAudioProcessor::createEditor()
{
    return new JiwooSonSpectralPannerAudioProcessorEditor (*this);
}

// declaring function for selecting LFO shape (sine, tri, sq, saw)
float JiwooSonSpectralPannerAudioProcessor::chooseLFO(float phase, int shape)
{
    switch (shape)
    {
        case 0: //sin
            return std::sin(phase * 2.0f * juce::MathConstants<float>::pi);
        case 1: //saw
            return phase * 2.0f - 1.0f;
        case 2: //sq
            return phase < 0.5f ? 1.0f : -1.0f;
        case 3: //tri
            return phase < 0.5f ? phase * 4.0f - 1.0f : 3.0f - phase * 4.0f;
        default:
            return 0.0f;
    }
}
//==============================================================================
void JiwooSonSpectralPannerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    
}

void JiwooSonSpectralPannerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new JiwooSonSpectralPannerAudioProcessor();
}
