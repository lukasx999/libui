#include <print>
#include <vector>
#include <memory>
#include <functional>
#include <stack>

#include <gfx/gfx.h>

#include "box.h"
#include "button.h"
#include "container.h"
#include "label.h"

namespace ui {

template <typename T>
class Context {
public:
    Context() = default;

    void add(T value) {
        m_context.top().push_back(std::move(value));
    }

    std::vector<T> with_frame(std::function<void()> fn) {
        m_context.push({});
        fn();
        auto items = std::move(m_context.top());
        m_context.pop();
        return items;
    }

private:
    std::stack<std::vector<T>> m_context;

};

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
    friend class UserInterface;

    const gfx::Window& m_window;

    // context, used for temporarily storing the child elements in the current element context
    std::stack<std::vector<std::unique_ptr<Box>>>& m_context;

    gfx::Vec m_axis = gfx::Vec::zero();
    Container::Direction m_direction = Container::Direction::Vertical;

    // add a child element into the current context
    // returns a reference to the newly created child element
    template <std::derived_from<Box> Element, typename... Args>
    Element& add_child(Style style, Args&&... args) {

        gfx::Vec pos(m_axis.x + style.margin, m_axis.y + style.margin);
        auto element = std::make_unique<Element>(m_window, pos, style, std::forward<Args>(args)...);

        if constexpr (std::is_same_v<Element, Container>)
            element->compute_dimensions();

        switch (m_direction) {
            using enum Container::Direction;

            case Vertical:
                m_axis.y += element->get_rect().height + style.margin * 2.0f;
                break;

            case Horizontal:
                m_axis.x += element->get_rect().width + style.margin * 2.0f;
                break;
        }

        Element& element_ref = *element;
        m_context.top().push_back(std::move(element));

        return element_ref;
    }

    void container(Fn fn, Style style, Container::Direction direction) {

        auto saved_direction = m_direction;
        auto saved_axis = m_axis;

        m_direction = direction;
        m_axis.x += style.padding;
        m_axis.y += style.padding;

        m_context.push({});
        fn();
        auto children = std::move(m_context.top());
        m_context.pop();

        m_axis = saved_axis;
        m_direction = saved_direction;

        add_child<Container>(style, std::move(children), direction);
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
