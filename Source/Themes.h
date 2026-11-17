#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

// ============================================================================
//  Theme system: each theme is a palette + a background pattern + a knob
//  style. PaisleyLookAndFeel renders every control from the active theme.
// ============================================================================

struct Theme
{
    enum class Pattern { Rays, Rings, Circles, Waves, Blobs, Stripes };
    enum class Knob    { ChickenHead, NeonRing, Bakelite, SoftDot };

    juce::String name;
    juce::Colour bgTop, bgBottom, patternA, patternB;
    juce::Colour panel, panelEdge, title, text, dimText;
    juce::Colour accent, accent2, knobBody, knobPointer, track;
    Pattern pattern;
    Knob knob;
    float corner;
    bool uppercaseTitles;
};

inline const std::vector<Theme>& getThemes()
{
    using C = juce::Colour;
    static const std::vector<Theme> themes =
    {
        { "Sunburst '73",
          C (0xff3b1f0e), C (0xff7a3412), C (0x30ffb347), C (0x18ff6a00),
          C (0xcc2a160a), C (0xffd98a3a), C (0xfff6c768), C (0xfff3e2c3), C (0xffb79a76),
          C (0xffff8c1a), C (0xffe8452c), C (0xff1d120b), C (0xfff3e2c3), C (0xff5a3a20),
          Theme::Pattern::Rays, Theme::Knob::ChickenHead, 10.0f, true },

        { "Kaleido Neon",
          C (0xff12002a), C (0xff3a0752), C (0x40ff2bd6), C (0x3000f0ff),
          C (0xb3150628), C (0xffff2bd6), C (0xff00f0ff), C (0xfff4e9ff), C (0xffa98ac9),
          C (0xffff2bd6), C (0xff00f0ff), C (0xff1c0b33), C (0xff00f0ff), C (0xff3d2363),
          Theme::Pattern::Rings, Theme::Knob::NeonRing, 14.0f, true },

        { "Space Age Library",
          C (0xffede4cf), C (0xffd9cdb0), C (0x30cc2b1d), C (0x22111111),
          C (0xccf6f0e1), C (0xff222222), C (0xffcc2b1d), C (0xff1b1b1b), C (0xff6d6656),
          C (0xffcc2b1d), C (0xff2a6f8a), C (0xff1a1a1a), C (0xfff6f0e1), C (0xffbdb39b),
          Theme::Pattern::Circles, Theme::Knob::Bakelite, 3.0f, true },

        { "Dream Pop Pastel",
          C (0xffc9b6f2), C (0xfff7c6c0), C (0x40ffffff), C (0x30a07de0),
          C (0x99ffffff), C (0xffffffff), C (0xff6b4fa8), C (0xff4b3a6e), C (0xff8d7bb0),
          C (0xff9d7be6), C (0xfff08aa0), C (0xfffdfaff), C (0xff9d7be6), C (0xffe3d6f7),
          Theme::Pattern::Waves, Theme::Knob::SoftDot, 18.0f, false },

        { "Oil Light Show",
          C (0xff050505), C (0xff120818), C (0x55ff3b30), C (0x5530c0ff),
          C (0xbb0b0b0f), C (0xff6b4cff), C (0xffffd23f), C (0xffefeff5), C (0xff8a8a99),
          C (0xffffd23f), C (0xffff4f8b), C (0xff17171d), C (0xffffd23f), C (0xff2a2a35),
          Theme::Pattern::Blobs, Theme::Knob::NeonRing, 8.0f, true },

        { "Desert Fuzz",
          C (0xff1f4a4a), C (0xff12302f), C (0x26e8b54a), C (0x1ae36b3b),
          C (0xcc0f2827), C (0xffe8b54a), C (0xffe8b54a), C (0xfff0e6cc), C (0xff9db8ad),
          C (0xffe8b54a), C (0xffe36b3b), C (0xff0b1b1b), C (0xfff0e6cc), C (0xff2c5654),
          Theme::Pattern::Stripes, Theme::Knob::ChickenHead, 6.0f, true },
    };
    return themes;
}

// ---------------------------------------------------------------------------
class PaisleyLookAndFeel : public juce::LookAndFeel_V4
{
public:
    PaisleyLookAndFeel() { setTheme (0); }

    const Theme& theme() const { return getThemes()[(size_t) themeIndex]; }
    int getThemeIndex() const { return themeIndex; }

