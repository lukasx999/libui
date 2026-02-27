#pragma once

#include <gfx/gfx.h>

#include "box.h"
#include "style.h"

namespace ui {

class Label : public Box {
public:
    Label(std::string_view text, const gfx::Font& font, Style style)
        : Box(style)
        , m_text(text)
        , m_font(font)
    {
        m_box.height = m_fontsize;
        m_box.width = m_font.measure_text(m_text, m_fontsize);
    }

    void draw(gfx::Renderer& rd) const override {
        Box::draw(rd);
        rd.draw_text(m_box.x, m_box.y, m_fontsize, m_text, m_font, m_font_color);
    }

    void print(int spacing) override {
        for (int i = 0; i < spacing; ++i)
            std::print(" ");

        if (m_is_debug_selected)
            std::print("> ");
        std::println("Label {} ({})", m_box, m_text);
    }

private:
    const std::string_view m_text;
    const gfx::Font& m_font;
    const gfx::Color m_font_color = gfx::Color::white();
    const int m_fontsize = 50;

};

} // namespace ui
