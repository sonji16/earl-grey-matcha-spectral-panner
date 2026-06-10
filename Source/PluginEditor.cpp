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

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "PannerlookAndFeel.h"

//==============================================================================

// ===== EARL GREY MATCHA palette =====
static const juce::Colour COL_BG         { 0xFF838F58 };  // matcha olive (brand bg)
static const juce::Colour COL_CARD       { 0xFF6F4E37 };  // brown — card body
static const juce::Colour COL_BORDER     { 0xFF4A3020 };  // dark brown rim
static const juce::Colour COL_TEXT_PRI   { 0xFFFDF6E4 };  // ivory
static const juce::Colour COL_TEXT_SEC   { 0xFFE8DFC8 };  // soft ivory
static const juce::Colour COL_MATCHA     { 0xFFB4C77E };  // bright matcha — LPF / knobs
static const juce::Colour COL_MATCHA_FILL{ 0x33B4C77E };
static const juce::Colour COL_BERGAMOT   { 0xFFD9A441 };  // earl grey amber — HPF
static const juce::Colour COL_BERGAMOT_FILL{ 0x33D9A441 };
static const juce::Colour COL_TEAL       { 0xFFB4C77E };  // matcha for snap knob
static const juce::Colour COL_TRACK_BG   { 0xFF5A6640 };  // muted olive track
static const juce::Colour COL_BLUE       = COL_MATCHA;    // alias so old refs work
static const juce::Colour COL_BLUE_FILL  = COL_MATCHA_FILL;
static const juce::Colour COL_CORAL      = COL_BERGAMOT;
static const juce::Colour COL_CORAL_FILL = COL_BERGAMOT_FILL;

