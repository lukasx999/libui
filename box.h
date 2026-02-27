#pragma once

#include <print>
#include <gfx/gfx.h>

#include "style.h"

namespace ui {

class Box {
public:
    explicit Box(Style style) : m_style(style) { }

    virtual ~Box() = default;

    [[nodiscard]] const Style& get_style() const {
        return m_style;
    }

    [[nodiscard]] gfx::Rect& get_rect() {
        return m_rect;
    }

    virtual void draw(gfx::Renderer& rd) const {
        auto color = m_is_debug_selected
            ? gfx::lerp(m_style.color, gfx::Color::white(), 0.75f)
            : m_style.color;

        // draw_rectangle_rounded() actually draws 4 circles and 2 rectangles,
        // which might impact performance, even when the border radius is 0.
        if (m_style.border_radius == 0.0f)
            rd.draw_rectangle(m_rect, color);
        else
            rd.draw_rectangle_rounded(m_rect, color, m_style.border_radius);
    }

    virtual void compute_layout() { }

    // returns whether the current element is selected by the cursor
    virtual bool debug(gfx::Window& window) {
        auto mouse = window.get_mouse_pos();
        return m_is_debug_selected = m_rect.check_collision_point(mouse);
    }

    virtual void print(int spacing) {
        for (int i = 0; i < spacing; ++i)
            std::print(" ");

        if (m_is_debug_selected)
            std::print("> ");
        std::println("Box {}", m_rect);
    }

protected:
    const Style m_style;
    bool m_is_debug_selected = false;
    gfx::Rect m_rect;

};

} // namespace ui
