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
    float m_min_width = 0.0f;
    bool m_fixed_height = false;
    bool m_fixed_width = false;

public:
    Box(gfx::Color color, float margin=0.0f)
        : m_color(color)
        , m_margin(margin)
    { }

    virtual ~Box() = default;

    [[nodiscard]] bool has_fixed_height() const {
        return m_fixed_height;
    }

    [[nodiscard]] bool has_fixed_width() const {
        return m_fixed_width;
    }

    [[nodiscard]] float get_min_width() const {
        return m_min_width;
    }

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
        m_fixed_height = true;
        m_box.height = m_fontsize;
        m_min_width = m_font.measure_text(m_text, m_fontsize);
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
        int flex_width_count = std::ranges::count_if(m_children, [](auto& child) {
            return not child->has_fixed_width();
        });

        int fixed_width = std::accumulate(m_children.begin(), m_children.end(), 0, [](int acc, auto& child) {
            if (child->has_fixed_width())
                acc += child->get_box().width;

            return acc;
        });

        int available_width = m_box.width - fixed_width;
        int flex_elem_width = available_width / flex_width_count;

        int x = 0;
        for (auto& child : m_children) {
            auto& box = child->get_box();

            if (not child->has_fixed_height())
                box.height = m_box.height;

            if (not child->has_fixed_width())
                box.width = flex_elem_width;

            box.y = m_box.y;
            box.x = m_box.x + x;

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

    void calculate_layout() override {

        int flex_height_count = std::ranges::count_if(m_children, [](auto& child) {
            return not child->has_fixed_height();
        });

        int fixed_height = std::accumulate(m_children.begin(), m_children.end(), 0, [](int acc, auto& child) {
            if (child->has_fixed_height())
                acc += child->get_box().height;

            return acc;
        });

        int available_height = m_box.height - fixed_height;
        int flex_elem_height = available_height / flex_height_count;

        int y = 0;
        for (auto& child : m_children) {
            auto& box = child->get_box();

            if (not child->has_fixed_height())
                box.height = flex_elem_height;

            if (not child->has_fixed_width())
                box.width = m_box.width;

            box.x = m_box.x;
            box.y = m_box.y + y;

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
    auto font = window.load_font("/usr/share/fonts/TTF/FiraCodeNerdFont-Regular.ttf");
    ui::UserInterface ui;

    window.draw_loop([&](gfx::Renderer& rd) {
        rd.clear_background(gfx::Color::black());

        ui.root(rd, gfx::Color::black(), [&](ui::Ui& ui) {

            ui.horizontal(gfx::Color::black(), [&] {
                ui.vertical(gfx::Color::black(), [&] {
                    ui.label("Hello, World!", gfx::Color::orange(), font);
                    ui.box(gfx::Color::blue());
                    ui.label("bar", gfx::Color::lightblue(), font);
                    ui.box(gfx::Color::red());
                });
                ui.box(gfx::Color::green());
                ui.label("ui library", gfx::Color::lightblue(), font);
            });


        });

        if (window.get_key_state(gfx::Key::Escape).pressed())
            window.close();
    });

}
