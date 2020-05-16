#include "util/log.h"

#include "main.h"
#include "test.h"
#include "renderer/opengl.h"
#include "renderer/renderer.h"

GameState _global_gs;

#ifdef TESTS

int main() { // Test entry point
    _global_tests.run();
    return 0;
}

#else

int main() { // Game entry point
    GFX::init();

    std::vector<Vec3> points = {
        Vec3(-0.5,  0.5, 0.0),
        Vec3( 0.0, -0.5, 0.0),
        Vec3( 0.5,  0.5, 0.0),
    };

    GFX::Mesh rect = GFX::Mesh::init(points);
    GFX::Shader shader = GFX::default_shader();

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_WINDOWEVENT) {
                if (event.window.event == SDL_WINDOWEVENT_CLOSE)
                    running = false;
            }
        }

        SDL_Delay(10);

        GL::ClearColor(0.2, 0.1, 0.3, 1); // We don't need to do this...
        GL::Clear(GL::cCOLOR_BUFFER_BIT);

        shader.use();
        rect.draw();

        SDL_GL_SwapWindow(_global_gs.window);
    }

    GFX::deinit();
    return 0;
}

#endif
