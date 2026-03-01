#pragma once

#include <gfx/gfx.h>

namespace ui {

// TODO: font
// TODO: fixed width/height
struct Style {
    gfx::Color color_bg = gfx::Color::black();
    float margin = 0.0f;
    float padding = 0.0f;
    float border_radius = 0.0f;

    // Label
    gfx::Color color_text = gfx::Color::white();

    // Button
    gfx::Color color_hover = gfx::Color::white();
    gfx::Color color_press = gfx::Color::black();
};

} // namespace ui
