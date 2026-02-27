#pragma once

#include <gfx/gfx.h>

namespace ui {

// TODO: font
// TODO: padding
struct Style {
    gfx::Color color = gfx::Color::black();
    float margin = 0.0f;
    float padding = 0.0f;
};

} // namespace ui
