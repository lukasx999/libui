#pragma once

#include <print>

#include <gfx/gfx.h>

#include "box.h"
#include "style.h"

namespace ui {

class TextInput : public Box {
public:
    TextInput(Id id, gfx::Window& window, gfx::Vec position, Style style, const gfx::Font& font, float width, std::string& text)
        : Box(id, window, position, style, 0.0f, 0.0f)
        , m_text(text)
        , m_font(font)
    {
        m_rect.width = std::max(static_cast<int>(width), m_font.measure_text(m_text, m_fontsize)) + m_style.padding * 2.0f;
        m_rect.height = m_fontsize + m_style.padding * 2.0f;

        m_callback_id = m_window.add_char_callback([&](std::string string, [[maybe_unused]] char32_t codepoint) {
            if (m_is_selected)
                m_text.append(std::move(string));
        });
    }

    ~TextInput() {
        m_window.remove_char_callback(m_callback_id);
    }

    [[nodiscard]] std::any export_state() const override {
        return m_is_selected;
    }

    void apply_state(std::any state) override {
        m_is_selected = std::any_cast<bool>(state);
    }

    [[nodiscard]] bool is_selected() const {
        return m_is_selected;
    }

    void handle_input() override {
        handle_key_input();
        handle_selection_input();
    }

    void draw(gfx::Renderer& rd) const override {
        Box::draw(rd);
        float padding = m_style.padding;
        rd.draw_text(m_rect.x + padding, m_rect.y + padding, m_fontsize, m_text, m_font, m_style.color_text);
    }

    [[nodiscard]] std::string format() const override {
        return std::format("TextInput ({}) ({})", m_text, m_is_selected ? "Selected" : "");
    }

protected:
    const int m_fontsize = 50;
    std::string& m_text;
    const gfx::Font& m_font;
    gfx::Window::CallbackId m_callback_id;
    bool m_is_selected = false;

    void handle_key_input() {

        auto key = m_window.get_key_state(gfx::Key::Backspace);

        if (key.is_clicked() and m_is_selected)
            if (not m_text.empty())
                m_text.pop_back();
    }

    void handle_selection_input() {
        auto mouse = m_window.get_mouse_pos();
        bool is_selected = m_rect.check_collision_point(mouse);
        bool is_clicked = m_window.get_mouse_button_state(gfx::MouseButton::Left).is_clicked();

        if (is_selected and is_clicked)
            m_is_selected = true;
        else if (is_clicked)
            m_is_selected = false;
    }

};

} // namespace ui
