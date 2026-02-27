#pragma once

#include <gfx/gfx.h>

namespace ui {

// TODO: font
// TODO: LabelStyle : Style (text_color)
// TODO: fixed width/height
struct Style {
    gfx::Color color_bg = gfx::Color::black();
    float margin = 0.0f;
    float padding = 0.0f;
    float border_radius = 0.0f;
};

} // namespace ui
