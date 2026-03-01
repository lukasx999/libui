#include <print>
#include <vector>
#include <memory>
#include <functional>
#include <stack>

#include <gfx/gfx.h>

#include "box.h"
#include "clickable.h"
#include "button.h"
#include "container.h"
#include "label.h"

namespace ui {

class Context {
public:
    Context() = default;

    // add an element into the current frame
    void add_element(std::unique_ptr<Box> element) {
        m_context.top().push_back(std::move(element));
    }

    // invoke a function in a newly created frame and return the child elements
    // created in that frame
    auto with_frame(std::function<void()> fn) -> std::vector<std::unique_ptr<Box>> {
        m_context.push({});
        fn();
        auto items = std::move(m_context.top());
        m_context.pop();
        return items;
    }

private:
    std::stack<std::vector<std::unique_ptr<Box>>> m_context;

};

class Ui {
    using Fn = std::function<void()>;

public:
    Ui(const gfx::Window& window, Context& context)
        : m_window(window)
        , m_context(context)
    { }

    ~Ui() = default;
    Ui(const Ui&) = delete;
    Ui(Ui&&) = delete;
    Ui& operator=(const Ui&) = delete;
    Ui& operator=(Ui&&) = delete;

    void label(std::string_view text, const gfx::Font& font, Style style={}) {
        add_child<Label>(style, text, font);
    }

    Clickable::State button(std::string_view text, const gfx::Font& font, Style style={}) {
        return add_child<Button>(style, text, font).get_state();
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
    Context& m_context;
    gfx::Vec m_axis = gfx::Vec::zero();
    Container::Direction m_direction = Container::Direction::Vertical;

    // add a child element into the current context
    // returns a reference to the newly created child element
    template <class Element, typename... Args> requires std::is_base_of_v<Box, Element>
    Element& add_child(Style style, Args&&... args) {

        gfx::Vec pos(m_axis.x + style.margin, m_axis.y + style.margin);
        auto element = std::make_unique<Element>(m_window, pos, style, std::forward<Args>(args)...);

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
        m_context.add_element(std::move(element));

        element_ref.handle_input();
        return element_ref;
    }

    void container(Fn fn, Style style, Container::Direction direction) {

        auto saved_direction = m_direction;
        auto saved_axis = m_axis;

        m_direction = direction;
        m_axis.x += style.padding;
        m_axis.y += style.padding;

        auto children = m_context.with_frame(fn);

        m_axis = saved_axis;
        m_direction = saved_direction;

        add_child<Container>(style, std::move(children), direction);
    }

};

class UserInterface {
    const gfx::Window& m_window;
    Context m_context;
    Ui m_ui{m_window, m_context};

public:
    explicit UserInterface(const gfx::Window& window) : m_window(window) { }

    ~UserInterface() = default;
    UserInterface(const UserInterface&) = delete;
    UserInterface(UserInterface&&) = delete;
    UserInterface& operator=(const UserInterface&) = delete;
    UserInterface& operator=(UserInterface&&) = delete;

    void root(gfx::Renderer& rd, std::function<void(Ui&)> fn, Style style={}) {

        auto children = m_context.with_frame([&] {
            m_ui.vertical(std::bind(fn, std::ref(m_ui)), style);
        });

        assert(children.size() == 1);
        auto& root = children.front();

        root->debug();
        root->draw(rd);

        system("clear");
        print_tree(*root, 0);

        m_ui.m_axis = gfx::Vec::zero();
    }

    static void print_tree(const Box& box, int spacing) {
        using namespace std::placeholders;

        for (int i = 0; i < spacing; ++i)
            std::print(" ");

        if (box.is_debug_selected())
            std::print(">");
        else
            std::print(" ");

        std::print("{}", box.format());

        auto rect = box.get_rect();
        std::println(" | {} {} {} {}", rect.x, rect.y, rect.width, rect.height);

        box.for_each_child(std::bind(print_tree, _1, spacing+1));
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

            // ui.horizontal([&] {
            //     ui.box(200, 50, {.color_bg=gfx::Color::orange()});
            //     ui.box(200, 50, {.color_bg=gfx::Color::red()});
            // });

            ui.button("hello", font);

            ui.box(200, 50, {.color_bg=gfx::Color::blue()});
            ui.box(200, 50, {.color_bg=gfx::Color::lightblue()});

            // bool is_pressed = ui.button({.color_bg=gfx::Color::red()}) == ui::Button::State::Pressed;
            // auto color = is_pressed
            //     ? gfx::Color::blue()
            //     : gfx::Color::gray();
            // ui.label("you pressed the button", font, { .color_bg=color, .color_text=gfx::Color::red() });

        }, { .color_bg=gfx::Color::black(), .padding=10.0f });

        if (window.get_key_state(gfx::Key::Escape).pressed())
            window.close();
    });

}
