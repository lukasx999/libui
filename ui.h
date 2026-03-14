#pragma once

#include <stack>
#include <vector>
#include <memory>
#include <functional>

#include <gfx/gfx.h>

#include "box.h"
#include "clickable.h"
#include "button.h"
#include "container.h"
#include "label.h"
#include "text_input.h"

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
    Ui(gfx::Window& window, const gfx::Font& font, Context& context, const std::unordered_map<Box::Id, std::any>& stored_state)
        : m_window(window)
        , m_font(font)
        , m_context(context)
        , m_stored_state(stored_state)
    { }

    ~Ui() = default;
    Ui(const Ui&) = delete;
    Ui(Ui&&) = delete;
    Ui& operator=(const Ui&) = delete;
    Ui& operator=(Ui&&) = delete;

    void label(std::string_view text, Style style={}) {
        add_child<Label>(style, text, m_font);
    }

    Clickable::State button(std::string_view text, Style style={}) {
        return add_child<Button>(style, text, m_font).get_state();
    }

    void box(float width, float height, Style style={}) {
        add_child<Box>(style, width, height);
    }

    void text_input(float width, std::string& text, Style style={}) {
        add_child<TextInput>(style, m_font, width, text);
    }

    void horizontal(Fn fn, Style style={}) {
        container(fn, style, Container::Direction::Horizontal);
    }

    void vertical(Fn fn, Style style={}) {
        container(fn, style, Container::Direction::Vertical);
    }

private:
    friend class UserInterface;

    gfx::Window& m_window;
    const gfx::Font& m_font;
    Context& m_context;
    gfx::Vec m_axis = gfx::Vec::zero();
    Container::Direction m_direction = Container::Direction::Vertical;
    Box::Id m_id_counter = 0;
    const std::unordered_map<Box::Id, std::any>& m_stored_state;

    // add a child element into the current context
    // returns a reference to the newly created child element
    template <class Element, typename... Args> requires std::is_base_of_v<Box, Element>
    Element& add_child(Style style, Args&&... args) {

        gfx::Vec pos(m_axis.x + style.margin, m_axis.y + style.margin);
        auto element = std::make_unique<Element>(m_id_counter++, m_window, pos, style, std::forward<Args>(args)...);

        // restore state
        auto id = element->get_id();
        if (m_stored_state.contains(id))
            element->apply_state(m_stored_state.at(id));

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
public:
    explicit UserInterface(gfx::Window& window)
        : m_window(window)
        , m_font(window.load_font("/usr/share/fonts/TTF/FiraCodeNerdFont-Regular.ttf"))
    { }

    ~UserInterface() = default;
    UserInterface(const UserInterface&) = delete;
    UserInterface(UserInterface&&) = delete;
    UserInterface& operator=(const UserInterface&) = delete;
    UserInterface& operator=(UserInterface&&) = delete;

    void root(gfx::Renderer& rd, std::function<void(Ui&)> fn, Style style={}) {

        Ui ui(m_window, m_font, m_context, m_stored_state);

        m_children = m_context.with_frame([&] {
            ui.vertical(std::bind(fn, std::ref(ui)), style);
        });

        if (m_children.empty()) return;

        assert(m_children.size() == 1);
        auto& root = m_children.front();

        root->debug();
        root->draw(rd);

        system("clear");
        print_tree(*root, 0);

        ui.m_axis = gfx::Vec::zero();

        save_state();
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
        std::print(" | {} {} {} {}", rect.x, rect.y, rect.width, rect.height);

        std::println(" | {}", box.get_id());

        box.for_each_child(std::bind(print_tree, _1, spacing+1));
    }

private:
    gfx::Window& m_window;
    gfx::Font m_font;
    Context m_context;
    std::vector<std::unique_ptr<Box>> m_children;
    std::unordered_map<Box::Id, std::any> m_stored_state;

    void save_state() {
        auto& root = *m_children.front();

        m_stored_state.clear();

        [&](this const auto& self, const Box& box) -> void {
            m_stored_state[box.get_id()] = box.export_state();
            box.for_each_child(self);
        }(root);
    }

};


} // namespace ui
