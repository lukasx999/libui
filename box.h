#pragma once

#include <print>
#include <gfx/gfx.h>

namespace ui {
class Box {
public:
    Box(gfx::Color color, float margin=0.0f)
        : m_color(color)
        , m_margin(margin)
    { }

    virtual ~Box() = default;

    [[nodiscard]] float get_margin() const {
        return m_margin;
    }

    [[nodiscard]] gfx::Rect& get_box() {
        return m_box;
    }

    virtual void draw(gfx::Renderer& rd) const {
        auto color = m_is_debug_selected
            ? gfx::lerp(m_color, gfx::Color::white(), 0.75f)
            : m_color;
        rd.draw_rectangle(m_box, color);
    }

    virtual void compute_layout() { }

    // returns whether the current element is selected by the cursor
    virtual bool debug(gfx::Window& window) {
        auto mouse = window.get_mouse_pos();
        return m_is_debug_selected = m_box.check_collision_point(mouse);
    }

    virtual void print(int spacing) {
        for (int i = 0; i < spacing; ++i)
            std::print(" ");

        if (m_is_debug_selected)
            std::print("> ");
        std::println("Box {}", m_box);
    }

protected:
    const gfx::Color m_color;
    bool m_is_debug_selected = false;
    const float m_margin;
    gfx::Rect m_box;

};

} // namespace ui
