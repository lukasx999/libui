#include <numeric>
#include <print>
#include <vector>
#include <memory>
#include <ranges>
#include <functional>
#include <stack>

#include <gfx/gfx.h>

#include "box.h"
#include "container.h"
#include "label.h"

namespace ui {

class Ui {
    using Context = std::stack<std::vector<std::unique_ptr<Box>>>;
    using Fn = std::function<void()>;

public:
    explicit Ui(Context& context) : m_context(context) { }

    void box(Style style={}) {
        m_context.top().push_back(std::make_unique<Box>(style));
    }

    void label(std::string_view text, const gfx::Font& font, Style style={}) {
        m_context.top().push_back(std::make_unique<Label>(text, font, style));
    }

    void horizontal(Fn fn, Style style={}) {
        m_context.push({});
        fn();
        auto container = std::make_unique<Container>(std::move(m_context.top()), Container::Direction::Horizontal, style);
        m_context.pop();
        m_context.top().push_back(std::move(container));
    }

    void vertical(Fn fn, Style style={}) {
        m_context.push({});
        fn();
        auto container = std::make_unique<Container>(std::move(m_context.top()), Container::Direction::Vertical, style);
        m_context.pop();
        m_context.top().push_back(std::move(container));
    }

private:
    // context, used for temporarily storing the child elements in the current element context
    Context& m_context;

};

class UserInterface {
    std::stack<std::vector<std::unique_ptr<Box>>> m_context;
    Ui m_ui { m_context };

public:
    UserInterface() = default;

    void root(gfx::Renderer& rd, gfx::Window& window, std::function<void(Ui&)> fn, Style style={}) {
        m_context.push({});
        m_ui.vertical(std::bind(fn, m_ui), style);

        assert(m_context.size() == 1);
        auto& root = m_context.top().front();
        root->compute_layout();


        root->debug(window);
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
    ui::UserInterface ui;

    window.draw_loop([&](gfx::Renderer& rd) {
        rd.clear_background(gfx::Color::black());

        ui.root(rd, window, [&](ui::Ui& ui) {

            ui.label("foo", font, {gfx::Color::green()});

            ui.vertical([&] {

                ui.vertical([&] {
                    ui.label("bar", font, {gfx::Color::red()});
                    ui.label("baz", font, {gfx::Color::blue()});
                }, {.color=gfx::Color::orange(), .padding=10.0f});

                ui.label("world", font, {gfx::Color::green()});

            });

        }, {gfx::Color::black()});

        if (window.get_key_state(gfx::Key::Escape).pressed())
            window.close();
    });

}
