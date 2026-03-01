#pragma once

#include <gfx/gfx.h>

#include "box.h"

namespace ui {

class Button : public Box {
public:
    enum class State { Idle, Hovered, Pressed };

    Button(const gfx::Window& window, gfx::Vec position, Style style, float width, float height)
    : Box(window, position, style, width, height)
    { }

    [[nodiscard]] State get_state() const{
        return m_state;
    }

    void handle_input() override {

        auto mouse = m_window.get_mouse_pos();
        bool is_selected = m_rect.check_collision_point(mouse);

        if (not is_selected) {
            m_state = State::Idle;
            return;
        }

        bool is_pressed = m_window.get_mouse_button_state(gfx::MouseButton::Left).pressed();
        if (is_pressed) {
            m_state = State::Pressed;
        } else {
            m_state = State::Hovered;
        }
    }

    void draw(gfx::Renderer& rd) const override {

        auto color = [&] {
            switch (m_state) {
                using enum State;
                case Idle:    return m_style.color_bg;
                case Hovered: return m_style.color_hover;
                case Pressed: return m_style.color_press;
            }
            std::unreachable();
        }();

        rd.draw_rectangle_rounded(m_rect, color, m_style.border_radius);
    }

    [[nodiscard]] std::string format() const override;

private:
    State m_state = State::Idle;

};

} // namespace ui

template <>
struct std::formatter<ui::Button::State> : std::formatter<std::string> {
    auto format(const ui::Button::State& state, std::format_context& ctx) const {

        auto fmt = [&] {
            switch (state) {
                using enum ui::Button::State;
                case Idle:    return "Idle";
                case Hovered: return "Hovered";
                case Pressed: return "Pressed";
            }
            std::unreachable();
        }();

        return std::formatter<std::string>::format(fmt, ctx);
    }
};

inline std::string ui::Button::format() const {
    return std::format("Button ({})", m_state);
}
