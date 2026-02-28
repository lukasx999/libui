#pragma once

#include <gfx/gfx.h>

#include "box.h"
#include "style.h"

namespace ranges = std::ranges;

namespace ui {

class Container : public Box {
public:
    enum class Direction { Horizontal, Vertical };

    Container(const gfx::Window& window, gfx::Vec position, Style style, std::vector<std::unique_ptr<Box>> children, Direction direction)
        : Box(window, position, style, 0.0f, 0.0f)
        , m_children(std::move(children))
        , m_direction(direction)
    {

        switch (m_direction) {
            using enum Direction;

            case Horizontal:
                m_moving_side = &gfx::Rect::width;
                m_static_side = &gfx::Rect::height;
                break;

            case Vertical:
                m_moving_side = &gfx::Rect::height;
                m_static_side = &gfx::Rect::width;
                break;
        }
    }

    void compute_dimensions() {
        if (m_children.empty()) return;
        compute_static_side();
        compute_moving_side();
    }

    void for_each_child(std::function<void(Box&)> fn) const override {
        for (auto& child : m_children) {
            fn(*child);
        }
    }

    [[nodiscard]] std::string format() const override;

    bool debug() override {

        bool found = ranges::any_of(m_children, [&](const std::unique_ptr<Box>& child) {
            return child->debug();
        });

        if (not found)
            return Box::debug();

        return true;
    }

    void draw(gfx::Renderer& rd) const override {
        Box::draw(rd);

        for (const auto& child : m_children) {
            child->draw(rd);
        }
    }

protected:
    const std::vector<std::unique_ptr<Box>> m_children;
    const Direction m_direction;

    float gfx::Rect::* m_moving_side;
    float gfx::Rect::* m_static_side;

    void compute_static_side() {
        auto largest_static_side = ranges::max_element(m_children, [&](const std::unique_ptr<Box>& a, decltype(a) b) {
            auto get_size = [&](decltype(a) child) {
                return child->get_rect().*m_static_side + child->get_style().margin;
            };
            return get_size(a) < get_size(b);
        });

        assert(largest_static_side != m_children.end());
        auto& largest = **largest_static_side;

        m_rect.*m_static_side = largest.get_rect().*m_static_side
            + largest.get_style().margin * 2.0f
            + m_style.padding * 2.0f;
    }

    void compute_moving_side() {
        float child_sum = ranges::fold_left(m_children, 0.0f, [&](float acc, const std::unique_ptr<Box>& child) {
            return acc + child->get_rect().*m_moving_side + child->get_style().margin * 2.0f;
        });

        m_rect.*m_moving_side = child_sum + m_style.padding * 2.0f;
    }

};

} // namespace ui

template <>
struct std::formatter<ui::Container::Direction> : std::formatter<std::string> {
    auto format(const ui::Container::Direction& state, std::format_context& ctx) const {
        auto fmt = [&] {
            switch (state) {
                using enum ui::Container::Direction;
                case Horizontal: return "Horizontal";
                case Vertical: return "Vertical";
            }
            std::unreachable();
        }();

        return std::formatter<std::string>::format(fmt, ctx);
    }
};

inline std::string ui::Container::format() const {
    return std::format("Container ({})", m_direction);
}