JiwooSonSpectralPannerAudioProcessorEditor::JiwooSonSpectralPannerAudioProcessorEditor (JiwooSonSpectralPannerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setLookAndFeel(&laf);
    setSize (680, 600);
    
    // LFO
    styleRotaryKnob(rateSlider);
    rateSlider.setNumDecimalPlacesToDisplay(2);
    rateSlider.setTextValueSuffix(" Hz");
    
    // DEPTH
    styleRotaryKnob(depthSlider);
    

    // WIDTH
    styleRotaryKnob(widthSlider);
    

    // MIX
    styleRotaryKnob(mixSlider);
    
    
    styleLabel(rateLabel, "Rate");
    styleLabel(depthLabel, "Depth");
    styleLabel(widthLabel, "Width");
    styleLabel(lfoShapeLabel, "Shape");
    
    // LFO shape ComboBox dropdown
    lfoShapeBox.addItem ("Sine", 1);
    lfoShapeBox.addItem ("Saw", 2);
    lfoShapeBox.addItem ("Square",3);
    lfoShapeBox.addItem ("Triangle",4);
    
    
    lfoShapeBox.setSelectedId (1, juce::dontSendNotification);
    
    // setting color
    lfoShapeBox.setColour (juce::ComboBox::backgroundColourId, COL_CARD);
    lfoShapeBox.setColour (juce::ComboBox::textColourId,       COL_TEXT_PRI);
    lfoShapeBox.setColour (juce::ComboBox::outlineColourId,    COL_BORDER);
    lfoShapeBox.setColour (juce::ComboBox::arrowColourId,      COL_TEXT_SEC);
    
    // addmakevisible
    addAndMakeVisible(rateSlider);
    addAndMakeVisible(depthSlider);
    addAndMakeVisible(widthSlider);
    addAndMakeVisible (lfoShapeBox);
    addAndMakeVisible (lfoShapeLabel);
    
    // filter (center freq and filter dB)
    styleRotaryKnob(centerFreqSlider);
    centerFreqSlider.setNumDecimalPlacesToDisplay(0);
    centerFreqSlider.setTextValueSuffix("Hz");
    styleSnapKnob(filterSlopeKnob, 4, {"12dB","24dB","36dB","48dB"});
    styleLabel(centerFreqLabel, "Center \nFrequency");
    styleLabel(filterSlopeLabel,"Filter \nSlope");
    
    //addmakevisible
    addAndMakeVisible(centerFreqSlider);
    addAndMakeVisible(filterSlopeKnob);
    addAndMakeVisible(centerFreqLabel);
    addAndMakeVisible(filterSlopeLabel);
    

    
    styleRotaryKnob(gainSlider);
    gainSlider.setNumDecimalPlacesToDisplay(1);
    gainSlider.setTextValueSuffix(" dB");
    
    styleLabel(mixLabel, "Mix");
    styleLabel(gainLabel, "Gain");
    
    
    //adandmakeitvisible
    addAndMakeVisible(mixSlider);
    addAndMakeVisible(gainSlider);
    addAndMakeVisible(mixLabel);
    addAndMakeVisible(gainLabel);
    
    
    // PARAMETER ATTACHMENTS
    
    rateAttachment = std::make_unique<SliderAttachment>(audioProcessor.parameters, "rate", rateSlider);
    
    depthAttachment = std::make_unique<SliderAttachment>(audioProcessor.parameters, "depth", depthSlider);
    widthAttachment = std::make_unique<SliderAttachment>(audioProcessor.parameters, "width", widthSlider);
    centerFreqAttachment = std::make_unique<SliderAttachment>(audioProcessor.parameters, "centerFreq", centerFreqSlider);

    lfoShapeAttachment = std::make_unique<ComboAttachment>(audioProcessor.parameters, "lfoShape", lfoShapeBox);
    filterSlopeAttachment = std::make_unique<SliderAttachment>(audioProcessor.parameters, "filterSlope", filterSlopeKnob);
    //mix and gain
    mixAttachment  = std::make_unique<SliderAttachment>(audioProcessor.parameters, "mix",  mixSlider);
    gainAttachment = std::make_unique<SliderAttachment>(audioProcessor.parameters, "gain", gainSlider);
    
    // change unit to % (depth, width, mix)
    depthSlider.textFromValueFunction = [](double val)  -> juce::String{
        return juce::String(juce::roundToInt(val * 100)) + "%";
    };
    depthSlider.valueFromTextFunction = [](const juce::String& t) -> double{
        return t.getDoubleValue() / 100.0;
    };
    
    widthSlider.textFromValueFunction = [](double val) -> juce::String{
        return juce::String(juce::roundToInt(val * 100)) + "%";
    };
    widthSlider.valueFromTextFunction = [](const juce::String& t) -> double {
        return t.getDoubleValue() / 100.0;
    };
    
    mixSlider.textFromValueFunction = [](double val) -> juce::String {
        return juce::String(juce::roundToInt(val * 100)) + "%";
    };
    mixSlider.valueFromTextFunction = [](const juce::String& t)-> double {
        return t.getDoubleValue() / 100.0;
    };
    
    
    depthSlider.updateText();
    widthSlider.updateText();
    mixSlider.updateText();
    
    //Timer for lfo graph display (metro)
    startTimerHz(30);
 
}
JiwooSonSpectralPannerAudioProcessorEditor::~JiwooSonSpectralPannerAudioProcessorEditor()
{
    setLookAndFeel(nullptr);  // must clear before LookAndFeel is destroyed
    stopTimer();
}

//==============================================================================
void JiwooSonSpectralPannerAudioProcessorEditor::timerCallback(){
    // read rate directly from the parameter for accuracy
        float rate = *audioProcessor.parameters.getRawParameterValue("rate");

        // increment phase at correct rate
        // timer runs at 30Hz, so each tick advances by rate/30 of a cycle
        lfoDisplayPhase += rate / 30.0f;
        if (lfoDisplayPhase >= 1.0f)
            lfoDisplayPhase -= 1.0f;

        repaint();
}

