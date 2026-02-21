#include <print>
#include <vector>
#include <stack>

#include <gfx/gfx.h>

namespace {

class Box {
protected:
    const gfx::Color m_color;
    gfx::Rect m_rect;

public:
    Box(gfx::Color color) : m_color(color) { }
    virtual ~Box() = default;

    [[nodiscard]] gfx::Rect& get_rect() {
        return m_rect;
    }

    virtual void draw(gfx::Renderer& rd) const {
        rd.draw_rectangle(m_rect, m_color);
    }

    virtual void calculate_layout() { }

    virtual void print(int spacing) {
        for (int i = 0; i < spacing; ++i)
            std::print(" ");
        std::println("Box {}", m_rect);
    }

};

class Container : public Box {
protected:
    std::vector<Box*> m_children;

public:
    Container(std::vector<Box*> children, gfx::Color color)
        : Box(color)
        , m_children(std::move(children))
    { }

    void draw(gfx::Renderer& rd) const override {
        Box::draw(rd);

        for (const auto& child : m_children)
            child->draw(rd);
    }

};

class HorizontalContainer : public Container {
public:
    HorizontalContainer(std::vector<Box*> children, gfx::Color color)
    : Container(std::move(children), color)
    { }

    void calculate_layout() override {
        int elem_width = m_rect.width / m_children.size();

        int x = 0;
        for (auto& child : m_children) {
            auto& rect = child->get_rect();
            rect.height = m_rect.height;
            rect.width = elem_width;
            rect.y = m_rect.y;
            rect.x = m_rect.x + x;
            x += elem_width;

            child->calculate_layout();
        }
    }

    void print(int spacing) override {
        for (int i = 0; i < spacing; ++i)
            std::print(" ");

        std::println("Horizontal {}", m_rect);

        for (auto& child : m_children)
            child->print(spacing+1);
    }

};

class VerticalContainer : public Container {
public:
    VerticalContainer(std::vector<Box*> children, gfx::Color color)
    : Container(std::move(children), color)
    { }

    void calculate_layout() override {
        int elem_height = m_rect.height / m_children.size();

        int y = 0;
        for (auto& child : m_children) {
            auto& rect = child->get_rect();
            rect.height = elem_height;
            rect.width = m_rect.width;
            rect.x = m_rect.x;
            rect.y = m_rect.y + y;
            y += elem_height;

            child->calculate_layout();
        }
    }

    void print(int spacing) override {
        for (int i = 0; i < spacing; ++i)
            std::print(" ");

        std::println("Vertical {}", m_rect);

        for (auto& child : m_children)
            child->print(spacing+1);
    }

};

class Ui {
    using Fn = std::function<void()>;
    std::stack<std::vector<Box*>> m_children;

public:
    Ui() = default;

    void box(gfx::Color color) {
        m_children.top().emplace_back(new Box(color));
    }

    void horizontal(gfx::Color color, Fn fn) {
        m_children.push({});
        fn();
        auto children = m_children.top();
        m_children.pop();
        m_children.top().emplace_back(new HorizontalContainer(children, color));
    }

    void vertical(gfx::Color color, Fn fn) {
        m_children.push({});
        fn();
        auto children = m_children.top();
        m_children.pop();
        m_children.top().emplace_back(new VerticalContainer(children, color));
    }

    void ui(gfx::Renderer& rd, gfx::Color color, Fn fn) {
        m_children.push({});
        vertical(color, fn);

        assert(m_children.size() == 1);
        auto root = m_children.top().front();
        root->get_rect() = rd.get_surface().get_as_rect();
        system("clear");
        root->calculate_layout();
        root->print(0);
        root->draw(rd);

        m_children.pop();
        assert(m_children.empty());
    }

};

} // namespace

int main() {

    auto flags = gfx::WindowFlags()
        .enable_resizing(true);

    gfx::Window window(1920, 1080, "ui", flags);

    Ui ui;

    window.draw_loop([&](gfx::Renderer& rd) {
        rd.clear_background(gfx::Color::black());

        ui.ui(rd, gfx::Color::black(), [&] {

            ui.horizontal(gfx::Color::black(), [&] {
                ui.box(gfx::Color::orange());
                ui.box(gfx::Color::red());
            });

            ui.box(gfx::Color::blue());

            ui.horizontal(gfx::Color::black(), [&] {
                ui.box(gfx::Color::orange());
                ui.box(gfx::Color::red());
            });

        });

        if (window.get_key_state(gfx::Key::Escape).pressed())
            window.close();
    });

}
