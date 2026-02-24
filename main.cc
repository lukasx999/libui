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

    void box(gfx::Color color, float margin=0.0f) {
        m_context.top().emplace_back(std::make_unique<Box>(color, margin));
    }

    void label(std::string_view text, gfx::Color color, const gfx::Font& font, float margin=0.0f) {
        m_context.top().emplace_back(std::make_unique<Label>(text, color, font, margin));
    }

    void horizontal(gfx::Color color, Fn fn) {
        m_context.push({});
        fn();
        auto container = std::make_unique<Container>(std::move(m_context.top()), Container::Direction::Horizontal, color);
        m_context.pop();
        m_context.top().emplace_back(std::move(container));
    }

    void vertical(gfx::Color color, Fn fn) {
        m_context.push({});
        fn();
        auto container = std::make_unique<Container>(std::move(m_context.top()), Container::Direction::Vertical, color);
        m_context.pop();
        m_context.top().emplace_back(std::move(container));
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

    void root(gfx::Renderer& rd, gfx::Window& window, gfx::Color color, std::function<void(Ui&)> fn) {
        m_context.push({});
        m_ui.vertical(color, std::bind(fn, m_ui));

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

        ui.root(rd, window, gfx::Color::black(), [&](ui::Ui& ui) {

            ui.vertical(gfx::Color::lightblue(), [&] {
                ui.label("foo", gfx::Color::blue(), font, 10.0f);
                ui.horizontal(gfx::Color::grey(), [&] {
                    ui.label("a", gfx::Color::blue(), font, 10.0f);
                    ui.label("b", gfx::Color::blue(), font);
                });
            });

        });

        if (window.get_key_state(gfx::Key::Escape).pressed())
            window.close();
    });

}
