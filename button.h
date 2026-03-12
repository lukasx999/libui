#pragma once

#include <gfx/gfx.h>

#include "label.h"
#include "clickable.h"

namespace ui {

class Button : public Label, public Clickable {
public:
    Button(Id id, gfx::Window& window, gfx::Vec position, Style style, std::string_view text, const gfx::Font& font)
        : Box(id, window, position, style, 0, 0)
        , Label(id, window, position, style, text, font)
        , Clickable(id, window, position, style, 0, 0)
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
