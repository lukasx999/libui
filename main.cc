#include <print>

#include <gfx/gfx.h>

int main() {

    gfx::Window window(1920, 1080, "ui");

    window.draw_loop([&](gfx::Renderer& rd) {
        rd.clear_background(gfx::Color::black());
        rd.draw_circle(window.get_center(), 100, gfx::Color::blue());

        if (window.get_key_state(gfx::Key::Escape).pressed())
            window.close();
    });

}
