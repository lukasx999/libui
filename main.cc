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
    using Context = std::stack<std::vector<std::unique_ptr<Box>>>;
    using Fn = std::function<void()>;

public:
    Ui(const gfx::Window& window, Context& context)
        : m_window(window)
        , m_context(context)
    { }

    void label(std::string_view text, const gfx::Font& font, Style style={}) {
        add_child<Label>(text, font, gfx::Vec(0.0f, 0.0f), style);
    }

    Button::State button(Style style={}) {
        auto& btn = add_child<Button>(gfx::Vec(0.0f, 0.0f), style);
        return btn.get_state();
    }

    void box(float width, float height, Style style={}) {
        gfx::Vec pos(m_axis.x + style.margin, m_axis.y + style.margin);

        auto& box = add_child<Box>(pos, style);
        box.get_rect().width = width;
        box.get_rect().height = height;

        m_axis.y += height + style.margin * 2.0f;
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
    Context& m_context;

    gfx::Vec m_axis;

    // add a child element into the current context
    // returns a reference to the newly created child element
    template <std::derived_from<Box> Element>
    Element& add_child(auto&&... args) {
        auto element = std::make_unique<Element>(m_window, std::forward<decltype(args)>(args)...);
        Element& ret = *element;
        m_context.top().push_back(std::move(element));
        return ret;
    }

    void container(Fn fn, Style style, Container::Direction direction) {
        m_axis.x = style.padding;
        m_axis.y = style.padding;

        m_context.push({});
        fn();
        auto children = std::move(m_context.top());
        m_context.pop();

        add_child<Container>(std::move(children), gfx::Vec(m_axis.x-style.padding, m_axis.y-style.padding), direction, style);

        m_axis = gfx::Vec::zero();
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
        // root->compute_layout();
        // root->handle_input();

        root->debug();
        system("clear");
        root->print(0);
        root->draw(rd);

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

            ui.box(200, 50, {.color_bg=gfx::Color::red()});
            ui.box(200, 50, {.color_bg=gfx::Color::blue()});

            // ui.label("foo", font, {.color_bg=gfx::Color::blue()});

            // bool is_pressed = ui.button({.color_bg=gfx::Color::red()}) == ui::Button::State::Pressed;
            // auto color = is_pressed
            //     ? gfx::Color::blue()
            //     : gfx::Color::gray();
            //
            // ui.label("you pressed the button", font, {.color_bg=color});
            //
            // ui.label("world", font, {gfx::Color::blue()});

        }, {gfx::Color::black()});

        if (window.get_key_state(gfx::Key::Escape).pressed())
            window.close();
    });

}