// paint
void JiwooSonSpectralPannerAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (COL_BG);
     
    // title
    g.setFont(laf.talinaFont.withHeight(26.0f).withExtraKerningFactor(0.05f));
    g.setColour(juce::Colour(0xFFF2cb42));
    g.drawText("EARL GREY MATCHA", 20, 12, 400, 30, juce::Justification::left);

    g.setFont(laf.talinaFont.withHeight(38.0f));
    g.setColour(juce::Colour(0xFFF2cb42));
    g.drawText("SPECTRAL PANNER", 20, 40, 500, 44, juce::Justification::left);

    g.setFont(laf.talinaFont.withHeight(26.0f));
    g.setColour(COL_TEXT_PRI);
    g.drawText("Jiwoo Son", getWidth() - 180, 16, 140, 16, juce::Justification::right);
    auto matchaImg = juce::ImageCache::getFromMemory(
        BinaryData::matcha_cup_png, BinaryData::matcha_cup_pngSize);

    if (matchaImg.isValid())
    {
        const int imgH = 70;
        const int imgW = (int)(matchaImg.getWidth() * (imgH / (float)matchaImg.getHeight()));
        const int imgX = getWidth() - imgW - 14;
        const int imgY = 6;
        g.drawImage(matchaImg, imgX, imgY, imgW, imgH,
                    0, 0, matchaImg.getWidth(), matchaImg.getHeight());
    }
    
    // section cards (background + border + label)
    drawSectionCard (g, {20, 82, 640, 128}, "Frequency display");
    drawSectionCard (g, {20, 222, 640, 178}, "LFO");
    
    drawSectionCard (g, {20, 412, 308, 160}, "Filter");
    drawSectionCard (g, {340, 412, 320, 160}, "Output");
 
    // animated frequency display curves
    drawFrequencyDisplay (g, {20, 82, 640, 128});
 
    // L/R meters inside output card
    drawMeters (g, {328, 412, 320, 160});
    // lfo graph
    g.setFont(laf.talinaFont.withHeight(15.0f));
    g.setColour(COL_TEXT_PRI);
    g.drawText("LFO WAVEFORM", 351, 166, 640, 148,
               juce::Justification::left);

    drawLFOWaveform(g, {360, 246, 250, 140});
    
   
}

void JiwooSonSpectralPannerAudioProcessorEditor::resized()
{
    const int knobW   = 80;   // knob width
    const int knobH   = 80;   // knob height
    const int labelH  = 16;
    const int gapX    = 20;   // gap between knobs horizontally
    const int pad     = 16;   // section left padding

    // ----------------------------------------------------------------
    // LFO SECTION
    // ----------------------------------------------------------------
    int lfoY  = 222;
    int lfoX  = 20 + pad;
    int knobY = lfoY + 30;  // top padding inside card

    // Rate knob
    rateSlider.setBounds (lfoX, knobY, knobW, knobH);
    rateLabel .setBounds (lfoX, knobY + knobH + 2, knobW, labelH);

    // Depth knob
    int depthX = lfoX + knobW + gapX;
    depthSlider.setBounds (depthX, knobY, knobW, knobH);
    depthLabel .setBounds (depthX, knobY + knobH + 2, knobW, labelH);

    // Width knob — after depth
    int widthX = depthX + knobW + gapX;
    widthSlider.setBounds(widthX, knobY, knobW, knobH);
    widthLabel .setBounds(widthX, knobY + knobH + 2, knobW, labelH);
    
    // Shape dropdown — sits to the right of depth, vertically centred

    lfoShapeLabel.setBounds(lfoX-pad-12, knobY + knobH + 22+16, 100, 16);   // "Shape" text
    lfoShapeBox  .setBounds(lfoX+pad + 16+10+pad, knobY + knobH + 22+16, 70, 16);    // dropdown box

    // ----------------------------------------------------------------
    // FILTER SECTION
    // ----------------------------------------------------------------
    int filtY  = 412;
    int filtX  = 20 + pad;
    int fKnobY = filtY + 30;

    // Center freq knob
    centerFreqSlider.setBounds (filtX, fKnobY, knobW, knobH);
    centerFreqLabel .setBounds (filtX, fKnobY + knobH + 2, knobW + 5, labelH *2 + 15);

    // Slope snap knob
    int slopeX = filtX + knobW + gapX;
    filterSlopeKnob .setBounds (slopeX, fKnobY, knobW, knobH);
    filterSlopeLabel.setBounds (slopeX, fKnobY + knobH + 2, knobW, labelH *2 + 16);

    // ----------------------------------------------------------------
    // OUTPUT SECTION
    // ----------------------------------------------------------------
    int outY  = 412;
    int outX  = 340 + pad;
    int oKnobY = outY + 30;

    // Mix knob
    mixSlider.setBounds (outX, oKnobY, knobW, knobH);
    mixLabel .setBounds (outX, oKnobY + knobH + 2, knobW, labelH);

    // Gain knob
    int gainX = outX + knobW + gapX;
    gainSlider.setBounds (gainX, oKnobY, knobW, knobH);
    gainLabel .setBounds (gainX, oKnobY + knobH + 2, knobW, labelH);

}

