#pragma once

#include <gfx/gfx.h>

#include "box.h"
#include "style.h"

namespace ui {

class Label : public Box {
public:
    Label(const gfx::Window& window, gfx::Vec position, Style style, std::string_view text, const gfx::Font& font)
        : Box(window, position, style)
        , m_text(text)
        , m_font(font)
    {
        m_rect.height = m_fontsize;
        m_rect.width = m_font.measure_text(m_text, m_fontsize);
    }

    void draw(gfx::Renderer& rd) const override {
        Box::draw(rd);
        rd.draw_text(m_rect.x, m_rect.y, m_fontsize, m_text, m_font, m_font_color);
    }

    void print(int spacing) override {
        for (int i = 0; i < spacing; ++i)
            std::print(" ");

        if (m_is_debug_selected)
            std::print("> ");
        std::println("Label {} ({})", m_rect, m_text);
    }

private:
    const std::string_view m_text;
    const gfx::Font& m_font;
    const gfx::Color m_font_color = gfx::Color::white();
    const int m_fontsize = 50;

};

} // namespace ui
