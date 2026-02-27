#pragma once

#include <gfx/gfx.h>

#include "box.h"

namespace ui {

class Button : public Box {
public:
    Button(const gfx::Window& window, Style style={})
        : Box(window, style)
    {
        m_rect.width = 500;
        m_rect.height = 100;
    }

    void draw(gfx::Renderer& rd) const override {
        Box::draw(rd);
    }

    void print(int spacing) override {
        for (int i = 0; i < spacing; ++i)
            std::print(" ");

        if (m_is_debug_selected)
            std::print("> ");
        std::println("Button {}", m_rect);
    }

private:
    const gfx::Color m_color_hover = gfx::Color::white();
    const gfx::Color m_color_press = gfx::Color::black();

};

} // namespace ui