void JiwooSonSpectralPannerAudioProcessorEditor::drawSectionCard(juce::Graphics& g, juce::Rectangle<int> bounds, const juce::String &title){
    
    g.setColour(COL_CARD);
    g.fillRoundedRectangle(bounds.toFloat(), 8.0f);
    g.setColour(COL_BORDER);
    g.drawRoundedRectangle(bounds.toFloat(), 8.0f, 0.8f);

    g.setFont(laf.talinaFont.withHeight(15.0f).withExtraKerningFactor(0.08f));
    g.setColour(COL_TEXT_PRI);
    g.drawText(title.toUpperCase(),
              bounds.getX() + 12, bounds.getY() + 8, 200, 16,
              juce::Justification::left);
   }
void JiwooSonSpectralPannerAudioProcessorEditor::drawFrequencyDisplay(
    juce::Graphics& g,
    juce::Rectangle<int> bounds)
{
    auto plot = bounds.reduced(14).withTrimmedTop(18).withTrimmedBottom(28);

    float depth = (float)depthSlider.getValue();
    int   shape = lfoShapeBox.getSelectedId() - 1;

    float centerFreq   = (float)centerFreqSlider.getValue();
    float sweepOctaves = *audioProcessor.parameters.getRawParameterValue("sweepOctaves");

    float W   = (float)plot.getWidth();
    float H   = (float)plot.getHeight();
    float px0 = (float)plot.getX();
    float py0 = (float)plot.getY();
    float pyB = (float)plot.getBottom();

    // --- FIXED LOG AXIS 20 Hz – 20 kHz ---
    const float minF = 20.0f, maxF = 20000.0f;
    float minLog = std::log10(minF);
    float maxLog = std::log10(maxF);

    auto freqToX = [=](float f) -> float {
        float fLog = std::log10(juce::jlimit(minF, maxF, f));
        return px0 + (fLog - minLog) / (maxLog - minLog) * W;
    };

    // --- SWEEP RANGE BLOCK ---
    float freqLow  = centerFreq * std::pow(2.0f, -depth * sweepOctaves);
    float freqHigh = centerFreq * std::pow(2.0f,  depth * sweepOctaves);
    float blockL = freqToX(freqLow);
    float blockR = freqToX(freqHigh);

    g.setColour(juce::Colour(0x25FFFFFF));   // white fill, subtle
    g.fillRect(blockL, py0, blockR - blockL, H);
    g.setColour(juce::Colour(0x66FFFFFF));   // white edges
    g.drawVerticalLine((int)blockL, py0, pyB);
    g.drawVerticalLine((int)blockR, py0, pyB);

    // --- GRID LINES ---
    g.setColour(juce::Colour(0x12888780));
    for (float f : {100.0f, 1000.0f, 10000.0f})
        g.drawVerticalLine((int)freqToX(f), py0, pyB);

    // --- CURRENT CROSSOVER FREQUENCY ---
    float lfoRaw;
    switch (shape)
    {
        case 0: lfoRaw = std::sin(lfoDisplayPhase * 2.0f *
                         juce::MathConstants<float>::pi); break;
        case 1: lfoRaw = lfoDisplayPhase * 2.0f - 1.0f; break;
        case 2: lfoRaw = lfoDisplayPhase < 0.5f ? 1.0f : -1.0f; break;
        case 3: lfoRaw = lfoDisplayPhase < 0.5f
                         ? lfoDisplayPhase * 4.0f - 1.0f
                         : 3.0f - lfoDisplayPhase * 4.0f; break;
        default: lfoRaw = 0.0f; break;
    }
    float lfo = (lfoRaw * depth + 1.0f) * 0.5f;
    lfo = juce::jlimit(0.0f, 1.0f, lfo);

    float crossFreq = centerFreq * std::pow(2.0f, (lfo - 0.5f) * sweepOctaves * 2.0f);

    // filter steepness for the curve shape
   // visual steepness
    int slopeChoice = (int)*audioProcessor.parameters.getRawParameterValue("filterSlope");
    // slopeChoice: 0=12dB 1=24dB 2=36dB 3=48dB
    // each stage is one filter order, so order = stages
    float order = (float)(slopeChoice + 1);  // 1, 2, 3, or 4

    // --- LPF CURVE (blue) — rolls off ABOVE crossover ---
    juce::Path lpf;
    lpf.startNewSubPath(px0, pyB);
    for (int px = 0; px <= (int)W; px++)
    {
        float fLog = minLog + ((float)px / W) * (maxLog - minLog);
        float f    = std::pow(10.0f, fLog);
        float gain = 1.0f / (1.0f + std::pow(f / crossFreq, 2.0f * order));
        lpf.lineTo(px0 + px, pyB - gain * H * 0.85f);
    }
    lpf.lineTo(px0 + W, pyB);
    lpf.closeSubPath();
    g.setColour(COL_MATCHA_FILL);
    g.fillPath(lpf);
    g.setColour(COL_MATCHA);
    g.strokePath(lpf, juce::PathStrokeType(1.5f));

    // --- HPF CURVE (coral) — rolls off BELOW crossover ---
    juce::Path hpf;
    hpf.startNewSubPath(px0, pyB);
    for (int px = 0; px <= (int)W; px++)
    {
        float fLog = minLog + ((float)px / W) * (maxLog - minLog);
        float f    = std::pow(10.0f, fLog);
        float gain = 1.0f / (1.0f + std::pow(crossFreq / f, 2.0f * order));
        hpf.lineTo(px0 + px, pyB - gain * H * 0.85f);
    }
    hpf.lineTo(px0 + W, pyB);
    hpf.closeSubPath();
    g.setColour(COL_BERGAMOT_FILL);
    g.fillPath(hpf);
    g.setColour(COL_BERGAMOT);
    g.strokePath(hpf, juce::PathStrokeType(1.5f));

    // --- CROSSOVER LINE (where the curves meet) ---
    float crossX = freqToX(crossFreq);
    float dashLengths[] = {4.0f, 3.0f};
    juce::Path dash;
    dash.startNewSubPath(crossX, py0);
    dash.lineTo(crossX, pyB);
    juce::Path dashedPath;
    juce::PathStrokeType(1.0f).createDashedStroke(dashedPath, dash, dashLengths, 2);
    g.setColour(juce::Colour(0x99888780));
    g.strokePath(dashedPath, juce::PathStrokeType(1.0f));

    // current freq readout
    juce::String freqStr = crossFreq < 1000.0f
        ? juce::String(juce::roundToInt(crossFreq)) + " Hz"
        : juce::String(crossFreq / 1000.0f, 2) + " kHz";
    g.setFont(juce::Font(9.0f));
    g.setColour(COL_TEXT_PRI);
    g.drawText(freqStr,
               juce::jlimit((int)px0, (int)(px0 + W) - 60, (int)crossX - 30),
               (int)py0 - 14, 60, 12, juce::Justification::centred);

    // --- X AXIS LABELS ---
    int labelY = plot.getBottom() + 4;
    g.setFont(juce::Font(9.0f));
    g.setColour(COL_TEXT_SEC);
    auto tick = [&](float f, const juce::String& l) {
        g.drawText(l, (int)freqToX(f) - 25, labelY, 50, 12,
                   juce::Justification::centred);
    };
    tick(20.0f, "20");
    tick(100.0f, "100");
    tick(1000.0f, "1k");
    tick(10000.0f, "10k");
    tick(20000.0f, "20k");

    g.setColour(juce::Colour(COL_TEXT_PRI));
    g.drawText("Frequency (Hz)", plot.getX(), labelY + 12,
               140, 12, juce::Justification::left);

    auto fmt = [](float f) -> juce::String {
        return f < 1000.0f ? juce::String(juce::roundToInt(f)) + "Hz"
                           : juce::String(f / 1000.0f, 1) + "kHz";
    };
    g.setColour(COL_TEXT_PRI);
    g.drawText("Sweep: " + fmt(freqLow) + " - " + fmt(freqHigh),
               plot.getRight() - 160, labelY + 12, 160, 12,
               juce::Justification::right);
}
//------------------------------------------------------------
void JiwooSonSpectralPannerAudioProcessorEditor::drawLFOWaveform(
    juce::Graphics& g,
    juce::Rectangle<int> bounds)
{
    auto plot = bounds.reduced(12).withTrimmedTop(18);

    float W     = (float)plot.getWidth();
    float H     = (float)plot.getHeight();
    float px0   = (float)plot.getX();
    float midY  = plot.getY() + H * 0.5f;
    float depth = (float)depthSlider.getValue();
    float rate  = (float)rateSlider.getValue();
    int   shape = lfoShapeBox.getSelectedId() - 1;

    // how many cycles to show — scales with rate
    // at low rate show 1 cycle, at high rate show more
    float cyclesShown = juce::jlimit(1.0f, 8.0f, rate);

    // center line
    g.setColour(juce::Colour(0x20888780));
    g.drawHorizontalLine((int)midY, px0, px0 + W);

    // --- WAVEFORM ---
    juce::Path wave;
    for (int px = 0; px <= (int)W; px++)
    {
        // map pixel to phase, multiplied by cycles shown
        float phase = std::fmod((float)px / W * cyclesShown, 1.0f);
        float val;

        switch (shape)
        {
            case 0:  val = std::sin(phase * 2.0f *
                          juce::MathConstants<float>::pi); break;
            case 1:  val = phase * 2.0f - 1.0f; break;
            case 2:  val = phase < 0.5f ? 1.0f : -1.0f; break;
            case 3:  val = phase < 0.5f
                          ? phase * 4.0f - 1.0f
                          : 3.0f - phase * 4.0f; break;
            default: val = 0.0f; break;
        }

        float y = midY - val * depth * (H * 0.45f);

        if (px == 0)
            wave.startNewSubPath(px0, y);
        else
            wave.lineTo(px0 + px, y);
    }

    g.setColour(COL_MATCHA);
    g.strokePath(wave, juce::PathStrokeType(2.0f));

    // --- PLAYHEAD DOT ---
    // dot position scaled by cycles shown so it matches the visible wave
    float dotPhase = std::fmod(lfoDisplayPhase * cyclesShown, 1.0f);
    float dotVal;
    switch (shape)
    {
        case 0:  dotVal = std::sin(dotPhase * 2.0f *
                          juce::MathConstants<float>::pi); break;
        case 1:  dotVal = dotPhase * 2.0f - 1.0f; break;
        case 2:  dotVal = dotPhase < 0.5f ? 1.0f : -1.0f; break;
        case 3:  dotVal = dotPhase < 0.5f
                          ? dotPhase * 4.0f - 1.0f
                          : 3.0f - dotPhase * 4.0f; break;
        default: dotVal = 0.0f; break;
    }

    // dot X position follows the actual phase across the full width
    float dotX = px0 + (lfoDisplayPhase * cyclesShown -
                 std::floor(lfoDisplayPhase * cyclesShown)) * (W / cyclesShown)
                 + std::floor(lfoDisplayPhase * cyclesShown) * (W / cyclesShown);


    float dotY = midY - dotVal * depth * (H * 0.45f);

    g.setColour(COL_BERGAMOT);
    g.fillEllipse(dotX - 4.0f, dotY - 4.0f, 8.0f, 8.0f);
    
    // --- AXIS LABELS ---
    g.setFont(juce::Font(9.0f));
    g.setColour(COL_TEXT_SEC);

    // Y axis — amplitude
    g.drawText("+1", (int)px0 - 22, (int)plot.getY() - 2,
               20, 12, juce::Justification::right);
    g.drawText("0",  (int)px0 - 22, (int)midY - 6,
               20, 12, juce::Justification::right);
    g.drawText("-1", (int)px0 - 12, (int)plot.getBottom() - 10,
               20, 12, juce::Justification::right);
    g.drawText("Amplitude", (int)px0 - 22, (int)plot.getY() - 14,
               70, 12, juce::Justification::left);

    // time axis labels with actual values
    float totalTime = cyclesShown / rate;  // total seconds shown
    g.setFont(juce::Font(9.0f));
    g.setColour(COL_TEXT_SEC);

    // start = 0
    g.drawText("0s", (int)px0, (int)plot.getBottom() + 2,
               30, 12, juce::Justification::left);

    // end = total time
    juce::String timeStr = totalTime >= 1.0f
        ? juce::String(totalTime, 1) + "s"
        : juce::String(juce::roundToInt(totalTime * 1000)) + "ms";
    g.drawText(timeStr, (int)(px0 + W) - 35, (int)plot.getBottom() + 2,
               35, 12, juce::Justification::right);
    // X axis — time
    //g.drawText("Time", (int)px0, (int)plot.getBottom() + 2,
     //          80, 12, juce::Justification::right);
}
// ----------------------------------------------------------------------------
void JiwooSonSpectralPannerAudioProcessorEditor::drawMeters(
    juce::Graphics& g,
    juce::Rectangle<int> bounds)
{
    int mx   = bounds.getX() + bounds.getWidth() - 28;
    int mTop = bounds.getY() + 30;
    int mH   = 100;
    int mW   = 8;

    // read real levels from processor
    float levelL = audioProcessor.getLevelL();
    float levelR = audioProcessor.getLevelR();

    // clamp 0..1 for display
    levelL = juce::jlimit(0.0f, 1.0f, levelL);
    levelR = juce::jlimit(0.0f, 1.0f, levelR);

    // L bar background
    g.setColour(COL_TRACK_BG);
    g.fillRoundedRectangle((float)mx, (float)mTop,
                           (float)mW, (float)mH, 3.0f);
    // L bar fill — grows from bottom
    float fillH_L = levelL * mH;
    g.setColour(COL_MATCHA);
    g.fillRoundedRectangle((float)mx, (float)(mTop + mH - fillH_L),
                           (float)mW, fillH_L, 3.0f);

    // R bar background
    g.setColour(COL_TRACK_BG);
    g.fillRoundedRectangle((float)(mx + 12), (float)mTop,
                           (float)mW, (float)mH, 3.0f);
    // R bar fill
    float fillH_R = levelR * mH;
    g.setColour(COL_BERGAMOT);
    g.fillRoundedRectangle((float)(mx + 12), (float)(mTop + mH - fillH_R),
                           (float)mW, fillH_R, 3.0f);

    // L / R labels
    g.setFont(juce::Font(9.0f));
    g.setColour(COL_TEXT_SEC);
    g.drawText("L", mx - 1, mTop + mH + 3, mW, 12,
               juce::Justification::centred);
    g.drawText("R", mx + 11, mTop + mH + 3, mW, 12,
               juce::Justification::centred);
}
 
