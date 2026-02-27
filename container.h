#pragma once

#include <gfx/gfx.h>

#include "box.h"
#include "style.h"

namespace ranges = std::ranges;

namespace ui {

class Container : public Box {
public:
    enum class Direction { Horizontal, Vertical };

    Container(const gfx::Window& window, std::vector<std::unique_ptr<Box>> children, Direction direction, Style style)
        : Box(window, style)
        , m_children(std::move(children))
        , m_direction(direction)
    {

        switch (m_direction) {
            using enum Direction;

            case Horizontal: {
                m_moving_axis = &gfx::Rect::x;
                m_static_axis = &gfx::Rect::y;
                m_moving_side = &gfx::Rect::width;
                m_static_side = &gfx::Rect::height;
            } break;

            case Vertical: {
                m_moving_axis = &gfx::Rect::y;
                m_static_axis = &gfx::Rect::x;
                m_moving_side = &gfx::Rect::height;
                m_static_side = &gfx::Rect::width;
            } break;
        }
    }

    void compute_layout() override {

        if (m_children.empty()) return;

        // x/y-positions get passed down the tree, as individual elements dont know
        // their absolute position, and have to rely on their parent setting the position for them.
        // in this case the root position (0, 0) is known.
        //
        // dimensions (width/height) are passed up the tree, as individual elements know their size,
        // but their parents dont, because their size depends on how many children they have
        compute_child_layouts();

        // we can only calculate our own dimensions AFTER the child layouts
        // have been computed.
        compute_dimensions();

    }

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

    void print(int spacing) override {
        for (int i = 0; i < spacing; ++i)
            std::print(" ");

        if (m_is_debug_selected)
            std::print("> ");
        std::println("Container {}", m_rect);


        for (auto& child : m_children) {
            child->print(spacing+1);
        }
    }

protected:
    const std::vector<std::unique_ptr<Box>> m_children;
    const Direction m_direction;

    // the "moving" components correspond to the direction in which the containers
    // children get laid out.
    //
    // the moving axis is incremented to place the children along the xy-axis.
    // the static axis just stays the same for all children.
    //
    // eg: vertical layout
    // moving: y, height
    // static: x, width
    //
    // eg: horizontal layout
    // moving: x, width
    // static: y, height
    //
    // we use ptr-to-member syntax here to avoid code duplication.
    float gfx::Rect::* m_moving_axis;
    float gfx::Rect::* m_static_axis;
    float gfx::Rect::* m_moving_side;
    float gfx::Rect::* m_static_side;

    void compute_child_layouts() {

        float moving_axis = m_rect.*m_moving_axis + m_children.front()->get_style().margin + m_style.padding;

        for (auto& child : m_children) {
            gfx::Rect& rect = child->get_rect();
            const Style& style = child->get_style();
            float margin = style.margin;

            rect.*m_moving_axis = moving_axis + margin;
            rect.*m_static_axis = m_rect.*m_static_axis + margin + m_style.padding;

            child->compute_layout();

            // we have to do this AFTER the child layout has been computed
            // as its dimensions (width/height) are not known before that point.
            moving_axis += rect.*m_moving_side + margin * 2.0f;
        }
    }

    void compute_dimensions() {
        compute_static_side();
        compute_moving_side();
    }

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

        m_rect.*m_moving_side = ranges::fold_left(m_children, 0.0f, [&](float acc, const std::unique_ptr<Box>& child) {
            return acc + child->get_rect().*m_moving_side + child->get_style().margin * 2.0f;
        }) + m_style.padding * 2.0f;
    }

};

} // namespace ui
