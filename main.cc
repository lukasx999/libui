#include <print>
#include <vector>
#include <memory>
#include <ranges>
#include <functional>
#include <stack>

#include <gfx/gfx.h>

namespace ui {

class Box {
protected:
    const gfx::Color m_color;
    const float m_margin;
    gfx::Rect m_box;

public:
    Box(gfx::Color color, float margin=1.0f)
        : m_color(color)
        , m_margin(margin)
    { }

    virtual ~Box() = default;

    [[nodiscard]] float get_margin() const {
        return m_margin;
    }

    [[nodiscard]] gfx::Rect& get_box() {
        return m_box;
    }

    virtual void draw(gfx::Renderer& rd) const {
        rd.draw_rectangle(m_box, m_color);
    }

    virtual void calculate_layout() { }

    virtual void print(int spacing) {
        for (int i = 0; i < spacing; ++i)
            std::print(" ");
        std::println("Box {}", m_box);
    }

};

class Container : public Box {
protected:
    std::vector<std::unique_ptr<Box>> m_children;

public:
    Container(std::vector<std::unique_ptr<Box>> children, gfx::Color color, float margin=1.0f)
        : Box(color, margin)
        , m_children(std::move(children))
    { }

    void draw(gfx::Renderer& rd) const override {
        Box::draw(rd);

        for (const auto& child : m_children) {
            child->draw(rd);
        }
    }

};

class HorizontalContainer : public Container {
public:
    HorizontalContainer(std::vector<std::unique_ptr<Box>> children, gfx::Color color, float margin=1.0f)
    : Container(std::move(children), color, margin)
    { }

    void calculate_layout() override {
        int elem_width = m_box.width / m_children.size();

        int x = 0;
        for (auto& child : m_children) {
            auto& box = child->get_box();
            float margin = child->get_margin();

            box.height = m_box.height - margin*2.0f;
            box.width = elem_width - margin*2.0f;
            box.y = m_box.y + margin;
            box.x = m_box.x + x + margin;
            x += elem_width;

            child->calculate_layout();
        }
    }

    void print(int spacing) override {
        for (int i = 0; i < spacing; ++i)
            std::print(" ");

        std::println("Horizontal {}", m_box);

        for (auto& child : m_children) {
            child->print(spacing+1);
        }
    }

};

class VerticalContainer : public Container {
public:
    VerticalContainer(std::vector<std::unique_ptr<Box>> children, gfx::Color color, float margin=1.0f)
    : Container(std::move(children), color, margin)
    { }

    void calculate_layout() override {
        int elem_height = m_box.height / m_children.size();

        int y = 0;
        for (auto& child : m_children) {
            auto& box = child->get_box();
            float margin = child->get_margin();

            box.height = elem_height - margin*2.0f;
            box.width = m_box.width - margin*2.0f;
            box.x = m_box.x + margin;
            box.y = m_box.y + y + margin;
            y += elem_height;

            child->calculate_layout();
        }
    }

    void print(int spacing) override {
        for (int i = 0; i < spacing; ++i)
            std::print(" ");

        std::println("Vertical {}", m_box);

        for (auto& child : m_children) {
            child->print(spacing+1);
        }
    }

};

class Ui {
    using Fn = std::function<void()>;

    // context, used for temporarily storing the child elements in the current
    // stack frame
    using Context = std::stack<std::vector<std::unique_ptr<Box>>>;
    Context& m_context;

public:
    explicit Ui(Context& context) : m_context(context) { }

    void box(gfx::Color color, float margin=1.0f) {
        m_context.top().emplace_back(std::make_unique<Box>(color, margin));
    }

    void horizontal(gfx::Color color, Fn fn) {
        m_context.push({});
        fn();
        auto container = std::make_unique<HorizontalContainer>(std::move(m_context.top()), color);
        m_context.pop();
        m_context.top().emplace_back(std::move(container));
    }

    void vertical(gfx::Color color, Fn fn) {
        m_context.push({});
        fn();
        auto container = std::make_unique<VerticalContainer>(std::move(m_context.top()), color);
        m_context.pop();
        m_context.top().emplace_back(std::move(container));
    }

};

class UserInterface {
    std::stack<std::vector<std::unique_ptr<Box>>> m_context;
    Ui m_ui { m_context };

public:
    UserInterface() = default;

    void root(gfx::Renderer& rd, gfx::Color color, std::function<void(Ui&)> fn) {
        m_context.push({});
        m_ui.vertical(color, std::bind(fn, m_ui));

        assert(m_context.size() == 1);
        auto& root = m_context.top().front();
        root->get_box() = rd.get_surface().get_as_rect();
        root->calculate_layout();
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
    ui::UserInterface ui;

    window.draw_loop([&](gfx::Renderer& rd) {
        rd.clear_background(gfx::Color::black());

        ui.root(rd, gfx::Color::black(), [&](ui::Ui& ui) {

            ui.horizontal(gfx::Color::gray(), [&] {
                ui.box(gfx::Color::white(), 30);
                ui.box(gfx::Color::blue(), 30);
            });

            ui.box(gfx::Color::orange(), 30);
            ui.box(gfx::Color::red(), 30);

        });

        if (window.get_key_state(gfx::Key::Escape).pressed())
            window.close();
    });

}
