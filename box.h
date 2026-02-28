#pragma once

#include <print>
#include <gfx/gfx.h>

#include "style.h"

namespace ui {

class Box {
public:
    Box(const gfx::Window& window, gfx::Vec position, Style style, float width, float height)
        : m_window(window)
        , m_style(style)
        , m_rect(position.x, position.y, width, height)
    { }

    virtual ~Box() = default;

    [[nodiscard]] const Style& get_style() const {
        return m_style;
    }

    [[nodiscard]] gfx::Rect& get_rect() {
        return m_rect;
    }

    [[nodiscard]] bool is_debug_selected() const {
        return m_is_debug_selected;
    }

    virtual void for_each_child([[maybe_unused]] std::function<void(Box&)> fn) const { }

    virtual void handle_input() { }

    virtual void draw(gfx::Renderer& rd) const {
        auto color = m_is_debug_selected
            ? gfx::lerp(m_style.color_bg, gfx::Color::white(), 0.75f)
            : m_style.color_bg;

        // draw_rectangle_rounded() actually draws 4 circles and 2 rectangles,
        // which might impact performance, even when the border radius is 0.
        if (m_style.border_radius == 0.0f)
            rd.draw_rectangle(m_rect, color);
        else
            rd.draw_rectangle_rounded(m_rect, color, m_style.border_radius);
    }

    // returns whether the current element is selected by the cursor
    virtual bool debug() {
        auto mouse = m_window.get_mouse_pos();
        return m_is_debug_selected = m_rect.check_collision_point(mouse);
    }

    [[nodiscard]] virtual std::string format() const {
        return "Box";
    }

protected:
    const gfx::Window& m_window;
    const Style m_style;
    bool m_is_debug_selected = false;
    gfx::Rect m_rect;

};

} // namespace ui
