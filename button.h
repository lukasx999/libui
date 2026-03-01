#pragma once

#include <gfx/gfx.h>

#include "label.h"
#include "clickable.h"

namespace ui {

class Button : public Label, public Clickable {
public:
    Button(const gfx::Window& window, gfx::Vec position, Style style, std::string_view text, const gfx::Font& font)
        : Box(window, position, style, 0, 0)
        , Label(window, position, style, text, font)
        , Clickable(window, position, style, 0, 0)
    { }

    [[nodiscard]] std::string format() const override {
        return std::format("LabeledButton ({})", m_state);
    }

    void draw(gfx::Renderer& rd) const override {
        Clickable::draw(rd);
        Label::draw(rd);
    }

};

} // namespace ui
