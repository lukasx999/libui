#include <print>

#include <gfx/gfx.h>

#include "ui.h"

// TODO: auxilary layout class
// TODO: glfw repeated for text input backspace

int main() {

    auto flags = gfx::WindowFlags()
        .enable_resizing(true);

    gfx::Window window(1920, 1080, "ui", flags);
    ui::Ui ui(window);

    std::string input("hello, input");

    window.draw_loop([&](gfx::Renderer& rd) {
        rd.clear_background(gfx::Color::black());

        ui.root(rd, [&](ui::Ui& ui) {

            ui.horizontal([&] {
                ui.label("hello");
                ui.label("world");
                ui.label(":)");
            });

            ui::Style button_style {
                .color_bg=gfx::Color::orange(),
                .padding=20.0f,
                .border_radius=15.0f,
            };

            if (ui.button("click me", button_style).is_pressed()) {
                ui.label("you clicked the button!");
            }

            ui.text_input(500, input, { .color_bg=gfx::Color::blue() });

        }, { .color_bg=gfx::Color::gray(), .padding=10.0f });

        if (window.get_key_state(gfx::Key::Escape).is_pressed())
            window.close();
    });

}
