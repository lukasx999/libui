#pragma once

#include <gfx/gfx.h>

namespace ui {

// TODO: font
struct Style {
    gfx::Color color = gfx::Color::black();
    float margin = 0.0f;
    float padding = 0.0f;
    float border_radius = 0.0f;
};

} // namespace ui