    void setTheme (int index)
    {
        themeIndex = juce::jlimit (0, (int) getThemes().size() - 1, index);
        const auto& t = theme();
        const auto onBody = t.knobBody.contrasting (1.0f).interpolatedWith (t.knobBody.getPerceivedBrightness() > 0.5f ? t.text : t.knobPointer, 0.25f);
        const auto blackKey = t.knobBody.getPerceivedBrightness() > 0.5f ? t.text : t.knobBody;
        setColour (juce::Label::textColourId, t.text);
        setColour (juce::ComboBox::backgroundColourId, t.knobBody);
        setColour (juce::ComboBox::textColourId, onBody);
        setColour (juce::ComboBox::outlineColourId, t.panelEdge.withAlpha (0.6f));
        setColour (juce::ComboBox::arrowColourId, t.accent);
        setColour (juce::PopupMenu::backgroundColourId, t.knobBody.withAlpha (0.98f));
        setColour (juce::PopupMenu::textColourId, onBody);
        setColour (juce::PopupMenu::highlightedBackgroundColourId, t.accent);
        setColour (juce::PopupMenu::highlightedTextColourId, t.knobBody);
        setColour (juce::PopupMenu::headerTextColourId, t.title);
        setColour (juce::TextButton::buttonColourId, t.knobBody);
        setColour (juce::TextButton::buttonOnColourId, t.accent);
        setColour (juce::TextButton::textColourOffId, onBody);
        setColour (juce::TextButton::textColourOnId, t.knobBody);
        setColour (juce::Slider::textBoxTextColourId, onBody);
        setColour (juce::Label::textWhenEditingColourId, onBody);
        setColour (juce::BubbleComponent::backgroundColourId, t.knobBody);
        setColour (juce::BubbleComponent::outlineColourId, t.accent);
        setColour (juce::TooltipWindow::backgroundColourId, t.knobBody);
        setColour (juce::TooltipWindow::textColourId, onBody);
        setColour (juce::AlertWindow::backgroundColourId, t.knobBody);
        setColour (juce::AlertWindow::textColourId, onBody);
        setColour (juce::TextEditor::backgroundColourId, t.panel.withAlpha (1.0f));
        setColour (juce::TextEditor::textColourId, t.text);
        setColour (juce::TextEditor::outlineColourId, t.panelEdge);
        setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId, t.accent.withAlpha (0.7f));
        setColour (juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, t.accent2.withAlpha (0.35f));
        setColour (juce::MidiKeyboardComponent::whiteNoteColourId, t.knobPointer.interpolatedWith (juce::Colours::white, 0.85f));
        setColour (juce::MidiKeyboardComponent::blackNoteColourId, blackKey);
        setColour (juce::MidiKeyboardComponent::keySeparatorLineColourId, t.knobBody.withAlpha (0.5f));
        setColour (juce::MidiKeyboardComponent::shadowColourId, juce::Colours::transparentBlack);
        setColour (juce::MidiKeyboardComponent::upDownButtonBackgroundColourId, t.knobBody);
        setColour (juce::MidiKeyboardComponent::upDownButtonArrowColourId, t.accent);
    }

    juce::Font titleFont (float h) const
    {
        return juce::Font (juce::FontOptions (h, juce::Font::bold)).withExtraKerningFactor (theme().uppercaseTitles ? 0.12f : 0.03f);
    }

    // ------------------------------------------------------------ background
    void drawBackground (juce::Graphics& g, juce::Rectangle<float> r) const
    {
        const auto& t = theme();
        g.setGradientFill (juce::ColourGradient (t.bgTop, r.getTopLeft(), t.bgBottom, r.getBottomRight(), false));
        g.fillRect (r);
        const auto c = r.getCentre();
        const float maxR = std::hypot (r.getWidth(), r.getHeight());
        juce::Random rnd (1969);

        switch (t.pattern)
        {
            case Theme::Pattern::Rays:
            {
                const auto o = juce::Point<float> (r.getCentreX(), r.getBottom() + 60.0f);
                for (int i = 0; i < 36; ++i)
                {
                    const float a0 = juce::MathConstants<float>::twoPi * i / 36.0f;
                    const float a1 = a0 + juce::MathConstants<float>::twoPi / 72.0f;
                    juce::Path p; p.startNewSubPath (o);
                    p.lineTo (o + juce::Point<float> (std::sin (a0), -std::cos (a0)) * maxR);
                    p.lineTo (o + juce::Point<float> (std::sin (a1), -std::cos (a1)) * maxR);
                    p.closeSubPath();
                    g.setColour (i % 2 ? t.patternA : t.patternB);
                    g.fillPath (p);
                }
                for (int i = 6; i > 0; --i)
                {
                    g.setColour ((i % 2 ? t.patternA : t.patternB).withMultipliedAlpha (1.4f));
                    const float rad = 80.0f * i;
                    g.fillEllipse (o.x - rad, o.y - rad, rad * 2, rad * 2);
                }
                break;
            }
            case Theme::Pattern::Rings:
            {
                for (int i = 40; i > 0; --i)
                {
                    const float rad = i * maxR / 40.0f * 0.6f;
                    g.setColour ((i % 2 ? t.patternA : t.patternB).withMultipliedAlpha (0.6f));
                    g.drawEllipse (c.x - rad, c.y - rad * 0.8f, rad * 2, rad * 1.6f, 3.0f);
                }
                for (int k = 0; k < 12; ++k)
                {   // kaleidoscope petals
                    juce::Path petal;
                    petal.addEllipse (-40.0f, -maxR * 0.45f, 80.0f, maxR * 0.45f);
                    g.setColour (t.patternA.withMultipliedAlpha (0.35f));
                    g.fillPath (petal, juce::AffineTransform::rotation (juce::MathConstants<float>::twoPi * k / 12.0f).translated (c));
                }
                break;
            }
            case Theme::Pattern::Circles:
            {
                const float s = 90.0f;
                for (float y = r.getY() - s; y < r.getBottom() + s; y += s)
                    for (float x = r.getX() - s; x < r.getRight() + s; x += s)
                    {
                        const int k = rnd.nextInt (4);
                        g.setColour (k == 0 ? t.patternA : t.patternB.withMultipliedAlpha (0.5f));
                        if (k < 2) g.fillEllipse (x + 10, y + 10, s - 20, s - 20);
                        else if (k == 2) g.drawEllipse (x + 18, y + 18, s - 36, s - 36, 6.0f);
                        else g.fillRect (x + s * 0.5f - 3, y + 8, 6.0f, s - 16);
                    }
                break;
            }
            case Theme::Pattern::Waves:
            {
                for (int i = 0; i < 14; ++i)
                {
                    juce::Path p;
                    const float base = r.getY() + r.getHeight() * (i + 0.5f) / 14.0f;
                    p.startNewSubPath (r.getX(), base);
                    for (float x = r.getX(); x <= r.getRight(); x += 8.0f)
                        p.lineTo (x, base + 22.0f * std::sin (x * 0.008f + i * 0.7f) + 10.0f * std::sin (x * 0.021f - i));
                    g.setColour (i % 2 ? t.patternA : t.patternB);
                    g.strokePath (p, juce::PathStrokeType (14.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
                }
                break;
            }
            case Theme::Pattern::Blobs:
            {
                const juce::Colour cols[] = { t.patternA, t.patternB, t.accent.withAlpha (0.35f), t.accent2.withAlpha (0.35f), t.panelEdge.withAlpha (0.35f) };
                for (int i = 0; i < 26; ++i)
                {
                    const float x = r.getX() + rnd.nextFloat() * r.getWidth();
                    const float y = r.getY() + rnd.nextFloat() * r.getHeight();
                    const float rad = 60.0f + rnd.nextFloat() * 220.0f;
                    auto col = cols[i % 5];
                    g.setGradientFill (juce::ColourGradient (col, x, y, col.withAlpha (0.0f), x + rad, y, true));
                    g.fillEllipse (x - rad, y - rad, rad * 2, rad * 2);
                }
                break;
            }
            case Theme::Pattern::Stripes:
            {
                for (int i = 0; i < 9; ++i)
                {
                    juce::Path p;
                    const float base = r.getY() + r.getHeight() * (0.15f + i * 0.1f);
                    p.startNewSubPath (r.getX(), base);
                    for (float x = r.getX(); x <= r.getRight(); x += 10.0f)
                        p.lineTo (x, base + 40.0f * std::sin (x * 0.004f + i * 0.35f));
                    g.setColour (i % 3 == 0 ? t.patternB : t.patternA);
                    g.strokePath (p, juce::PathStrokeType (28.0f));
                }
                break;
            }
        }
    }

    void drawPanel (juce::Graphics& g, juce::Rectangle<float> r, const juce::String& title, bool active) const
    {
        const auto& t = theme();
        g.setColour (juce::Colours::black.withAlpha (0.25f));
        g.fillRoundedRectangle (r.translated (0, 3), t.corner);
        g.setColour (t.panel);
        g.fillRoundedRectangle (r, t.corner);
        g.setColour (t.panelEdge.withAlpha (active ? 0.9f : 0.35f));
        g.drawRoundedRectangle (r.reduced (0.5f), t.corner, 1.4f);
        g.setColour (active ? t.title : t.dimText);
        g.setFont (titleFont (13.0f));
        g.drawText (t.uppercaseTitles ? title.toUpperCase() : title, r.reduced (12, 4).withHeight (18),
                    juce::Justification::centredLeft);
    }

    // ----------------------------------------------------------------- knobs
    void drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos,
                           float startA, float endA, juce::Slider& s) override
    {
        const auto& t = theme();
        auto r = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (3.0f);
        const float size = juce::jmin (r.getWidth(), r.getHeight());
        r = r.withSizeKeepingCentre (size, size);
        const auto c = r.getCentre();
        const float rad = size * 0.5f;
        const float ang = startA + pos * (endA - startA);
        const bool bipolar = s.getMinimum() < 0 && s.getMaximum() > 0;
        const float zeroAng = bipolar ? startA + (float) s.valueToProportionOfLength (0.0) * (endA - startA) : startA;

        auto arc = [&] (float radius, float a0, float a1, juce::Colour col, float thick)
        {
            juce::Path p; p.addCentredArc (c.x, c.y, radius, radius, 0, a0, a1, true);
            g.setColour (col); g.strokePath (p, juce::PathStrokeType (thick, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        };
        auto pointerAt = [&] (float r0, float r1) { return std::make_pair (c.getPointOnCircumference (r0, ang), c.getPointOnCircumference (r1, ang)); };

        switch (t.knob)
        {
            case Theme::Knob::ChickenHead:
            {
                for (int i = 0; i <= 10; ++i)
                {   // scale ticks
                    const float a = startA + (endA - startA) * i / 10.0f;
                    g.setColour (t.dimText.withAlpha (0.8f));
                    g.drawLine ({ c.getPointOnCircumference (rad * 0.86f, a), c.getPointOnCircumference (rad, a) }, 1.2f);
                }
                arc (rad * 0.78f, juce::jmin (zeroAng, ang), juce::jmax (zeroAng, ang), t.accent.withAlpha (0.8f), 2.0f);
                g.setColour (t.knobBody);
                g.fillEllipse (r.reduced (rad * 0.3f));
                juce::Path head; // pointer-shaped "chicken head" knob
                const float hl = rad * 0.72f, hw = rad * 0.26f;
                head.startNewSubPath (0, -hl);
                head.quadraticTo (hw, -hl * 0.2f, hw * 1.2f, hl * 0.35f);
                head.quadraticTo (0, hl * 0.6f, -hw * 1.2f, hl * 0.35f);
                head.quadraticTo (-hw, -hl * 0.2f, 0, -hl);
                head.closeSubPath();
                const auto tr = juce::AffineTransform::rotation (ang).translated (c);
                g.setColour (juce::Colours::black.withAlpha (0.3f));
                g.fillPath (head, tr.translated (1.5f, 2.0f));
                g.setColour (t.knobBody.brighter (0.15f));
                g.fillPath (head, tr);
                g.setColour (t.knobPointer);
                auto [p0, p1] = pointerAt (rad * 0.05f, rad * 0.66f);
                g.drawLine ({ p0, p1 }, 2.2f);
                break;
            }
            case Theme::Knob::NeonRing:
            {
                arc (rad * 0.86f, startA, endA, t.track, 4.0f);
                for (int glow = 3; glow > 0; --glow)
                    arc (rad * 0.86f, juce::jmin (zeroAng, ang), juce::jmax (zeroAng, ang), t.accent.withAlpha (0.18f), 4.0f + glow * 3.0f);
                arc (rad * 0.86f, juce::jmin (zeroAng, ang), juce::jmax (zeroAng, ang), t.accent, 3.5f);
                g.setColour (t.knobBody);
                g.fillEllipse (r.reduced (rad * 0.3f));
                g.setColour (t.accent2.withAlpha (0.5f));
                g.drawEllipse (r.reduced (rad * 0.3f), 1.0f);
                auto [p0, p1] = pointerAt (rad * 0.25f, rad * 0.62f);
                g.setColour (t.knobPointer);
                g.drawLine ({ p0, p1 }, 2.5f);
                break;
            }
            case Theme::Knob::Bakelite:
            {
                arc (rad * 0.92f, startA, endA, t.track, 1.5f);
                arc (rad * 0.92f, juce::jmin (zeroAng, ang), juce::jmax (zeroAng, ang), t.accent, 3.0f);
                auto body = r.reduced (rad * 0.18f);
                g.setGradientFill (juce::ColourGradient (t.knobBody.brighter (0.35f), body.getTopLeft(), t.knobBody, body.getBottomRight(), false));
                g.fillEllipse (body);
                g.setColour (t.knobBody.brighter (0.6f).withAlpha (0.5f));
                for (int i = 0; i < 24; ++i)
                {   // knurled edge
                    const float a = juce::MathConstants<float>::twoPi * i / 24.0f + ang;
                    g.drawLine ({ c.getPointOnCircumference (rad * 0.66f, a), c.getPointOnCircumference (rad * 0.8f, a) }, 1.0f);
                }
                auto [p0, p1] = pointerAt (rad * 0.4f, rad * 0.62f);
                g.setColour (t.knobPointer);
                g.fillEllipse (juce::Rectangle<float> (5.0f, 5.0f).withCentre (p1));
                juce::ignoreUnused (p0);
                break;
            }
            case Theme::Knob::SoftDot:
            {
                arc (rad * 0.88f, startA, endA, t.track, 5.0f);
                arc (rad * 0.88f, juce::jmin (zeroAng, ang), juce::jmax (zeroAng, ang), t.accent, 5.0f);
                auto body = r.reduced (rad * 0.28f);
                g.setColour (juce::Colours::black.withAlpha (0.08f));
                g.fillEllipse (body.translated (0, 2));
                g.setColour (t.knobBody);
                g.fillEllipse (body);
                auto [p0, p1] = pointerAt (0, rad * 0.5f);
                juce::ignoreUnused (p0);
                g.setColour (t.accent2);
                g.fillEllipse (juce::Rectangle<float> (6.0f, 6.0f).withCentre (p1));
                break;
            }
        }
    }

    // --------------------------------------------------------------- toggles
    void drawToggleButton (juce::Graphics& g, juce::ToggleButton& b, bool over, bool) override
    {
        const auto& t = theme();
        auto r = b.getLocalBounds().toFloat();
        const bool on = b.getToggleState();
        auto pill = r.withSizeKeepingCentre (juce::jmin (r.getWidth() - 4, 34.0f), juce::jmin (r.getHeight() - 4, 16.0f));
        g.setColour (on ? t.accent : t.track);
        g.fillRoundedRectangle (pill, pill.getHeight() * 0.5f);
        if (over) { g.setColour (t.text.withAlpha (0.15f)); g.fillRoundedRectangle (pill, pill.getHeight() * 0.5f); }
        const float d = pill.getHeight() - 4;
        auto dot = juce::Rectangle<float> (d, d).withCentre ({ on ? pill.getRight() - d * 0.5f - 2 : pill.getX() + d * 0.5f + 2, pill.getCentreY() });
        g.setColour (on ? t.knobBody : t.dimText);
        g.fillEllipse (dot);
    }

    void drawComboBox (juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox& box) override
    {
        const auto& t = theme();
        auto r = juce::Rectangle<float> (0, 0, (float) w, (float) h).reduced (0.5f);
        g.setColour (t.knobBody);
        g.fillRoundedRectangle (r, juce::jmin (6.0f, t.corner));
        g.setColour (box.hasKeyboardFocus (false) ? t.accent : t.panelEdge.withAlpha (0.5f));
        g.drawRoundedRectangle (r, juce::jmin (6.0f, t.corner), 1.0f);
        juce::Path arrow;
        const float ax = (float) w - 10.0f, ay = (float) h * 0.5f;
        arrow.addTriangle (ax - 4, ay - 2, ax + 4, ay - 2, ax, ay + 3);
        g.setColour (t.accent);
        g.fillPath (arrow);
    }

    juce::Font getComboBoxFont (juce::ComboBox&) override { return juce::Font (juce::FontOptions (12.0f)); }

    void positionComboBoxText (juce::ComboBox& box, juce::Label& label) override
    {
        label.setBounds (2, 1, box.getWidth() - 16, box.getHeight() - 2);
        label.setFont (getComboBoxFont (box));
    }

    void drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&, bool over, bool down) override
    {
        const auto& t = theme();
        auto r = b.getLocalBounds().toFloat().reduced (0.5f);
        g.setColour (down ? t.accent : (over ? t.knobBody.brighter (0.2f) : t.knobBody));
        g.fillRoundedRectangle (r, juce::jmin (6.0f, t.corner));
        g.setColour (t.panelEdge.withAlpha (0.5f));
        g.drawRoundedRectangle (r, juce::jmin (6.0f, t.corner), 1.0f);
    }

private:
    int themeIndex = 0;
};
