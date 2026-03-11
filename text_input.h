#pragma once

#include <print>

#include <gfx/gfx.h>

#include "box.h"
#include "style.h"

namespace ui {

class TextInput : public Box {
public:
    TextInput(gfx::Window& window, gfx::Vec position, Style style, const gfx::Font& font, float width, std::string& text)
        : Box(window, position, style, 0.0f, 0.0f)
        , m_text(text)
        , m_font(font)
    {
        m_rect.width = width + m_style.padding * 2.0f;
        m_rect.height = m_fontsize + m_style.padding * 2.0f;

        m_callback_id = m_window.add_char_callback([&](std::string string, [[maybe_unused]] char32_t codepoint) {
            m_text.append(std::move(string));
        });
    }

    ~TextInput() {
        m_window.remove_char_callback(m_callback_id);
    }

    void handle_input() override {
        auto key = m_window.get_key_state(gfx::Key::Backspace);

        if (key.is_clicked())
            if (not m_text.empty())
                m_text.pop_back();
    }

    void draw(gfx::Renderer& rd) const override {
        Box::draw(rd);
        float padding = m_style.padding;
        rd.draw_text(m_rect.x + padding, m_rect.y + padding, m_fontsize, m_text, m_font, m_style.color_text);
    }

    [[nodiscard]] std::string format() const override {
        return std::format("TextInput ({})", m_text);
    }

protected:
    const int m_fontsize = 50;
    std::string& m_text;
    const gfx::Font& m_font;
    gfx::Window::CallbackId m_callback_id;

};

} // namespace ui
