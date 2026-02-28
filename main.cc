#include <numeric>
#include <print>
#include <vector>
#include <memory>
#include <ranges>
#include <functional>
#include <stack>

#include <gfx/gfx.h>

#include "box.h"
#include "button.h"
#include "container.h"
#include "label.h"

namespace ui {

class Ui {
    using Fn = std::function<void()>;

public:
    Ui(const gfx::Window& window, std::stack<std::vector<std::unique_ptr<Box>>>& context)
        : m_window(window)
        , m_context(context)
    { }

    void label(std::string_view text, const gfx::Font& font, Style style={}) {
        add_child<Label>(style, text, font);
    }

    Button::State button(Style style={}) {
        auto& btn = add_child<Button>(style);
        btn.handle_input();
        return btn.get_state();
    }

    void box(float width, float height, Style style={}) {
        add_child<Box>(style, width, height);
    }

    void horizontal(Fn fn, Style style={}) {
        container(fn, style, Container::Direction::Horizontal);
    }

    void vertical(Fn fn, Style style={}) {
        container(fn, style, Container::Direction::Vertical);
    }

private:
    const gfx::Window& m_window;

    // context, used for temporarily storing the child elements in the current element context
    std::stack<std::vector<std::unique_ptr<Box>>>& m_context;

    friend class UserInterface;

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
    // TODO: dont initialize for vertical root
    float gfx::Vec::*  m_moving_axis = &gfx::Vec::y;
    float gfx::Vec::*  m_static_axis = &gfx::Vec::x;
    float gfx::Rect::* m_moving_side = &gfx::Rect::height;
    float gfx::Rect::* m_static_side = &gfx::Rect::width;

    gfx::Vec m_axis = gfx::Vec::zero();

    // add a child element into the current context
    // returns a reference to the newly created child element
    template <std::derived_from<Box> Element, typename... Args>
    Element& add_child(Style style, Args&&... args) {

        gfx::Vec pos(m_axis.x + style.margin, m_axis.y + style.margin);
        auto element = std::make_unique<Element>(m_window, pos, style, std::forward<Args>(args)...);

        if constexpr (std::is_same_v<Element, Container>)
            element->compute_dimensions();

        // we have to do this AFTER the child container layouts have been computed,
        // as their dimensions (width/height) are not known before that point.
        m_axis.*m_moving_axis += element->get_rect().*m_moving_side + style.margin * 2.0f;

        Element& element_ref = *element;
        m_context.top().push_back(std::move(element));

        return element_ref;
    }

    void container(Fn fn, Style style, Container::Direction direction) {

        auto saved_axis = m_axis;
        auto moving_axis = m_moving_axis;
        auto static_axis = m_static_axis;
        auto moving_side = m_moving_side;
        auto static_side = m_static_side;

        set_axis(direction);
        m_axis.x += style.padding;
        m_axis.y += style.padding;

        m_context.push({});
        fn();
        auto children = std::move(m_context.top());
        m_context.pop();

        m_axis = saved_axis;
        m_moving_axis = moving_axis;
        m_static_axis = static_axis;
        m_moving_side = moving_side;
        m_static_side = static_side;

        add_child<Container>(style, std::move(children), direction);
    }

    void set_axis(Container::Direction direction) {
        switch (direction) {
            using enum Container::Direction;

            case Horizontal: {
                m_moving_axis = &gfx::Vec::x;
                m_static_axis = &gfx::Vec::y;
                m_moving_side = &gfx::Rect::width;
                m_static_side = &gfx::Rect::height;
            } break;

            case Vertical: {
                m_moving_axis = &gfx::Vec::y;
                m_static_axis = &gfx::Vec::x;
                m_moving_side = &gfx::Rect::height;
                m_static_side = &gfx::Rect::width;
            } break;
        }
    }

};

class UserInterface {
    std::stack<std::vector<std::unique_ptr<Box>>> m_context;
    const gfx::Window& m_window;
    Ui m_ui{m_window, m_context};

public:
    explicit UserInterface(const gfx::Window& window)
    : m_window(window)
    { }

    void root(gfx::Renderer& rd, std::function<void(Ui&)> fn, Style style={}) {
        m_context.push({});
        m_ui.vertical(std::bind(fn, m_ui), style);

        assert(m_context.size() == 1);
        auto& root = m_context.top().front();
        root->debug();
        system("clear");
        root->print(0);
        root->draw(rd);

        m_ui.m_axis = gfx::Vec::zero();
        m_context.pop();
        assert(m_context.empty());
    }
};


} // namespace ui

int main() {

    auto flags = gfx::WindowFlags()
        .enable_resizing(true);

    gfx::Window window(1920, 1080, "ui", flags);
    auto font = window.load_font("/usr/share/fonts/TTF/FiraCodeNerdFont-Regular.ttf");
    ui::UserInterface ui(window);

    window.draw_loop([&](gfx::Renderer& rd) {
        rd.clear_background(gfx::Color::black());

        ui.root(rd, [&](ui::Ui& ui) {

            ui.horizontal([&] {
                ui.box(200, 50, {.color_bg=gfx::Color::orange()});
                ui.box(200, 50, {.color_bg=gfx::Color::red()});
            });

            ui.box(200, 50, {.color_bg=gfx::Color::blue()});
            ui.box(200, 50, {.color_bg=gfx::Color::lightblue()});

            bool is_pressed = ui.button({.color_bg=gfx::Color::red()}) == ui::Button::State::Pressed;
            auto color = is_pressed
                ? gfx::Color::blue()
                : gfx::Color::gray();
            ui.label("you pressed the button", font, {.color_bg=color});


        }, {gfx::Color::black()});

        if (window.get_key_state(gfx::Key::Escape).pressed())
            window.close();
    });

}
