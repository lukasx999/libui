#pragma once

#include <gfx/gfx.h>
#include "box.h"

namespace ui {

class Container : public Box {
public:
    enum class Direction { Horizontal, Vertical };

    Container(std::vector<std::unique_ptr<Box>> children, Direction direction, gfx::Color color, float margin=0.0f)
        : Box(color, margin)
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

        // dimensions (width/height) are passed up the tree, as individual elements know their size,
        // but their parents dont, because their size depends on how many children they have

        float axis = m_children.front()->get_margin();
        for (auto& child : m_children) {
            gfx::Rect& box = child->get_box();
            float margin = child->get_margin();

            box.*m_moving_axis = axis;
            box.*m_static_axis = m_box.*m_static_axis + margin;
            axis += box.*m_moving_side + margin;

            child->compute_layout();
        }

        // we can only calculate our own dimensions AFTER the child layouts
        // have been computed.

        compute_dimensions();

    }

    bool debug(gfx::Window& window) override {

        bool found = std::ranges::any_of(m_children, [&](std::unique_ptr<Box>& child) {
            return child->debug(window);
        });

        if (not found)
            return Box::debug(window);

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
        std::println("Container {}", m_box);


        for (auto& child : m_children) {
            child->print(spacing+1);
        }
    }

protected:
    std::vector<std::unique_ptr<Box>> m_children;
    const Direction m_direction;

    float gfx::Rect::* m_moving_axis;
    float gfx::Rect::* m_static_axis;
    float gfx::Rect::* m_moving_side;
    float gfx::Rect::* m_static_side;

    void compute_dimensions() {

        auto largest_static_side = std::ranges::max_element(m_children, [&](std::unique_ptr<Box>& a, decltype(a) b) {
            return a->get_box().*m_static_side + a->get_margin() < b->get_box().*m_static_side + b->get_margin();
        });

        m_box.*m_static_side = (*largest_static_side)->get_box().*m_static_side + (*largest_static_side)->get_margin()*2.0f;

        assert(largest_static_side != m_children.end());

        m_box.*m_moving_side = std::ranges::fold_left(m_children, 0.0f, [&](float acc, std::unique_ptr<Box>& child) {
            return acc + child->get_box().*m_moving_side + child->get_margin()*2.0f;
        });
    }

};

} // namespace ui
