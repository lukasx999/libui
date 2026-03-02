#pragma once

#include <gfx/gfx.h>

#include "box.h"

namespace ui {

class Clickable : public virtual Box {
public:
    enum class ClickState { Idle, Hovered, Pressed, Clicked };

    class State {
    public:
        explicit State(ClickState state) : m_state(state) { }

        [[nodiscard]] bool is_pressed() const {
            return m_state == ClickState::Pressed;
        }

        [[nodiscard]] bool is_clicked() const {
            return m_state == ClickState::Clicked;
        }

        [[nodiscard]] bool is_hovered() const {
            return m_state == ClickState::Hovered;
        }

        [[nodiscard]] bool is_idle() const {
            return m_state == ClickState::Idle;
        }

    private:
        ClickState m_state;
    };

    Clickable(const gfx::Window& window, gfx::Vec position, Style style, float width, float height)
    : Box(window, position, style, width, height)
    { }

    [[nodiscard]] State get_state() const{
        return State(m_state);
    }

    void handle_input() override {

        auto mouse = m_window.get_mouse_pos();
        bool is_selected = m_rect.check_collision_point(mouse);

        if (not is_selected) {
            m_state = ClickState::Idle;
            return;
        }

        auto state = m_window.get_mouse_button_state(gfx::MouseButton::Left);
        bool is_pressed = state.is_pressed();
        bool is_clicked = state.is_clicked();

        if (is_clicked)
            m_state = ClickState::Clicked;

        else if (is_pressed)
            m_state = ClickState::Pressed;

        else
            m_state = ClickState::Hovered;
    }

    void draw(gfx::Renderer& rd) const override {

        auto color = [&] {
            switch (m_state) {
                using enum ClickState;
                case Idle:    return m_style.color_bg;
                case Hovered: return m_style.color_hover;
                case Clicked:
                case Pressed: return m_style.color_press;
            }
            std::unreachable();
        }();

        rd.draw_rectangle_rounded(m_rect, color, m_style.border_radius);
    }

    [[nodiscard]] std::string format() const override;

protected:
    ClickState m_state = ClickState::Idle;

};

} // namespace ui

template <>
struct std::formatter<ui::Clickable::ClickState> : std::formatter<std::string> {
    auto format(const ui::Clickable::ClickState& state, std::format_context& ctx) const {

        auto fmt = [&] {
            switch (state) {
                using enum ui::Clickable::ClickState;
                case Idle:    return "Idle";
                case Hovered: return "Hovered";
                case Pressed: return "Pressed";
                case Clicked: return "Clicked";
            }
            std::unreachable();
        }();

        return std::formatter<std::string>::format(fmt, ctx);
    }
};

inline std::string ui::Clickable::format() const {
    return std::format("Button ({})", m_state);
}
