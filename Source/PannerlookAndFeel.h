#pragma once
#include <JuceHeader.h>

class PannerLookAndFeel : public juce::LookAndFeel_V4
{
public:
    // ===== EARL GREY MATCHA palette =====
    static constexpr uint32_t colBackground  = 0xFF838F58;  // matcha olive
    static constexpr uint32_t colSurface     = 0xFF6F4E37;  // brown knob body
    static constexpr uint32_t colBorder      = 0xFF4A3020;  // dark brown rim
    static constexpr uint32_t colMatcha      = 0xFFB4C77E;  // bright matcha — arc/dot
    static constexpr uint32_t colBergamot    = 0xFFD9A441;  // earl grey amber
    static constexpr uint32_t colTextPrimary = 0xFFFDF6E4;  // ivory
    static constexpr uint32_t colTrackBg     = 0xFF5A6640;  // muted olive arc
    static constexpr uint32_t colHighlight   = 0x18FFFFFF;  // glint

    juce::Font talinaFont;

    PannerLookAndFeel()
    {
        auto typeface = juce::Typeface::createSystemTypefaceFor(
            BinaryData::Talina_DEMO_otf,
            BinaryData::Talina_DEMO_otfSize);
        talinaFont = juce::Font(typeface);

        setColour(juce::ResizableWindow::backgroundColourId,
                  juce::Colour(colBackground));
        setColour(juce::Slider::textBoxTextColourId,
                  juce::Colour(colTextPrimary));
        setColour(juce::Slider::textBoxBackgroundColourId,
                  juce::Colour(0x00000000));
        setColour(juce::Slider::textBoxOutlineColourId,
                  juce::Colour(0x00000000));
        setColour(juce::Label::textColourId,
                  juce::Colour(colTextPrimary));
    }

    // ===== Rotary knob — brown body, matcha arc =====
    void drawRotarySlider(juce::Graphics& g,
                          int x, int y, int width, int height,
                          float sliderPos,
                          float startAngle, float endAngle,
                          juce::Slider& slider) override
    {
        const float cx = x + width * 0.5f;
        const float cy = y + height * 0.5f;
        const float radius = juce::jmin(width, height) * 0.5f - 4.0f;

        // pick arc color: matcha by default, but snap knobs can override
        juce::Colour arcColour = slider.findColour(
            juce::Slider::rotarySliderFillColourId);

        // track arc (muted olive)
        juce::Path trackArc;
        trackArc.addCentredArc(cx, cy, radius, radius, 0.0f,
                               startAngle, endAngle, true);
        g.setColour(juce::Colour(colTrackBg));
        g.strokePath(trackArc, juce::PathStrokeType(3.0f,
                     juce::PathStrokeType::curved,
                     juce::PathStrokeType::rounded));

        // fill arc
        const float angle = startAngle + sliderPos * (endAngle - startAngle);
        juce::Path fillArc;
        fillArc.addCentredArc(cx, cy, radius, radius, 0.0f,
                              startAngle, angle, true);
        g.setColour(arcColour);
        g.strokePath(fillArc, juce::PathStrokeType(3.0f,
                     juce::PathStrokeType::curved,
                     juce::PathStrokeType::rounded));

        // knob body (brown)
        const float bodyR = radius * 0.72f;
        g.setColour(juce::Colour(colSurface));
        g.fillEllipse(cx - bodyR, cy - bodyR, bodyR * 2.0f, bodyR * 2.0f);
        g.setColour(juce::Colour(colBorder));
        g.drawEllipse(cx - bodyR, cy - bodyR, bodyR * 2.0f, bodyR * 2.0f, 1.0f);

        // glint
        juce::Path glint;
        glint.addCentredArc(cx, cy - bodyR * 0.25f,
                            bodyR * 0.45f, bodyR * 0.28f, 0.0f,
                            juce::MathConstants<float>::pi * 1.1f,
                            juce::MathConstants<float>::pi * 1.9f, true);
        g.setColour(juce::Colour(colHighlight));
        g.strokePath(glint, juce::PathStrokeType(1.0f));

        // indicator dot
        const float dotAngle = angle - juce::MathConstants<float>::halfPi;
        const float dotR = bodyR * 0.82f;
        const float dotX = cx + dotR * std::cos(dotAngle);
        const float dotY = cy + dotR * std::sin(dotAngle);
        g.setColour(arcColour);
        g.fillEllipse(dotX - 4.0f, dotY - 4.0f, 8.0f, 8.0f);
    }

    juce::Label* createSliderTextBox(juce::Slider& slider) override
    {
        auto* label = LookAndFeel_V4::createSliderTextBox(slider);
        label->setFont(talinaFont.withHeight(15.0f));
        label->setJustificationType(juce::Justification::centred);
        return label;
    }
};