// ============================================================================
// STYLE HELPERS
// ============================================================================
void JiwooSonSpectralPannerAudioProcessorEditor::styleRotaryKnob (
    juce::Slider& s)
{
    s.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 72, 18);
    s.setNumDecimalPlacesToDisplay(2);
    // only set the arc color — LookAndFeel draws everything else
    s.setColour(juce::Slider::rotarySliderFillColourId,
                juce::Colour(PannerLookAndFeel::colMatcha));
}
 
// ----------------------------------------------------------------------------
void JiwooSonSpectralPannerAudioProcessorEditor::styleSnapKnob (
    juce::Slider& s,
    int numSteps,
    const juce::StringArray& labels)
{
    s.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 72, 18);
    s.setRange(0.0, (double)(numSteps - 1), 1.0);
    // bergamot arc to distinguish snap knobs
    s.setColour(juce::Slider::rotarySliderFillColourId,
                juce::Colour(PannerLookAndFeel::colMatcha));
    s.textFromValueFunction = [labels](double val) -> juce::String {
        int idx = juce::jlimit(0, labels.size() - 1, (int)std::round(val));
        return labels[idx];
    };

}
 
// ----------------------------------------------------------------------------
void JiwooSonSpectralPannerAudioProcessorEditor::styleLabel (
    juce::Label& l,
    const juce::String& text)
{
    l.setText(text, juce::dontSendNotification);
      l.setFont(laf.talinaFont.withHeight(15.0f).withExtraKerningFactor(0.05f));
      l.setColour(juce::Label::textColourId, COL_TEXT_PRI);
      l.setJustificationType(juce::Justification::centred);
      addAndMakeVisible(l);
}
 
// ----------------------------------------------------------------------------
/*
void JiwooSonSpectralPannerAudioProcessorEditor::styleButton (
    juce::TextButton& b,
    bool active)
{
    b.setColour (juce::TextButton::buttonColourId,
                 active ? COL_BTN_ACTIVE_BG : juce::Colours::transparentWhite);
    b.setColour (juce::TextButton::textColourOffId,
                 active ? COL_BTN_ACTIVE_TEXT : COL_TEXT_SEC);
    b.setColour (juce::ComboBox::outlineColourId,
                 active ? COL_BTN_ACTIVE_TEXT
                        : juce::Colour (0x4D888780));
}
 */
