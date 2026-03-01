#pragma once

#include <gfx/gfx.h>

#include "box.h"
#include "style.h"

namespace ui {

class Label : public virtual Box {
public:
    Label(const gfx::Window& window, gfx::Vec position, Style style, std::string_view text, const gfx::Font& font)
        : Box(window, position, style, 0.0f, 0.0f)
        , m_text(text)
        , m_font(font)
    {
        m_rect.height = m_fontsize;
        m_rect.width = m_font.measure_text(m_text, m_fontsize);
    }

    void draw(gfx::Renderer& rd) const override {
        Box::draw(rd);
        rd.draw_text(m_rect.x, m_rect.y, m_fontsize, m_text, m_font, m_style.color_text);
    }

    [[nodiscard]] std::string format() const override {
        return std::format("Label ({})", m_text);
    }

protected:
    const int m_fontsize = 50;
    const std::string_view m_text;
    const gfx::Font& m_font;

};

} // namespace ui
