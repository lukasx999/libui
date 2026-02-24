#include <numeric>
#include <print>
#include <vector>
#include <memory>
#include <ranges>
#include <functional>
#include <stack>

#include <gfx/gfx.h>

#define DBG(value) std::println("{}: {}", #value, value);

namespace ui {

class Box {
protected:
    const gfx::Color m_color;
    const float m_margin;
    gfx::Rect m_box;

public:
    Box(gfx::Color color, float margin=0.0f)
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

class Label : public Box {
    const std::string_view m_text;
    const gfx::Font& m_font;
    const gfx::Color m_font_color = gfx::Color::white();
    const int m_fontsize = 50;

public:
    Label(std::string_view text, gfx::Color color, const gfx::Font& font, float margin=1.0f)
        : Box(color, margin)
        , m_text(text)
        , m_font(font)
    {
        m_box.height = m_fontsize;
        m_box.width = m_font.measure_text(m_text, m_fontsize);
    }

    void draw(gfx::Renderer& rd) const override {
        Box::draw(rd);
        rd.draw_text(m_box.x, m_box.y, m_fontsize, m_text, m_font, m_font_color);
    }

};

class Container : public Box {
protected:
    std::vector<std::unique_ptr<Box>> m_children;

public:
    Container(std::vector<std::unique_ptr<Box>> children, gfx::Color color, float margin=0.0f)
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
    HorizontalContainer(std::vector<std::unique_ptr<Box>> children, gfx::Color color, float margin=0.0f)
    : Container(std::move(children), color, margin)
    { }

    void calculate_layout() override {

        if (m_children.empty()) return;

        auto tallest = std::ranges::max_element(m_children, [](std::unique_ptr<Box>& a, decltype(a) b) {
            return a->get_box().height < b->get_box().height;
        });

        m_box.height = (*tallest)->get_box().height;

        assert(tallest != m_children.end());

        m_box.width = std::ranges::fold_left(m_children, 0.0f, [](float acc, std::unique_ptr<Box>& child) {
            return acc + child->get_box().width;
        });

        float x = 0;
        for (auto& child : m_children) {
            gfx::Rect& box = child->get_box();
            // set the childs xy-position so it knows where to position its own children
            box.x = x;
            box.y = m_box.y;
            child->calculate_layout();
            x += box.width;
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
    VerticalContainer(std::vector<std::unique_ptr<Box>> children, gfx::Color color, float margin=0.0f)
    : Container(std::move(children), color, margin)
    { }

    // TODO: maybe have an xy parameter?
    void calculate_layout() override {

        if (m_children.empty()) return;

        auto widest = std::ranges::max_element(m_children, [](std::unique_ptr<Box>& a, decltype(a) b) {
            return a->get_box().width < b->get_box().width;
        });

        m_box.width = (*widest)->get_box().width;

        assert(widest != m_children.end());

        m_box.height = std::ranges::fold_left(m_children, 0.0f, [](float acc, std::unique_ptr<Box>& child) {
            return acc + child->get_box().height;
        });

        int y = m_box.y;
        for (auto& child : m_children) {
            gfx::Rect& box = child->get_box();
            box.y = y;
            box.x = m_box.x;
            child->calculate_layout();
            y += box.height;
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
    explicit Ui(Context& context)
        : m_context(context)
    { }

    void box(gfx::Color color, float margin=0.0f) {
        m_context.top().emplace_back(std::make_unique<Box>(color, margin));
    }

    void label(std::string_view text, gfx::Color color, const gfx::Font& font) {
        m_context.top().emplace_back(std::make_unique<Label>(text, color, font));
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
        root->calculate_layout();
        root->draw(rd);
        system("clear");
        root->print(0);

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

        ui.root(rd, gfx::Color::black(), [&](ui::Ui& ui) {

            ui.horizontal(gfx::Color::grey(), [&] {
                ui.vertical(gfx::Color::lightblue(), [&] {
                    ui.label("foo", gfx::Color::blue(), font);
                    ui.label("barrr", gfx::Color::blue(), font);
                });
                ui.vertical(gfx::Color::orange(), [&] {
                    ui.label("BAZ", gfx::Color::red(), font);
                    ui.label("quuux", gfx::Color::red(), font);
                });
            });

        });

        if (window.get_key_state(gfx::Key::Escape).pressed())
            window.close();
    });

}
