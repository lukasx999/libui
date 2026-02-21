#include <print>

#include <gfx/gfx.h>

namespace {

class Box {
protected:
    const gfx::Color m_color;
    gfx::Rect m_rect;

public:
    Box(gfx::Color color) : m_color(color) { }

    [[nodiscard]] gfx::Rect& get_rect() {
        return m_rect;
    }

    virtual void draw(gfx::Renderer& rd) const {
        rd.draw_rectangle(m_rect, m_color);
    }

    virtual void calculate_children_layouts() { }

};

class Container : public Box {
protected:
    std::vector<Box*> m_children;

public:
    Container(std::initializer_list<Box*> children, gfx::Color color)
        : Box(color)
        , m_children(children)
    { }

    void draw(gfx::Renderer& rd) const override {
        Box::draw(rd);

        for (const auto& child : m_children)
            child->draw(rd);
    }

};

class HorizontalContainer : public Container {
public:
    HorizontalContainer(std::initializer_list<Box*> children, gfx::Color color)
    : Container(children, color)
    { }

    void calculate_children_layouts() override {
        int elem_width = m_rect.width / m_children.size();

        int x = 0;
        for (auto& child : m_children) {
            auto& rect = child->get_rect();
            rect.height = m_rect.height;
            rect.width = elem_width;
            rect.y = 0;
            rect.x = x;
            x += elem_width;

            child->calculate_children_layouts();
        }
    }

};

class VerticalContainer : public Container {
public:
    VerticalContainer(std::initializer_list<Box*> children, gfx::Color color)
    : Container(children, color)
    { }

    void calculate_children_layouts() override {
        int elem_height = m_rect.height / m_children.size();

        int y = 0;
        for (auto& child : m_children) {
            auto& rect = child->get_rect();
            rect.height = elem_height;
            rect.width = m_rect.width;
            rect.x = 0;
            rect.y = y;
            y += elem_height;

            child->calculate_children_layouts();
        }
    }

};

} // namespace

int main() {

    gfx::Window window(1920, 1080, "ui");

    Box a(gfx::Color::blue());
    Box b(gfx::Color::orange());
    Box c(gfx::Color::red());
    HorizontalContainer container({ &a, &b, &c }, gfx::Color::black());
    Box foo(gfx::Color::green());
    VerticalContainer root({ &container, &foo }, gfx::Color::gray());
    root.get_rect() = window.get_as_rect();
    root.calculate_children_layouts();

    window.draw_loop([&](gfx::Renderer& rd) {
        rd.clear_background(gfx::Color::black());

        root.draw(rd);

        if (window.get_key_state(gfx::Key::Escape).pressed())
            window.close();
    });

}
